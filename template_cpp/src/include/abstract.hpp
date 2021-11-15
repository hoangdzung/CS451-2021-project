#pragma once
#include "msg.hpp"
#include "parser.hpp"

class AbstractLayer {
    public:
        AbstractLayer() {}
        ~AbstractLayer() {}
        std::vector<std::string> logs;
        virtual void deliver(Msg wrapedMsg) = 0;
        virtual void selfDeliver(unsigned int msg) = 0;
        
        // void deliver(Msg wrapedMsg) {
        //     std::ostringstream oss;
        //     std::cout << this <<  " " << localhost.id << " d " << wrapedMsg.sender.id << " " << wrapedMsg.content << "\n";
        //     oss << this <<  " d " << wrapedMsg.sender.id << " " << wrapedMsg.content;
        //     logs.push_back(oss.str());
        // }

        // void selfDeliver(unsigned int msg) {
        //     std::ostringstream oss;
        //     std::cout << this <<  " " << localhost.id  << " self d " << localhost.id << " " << msg << "\n";
        //     oss << this << " self d " << localhost.id << " " << msg;
        //     logs.push_back(oss.str());
        // }

};
