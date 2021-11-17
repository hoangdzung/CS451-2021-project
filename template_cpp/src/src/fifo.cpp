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
    for (const auto host : networks) {
        this->next.insert({host.id, 1});
        this->pendings.insert({host.id, std::priority_queue<Payload>()});
    }
}

FIFOBroadcast& FIFOBroadcast::operator=(const FIFOBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->lsn = other.lsn;
    this->next = other.next;
    this->pendings = other.pendings;
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
    this->writeLogs(oss.str());
    this->lsn += 1;
    Payload extendMsg = this->addSelfHost(msg, this->lsn);
    this->uniReliableBroadcast.broadcast(extendMsg);
}

std::vector<std::string> FIFOBroadcast::getLogs() {
    return this->logs;
} 

void FIFOBroadcast::receive(Msg wrapedMsg) {
    // std::cout << "Received (" << wrapedMsg.payload.content << "," << wrapedMsg.payload.id << ") from " << wrapedMsg.senderId << "\n";
    // this->pendingLock.lock();
    auto pendingIt = this->pendings.find(wrapedMsg.payload.id);
    auto nextIt = this->next.find(wrapedMsg.payload.id);
    if ((pendingIt != this->pendings.end()) && (nextIt != this->next.end())) {
        // std::cout << nextIt->first << ":" << nextIt->second << "\n";
        pendingIt->second.push(wrapedMsg.payload);
        while (!(pendingIt->second.empty())) {
            if (pendingIt->second.top().seqNum == nextIt->second) {
                nextIt->second++;
                this->deliver(pendingIt->second.top());
                pendingIt->second.pop();
            } else {
                break;
            }
        }
    }
    // this->pendingLock.unlock();    
}

void FIFOBroadcast::deliver(Msg wrapedMsg) {
    std::ostringstream oss;
    std::cout << "FIFO Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.payload;
    oss << "d " << wrapedMsg.payload.id << " " << wrapedMsg.payload.content;
    this->writeLogs(oss.str());
}

void FIFOBroadcast::deliver(Payload payload) {
    std::ostringstream oss;
    std::cout << "FIFO Delivered " << payload.content << " from " << payload.id <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.payload;
    oss << "d " << payload.id << " " << payload.content;
    this->writeLogs(oss.str());
}

Payload FIFOBroadcast::addSelfHost(unsigned int msg, unsigned long seqNum) {
    // return std::make_pair(this->localhost.id, msg);
    return Payload({this->localhost.id, msg, seqNum});
}

void FIFOBroadcast::writeLogs(std::string log) {
    this->logsLock.lock();
    this->logs.push_back(log);
    this->logsLock.unlock();
}
