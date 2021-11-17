#include "beb.hpp"
#include "urb.hpp"
#include <algorithm>
#include <utility> 

UniReliableBroadcast::UniReliableBroadcast() {
    std::cout <<1 <<"\n";
    this->networkSize = 0;
    this->deliverCallBack = [](Msg msg) {std::cout <<"test1\n";};
}

UniReliableBroadcast::UniReliableBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    std::cout <<2 <<"\n";
    this->localhost = localhost;
    this->networks = networks;
    this->networkSize = networks.size();
    this->deliverCallBack = [](Msg msg) {std::cout <<"test2\n";};
    this->networks.erase(
        std::remove_if(this->networks.begin(), this->networks.end(),
        [localhost](const Parser::Host & o) { return o.id == localhost.id; }),
        this->networks.end()
    );
}

UniReliableBroadcast::UniReliableBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack) {
    std::cout <<3 <<"\n";
    this->localhost = localhost;
    this->networks = networks;
    this->networkSize = networks.size();
    this->deliverCallBack = deliverCallBack;
    this->networks.erase(
        std::remove_if(this->networks.begin(), this->networks.end(),
        [localhost](const Parser::Host & o) { return o.id == localhost.id; }),
        this->networks.end()
    );
}

UniReliableBroadcast& UniReliableBroadcast::operator=(const UniReliableBroadcast & other) {
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->networkSize = other.networkSize;
    this->pending = other.pending;
    this->delivered = other.delivered;
    this->acks = other.acks;
    this->logs = other.logs;
    this->deliverCallBack = other.deliverCallBack;

    return *this;
}

UniReliableBroadcast::~UniReliableBroadcast() {
    // delete this->perfectLink;
}

void UniReliableBroadcast::start() {
    this->bestEffortBroadcast = BestEffortBroadcast(this->localhost, this->networks, [this](Msg msg){this->receive(msg);});
    this->bestEffortBroadcast.start();
}

void UniReliableBroadcast::broadcast(unsigned int msg, unsigned long seqNum) {
    std::ostringstream oss;
    oss << "b " << msg;
    this->writeLogs(oss.str());
    Payload extendMsg = this->addSelfHost(msg, seqNum);
    this->bestEffortBroadcast.broadcast(extendMsg);
    this->acksPendingLock.lock();
    this->addAck(extendMsg, this->localhost.id);
    this->addPending(extendMsg);
    this->acksPendingLock.unlock();

}

void UniReliableBroadcast::broadcast(Payload payload) {
    std::ostringstream oss;
    oss << "b " << payload.content;
    this->writeLogs(oss.str());
    this->bestEffortBroadcast.broadcast(payload);
    this->acksPendingLock.lock();
    this->addAck(payload, this->localhost.id);
    this->addPending(payload);
    this->acksPendingLock.unlock();
}

std::vector<std::string> UniReliableBroadcast::getLogs() {
    return this->logs;
} 

void UniReliableBroadcast::addAck(Msg wrapedMsg) {
    // std::cout << "addAck " << wrapedMsg.payload.content << " " << wrapedMsg.payload.id << " from " << wrapedMsg.sender.id << "\n";
    if (this->acks.find(wrapedMsg.payload) != this->acks.end()) {
        this->acks[wrapedMsg.payload].insert(wrapedMsg.sender.id);
    } else {
        this->acks.insert({wrapedMsg.payload, std::unordered_set<unsigned long>({wrapedMsg.sender.id})});
    }
}

void UniReliableBroadcast::addAck(Payload msg,unsigned long senderId) {
    // std::cout << "addAck " << msg.content << " " << msg.id << " from " << senderId << "\n";
    if (this->acks.find(msg) != this->acks.end()) {
        this->acks[msg].insert(senderId);
    } else {
        this->acks.insert({msg, std::unordered_set<unsigned long>({senderId})});
    }
}

void UniReliableBroadcast::addPending(Payload payload) {
    this->pending.insert(payload);
}

bool UniReliableBroadcast::isPending(Msg wrapedMsg) {
    return this->pending.find(wrapedMsg.payload) != this->pending.end();
}

bool UniReliableBroadcast::isDelivered(Msg wrapedMsg) {
    return this->delivered.find(wrapedMsg.payload) != this->delivered.end();
}

bool UniReliableBroadcast::canDeliver(Msg wrapedMsg) {
    if (this->acks.find(wrapedMsg.payload) != this->acks.end()) {
        return this->acks[wrapedMsg.payload].size() > this->networkSize /2;
    } else {
        return false;
    }
}

void UniReliableBroadcast::receive(Msg wrapedMsg) {
    // std::cout << "Received (" << wrapedMsg.payload.content << "," << wrapedMsg.payload.id << ") from " << wrapedMsg.sender.id << "\n";
    bool shouldBroadcast = false; // Use this to avoid deadlock
    bool shouldDeliver = false; // Use this to avoid deadlock
    this->acksPendingLock.lock();
    this->addAck(wrapedMsg);
    if (!isPending(wrapedMsg)) {
        this->addAck(wrapedMsg.payload, this->localhost.id);
        this->addPending(wrapedMsg.payload);
        shouldBroadcast = true;
    }
    this->acksPendingLock.unlock();

    if (shouldBroadcast) {
        // std::cout <<"Relay " << wrapedMsg.payload.content << " " << wrapedMsg.payload.id << " from " << this->localhost.id << "\n";
        this->bestEffortBroadcast.broadcast(wrapedMsg.payload);
    }

    this->acksPendingLock.lock();
    if (canDeliver(wrapedMsg) && !isDelivered(wrapedMsg)) {
        shouldDeliver = true;
    }
    this->acksPendingLock.unlock();

    if (shouldDeliver) {
        this->deliver(wrapedMsg);
    }
}

void UniReliableBroadcast::deliver(Msg wrapedMsg) {
    this->delivered.insert(wrapedMsg.payload);
    std::ostringstream oss;
    // std::cout << "URB Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";
    oss << "d " << wrapedMsg.payload.content << " " << wrapedMsg.payload.id;
    this->writeLogs(oss.str());
    this->deliverCallBack(wrapedMsg);

}

Payload UniReliableBroadcast::addSelfHost(unsigned int msg, unsigned long seqNum) {
    // return std::make_pair(this->localhost.id, msg);
    return Payload({this->localhost.id, msg, seqNum});
}

void UniReliableBroadcast::writeLogs(std::string log) {
    this->logsLock.lock();
    this->logs.push_back(log);
    this->logsLock.unlock();
}

