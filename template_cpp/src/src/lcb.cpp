#include "urb.hpp"
#include "lcb.hpp"
#include <algorithm>
#include <utility> 

#define MAX_PROCESSES 128
LCausalBroadcast::LCausalBroadcast() {
}

LCausalBroadcast::LCausalBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks, std::vector<host_id_type> dependencies) {
    this->localhost = localhost;
    this->networks = networks;
    this->lsn = 0;
    this->vectorClock = std::vector<unsigned long>(MAX_PROCESSES,0);
    this->dependencies = dependencies;

}

LCausalBroadcast& LCausalBroadcast::operator=(const LCausalBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->lsn = other.lsn;
    this->vectorClock = other.vectorClock;
    this->dependencies = other.dependencies;
    this->pendings = other.pendings;
    this->logs = other.logs;

    return *this;
}

LCausalBroadcast::~LCausalBroadcast() {
    // delete this->perfectLink;
}

void LCausalBroadcast::start() {
    this->uniReliableBroadcast = UniReliableBroadcast(this->localhost, this->networks, [this](Msg msg){this->receive(msg);});
    this->uniReliableBroadcast.start();
}

void LCausalBroadcast::broadcast(unsigned int msg) {
    std::ostringstream oss;
    oss << "b " << msg;
    this->writeLogs(oss.str());
    Payload extendMsg = this->addSelfHost(msg, this->lsn);
    // std::cout << "LCB bc " << extendMsg.vectorClock[0] <<" "<< extendMsg.vectorClock[1] << " "<< extendMsg.vectorClock[2] <<"\n";
    this->lsn++;
    this->uniReliableBroadcast.broadcast(extendMsg);
}

std::vector<std::string> LCausalBroadcast::getLogs() {
    return this->logs;
} 

bool LCausalBroadcast::deliverable(Msg msg) {
    bool res = true;
    vectorClockLock.lock();
    for (const auto i: this->dependencies) {
        if (msg.payload.vectorClock[i] > this->vectorClock[i]) {
            res = false;
            break;
        }
    }
    vectorClockLock.unlock();

    return res;
}

bool LCausalBroadcast::completelyNotDeliverable(Msg msg) {
    bool res = true;
    vectorClockLock.lock();
    for (const auto i: this->dependencies) {
        if (msg.payload.vectorClock[i] <= this->vectorClock[i]) {
            res = false;
            break;
        }
    }
    vectorClockLock.unlock();
    return res;
}

void LCausalBroadcast::increaseVectorClock(host_id_type id) {
    vectorClockLock.lock();
    this->vectorClock[id-1]++;
    vectorClockLock.unlock();
}

void LCausalBroadcast::receive(Msg wrapedMsg) {
    // std::cout << "Received (" << wrapedMsg.payload.content << "," << wrapedMsg.payload.id << ") from " << wrapedMsg.senderId << "\n";
    this->pendings.push_back(wrapedMsg);


    // if (!completelyNotDeliverable(wrapedMsg)) {
        bool found = true;
        while(found) {
            found = false;

            auto it = pendings.begin();
            while(it != pendings.end())
            {
                if (deliverable(*it)) {
                    this->increaseVectorClock(it->payload.id);
                    this->deliver(*it);
                    it = pendings.erase(it);
                    found = true;
                } else {
                    it++;
                }
            }
        }
    // }
}

void LCausalBroadcast::deliver(Msg wrapedMsg) {
    std::ostringstream oss;
    std::cout << "LCB Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.payload;
    oss << "d " << wrapedMsg.payload.id << " " << wrapedMsg.payload.content;
    this->writeLogs(oss.str());
}

void LCausalBroadcast::deliver(Payload payload) {
    std::ostringstream oss;
    std::cout << "LCB Delivered " << payload.content << " from " << payload.id <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.payload;
    oss << "d " << payload.id << " " << payload.content;
    this->writeLogs(oss.str());
}

Payload LCausalBroadcast::addSelfHost(unsigned int msg, unsigned long seqNum) {
    // return std::make_pair(this->localhost.id, msg);
    vectorClockLock.lock();
    std::vector<unsigned long> W = this->vectorClock;
    vectorClockLock.unlock();
    W[this->localhost.id-1] = this->lsn;
    return Payload(this->localhost.id, msg, W);
}

void LCausalBroadcast::writeLogs(std::string log) {
    this->logsLock.lock();
    this->logs.push_back(log);
    this->logsLock.unlock();
}
