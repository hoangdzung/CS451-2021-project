#include "beb.hpp"
#include "udp.hpp"

BestEffortBroadcast::BestEffortBroadcast() {
    
}

BestEffortBroadcast::BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->perfectLink = UDPSocket(localhost, [this](Msg msg){this->deliver(msg);});
}

BestEffortBroadcast& BestEffortBroadcast::operator=(const BestEffortBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->perfectLink = other.perfectLink;
    this->logs = other.logs;

    return *this;
}

BestEffortBroadcast::~BestEffortBroadcast() {
    // delete this->perfectLink;
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

void BestEffortBroadcast::deliver(Msg wrapedMsg) {
    std::ostringstream oss;
    // std::cout << this <<  " " << localhost.id << " d " << wrapedMsg.sender.id << " " << wrapedMsg.content << "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.content;
    oss << "d " << wrapedMsg.sender.id << " " << wrapedMsg.content;
    logs.push_back(oss.str());
}

void BestEffortBroadcast::selfDeliver(unsigned int msg) {
    std::ostringstream oss;
    // std::cout << this <<  " " << localhost.id  << " self d " << localhost.id << " " << msg << "\n";
    // oss << this << " self d " << localhost.id << " " << msg;
    oss << "d " << localhost.id << " " << msg;
    logs.push_back(oss.str());
}

