#include<thread>
#include "udp.hpp"

// Reference: https://www.geeksforgeeks.org/udp-server-client-implementation-c/

UDPSocket::UDPSocket(Parser::Host localhost) {
    this->localhost = localhost;
    sockfd = this->setupSocket(localhost);
    msg_id = 0;
    received_ack = false;
}
struct sockaddr_in UDPSocket::setUpDestAddr(Parser::Host dest) {
    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET; //IPv4
    destaddr.sin_addr.s_addr = dest.ip;
    destaddr.sin_port = dest.port;
    return destaddr;
}

void UDPSocket::send(Parser::Host dest, unsigned int msg) {
    // Reference: https://stackoverflow.com/questions/17472827/create-thread-inside-class-with-function-from-same-class
    std::thread list_ack(&UDPSocket::receive, this);
    list_ack.detach();
    
    struct sockaddr_in destaddr = this->setUpDestAddr(dest);
    struct Msg wrapedMsg = {
        this->localhost,
        msg_id,
        msg,
        false
        };
    msg_id++;
    // Reference: https://stackoverflow.com/questions/5249418/warning-use-of-old-style-cast-in-g just try all of them until no error
    while(!received_ack) {
        sleep(1);
        // std::cout << "send" << "\n";
        sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
    }
}

Msg UDPSocket::receive() {
    // Reference: https://stackoverflow.com/questions/18670807/sending-and-receiving-stdstring-over-socket
    struct Msg wrapedMsg; 
    while (true) {
        if (recv(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0) < 0) {
            throw std::runtime_error("Receive failed");
        } else {
            if (wrapedMsg.is_ack) {
                received_ack = true;
                return wrapedMsg;
            } else {
                //normal msg
                if (std::find(receivedMsgs.begin(), receivedMsgs.end(), wrapedMsg) != receivedMsgs.end()) {
                    // if already received, just ignore
                    std::cout<< "Rejected " << wrapedMsg.content << " from "<< wrapedMsg.sender.id << "\n";
                } else {
                    //otherwise, save it
                    receivedMsgs.push_back(wrapedMsg);
                    std::cout<< "Received " << wrapedMsg.content << " from "<< wrapedMsg.sender.id << "\n";
                    
                    // send Ack back to sender
                    wrapedMsg.is_ack = true;
                    struct sockaddr_in destaddr = this->setUpDestAddr(wrapedMsg.sender);
                    wrapedMsg.sender = this->localhost;
                    
                    sendto(this->sockfd, &wrapedMsg, sizeof(wrapedMsg), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
                    return wrapedMsg;
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