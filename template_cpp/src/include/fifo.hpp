#pragma once
#include "parser.hpp"
#include "urb.hpp"
#include "msg.hpp"
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <utility> 
#include <queue>

class FIFOBroadcast {
    public:
        FIFOBroadcast();
        FIFOBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks);
        FIFOBroadcast(const FIFOBroadcast &);
        ~FIFOBroadcast();
        void start();
        void deliver(Msg wrapedMsg);
        void deliver(Payload payload);
        void receive(Msg wrapedMsg);
        void broadcast(unsigned int msg);
        std::vector<std::string> getLogs();
        FIFOBroadcast& operator=(const FIFOBroadcast & other);
    private:
        void writeLogs(std::string log);
        // void addPending(Payload payload);
        // void removePending(Payload payload);
        unsigned long lsn;
        std::unordered_map<unsigned long, unsigned long> next;
        std::unordered_map<unsigned long,
        std::priority_queue<Payload>> pendings;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UniReliableBroadcast uniReliableBroadcast;
        Payload addSelfHost(unsigned int msg, unsigned long seqNum=0);
        std::mutex logsLock;
        // std::mutex pendingLock;
};
