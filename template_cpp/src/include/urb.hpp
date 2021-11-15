#pragma once
#include "parser.hpp"
#include "beb.hpp"
#include "msg.hpp"
#include "abstract.hpp"

class UniReliableBroadcast : public AbstractLayer{
    public:
        UniReliableBroadcast() =delete;
        UniReliableBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks);
        UniReliableBroadcast(const UniReliableBroadcast &);
        ~UniReliableBroadcast();
        void start();
        void deliver(Msg wrapedMsg);
        void selfDeliver(unsigned int msg);
        void put(unsigned int msg);
        std::vector<std::string> getLogs();
        UniReliableBroadcast& operator=(const UniReliableBroadcast & other);
    private:
        Parser::Host localhost;
        std::vector<Parser::Host> networks;
        UDPSocket perfectLink;
};
