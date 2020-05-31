//
// Created by root on 2020/5/31.
//

#pragma once

#include <unordered_map>
#include "CommandHandler.h"
#include "../Utils/Singleton.h"

class CommandDispatch : public Singleton<CommandDispatch> {
public:

    void Register(int command_code, CommandDefaultHandler handler) {
        if (this->dispatch_map.find(command_code) != this->dispatch_map.end()) {
            throw std::runtime_error("[CommandDispatch::Register] command_code collision");
        }
        this->dispatch_map.insert({command_code, handler});
    }

    void Dispatch(int command_code, int payload_len, boost::intrusive_ptr<Session>& session) {
        this->dispatch_map[command_code](session, payload_len);
    }

private:
    std::unordered_map<int, CommandDefaultHandler> dispatch_map;
};


