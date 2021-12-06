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
    packedSize = 100;
}

UDPSocket::UDPSocket(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = deliverCallBack;
    msg_id = 0;
    packedSize = 100;

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
    this->packedSize = other.packedSize;
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
    // struct sockaddr_in destaddr = this->setUpDestAddr(dest);
    struct Payload payload = {this->localhost.id, msg, seqNum};
    
    msgQueueLock.lock();
    auto it = tempBuffer.find(dest.id);
    if (it!=tempBuffer.end()) {
        it->second.push_back(payload);
        while(it->second.size() >= this->packedSize) {
            PackedMsg packedMsg = {
                this->localhost.id,
                dest.id,
                this->msg_id,
                false,
                it->second,
                this->packedSize
            };
            msg_id++;
            msgQueue.push_back(packedMsg);
            it->second.erase(it->second.begin(), it->second.begin()+this->packedSize);
        }
    } else {
        tempBuffer.insert({dest.id, std::vector<Payload>({payload})});
    }

    msgQueueLock.unlock();

    std::ostringstream oss;
    oss << "b " << msg;
    logsLock.lock();
    logs.push_back(oss.str());
    logsLock.unlock();

}

void UDPSocket::put(Parser::Host dest, Payload payload) {    
    msgQueueLock.lock();
    auto it = tempBuffer.find(dest.id);
    if (it!=tempBuffer.end()) {
        it->second.push_back(payload);
        while(it->second.size() >= this->packedSize) {
            PackedMsg packedMsg = {
                this->localhost.id,
                dest.id,
                this->msg_id,
                false,
                it->second,
                this->packedSize
            };
            msg_id++;
            msgQueue.push_back(packedMsg);
            it->second.erase(it->second.begin(), it->second.begin()+this->packedSize);
        }
    } else {
        tempBuffer.insert({dest.id, std::vector<Payload>({payload})});
    }

    msgQueueLock.unlock();

    std::ostringstream oss;
    oss << "b " << payload.content;
    this->writeLogs(oss.str());
}

void UDPSocket::send() {
    // Reference: https://stackoverflow.com/questions/5249418/warning-use-of-old-style-cast-in-g just try all of them until no error
    unsigned long int nSent = 0;
    while(true) {
        std::this_thread::sleep_for (std::chrono::microseconds(10*nSent));
        nSent = 0;
        msgQueueLock.lock();
        // std::set<Msg> copiedMsgQueue = msgQueue;
        if (msgQueue.empty()) {
            for (auto& it :tempBuffer) {
                if (it.second.empty()) {
                    continue;
                } else {
                    if (it.second.size() >= this->packedSize) {
                        throw std::runtime_error("Something wrong");
                    }
                    PackedMsg packedMsg = {
                        this->localhost.id,
                        it.first,
                        this->msg_id,
                        false,
                        it.second,
                        it.second.size()
                    };
                    msg_id++;
                    msgQueue.push_back(packedMsg);
                    it.second.clear();
                }
            } 
        }
        std::vector<PackedMsg> copiedMsgQueue = msgQueue;
        // std::cout << "NReceive:" << nReceive << "\n";
        msgQueueLock.unlock();
        // sort(copiedMsgQueue.begin(), copiedMsgQueue.end());

        // if (copiedMsgQueue.size()==0) {
        //     std::cout << "Empty msqQueue" << "\n";
        // }
        for (const auto wrapedMsg : copiedMsgQueue) {
            struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.receiverId);
            
            // sentLock.lock();
            sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
            nSent++;
            // sentLock.unlock();

            // std::this_thread::sleep_for (std::chrono::milliseconds(10));

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
            if (packedMsg.is_ack) {
                msgQueueLock.lock();
                // std::cout << "Receive ack from " << wrapedMsg.sender.id << " with content " << wrapedMsg.payload << "\n";
                // std::cout << "Before:" << msgQueue.size() << "\n";
                msgQueue.erase(std::remove(msgQueue.begin(), msgQueue.end(), packedMsg), msgQueue.end());
                // nReceive += 1;
                // msgQueue.erase(wrapedMsg);
                // std::cout << "After:" << msgQueue.size() << "\n";
                msgQueueLock.unlock();
            } else {
                //normal msg
                if (std::find(receivedMsgs.begin(), receivedMsgs.end(), packedMsg) != receivedMsgs.end()) {
                    // if already receive
                    // std::cout<< "Rejected " << wrapedMsg.payload << " from "<< wrapedMsg.sender.id << "\n";
                } else {
                    //otherwise, save it
                    receivedMsgs.push_back(packedMsg);
                    // std::cout << "Received " << wrapedMsg.payload.content << " from " << wrapedMsg.sender.id <<  " " << &wrapedMsg << "\n";
                    // std::cout << "PL Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";
                    for (long unsigned int i=0; i<packedMsg.nMsg;i++) {
                        std::ostringstream oss;
                        oss << "d " << packedMsg.senderId << " " << packedMsg.payloads[i].content;
                        this->writeLogs(oss.str());

                        this->deliverCallBack({packedMsg.senderId, packedMsg.receiverId, packedMsg.payloads[i], packedMsg.is_ack});
                    }
                    // std::cout<< "Received " << wrapedMsg.payload << " from "<< wrapedMsg.sender.id << "\n";
                }    
                // std::this_thread::sleep_for (std::chrono::milliseconds(1));
                // send Ack back to sender
                packedMsg.is_ack = true;
                struct sockaddr_in destaddr = this->setUpDestAddr(packedMsg.senderId);
                
                host_id_type tempAddr = packedMsg.senderId;
                packedMsg.senderId = this->localhost.id;
                packedMsg.receiverId = tempAddr;
                // sentLock.lock();
                // sendto(this->sockfd, &packedMsg, sizeof(packedMsg)-sizeof(Payload)*100 , 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
                sendto(this->sockfd, &packedMsg, sizeof(packedMsg) , 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
                // sentLock.unlock();
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