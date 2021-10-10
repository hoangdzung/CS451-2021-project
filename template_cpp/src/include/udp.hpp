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
        // bool UDPSocket& operator=(const UDPSocket&);
        void start();
        void put(Parser::Host dest, unsigned int msg);
        std::vector<std::string> getLogs();
        UDPSocket& operator=(const UDPSocket & other);

    private:
        std::vector<std::string> logs;
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
