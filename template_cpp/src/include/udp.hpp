#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <mutex>
#include "parser.hpp"
#include "msg.hpp"

class UDPSocket {
    public:
        UDPSocket(){};
        UDPSocket(Parser::Host localhost);
        UDPSocket(const UDPSocket &);
        void put(Parser::Host dest, unsigned int msg);
    private:
        Parser::Host localhost;
        int sockfd; // socket file descriptor
        unsigned long msg_id;
        std::vector<Msg> msgQueue;
        std::mutex msgQueueLock;

        std::vector<Msg> receivedMsgs;
        int setupSocket(Parser::Host host);
        struct sockaddr_in setUpDestAddr(Parser::Host dest);
        void send();
        void receive();
};
