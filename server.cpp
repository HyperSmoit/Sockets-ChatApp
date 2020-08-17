#include <bits/stdc++.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <iostream>
#include "helpers.h"
#include <vector>
#include <fcntl.h>
#include <netdb.h>
#include "serverhelper.h"
#include <sstream>
#include <netinet/tcp.h>

#define PORTNO 5000

void usage(char *file)
{
	fprintf(stderr, "Usage:%s\n", file);
	exit(0);
}
 
int main(int argc, char *argv[])
{
	int sockfd, newsockfd;
	char buffer[BUFLEN];

	int recvSock;

	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;
	char* IP_ADDR = "127.0.0.1";

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc > 1) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds)
	// si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");


	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORTNO);
	ret = inet_aton(IP_ADDR, &serv_addr.sin_addr);
	DIE(ret == 0, "wrong IP");

	int flag = 1;
	if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
		perror("setsockopt fail");
	}

	ret = bind(sockfd, (struct sockaddr *) &serv_addr,
	sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni)
	// in multimea read_fds
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	// Vectori de clienti pentru a distribui mesajele clientilor, asemanator unui chat
	// std::vector<sub> cl;
	std::unordered_map<int, sub> cl;

	while (1) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					//======================================================================
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					//======================================================================
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen);
					DIE(newsockfd < 0, "accept");

					//======================================================================
					//DEZACTIVARE NEAGLE
					//======================================================================
					int flag = 1;
					setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY,
					(char *)&flag, sizeof(int));

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}
					std::string name = "Guest";
					sub client(name + std::to_string(newsockfd));
					cl.insert(std::make_pair(newsockfd, client));
					std::cout << client.name + " connected\n" << std::endl;
				} else {
					// ================================================
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					// ================================================
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");
					sscanf(buffer, "%d", &recvSock);
					std::string message = buffer;

					// =============================================================================
					// Daca nu s-au receptionat date in buffer, se inchide conexiunea pe acel buffer
					// =============================================================================
					if (n == 0) {
						// conexiunea s-a inchis
						close(i);
						std::string disconnected_buff = "Guest" + std::to_string(i) + " disconnected\n"; 
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
						// eliminam clientul din structura de date specifica acestuia
						cl.erase(i);
						for (auto j = cl.begin(); j != cl.end(); ++j) {
							// Mesaj de informare ca un user s-a deconectat
							send(j->first, disconnected_buff.c_str(), strlen(disconnected_buff.c_str()), 0);
						}
						std::cout << disconnected_buff << std::endl;
						break;

						// ==============================================================
						// Daca s-au receptionat date, se prelucreaza datele receptionate
						// ==============================================================
					} else {
						message.resize(message.size() - 1);
						if (message.compare("1") == 0 && !cl.at(i).inChat && !cl.at(i).pm) {
							std::cout << "Enter chat command initiated by socket " + std::to_string(i) << std::endl;
							cl.at(i).inChat = true;

							// Trimitem mesaj de curatare a ecranului clientului
							sysClear(i);

							std::string connected_user = "Guest" + std::to_string(i) + " joined chatroom";
							for (auto it = cl.begin(); it != cl.end(); ++it) {
								if (it->first != i && it->second.inChat) {
									send(it->first, connected_user.c_str(), strlen(connected_user.c_str()), 0);
								}
							}
						} else if (message.compare("2") == 0 && !cl.at(i).inChat && !cl.at(i).pm) {
							sysClear(i);
							std::cout << "Active users command initiated by socket" + std::to_string(i) << std::endl;
							std::string a_users = "";
							for (auto it = cl.begin(); it != cl.end(); ++it) {
								if (it->second.inChat) {
									a_users += "Guest" + std::to_string(it->first) + " ";
								}
							}
							if (a_users.compare("") != 0) {
								send(i, a_users.c_str(), strlen(a_users.c_str()), 0);
								
							} else {
								std::cout << "No users available" << std::endl;
							}
							dispMenu(i);
						} else if (message.compare("3") == 0 && !cl.at(i).inChat && !cl.at(i).pm) {
							sysClear(i);
							std::cout << "sending private message..." << std::endl;
							std::string insertUser = "Insert username followed by ':' and your message";
							send(i, insertUser.c_str(), strlen(insertUser.c_str()), 0);							
							cl.at(i).pm = true;
							dispMenu(i);
						} else if (cl.at(i).pm) {
							std::string receiverUser;
							cl.at(i).pm = false;
							std::size_t sspoz = message.find(" ");
							if (sspoz != std::string::npos) {
								std::string usr = message.substr(0, sspoz);
								for (auto it = cl.begin(); it != cl.end(); ++it) {
									if (it->second.name.compare(usr) == 0) {
										std::string payload = message.substr(sspoz + 1, message.size());
										send(it->first, payload.c_str(), strlen(payload.c_str()), 0);
										break;
									}
								}
							}
						} else {	
							for (auto client = cl.begin(); client != cl.end(); ++client) {
								if (client->first != i && client->second.inChat) {
									std::string send_buff = "Guest" + std::to_string(i) + ": " + buffer;
									send_buff.resize(send_buff.size() - 1);
									send(client->first, send_buff.c_str(), strlen(send_buff.c_str()), 0);
								}
							}
						}
					}
				}
			}
		}
	}
	close(sockfd); 

	return 0;
}
