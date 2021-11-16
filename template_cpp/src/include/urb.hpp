#pragma once
#include "parser.hpp"
#include "beb.hpp"
#include "msg.hpp"
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
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
        void broadcast(unsigned int msg);
        std::vector<std::string> getLogs();
        UniReliableBroadcast& operator=(const UniReliableBroadcast & other);
    private:
        void addAck(Msg wrapedMsg);
        void addAck(host_msg_type msg);
        bool isPending(Msg wrapedMsg);
        bool canDeliver(Msg wrapedMsg);
        bool isDelivered(Msg wrapedMsg);
        host_msg_type addSelfHost(unsigned int msg);
        long unsigned int networkSize;
        std::unordered_set<host_msg_type,pair_hash> pending;
        std::unordered_set<host_msg_type,pair_hash> delivered;
        std::unordered_map<
            host_msg_type, 
            std::unordered_set<unsigned long>,
            pair_hash
            > acks;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        BestEffortBroadcast bestEffortBroadcast;
};
