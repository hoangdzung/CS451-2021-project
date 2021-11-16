#pragma once
#include "parser.hpp"
#include <utility> 

typedef std::pair<unsigned long, unsigned int> host_msg_type;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return 9999*h1 + h2;  
    }
};

struct Msg {
    Parser::Host sender;
    Parser::Host receiver;
    unsigned long msg_id;
    // unsigned int content;
    host_msg_type content; 
    bool is_ack;
    public:
    bool operator==( const Msg& other ) {
        if (other.is_ack) 
            return sender.ip == other.receiver.ip &&
                    sender.port == other.receiver.port &&
                    receiver.ip == other.sender.ip &&
                    receiver.port == other.sender.port &&
                    msg_id == other.msg_id;
        else
            return sender.ip == other.sender.ip &&
                    sender.port == other.sender.port &&
                    receiver.ip == other.receiver.ip &&
                    receiver.port == other.receiver.port &&
                    msg_id == other.msg_id;
    }
};
