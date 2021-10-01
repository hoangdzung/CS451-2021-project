// #pragma once 
#ifndef UDP_H

#define UDP_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include "parser.hpp"

class UDPSocket {
    public:
        UDPSocket(){};
        UDPSocket(Parser::Host localhost);
        void send(Parser::Host dest, std::string msg);
        std::string receive();
    private:
        Parser::Host localhost;
        int sockfd; // socket file descriptor
        int setupSocket(Parser::Host host);
};
#endif
