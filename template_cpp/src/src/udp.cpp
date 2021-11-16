#include<thread>
#include <chrono>         // std::chrono::seconds
#include <algorithm>

#include "udp.hpp"

// Reference: https://www.geeksforgeeks.org/udp-server-client-implementation-c/

UDPSocket::UDPSocket() {
    this->deliverCallBack = [](Msg msg) {std::cout << "Callback Received " << msg.payload.content << " from " << msg.sender.id <<  " " << &msg << "\n";};
    this->shouldStop = false;
}
UDPSocket::UDPSocket(Parser::Host localhost) {
    this->localhost = localhost;
    this->deliverCallBack = [](Msg msg) {std::cout << "Callback Received " << msg.payload.content << " from " << msg.sender.id <<  " " << &msg << "\n";};
    // sockfd = this->setupSocket(localhost);
    msg_id = 0;
    this->shouldStop = false;
}

UDPSocket::UDPSocket(Parser::Host localhost, std::function<void(Msg)> deliverCallBack) {
    this->localhost = localhost;
    this->deliverCallBack = deliverCallBack;
    this->shouldStop = false;
    // sockfd = this->setupSocket(localhost);
    msg_id = 0;
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
    this->deliverCallBack = other.deliverCallBack;
    this->sockfd = other.sockfd;
    this->msg_id = other.msg_id;
    this->msgQueue = other.msgQueue;
    this->receivedMsgs = other.receivedMsgs;
    this->shouldStop = other.shouldStop;
    return *this;
}

struct sockaddr_in UDPSocket::setUpDestAddr(Parser::Host dest) {
    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET; //IPv4
    destaddr.sin_addr.s_addr = dest.ip;
    destaddr.sin_port = dest.port;
    return destaddr;
}

void UDPSocket::put(Parser::Host dest, unsigned int msg, unsigned long seqNum) {    
    // struct sockaddr_in destaddr = this->setUpDestAddr(dest);
    struct Msg wrapedMsg = {
        this->localhost,
        dest,
        msg_id,
        // std::make_pair(this->localhost.id, msg),
        Payload ({this->localhost.id, msg, seqNum}),
        false
        };
    msg_id++;
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
        this->localhost,
        dest,
        msg_id,
        msg,
        false
        };
    msg_id++;
    msgQueueLock.lock();
    // std::cout << "Put msg " << wrapedMsg.payload << " to " <<wrapedMsg.receiver.id << "\n";
    msgQueue.push_back(wrapedMsg);
    msgQueueLock.unlock();

    std::ostringstream oss;
    oss << "b " << msg.content;
    logsLock.lock();
    logs.push_back(oss.str());
    logsLock.unlock();

}

void UDPSocket::stop(){
    stopLock.lock();
    this->shouldStop = true;
    stopLock.unlock();
}

void UDPSocket::send() {
    // Reference: https://stackoverflow.com/questions/5249418/warning-use-of-old-style-cast-in-g just try all of them until no error
    while(true) {
        std::this_thread::sleep_for (std::chrono::nanoseconds(1000));
        stopLock.lock();
        if (this->shouldStop) {
            std::cout << "Send thread stopped" << "\n";
            return;
        }
        stopLock.unlock();
        msgQueueLock.lock();
        std::vector<Msg> copiedMsgQueue = msgQueue;
        // std::cout << "Send:" << msgQueue.size() << "\n";
        msgQueueLock.unlock();
        // if (copiedMsgQueue.size()==0) {
        //     std::cout << "Empty msqQueue" << "\n";
        // }
        for (const auto wrapedMsg : copiedMsgQueue) {
            // struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.receiver);
            
            struct sockaddr_in destaddr;
            memset(&destaddr, 0, sizeof(destaddr));
            destaddr.sin_family = AF_INET; //IPv4
            destaddr.sin_addr.s_addr = wrapedMsg.receiver.ip;
            destaddr.sin_port = wrapedMsg.receiver.port;
            
            std::cout << "Send msg " << wrapedMsg.payload.content << " to " <<wrapedMsg.receiver.id << "\n";
            sentLock.lock();
            sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
            sentLock.unlock();
            std::this_thread::sleep_for (std::chrono::milliseconds(10));

        }
    }
}

void UDPSocket::receive() {
    // Reference: https://stackoverflow.com/questions/18670807/sending-and-receiving-stdstring-over-socket
    struct Msg wrapedMsg; 
    while (true) {
        std::this_thread::sleep_for (std::chrono::nanoseconds(1000));
        stopLock.lock();
        if (this->shouldStop) {
            std::cout << "Receive thread stopped" << "\n";
            return;
        }
        stopLock.unlock();
        if (recv(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0) < 0) {
            throw std::runtime_error("Receive failed");
        } else {
            if (wrapedMsg.is_ack) {
                msgQueueLock.lock();
                // std::cout << "Receive ack from " << wrapedMsg.sender.id << " with content " << wrapedMsg.payload << "\n";
                // std::cout << "Before:" << msgQueue.size() << "\n";
                msgQueue.erase(std::remove(msgQueue.begin(), msgQueue.end(), wrapedMsg), msgQueue.end());
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
                    std::cout << "Received " << wrapedMsg.payload.content << " from " << wrapedMsg.sender.id <<  " " << &wrapedMsg << "\n";
                    std::ostringstream oss;
                    oss << "d " << wrapedMsg.sender.id << " " << wrapedMsg.payload.content;
                    logsLock.lock();
                    logs.push_back(oss.str());
                    logsLock.unlock();

                    this->deliverCallBack(wrapedMsg);
                    std::this_thread::sleep_for (std::chrono::nanoseconds(1000));

                    // std::cout<< "Received " << wrapedMsg.payload << " from "<< wrapedMsg.sender.id << "\n";
                }    
                // send Ack back to sender
                wrapedMsg.is_ack = true;
                // struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.sender);
            
                struct sockaddr_in destaddr;
                memset(&destaddr, 0, sizeof(destaddr));
                destaddr.sin_family = AF_INET; //IPv4
                destaddr.sin_addr.s_addr = wrapedMsg.sender.ip;
                destaddr.sin_port = wrapedMsg.sender.port;
                
                Parser::Host tempAddr = wrapedMsg.sender;
                wrapedMsg.sender = this->localhost;
                wrapedMsg.receiver = tempAddr;
                sentLock.lock();
                sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
                sentLock.unlock();
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

std::vector<std::string> UDPSocket::getLogs() {
    return this->logs;
}