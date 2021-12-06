#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <mutex>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
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
        unsigned long packedSize;
        void writeLogs(std::string log);
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        std::function<void(Msg)> deliverCallBack;

        std::vector<std::string> logs;
        int sockfd; // socket file descriptor
        unsigned long msg_id;
        std::vector<PackedMsg> msgQueue;
        std::unordered_map<host_id_type, std::vector<Payload>> tempBuffer;

        std::mutex msgQueueLock;
        std::mutex logsLock;
        std::mutex sentLock;

        std::unordered_set<std::pair<host_id_type, unsigned long>, pair_hash> receivedMsgs;
        int setupSocket(Parser::Host host);
        struct sockaddr_in setUpDestAddr(host_id_type destId);
        void send();
        void receive();
};
