#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <iostream>


/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN		50000	// dimensiunea maxima a calupului de date
#define MAX_CLIENTS	100	// numarul maxim de clienti in asteptare

void displayMainMenu() {
	std::cout << "||	   1. Enter the chat			||" << std::endl;
	std::cout << "||	   2. Display active users		||" << std::endl;
	std::cout << "||	   3. Send private message		||" << std::endl;
	std::cout << "||	   4. Play a game			||" << std::endl;
	std::cout << "||	   5. Quit				||" << std::endl;
}

#endif