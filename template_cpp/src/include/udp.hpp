#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include "parser.hpp"
#include "msg.hpp"

class UDPSocket {
    public:
        UDPSocket(){};
        UDPSocket(Parser::Host localhost);
        void send(Parser::Host dest, unsigned int msg);
        Msg receive();
    private:
        Parser::Host localhost;
        int sockfd; // socket file descriptor
        int setupSocket(Parser::Host host);
};
