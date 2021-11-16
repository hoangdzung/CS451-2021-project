#pragma once
#include "parser.hpp"
#include "urb.hpp"
#include "msg.hpp"
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <utility> 

class FIFOBroadcast {
    public:
        FIFOBroadcast();
        FIFOBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks);
        FIFOBroadcast(const FIFOBroadcast &);
        ~FIFOBroadcast();
        void start();
        void deliver(Msg wrapedMsg);
        void receive(Msg wrapedMsg);
        void broadcast(unsigned int msg);
        std::vector<std::string> getLogs();
        FIFOBroadcast& operator=(const FIFOBroadcast & other);
    private:
        unsigned long lsn;
        std::unordered_map<unsigned long, unsigned long> next;
        std::unordered_set<Payload,hash_custom> pending;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UniReliableBroadcast uniReliableBroadcast;
        Payload addSelfHost(unsigned int msg, unsigned long seqNum=0);
};
