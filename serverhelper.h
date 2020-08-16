#ifndef _SERVERHELPER_H
#define _SERVERHELPER_H
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
#include <vector>
#include <map>

class sub {
 public:
    int socket;
    std::string name;
    bool inChat = false;
    sub(int socket, std::string name){
        this->socket = socket;
        this->name = name;
        inChat = true;
    }
};

#endif