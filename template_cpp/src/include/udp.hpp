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
        struct sockaddr_in setUpDestAddr(Parser::Host dest);
    private:
        bool received_ack;
        Parser::Host localhost;
        int sockfd; // socket file descriptor
        unsigned long msg_id;
        std::vector<Msg> receivedMsgs;
        int setupSocket(Parser::Host host);
};
