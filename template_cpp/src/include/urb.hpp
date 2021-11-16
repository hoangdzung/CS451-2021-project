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
        UniReliableBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack);
        UniReliableBroadcast(const UniReliableBroadcast &);
        ~UniReliableBroadcast();
        void start();
        void deliver(Msg wrapedMsg);
        void receive(Msg wrapedMsg);
        void broadcast(unsigned int msg, unsigned long seqNum=0);
        void broadcast(Payload payload);
        std::vector<std::string> getLogs();
        UniReliableBroadcast& operator=(const UniReliableBroadcast & other);
    private:
        void writeLogs(std::string log);
        std::function<void(Msg)> deliverCallBack;
        void addAck(Msg wrapedMsg);
        void addAck(Payload msg,unsigned long senderId);
        void addPending(Payload msg);
        bool isPending(Msg wrapedMsg);
        bool canDeliver(Msg wrapedMsg);
        bool isDelivered(Msg wrapedMsg);
        Payload addSelfHost(unsigned int msg, unsigned long seqNum=0);
        long unsigned int networkSize;
        std::unordered_set<Payload,hash_custom> pending;
        std::unordered_set<Payload,hash_custom> delivered;
        std::unordered_map<
            Payload, 
            std::unordered_set<unsigned long>,
            hash_custom
            > acks;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        BestEffortBroadcast bestEffortBroadcast;
        std::mutex logsLock;
        std::mutex acksPendingLock;

};
