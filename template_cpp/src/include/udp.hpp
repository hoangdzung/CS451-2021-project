#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <mutex>
#include <queue>
#include <set>
#include <functional>
#include <list>
#include <unordered_map>
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
        unsigned long nSend;
        unsigned long nPrevSend;
        unsigned long nReceive;
        void writeLogs(std::string log);
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        std::function<void(Msg)> deliverCallBack;

        std::vector<std::string> logs;
        int sockfd; // socket file descriptor
        unsigned long msg_id;
        // std::unordered_map<host_id_type, std::list<WrapedPayload>> msgQueue;
        std::unordered_map<host_id_type, std::vector<WrapedPayload>> msgQueue;
        std::mutex msgQueueLock;
        std::mutex logsLock;
        std::mutex sentLock;

        // std::unordered_map<host_id_type, std::list<WrapedPayload>> receivedMsgs;
        std::unordered_map<host_id_type, std::vector<WrapedPayload>> receivedMsgs;
        int setupSocket(Parser::Host host);
        struct sockaddr_in setUpDestAddr(host_id_type destId);
        void send();
        void receive();
};
