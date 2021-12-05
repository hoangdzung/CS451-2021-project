#include<thread>
#include <chrono>         // std::chrono::seconds
#include <algorithm>

#include "udp.hpp"

// Reference: https://www.geeksforgeeks.org/udp-server-client-implementation-c/

UDPSocket::UDPSocket() {
    this->deliverCallBack = [](Msg msg) {};
}
UDPSocket::UDPSocket(Parser::Host localhost, std::vector<Parser::Host> networks) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = [](Msg msg) {};
    msg_id = 0;
    nSend = 500;
    nPrevSend = 500;
    nReceive = 0;
}

UDPSocket::UDPSocket(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = deliverCallBack;
    msg_id = 0;
    nSend = 500;
    nPrevSend = 500;
    nReceive = 0;
}

void UDPSocket::start() {
    sockfd = this->setupSocket(localhost);
    std::thread sendThread(&UDPSocket::send, this);
    std::thread receiveThread(&UDPSocket::receive, this);

    sendThread.detach(); 
    receiveThread.detach(); 
}
UDPSocket& UDPSocket::operator=(const UDPSocket & other) {
    this->logs = other.logs;
    this->localhost = other.localhost;
    this->networks = other.networks;
    this->deliverCallBack = other.deliverCallBack;
    this->sockfd = other.sockfd;
    this->msg_id = other.msg_id;
    this->nSend = other.nSend;
    this->nPrevSend = other.nPrevSend;
    this->nReceive = other.nReceive;
    this->msgQueue = other.msgQueue;
    this->receivedMsgs = other.receivedMsgs;
    return *this;
}

struct sockaddr_in UDPSocket::setUpDestAddr(unsigned long destId) {
    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET; //IPv4
    destaddr.sin_addr.s_addr = this->networks[destId-1].ip;
    destaddr.sin_port = this->networks[destId-1].port;
    return destaddr;
}

void UDPSocket::put(Parser::Host dest, unsigned int msg, unsigned long seqNum) {    
    struct WrapedPayload wrapedPayload = {
        Payload ({this->localhost.id, msg, seqNum}),
        false
        };

    msgQueueLock.lock();
    auto it = msgQueue.find(dest.id);
    if (it!=msgQueue.end()) {
        it->second.push_back(wrapedPayload);
    } else {
        // msgQueue.insert({dest.id, std::list<WrapedPayload>({wrapedPayload})});
        msgQueue.insert({dest.id, std::vector<WrapedPayload>({wrapedPayload})});
    }
    msgQueueLock.unlock();

    std::ostringstream oss;
    oss << "b " << msg;
    logsLock.lock();
    logs.push_back(oss.str());
    logsLock.unlock();

}

void UDPSocket::put(Parser::Host dest, Payload msg) {    
    struct WrapedPayload wrapedPayload = {
        msg,
        false
        };
    msgQueueLock.lock();
    auto it = msgQueue.find(dest.id);
    if (it!=msgQueue.end()) {
        it->second.push_back(wrapedPayload);
    } else {
        // msgQueue.insert({dest.id, std::list<WrapedPayload>({wrapedPayload})});
        msgQueue.insert({dest.id, std::vector<WrapedPayload>({wrapedPayload})});
    }
    msgQueueLock.unlock();

    std::ostringstream oss;
    oss << "b " << msg.content;
    this->writeLogs(oss.str());
}

