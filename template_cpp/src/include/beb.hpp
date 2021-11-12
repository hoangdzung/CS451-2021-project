#pragma once
#include "parser.hpp"
#include "udp.hpp"
#include "msg.hpp"
#include "abstract.hpp"

class BestEffortBroadcast : public AbstractLayer{
    public:
        BestEffortBroadcast(){};
        BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks);
        BestEffortBroadcast(const BestEffortBroadcast &);
        void start();
        void put(unsigned int msg);
        std::vector<std::string> getLogs();
        BestEffortBroadcast& operator=(const BestEffortBroadcast & other);
        void setAttr(Parser::Host localhost, std::vector<Parser::Host> networks);

    private:
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UDPSocket perfectLink;
};
