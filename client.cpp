#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"
#include <iostream>
#include <sstream>
#include <vector>

#define PORTNO 5000

void usage(char *file)
{
	fprintf(stderr, "Usage: %s\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc > 1) {
		usage(argv[0]);
	}
	

	fd_set read_fds;	
	fd_set tmp_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	char* IPADDR = "127.0.0.1";

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORTNO);
	ret = inet_aton(IPADDR, &serv_addr.sin_addr);
	DIE(ret == 0, "wrong IP");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "wrong port");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	int fdmax;
	FD_SET(sockfd, &read_fds);
	FD_SET(STDIN_FILENO, &read_fds);
	fdmax = sockfd;

	// displayMainMenu();
	while (1) {
		tmp_fds = read_fds;
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);

		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
  		// se citeste de la tastatura
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			std::istringstream iss(buffer);
			std::string word;
			std::vector<std::string> input;

			//=================================================
			//Verificam corectitudinea bufferului inainte de a-l
			//trimite. Verificam daca comanda de exit este singura
			//din buffer si daca comenzile "subscribe" si 
			//"unsubscribe" sunt corecte
			//=================================================
			while (iss >> word) {
				input.push_back(word);
			}

			//===========================================
			// se trimite mesaj la server
			// apoi se afiseaza daca buffer-ul trimis este
			// de tip subscribe sau unsubscribe
			//===========================================
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");

		} else if (FD_ISSET(sockfd, &tmp_fds)) {
			//==============================
			//Primeste mesajul de la server
			//==============================
			memset(buffer, 0 , BUFLEN);
			n = recv(sockfd, buffer, BUFLEN, 0);
			DIE(n < 0, "recv");
			if (n == 0) {
				std::cout << "Server disconnected. Session will be ended" <<
				std::endl;
				break;
			}

			if (strcmp(buffer, "exit") == 0) {
				break;
			}

			std::cout << buffer;
		}
	}
	close(sockfd);

	return 0;
}
