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
        UDPSocket(Parser::Host localhost, std::vector<Parser::Host> networks);
        UDPSocket(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack);
        UDPSocket(const UDPSocket &);
        // bool UDPSocket& operator=(const UDPSocket&);
        void start();
        void put(Parser::Host dest, unsigned int msg, unsigned long seqNum=0);
        void put(Parser::Host dest, Payload msg);
        std::vector<std::string> getLogs();
        UDPSocket& operator=(const UDPSocket & other);

    private:
        void writeLogs(std::string log);
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        std::function<void(Msg)> deliverCallBack;

        std::vector<std::string> logs;
        int sockfd; // socket file descriptor
        unsigned long msg_id;
        std::vector<Msg> msgQueue;
        std::mutex msgQueueLock;
        std::mutex logsLock;
        std::mutex sentLock;

        std::vector<Msg> receivedMsgs;
        int setupSocket(Parser::Host host);
        struct sockaddr_in setUpDestAddr(host_id_type destId);
        void send();
        void receive();
};
