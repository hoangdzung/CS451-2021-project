#pragma once
#include "parser.hpp"
#include "beb.hpp"
#include "msg.hpp"
#include <unordered_map>
#include <utility> 

class UniReliableBroadcast {
    public:
        UniReliableBroadcast();
        UniReliableBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks);
        UniReliableBroadcast(const UniReliableBroadcast &);
        ~UniReliableBroadcast();
        void start();
        void deliver(Msg wrapedMsg);
        void receive(Msg wrapedMsg);
        void selfDeliver(unsigned int msg);
        void broadcast(unsigned int msg);
        std::vector<std::string> getLogs();
        UniReliableBroadcast& operator=(const UniReliableBroadcast & other);
    private:
        int networkSize;
        std::vector<Msg> pending;
        std::vector<Msg> delivered;
        std::unordered_map<
            std::pair<Parser::Host, unsigned long>, 
            std::vector<Parser::Host>
            > acks;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        BestEffortBroadcast bestEffortBroadcast;
};
