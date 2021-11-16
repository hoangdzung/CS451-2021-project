#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <mutex>
#include <queue>
#include <functional>
#include "parser.hpp"
#include "msg.hpp"
// #include "abstract.hpp"

class UDPSocket {
    public:
        UDPSocket();
        UDPSocket(Parser::Host localhost);
        UDPSocket(Parser::Host localhost, std::function<void(Msg)> deliverCallBack);
        UDPSocket(const UDPSocket &);
        // bool UDPSocket& operator=(const UDPSocket&);
        void start();
        void put(Parser::Host dest, unsigned int msg, unsigned long seqNum);
        void put(Parser::Host dest, Payload msg);
        void putAndSend(Parser::Host dest, unsigned int msg, unsigned long seqNum);
        void putAndSend(Parser::Host dest, Payload msg);
        std::vector<std::string> getLogs();
        UDPSocket& operator=(const UDPSocket & other);

    private:
        Parser::Host localhost;
        std::function<void(Msg)> deliverCallBack;

        std::vector<std::string> logs;
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
