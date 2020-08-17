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
    std::string name;
    bool inChat;
    bool pm;
    sub(std::string name){
        this->name = name;
        inChat = false;
        pm = false;
    }
};

void sysClear(int socket) {
    std::string clc = "clc";
    send(socket, clc.c_str(), strlen(clc.c_str()), 0);
}

void dispMenu(int socket) {
    std::string dispMainMenu = "dmm";
        send(socket, dispMainMenu.c_str(), strlen(dispMainMenu.c_str()), 0);

}

#endif