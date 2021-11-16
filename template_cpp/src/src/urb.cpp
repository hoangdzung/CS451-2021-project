#include "beb.hpp"
#include "urb.hpp"
#include <algorithm>
#include <utility> 

UniReliableBroadcast::UniReliableBroadcast() {
    this->networkSize = 0;
    this->deliverCallBack = [](Msg msg) {};
}

UniReliableBroadcast::UniReliableBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->networkSize = networks.size();
    this->deliverCallBack = [](Msg msg) {};
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
    this->deliverCallBack = deliverCallBack;

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
    logs.push_back(oss.str());
    Payload extendMsg = this->addSelfHost(msg, seqNum);
    this->bestEffortBroadcast.broadcast(extendMsg);
    this->addAck(extendMsg);
    pending.insert(extendMsg);

}

void UniReliableBroadcast::broadcast(Payload payload) {
    std::ostringstream oss;
    oss << "b " << payload.content;
    logs.push_back(oss.str());
    this->bestEffortBroadcast.broadcast(payload);
    this->addAck(payload);
    pending.insert(payload);

}

std::vector<std::string> UniReliableBroadcast::getLogs() {
    return this->logs;
} 

void UniReliableBroadcast::addAck(Msg wrapedMsg) {
    if (this->acks.find(wrapedMsg.payload) != this->acks.end()) {
        this->acks[wrapedMsg.payload].insert(wrapedMsg.sender.id);
    } else {
        this->acks.insert({wrapedMsg.payload, std::unordered_set<unsigned long>({wrapedMsg.sender.id})});
    }
}

void UniReliableBroadcast::addAck(Payload msg) {
    if (this->acks.find(msg) != this->acks.end()) {
        this->acks[msg].insert(msg.id);
    } else {
        this->acks.insert({msg, std::unordered_set<unsigned long>({msg.id})});
    }
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
    std::cout << "Received (" << wrapedMsg.payload.content << "," << wrapedMsg.payload.id << ") from " << wrapedMsg.sender.id << "\n";
    this->addAck(wrapedMsg);
    if (!isPending(wrapedMsg)) {
        this->pending.insert(wrapedMsg.payload);
        this->bestEffortBroadcast.broadcast(wrapedMsg.payload);
    }
    if (canDeliver(wrapedMsg) && !isDelivered(wrapedMsg)) {
        this->deliver(wrapedMsg);
    }
}

void UniReliableBroadcast::deliver(Msg wrapedMsg) {
    this->delivered.insert(wrapedMsg.payload);
    std::ostringstream oss;
    std::cout << "Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.payload;
    oss << "d " << wrapedMsg.payload.content << " " << wrapedMsg.payload.id;
    logs.push_back(oss.str());
    this->deliverCallBack(wrapedMsg);
}

Payload UniReliableBroadcast::addSelfHost(unsigned int msg, unsigned long seqNum) {
    // return std::make_pair(this->localhost.id, msg);
    return Payload({this->localhost.id, msg, seqNum});
}


