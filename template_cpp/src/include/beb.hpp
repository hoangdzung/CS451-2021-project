#pragma once
#include "parser.hpp"
#include "udp.hpp"

class BestEffortBroadcast {
    public:
        BestEffortBroadcast(){};
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
