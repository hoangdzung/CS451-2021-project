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
        void deliver(Msg wrapedMsg);
        void selfDeliver(unsigned int msg);
        void broadcast(unsigned int msg);
        void broadcast(host_msg_type msg);

        std::vector<std::string> getLogs();
        BestEffortBroadcast& operator=(const BestEffortBroadcast & other);
    private:
        std::function<void(Msg)> deliverCallBack;
        std::vector<std::string> logs;
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UDPSocket perfectLink;
};
