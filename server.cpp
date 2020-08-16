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
	std::vector<sub> cl;


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

					//Mesaj de informare pentru userii deja activi ca un nou user s-a conectat                
					std::string connected_buff = "Guest" + std::to_string(newsockfd) + " connected\n"; 
					for (auto client = cl.begin(); client != cl.end(); ++client) {
						send(client->socket, connected_buff.c_str(), strlen(connected_buff.c_str()), 0);
					}
					std::string name = "Guest:";
					sub client(newsockfd, name + std::to_string(newsockfd));
					cl.push_back(client);

				} else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");
					sscanf(buffer, "%d", &recvSock);
					if (n == 0) {
						// conexiunea s-a inchis
						close(i);
						std::string disconnected_buff = "Guest" + std::to_string(i) + " disconnected\n"; 
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);

						// eliminam clientul din structura de date specifica acestuia
						for (int j = 0; j < cl.size(); ++j) {
							if (cl[j].socket == i) {
								cl.erase(cl.begin() + j);
							}
							// Mesaj de informare ca un user s-a deconectat
							send(cl[j].socket, disconnected_buff.c_str(), strlen(disconnected_buff.c_str()), 0);
						}
						break;
						
					} else {
						for (auto client = cl.begin(); client != cl.end(); ++client) {
							if (client->socket != i) {
								std::string send_buff = "Guest" + std::to_string(i) + ": " + buffer;
								send(client->socket, send_buff.c_str(), strlen(send_buff.c_str()), 0);
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
