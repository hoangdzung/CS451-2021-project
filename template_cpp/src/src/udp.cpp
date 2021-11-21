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
    nSend = 100;
    nReceive = 0;
}

UDPSocket::UDPSocket(Parser::Host localhost, std::vector<Parser::Host> networks, std::function<void(Msg)> deliverCallBack) {
    this->localhost = localhost;
    this->networks = networks;
    this->deliverCallBack = deliverCallBack;
    msg_id = 0;
    nSend = 100;
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
    // struct sockaddr_in destaddr = this->setUpDestAddr(dest);
    struct Msg wrapedMsg = {
        this->localhost.id,
        dest.id,
        // msg_id,
        // std::make_pair(this->localhost.id, msg),
        Payload ({this->localhost.id, msg, seqNum}),
        false
        };
    msg_id++;
    // struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.receiverId);
    
    // // std::cout << "Send msg " << wrapedMsg.payload.content << " to " <<wrapedMsg.receiver.id << "\n";
    // // sentLock.lock();
    // sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
    msgQueueLock.lock();
    // std::cout << "Put msg " << wrapedMsg.payload << " to " <<wrapedMsg.receiver.id << "\n";
    msgQueue.push_back(wrapedMsg);
    msgQueueLock.unlock();

    std::ostringstream oss;
    oss << "b " << msg;
    logsLock.lock();
    logs.push_back(oss.str());
    logsLock.unlock();

}

void UDPSocket::put(Parser::Host dest, Payload msg) {    
    // struct sockaddr_in destaddr = this->setUpDestAddr(dest);
    struct Msg wrapedMsg = {
        this->localhost.id,
        dest.id,
        // msg_id,
        msg,
        false
        };
    msg_id++;
    // struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.receiverId);
    
    // // std::cout << "Send msg " << wrapedMsg.payload.content << " to " <<wrapedMsg.receiver.id << "\n";
    // // sentLock.lock();
    // sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
    msgQueueLock.lock();
    // std::cout << "Put msg " << wrapedMsg.payload << " to " <<wrapedMsg.receiver.id << "\n";
    msgQueue.push_back(wrapedMsg);
    msgQueueLock.unlock();

    std::ostringstream oss;
    oss << "b " << msg.content;
    this->writeLogs(oss.str());
}

void UDPSocket::send() {
    // Reference: https://stackoverflow.com/questions/5249418/warning-use-of-old-style-cast-in-g just try all of them until no error
    while(true) {
        std::this_thread::sleep_for (std::chrono::milliseconds(100));

        msgQueueLock.lock();
        // std::set<Msg> copiedMsgQueue = msgQueue;
        std::vector<Msg> copiedMsgQueue;
        // std::cout << "NReceive:" << nReceive << "\n";
        if (nReceive ==0) {
            nSend += 100;
            // std::cout << "Nsend:" << nSend << "\n";
        } 
        nReceive = 0;
        if (msgQueue.size()>nSend) {
            std::partial_sort (msgQueue.begin(), msgQueue.begin()+nSend, msgQueue.end());
            copiedMsgQueue = std::vector<Msg>{msgQueue.begin(),std::next(msgQueue.begin(),nSend)};
        } else {
            sort(msgQueue.begin(), msgQueue.end());
            copiedMsgQueue = msgQueue;
        }
        msgQueueLock.unlock();
        // sort(copiedMsgQueue.begin(), copiedMsgQueue.end());

        // if (copiedMsgQueue.size()==0) {
        //     std::cout << "Empty msqQueue" << "\n";
        // }
        for (const auto wrapedMsg : copiedMsgQueue) {
            struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.receiverId);
            
            // std::cout << "Send msg " << wrapedMsg.payload.content << " to " <<wrapedMsg.receiver.id << "\n";
            // sentLock.lock();
            sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
            // sentLock.unlock();

            // std::this_thread::sleep_for (std::chrono::milliseconds(10));

        }
    }
}

void UDPSocket::receive() {
    // Reference: https://stackoverflow.com/questions/18670807/sending-and-receiving-stdstring-over-socket
    struct Msg wrapedMsg; 
    while (true) {
        // std::this_thread::sleep_for (std::chrono::milliseconds(1));

        if (recv(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0) < 0) {
            throw std::runtime_error("Receive failed");
        } else {
            if (wrapedMsg.is_ack) {
                msgQueueLock.lock();
                // std::cout << "Receive ack from " << wrapedMsg.sender.id << " with content " << wrapedMsg.payload << "\n";
                // std::cout << "Before:" << msgQueue.size() << "\n";
                msgQueue.erase(std::remove(msgQueue.begin(), msgQueue.end(), wrapedMsg), msgQueue.end());
                nReceive += 1;
                // msgQueue.erase(wrapedMsg);
                // std::cout << "After:" << msgQueue.size() << "\n";
                msgQueueLock.unlock();
            } else {
                //normal msg
                if (std::find(receivedMsgs.begin(), receivedMsgs.end(), wrapedMsg) != receivedMsgs.end()) {
                    // if already receive
                    // std::cout<< "Rejected " << wrapedMsg.payload << " from "<< wrapedMsg.sender.id << "\n";
                } else {
                    //otherwise, save it
                    receivedMsgs.push_back(wrapedMsg);
                    // std::cout << "Received " << wrapedMsg.payload.content << " from " << wrapedMsg.sender.id <<  " " << &wrapedMsg << "\n";
                    // std::cout << "PL Delivered " << wrapedMsg.payload.content << " from " << wrapedMsg.payload.id <<  "\n";

                    std::ostringstream oss;
                    oss << "d " << wrapedMsg.senderId << " " << wrapedMsg.payload.content;
                    this->writeLogs(oss.str());

                    this->deliverCallBack(wrapedMsg);

                    // std::cout<< "Received " << wrapedMsg.payload << " from "<< wrapedMsg.sender.id << "\n";
                }    
                // std::this_thread::sleep_for (std::chrono::milliseconds(1));
                // send Ack back to sender
                wrapedMsg.is_ack = true;
                struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.senderId);
                
                host_id_type tempAddr = wrapedMsg.senderId;
                wrapedMsg.senderId = this->localhost.id;
                wrapedMsg.receiverId = tempAddr;
                // sentLock.lock();
                sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
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