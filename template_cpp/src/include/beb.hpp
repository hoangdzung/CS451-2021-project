#pragma once
#include "parser.hpp"
#include "udp.hpp"
#include "msg.hpp"
// #include "abstract.hpp"

class BestEffortBroadcast {
    public:
        BestEffortBroadcast();
        BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks);
        BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack);
        BestEffortBroadcast(const BestEffortBroadcast &);
        ~BestEffortBroadcast();
        void start();
        void deliver(const Msg wrapedMsg);
        void selfDeliver(unsigned int msg);
        void broadcast(unsigned int msg, unsigned long seqNum=0);
        void broadcast(Payload msg);

        std::vector<std::string> getLogs();
        BestEffortBroadcast& operator=(const BestEffortBroadcast & other);
    private:
        std::function<void(Msg)> deliverCallBack;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UDPSocket perfectLink;
        std::mutex logsLock;

};
