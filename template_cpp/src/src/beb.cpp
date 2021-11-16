#include "beb.hpp"
#include "udp.hpp"

BestEffortBroadcast::BestEffortBroadcast() {
    this->deliverCallBack = [](Msg msg) {};
}

BestEffortBroadcast::BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = [](Msg msg) {};
    // this->perfectLink = UDPSocket(localhost, [this](Msg msg){this->deliver(msg);});
}

BestEffortBroadcast::BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = deliverCallBack;
    // this->perfectLink = UDPSocket(localhost, [this](Msg msg){this->deliver(msg);});
}

BestEffortBroadcast& BestEffortBroadcast::operator=(const BestEffortBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->deliverCallBack = other.deliverCallBack;
    this->logs = other.logs;

    return *this;
}

BestEffortBroadcast::~BestEffortBroadcast() {
    // delete this->perfectLink;
}

void BestEffortBroadcast::start() {
    this->perfectLink = UDPSocket(this->localhost, [this](Msg msg){this->deliver(msg);});
    this->perfectLink.start();
}

void BestEffortBroadcast::broadcast(unsigned int msg) {
    std::ostringstream oss;
    oss << "b " << msg;
    logs.push_back(oss.str());
    for (auto host : this->networks) {
        if (host.id == this->localhost.id) {
            selfDeliver(msg);
        } else {
            this->perfectLink.putAndSend(host, msg);  // broadcasting the message instead of just putting it in the queue  
        }
    }
}

void BestEffortBroadcast::broadcast(host_msg_type msg) {
    std::ostringstream oss;
    oss << "b " << msg.second;
    logs.push_back(oss.str());
    for (auto host : this->networks) {
        if (host.id == this->localhost.id) {
            selfDeliver(msg.second);
        } else {
            this->perfectLink.putAndSend(host, msg);  // broadcasting the message instead of just putting it in the queue  
        }
    }
}

std::vector<std::string> BestEffortBroadcast::getLogs() {
    return this->logs;
} 

void BestEffortBroadcast::deliver(Msg wrapedMsg) {
    std::ostringstream oss;
    // std::cout << "Received " << wrapedMsg.content.second << " from " << wrapedMsg.content.first <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.content;
    oss << "d " << wrapedMsg.content.first << " " << wrapedMsg.content.second;
    logs.push_back(oss.str());
    this->deliverCallBack(wrapedMsg);
}

void BestEffortBroadcast::selfDeliver(unsigned int msg) {
    std::ostringstream oss;
    // std::cout << this <<  " " << localhost.id  << " self d " << localhost.id << " " << msg << "\n";
    // oss << this << " self d " << localhost.id << " " << msg;
    oss << "d " << localhost.id << " " << msg;
    logs.push_back(oss.str());
}

