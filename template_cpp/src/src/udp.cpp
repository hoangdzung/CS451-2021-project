#include<thread>
#include "udp.hpp"

// Reference: https://www.geeksforgeeks.org/udp-server-client-implementation-c/

UDPSocket::UDPSocket(Parser::Host localhost) {
    this->localhost = localhost;
    sockfd = this->setupSocket(localhost);
    msg_id = 0;
    std::thread sendThread(&UDPSocket::send, this);
    std::thread receiveThread(&UDPSocket::receive, this);

    sendThread.detach(); 
    receiveThread.detach(); 
}

struct sockaddr_in UDPSocket::setUpDestAddr(Parser::Host dest) {
    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET; //IPv4
    destaddr.sin_addr.s_addr = dest.ip;
    destaddr.sin_port = dest.port;
    return destaddr;
}

void UDPSocket::put(Parser::Host dest, unsigned int msg) {    
    struct sockaddr_in destaddr = this->setUpDestAddr(dest);
    struct Msg wrapedMsg = {
        this->localhost,
        dest,
        msg_id,
        msg,
        false
        };
    msg_id++;
    msgQueueLock.lock();
    std::cout << "Put msg " << wrapedMsg.content << " to " <<wrapedMsg.receiver.id << "\n";
    msgQueue.push_back(wrapedMsg);
    msgQueueLock.unlock();

}

void UDPSocket::send() {
    // Reference: https://stackoverflow.com/questions/5249418/warning-use-of-old-style-cast-in-g just try all of them until no error
    while(true) {
        msgQueueLock.lock();
        std::vector<Msg> copiedMsgQueue = msgQueue;
        msgQueueLock.unlock();
        // if (copiedMsgQueue.size()==0) {
        //     std::cout << "Empty msqQueue" << "\n";
        // }
        for (auto & wrapedMsg : copiedMsgQueue) {
            struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.receiver);
            std::cout << "Send msg " << wrapedMsg.content << " to " <<wrapedMsg.receiver.id << "\n";
            sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
        }
    }
}

void UDPSocket::receive() {
    // Reference: https://stackoverflow.com/questions/18670807/sending-and-receiving-stdstring-over-socket
    struct Msg wrapedMsg; 
    while (true) {
        if (recv(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0) < 0) {
            throw std::runtime_error("Receive failed");
        } else {
            if (wrapedMsg.is_ack) {
                std::cout << "Receive ack from " << wrapedMsg.sender.id << " with content " << wrapedMsg.content << "\n";
                msgQueueLock.lock();
                std::remove(msgQueue.begin(), msgQueue.end(), wrapedMsg);
                msgQueueLock.unlock();
            } else {
                //normal msg
                if (std::find(receivedMsgs.begin(), receivedMsgs.end(), wrapedMsg) != receivedMsgs.end()) {
                    // if already receive
                    std::cout<< "Rejected " << wrapedMsg.content << " from "<< wrapedMsg.sender.id << "\n";
                } else {
                    //otherwise, save it
                    receivedMsgs.push_back(wrapedMsg);
                    std::cout<< "Received " << wrapedMsg.content << " from "<< wrapedMsg.sender.id << "\n";
                }    
                // send Ack back to sender
                wrapedMsg.is_ack = true;
                struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.sender);
                Parser::Host tempAddr = wrapedMsg.sender;
                wrapedMsg.sender = this->localhost;
                wrapedMsg.receiver = tempAddr;
                
                sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
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