#pragma once
#include "parser.hpp"
#include <utility> 
#include <array>

// typedef std::pair<unsigned long, unsigned int> host_msg_type;
typedef unsigned long host_id_type;
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

struct Payload {
    unsigned long id;
    unsigned int content;
    unsigned long seqNum;
    public:
    bool operator==( const Payload& other ) const {
        return id == other.id &&
                content == other.content &&
                seqNum == other.seqNum;
    }
    bool operator <(const Payload& other) const {
        return seqNum > other.seqNum; //to get the smallest one first
    }
};

// bool operator==(const Payload& left, const Payload& right) {
//     return left.id == right.id &&
//             left.content == right.content &&
//             left.seqNum == right.seqNum;
// }

struct Msg {
    host_id_type senderId;
    host_id_type receiverId;
    // unsigned long msg_id;
    // unsigned int content;
    Payload payload; 
    bool is_ack;
    public:
    bool operator==( const Msg& other ) const {
        if (other.is_ack) 
            return senderId == other.receiverId &&
                    receiverId == other.senderId &&
                    payload == other.payload;
        else
            return senderId == other.senderId &&
                    receiverId == other.receiverId &&
                     payload == other.payload;
    }
    bool operator <( const Msg& other ) const {
        // if (payload.seqNum == other.payload.seqNum) {
        //     return msg_id < other.msg_id;
        // } else {
            return payload.seqNum < other.payload.seqNum;
        // }
    }
};

struct hash_custom {
  size_t operator()(const Payload & x) const {
    return std::hash<host_id_type>()(x.id) ^ std::hash< unsigned int>()(x.content) ^ std::hash<long unsigned int>()(x.seqNum);
  }
};