void UDPSocket::send() {
    // Reference: https://stackoverflow.com/questions/5249418/warning-use-of-old-style-cast-in-g just try all of them until no error
    WrapedPayload wrapedPayloads[100];
    while(true) {
        std::this_thread::sleep_for (std::chrono::milliseconds(100));

        msgQueueLock.lock();
        // std::set<Msg> copiedMsgQueue = msgQueue;
        // std::unordered_map<host_id_type, std::list<WrapedPayload>> copiedMsgQueue = msgQueue;
        std::unordered_map<host_id_type, std::vector<WrapedPayload>> copiedMsgQueue = msgQueue;
        msgQueueLock.unlock();
        // sort(copiedMsgQueue.begin(), copiedMsgQueue.end());

        // if (copiedMsgQueue.size()==0) {
        //     std::cout << "Empty msqQueue" << "\n";
        // }
        bool done = false;
        while (!done) {
            done = true;
            for (auto& it :copiedMsgQueue) {
                if (it.second.size() == 0) {
                    continue;
                } else {
                    long unsigned int nSend = 100;
                    if (nSend > it.second.size()) {
                        nSend = it.second.size();
                    } else {
                        done = false;
                    }
                    // std::cout << "Have "<< it.second.size() << " msgs of " << it.first << "\n";
                    // std::cout << "Pack "<< nSend << " msgs of " << it.first << "\n";
                    // std::list<WrapedPayload>::iterator itBegin = it.second.begin();
                    // std::list<WrapedPayload>::iterator itEnd = it.second.begin();
                    // advance(itEnd,nSend);
                    // std::copy(itBegin, itEnd, wrapedPayloads);
                    std::copy(it.second.begin(), it.second.begin()+nSend, wrapedPayloads);
                    PackedMsg packedMsg = {
                        this->localhost.id,
                        it.first,
                        wrapedPayloads,
                        nSend
                    };
                    struct sockaddr_in destaddr = this->setUpDestAddr(packedMsg.receiverId);
                    sendto(this->sockfd, &packedMsg, sizeof(packedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
                    it.second.erase(it.second.begin(), it.second.begin()+nSend);
                    // std::cout << "Have "<< it.second.size() << " msgs of " << it.first << "\n";
                }
            }        
        }
    }
}

void UDPSocket::receive() {
    // Reference: https://stackoverflow.com/questions/18670807/sending-and-receiving-stdstring-over-socket
    struct PackedMsg packedMsg; 
    while (true) {
        // std::this_thread::sleep_for (std::chrono::milliseconds(1));

        if (recv(this->sockfd, &packedMsg, sizeof(packedMsg), 0) < 0) {
            throw std::runtime_error("Receive failed");
        } else {
            for (long unsigned int i=0; i<packedMsg.nMsg;i++) {
                // std::cout << packedMsg.payloads[i].payload.content << " " << packedMsg.payloads[i].is_ack << "\n";
                if (packedMsg.payloads[i].is_ack) {
                    msgQueueLock.lock();
                    // std::cout << "Receive ack from " << wrapedMsg.sender.id << " with content " << wrapedMsg.payload << "\n";
                    // std::cout << "Before:" << msgQueue.size() << "\n";
                    auto msgQueueIt = msgQueue.find(packedMsg.senderId);
                    if (msgQueueIt!=msgQueue.end()) {
                        msgQueueIt->second.erase(std::remove(msgQueueIt->second.begin(), msgQueueIt->second.end(), packedMsg.payloads[i]), msgQueueIt->second.end());
                    }                       
                    // msgQueue.erase(wrapedMsg);
                    // std::cout << "After:" << msgQueue.size() << "\n";
                    msgQueueLock.unlock();
                } else {
                    //normal msg
                    auto receivedIt = receivedMsgs.find(packedMsg.senderId);
                    if (receivedIt==receivedMsgs.end()) {
                        // receivedMsgs.insert({packedMsg.senderId, std::list<WrapedPayload>({})});
                        receivedMsgs.insert({packedMsg.senderId, std::vector<WrapedPayload>({})});
                        receivedIt = receivedMsgs.find(packedMsg.senderId);
                    }
                    if (std::find(receivedIt->second.begin(), receivedIt->second.end(), packedMsg.payloads[i]) != receivedIt->second.end()) {
                        // if already receive
                        // std::cout<< "Rejected " << wrapedMsg.payload << " from "<< wrapedMsg.sender.id << "\n";
                    } else {
                        //otherwise, save it
                     
                        receivedIt->second.push_back(packedMsg.payloads[i]);
                        // std::cout << "Received " << wrapedMsg.payload.content << " from " << wrapedMsg.sender.id <<  " " << &wrapedMsg << "\n";
                        // std::cout << "PL Delivered " << packedMsg.payloads[i].payload.content << " from " << packedMsg.payloads[i].payload.id <<  "\n";

                        std::ostringstream oss;
                        oss << "d " << packedMsg.senderId << " " << packedMsg.payloads[i].payload.content;
                        this->writeLogs(oss.str());

                        this->deliverCallBack({packedMsg.senderId, packedMsg.receiverId, packedMsg.payloads[i].payload, packedMsg.payloads[i].is_ack});
                        // std::cout<< "Received " << wrapedMsg.payload << " from "<< wrapedMsg.sender.id << "\n";
                    }    
                    // std::this_thread::sleep_for (std::chrono::milliseconds(1));
                    // send Ack back to sender
                    msgQueueLock.lock();
                    auto msgQueueIt = msgQueue.find(packedMsg.senderId);
                    if (msgQueueIt!=msgQueue.end()) {
                        // msgQueueIt->second.push_front({packedMsg.payloads[i].payload, true});
                        msgQueueIt->second.push_back({packedMsg.payloads[i].payload, true});
                    }  
                    msgQueueLock.unlock();

                    // wrapedMsg.is_ack = true;
                    // struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.senderId);
                    
                    // host_id_type tempAddr = wrapedMsg.senderId;
                    // wrapedMsg.senderId = this->localhost.id;
                    // wrapedMsg.receiverId = tempAddr;
                    // // sentLock.lock();
                    // sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
                    // // sentLock.unlock();
                } 
            } 
        }
    }
}

int UDPSocket::setupSocket(Parser::Host host) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Socket creation failed");
    }
    struct sockaddr_in hostaddr;
    memset(&hostaddr, 0, sizeof(hostaddr));
    hostaddr.sin_family = AF_INET; //IPv4
    hostaddr.sin_addr.s_addr = host.ip;
    hostaddr.sin_port = host.port;

    if (bind(sockfd, reinterpret_cast<const sockaddr *>(&hostaddr), sizeof(hostaddr)) < 0) {
        throw std::runtime_error("Bind failed");
    }
    return sockfd;
}

void UDPSocket::writeLogs(std::string log) {
    this->logsLock.lock();
    this->logs.push_back(log);
    this->logsLock.unlock();
}

std::vector<std::string> UDPSocket::getLogs() {
    return this->logs;
}