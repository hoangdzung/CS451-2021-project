
struct Msg {
    unsigned long sender_id;
    unsigned long msg_id;
    unsigned int content;
    public:
    bool operator==( const Msg& other ) {
        return sender_id == other.sender_id &&
               msg_id    == other.msg_id;
    }
};
