#include "parser.hpp"

struct Msg {
    Parser::Host sender;
    unsigned long msg_id;
    unsigned int content;
    bool is_ack;
    public:
    bool operator==( const Msg& other ) {
        return sender.ip == other.sender.ip &&
                sender.port == other.sender.port &&
               msg_id == other.msg_id &&
               is_ack == other.is_ack;
    }
};
