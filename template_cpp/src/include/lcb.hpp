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

class LCausalBroadcast {
    public:
        LCausalBroadcast();
        LCausalBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks, std::vector<host_id_type> dependencies);
        LCausalBroadcast(const LCausalBroadcast &);
        ~LCausalBroadcast();
        void start();
        void deliver(Msg wrapedMsg);
        void deliver(Payload payload);
        void receive(Msg wrapedMsg);
        void broadcast(unsigned int msg);
        std::vector<std::string> getLogs();
        LCausalBroadcast& operator=(const LCausalBroadcast & other);
    private:
        bool deliverable(Msg msg);
        bool completelyNotDeliverable(Msg msg);
        void increaseVectorClock(host_id_type id);
        void writeLogs(std::string log);
        // void addPending(Payload payload);
        // void removePending(Payload payload);
        unsigned long lsn;
        std::vector<unsigned long> vectorClock;
        std::vector<host_id_type> dependencies;
        // std::function<bool(Msg, Msg)> cmp = [this](Msg a, Msg b){
        //     for (auto i: this->dependencies) {
        //         if (a.payload.vectorClock[i] != b.payload.vectorClock[i])
        //             return a.payload.vectorClock[i] < b.payload.vectorClock[i];
        //     }
        //     return false; 
        // };
        // std::set<Msg, decltype(cmp)> pendings;
        std::vector<Msg> pendings;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UniReliableBroadcast uniReliableBroadcast;
        Payload addSelfHost(unsigned int msg, unsigned long seqNum=0);
        std::mutex logsLock;
        std::mutex vectorClockLock;
};
