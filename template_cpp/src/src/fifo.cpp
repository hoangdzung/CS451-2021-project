#include "urb.hpp"
#include "fifo.hpp"
#include <algorithm>
#include <utility> 

FIFOBroadcast::FIFOBroadcast() {
}

FIFOBroadcast::FIFOBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->lsn = 0;
    // this->networks.erase(
    //     std::remove_if(this->networks.begin(), this->networks.end(),
    //     [localhost](const Parser::Host & o) { return o.id == localhost.id; }),
    //     this->networks.end()
    // );
}

FIFOBroadcast& FIFOBroadcast::operator=(const FIFOBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->lsn = other.lsn;
    this->pending = other.pending;
    this->logs = other.logs;

    return *this;
}

FIFOBroadcast::~FIFOBroadcast() {
    // delete this->perfectLink;
}

void FIFOBroadcast::start() {
    this->uniReliableBroadcast = UniReliableBroadcast(this->localhost, this->networks, [this](Msg msg){this->receive(msg);});
    this->uniReliableBroadcast.start();
}

void FIFOBroadcast::broadcast(unsigned int msg) {
    std::ostringstream oss;
    oss << "b " << msg;
    logs.push_back(oss.str());
    Payload extendMsg = this->addSelfHost(msg, this->lsn);
    this->uniReliableBroadcast.broadcast(extendMsg);
    pending.insert(extendMsg);

}

std::vector<std::string> FIFOBroadcast::getLogs() {
    return this->logs;
} 

void FIFOBroadcast::receive(Msg wrapedMsg) {
    std::cout << "Received (" << wrapedMsg.payload.content << "," << wrapedMsg.payload.id << ") from " << wrapedMsg.sender.id << "\n";
    this->pending.insert(wrapedMsg.payload);
}

void FIFOBroadcast::deliver(Msg wrapedMsg) {
    std::ostringstream oss;
    std::cout << "Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.payload;
    oss << "d " << wrapedMsg.payload.content << " " << wrapedMsg.payload.id;
    logs.push_back(oss.str());
}

Payload FIFOBroadcast::addSelfHost(unsigned int msg, unsigned long seqNum) {
    // return std::make_pair(this->localhost.id, msg);
    return Payload({this->localhost.id, msg, seqNum});
}

