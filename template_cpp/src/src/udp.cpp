#include "udp.hpp"

UDPSocket::UDPSocket(Parser::Host localhost) {
    this->localhost = localhost;
    sockfd = this->setupSocket(localhost);
}
void UDPSocket::send(Parser::Host dest, std::string msg) {
    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET; //IPv4
    destaddr.sin_addr.s_addr = dest.ip;
    destaddr.sin_port = dest.port;
    // Reference: https://stackoverflow.com/questions/5249418/warning-use-of-old-style-cast-in-g just try all of them until no error
    sendto(this->sockfd, msg.c_str(), strlen(msg.c_str()), 0, reinterpret_cast<const sockaddr *>(&destaddr), sizeof(destaddr));
}

std::string UDPSocket::receive() {
    // Reference: https://stackoverflow.com/questions/18670807/sending-and-receiving-stdstring-over-socket
    const unsigned int MAX_BUF_LENGTH = 4096;
    std::vector<char> buffer(MAX_BUF_LENGTH);
    std::string msgRecv;  
    while (true) {
        if (recv(this->sockfd, &buffer[0], buffer.size(), 0) < 0) {
            throw std::runtime_error("Receive failed");
        } else {
            msgRecv.append( buffer.cbegin(), buffer.cend() );
            std::cout << msgRecv << "\n";
        }
    }
}

int UDPSocket::setupSocket(Parser::Host host) {
    // Reference: https://www.geeksforgeeks.org/udp-server-client-implementation-c/
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