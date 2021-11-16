#include "beb.hpp"
#include "urb.hpp"
#include <algorithm>
#include <utility> 

UniReliableBroadcast::UniReliableBroadcast() {
    this->networkSize = 0;
}

UniReliableBroadcast::UniReliableBroadcast(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->networkSize = networks.size();
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

    return *this;
}

UniReliableBroadcast::~UniReliableBroadcast() {
    // delete this->perfectLink;
}

void UniReliableBroadcast::start() {
    this->bestEffortBroadcast = BestEffortBroadcast(this->localhost, this->networks, [this](Msg msg){this->receive(msg);});
    this->bestEffortBroadcast.start();
}

void UniReliableBroadcast::broadcast(unsigned int msg) {
    std::ostringstream oss;
    oss << "b " << msg;
    logs.push_back(oss.str());
    host_msg_type extendMsg = this->addSelfHost(msg);
    this->bestEffortBroadcast.broadcast(extendMsg);
    this->addAck(extendMsg);
    pending.insert(extendMsg);

}

std::vector<std::string> UniReliableBroadcast::getLogs() {
    return this->logs;
} 

void UniReliableBroadcast::addAck(Msg wrapedMsg) {
    if (this->acks.find(wrapedMsg.content) != this->acks.end()) {
        this->acks[wrapedMsg.content].insert(wrapedMsg.sender.id);
    } else {
        this->acks.insert({wrapedMsg.content, std::unordered_set<unsigned long>({wrapedMsg.sender.id})});
    }
}

void UniReliableBroadcast::addAck(host_msg_type msg) {
    if (this->acks.find(msg) != this->acks.end()) {
        this->acks[msg].insert(msg.first);
    } else {
        this->acks.insert({msg, std::unordered_set<unsigned long>({msg.first})});
    }
}

bool UniReliableBroadcast::isPending(Msg wrapedMsg) {
    return this->pending.find(wrapedMsg.content) != this->pending.end();
}

bool UniReliableBroadcast::isDelivered(Msg wrapedMsg) {
    return this->delivered.find(wrapedMsg.content) != this->delivered.end();
}

bool UniReliableBroadcast::canDeliver(Msg wrapedMsg) {
    if (this->acks.find(wrapedMsg.content) != this->acks.end()) {
        return this->acks[wrapedMsg.content].size() > this->networkSize /2;
    } else {
        return false;
    }
}

void UniReliableBroadcast::receive(Msg wrapedMsg) {
    std::cout << "Received (" << wrapedMsg.content.first << "," << wrapedMsg.content.second << ") from " << wrapedMsg.sender.id << "\n";
    this->addAck(wrapedMsg);
    if (!isPending(wrapedMsg)) {
        this->pending.insert(wrapedMsg.content);
        this->bestEffortBroadcast.broadcast(wrapedMsg.content);
    }
    if (canDeliver(wrapedMsg) && !isDelivered(wrapedMsg)) {
        this->deliver(wrapedMsg);
    }
}

void UniReliableBroadcast::deliver(Msg wrapedMsg) {
    this->delivered.insert(wrapedMsg.content);
    std::ostringstream oss;
    std::cout << "Delivered " << wrapedMsg.content.second << " from " << wrapedMsg.content.first <<  "\n";
    // oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.content;
    oss << "d " << wrapedMsg.content.first << " " << wrapedMsg.content.second;
    logs.push_back(oss.str());
}

host_msg_type UniReliableBroadcast::addSelfHost(unsigned int msg) {
    return std::make_pair(this->localhost.id, msg);
}


