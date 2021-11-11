#include "beb.hpp"
#include "udp.hpp"

BestEffortBroadcast::BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->perfectLink = UDPSocket(localhost);
}

BestEffortBroadcast& BestEffortBroadcast::operator=(const BestEffortBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->perfectLink = other.perfectLink;

    return *this;
}

void BestEffortBroadcast::start() {
    this->perfectLink.start();
}

void BestEffortBroadcast::put(unsigned int msg) {
    for (auto host : this->networks) {
        this->perfectLink.put(host, msg);    
    }
}

std::vector<std::string> BestEffortBroadcast::getLogs() {
    return this->perfectLink.getLogs();
} 

