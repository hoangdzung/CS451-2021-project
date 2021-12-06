#pragma once
#include "parser.hpp"
#include <utility> 
#include <array>

#define MAX_PROCESSES 128
#define MAX_MSGS 50

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
    unsigned long vectorClock[MAX_PROCESSES];
    // unsigned long seqNum
    Payload() {};
    Payload(unsigned long id_, unsigned int content_, std::vector<unsigned long> vectorClock_) :
    id(id_), content(content_) {
        std::copy(vectorClock_.begin(), vectorClock_.begin() + MAX_PROCESSES, vectorClock);
    }

    public:
    bool operator==( const Payload& other ) const {
        for (int i =0;i<MAX_PROCESSES;i++) {
            if (vectorClock[i] != other.vectorClock[i])
                return false;
        }
        return id == other.id &&
                content == other.content;
    }
    bool operator <(const Payload& other) const {
        for (int i=0;i<MAX_PROCESSES;i++) {
            if (vectorClock[i] != other.vectorClock[i])
                return vectorClock[i] < other.vectorClock[i];
        }
        return false; 
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
        if (is_ack != other.is_ack) 
            return senderId == other.receiverId &&
                    receiverId == other.senderId &&
                    payload == other.payload;
        else
            return senderId == other.senderId &&
                    receiverId == other.receiverId &&
                     payload == other.payload;
    }
    // bool operator <( const Msg& other ) const {
    //     // if (payload.seqNum == other.payload.seqNum) {
    //     //     return msg_id < other.msg_id;
    //     // } else {
    //         return payload.seqNum < other.payload.seqNum;
    //     // }
    // }
};

struct PackedMsg {
    host_id_type senderId;
    host_id_type receiverId;
    unsigned long msg_id;
    bool is_ack;
    // unsigned int content;
    Payload payloads[MAX_MSGS];
    long unsigned int nMsg; 
    PackedMsg() {};
    PackedMsg(host_id_type senderId_, host_id_type receiverId_, unsigned long msg_id_, bool is_ack_, std::vector<Payload> payload_, long unsigned int nMsg_) :
    senderId(senderId_), receiverId(receiverId_), msg_id(msg_id_), is_ack(is_ack_), nMsg(nMsg_) {
        std::copy(payload_.begin(), payload_.begin() + nMsg_, payloads);
    }
    bool operator==( const PackedMsg& other ) const {
        if (is_ack != other.is_ack) 
            return senderId == other.receiverId &&
                    receiverId == other.senderId &&
                    msg_id == other.msg_id;
        else
            return senderId == other.senderId &&
                    receiverId == other.receiverId &&
                     msg_id == other.msg_id;    }
};


struct hash_custom {
  size_t operator()(const Payload & x) const {
    return std::hash<host_id_type>()(x.id) ^ std::hash< unsigned int>()(x.content) ^ std::hash<unsigned long>()(x.vectorClock[x.id-1]);
  }
};
