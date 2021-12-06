#include "beb.hpp"
#include "udp.hpp"
#include <thread>
#include <chrono>

BestEffortBroadcast::BestEffortBroadcast() {
    this->deliverCallBack = [](Msg msg) {};
}

BestEffortBroadcast::BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = [](Msg msg) {};
}

BestEffortBroadcast::BestEffortBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = deliverCallBack;
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
    this->perfectLink = UDPSocket(this->localhost, this->networks, [this](Msg msg){this->deliver(msg);});
    this->perfectLink.start();
}

// void BestEffortBroadcast::broadcast(unsigned int msg, unsigned long seqNum) {
//     std::ostringstream oss;
//     oss << "b " << msg;
//     this->writeLogs(oss.str());
    
//     for (auto host : this->networks) {
//         if (host.id == this->localhost.id) {
//             selfDeliver(msg);
//         } else {
//             this->perfectLink.put(host, msg, seqNum); 
//         }
//     }
// }

void BestEffortBroadcast::broadcast(Payload msg) {
    // std::ostringstream oss;
    // oss << "b " << msg.content;
    // this->writeLogs(oss.str());   
    for (auto host : this->networks) {
        // std::cout << "BEB bc " << msg.vectorClock[0] <<" "<< msg.vectorClock[1] << " "<< msg.vectorClock[2] <<" to "<< host.id <<"\n";
        if (host.id == this->localhost.id) {
            selfDeliver(msg.content);
        } else {
            this->perfectLink.put(host, msg);  
        }
    }
}

std::vector<std::string> BestEffortBroadcast::getLogs() {
    return this->logs;
} 

void BestEffortBroadcast::deliver(Msg wrapedMsg) {
    // std::ostringstream oss;
    // // std::cout << "BEB Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";
    // oss << "d " << wrapedMsg.payload.id << " " << wrapedMsg.payload.content;
    // this->writeLogs(oss.str());

    this->deliverCallBack(wrapedMsg);
}

void BestEffortBroadcast::selfDeliver(unsigned int msg) {
    // std::cout << "BEB Self Delivered " << msg << " from myself\n";
    // std::ostringstream oss;
    // oss << "d " << localhost.id << " " << msg;
    // this->writeLogs(oss.str());
    
}

void BestEffortBroadcast::writeLogs(std::string log) {
    this->logsLock.lock();
    this->logs.push_back(log);
    this->logsLock.unlock();
}
