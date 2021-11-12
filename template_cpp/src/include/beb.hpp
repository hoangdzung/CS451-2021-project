#pragma once
#include "parser.hpp"
#include "udp.hpp"
#include "msg.hpp"
#include "abstract.hpp"

class BestEffortBroadcast : public AbstractLayer{
    public:
        BestEffortBroadcast() =delete;
        BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks);
        BestEffortBroadcast(const BestEffortBroadcast &);
        void start();
        void put(unsigned int msg);
        std::vector<std::string> getLogs();
        BestEffortBroadcast& operator=(const BestEffortBroadcast & other);
    private:
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UDPSocket perfectLink;
};
