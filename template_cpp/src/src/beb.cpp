#include "beb.hpp"
#include "udp.hpp"

BestEffortBroadcast::BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->perfectLink = UDPSocket(localhost, this);
}

BestEffortBroadcast& BestEffortBroadcast::operator=(const BestEffortBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->perfectLink = other.perfectLink;

    return *this;
}

void BestEffortBroadcast::setAttr(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->perfectLink = UDPSocket(localhost, this);
}

void BestEffortBroadcast::start() {
    this->perfectLink.start();
}

void BestEffortBroadcast::put(unsigned int msg) {
    for (auto host : this->networks) {
        if (host.id == this->localhost.id) {
            continue;
        } else {
            this->perfectLink.putAndSend(host, msg);  // broadcasting the message instead of just putting it in the queue  
        }
    }
    std::ostringstream oss;
    oss << "b " << msg;
    logs.push_back(oss.str());
    selfDeliver(msg);
}

std::vector<std::string> BestEffortBroadcast::getLogs() {
    return this->logs;
} 

