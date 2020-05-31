//
// Created by root on 2020/5/31.
//

#pragma once

#include "CommandDispatch.h"
#include "StringCommand.h"

namespace command {
    void RegisterAll() {
        auto dispatcher = CommandDispatch::GetInstance();
        dispatcher->Register(string_command::GET, command::StringGet);
        dispatcher->Register(string_command::SET, command::StringSet);
        dispatcher->Register(string_command::GETSET, command::StringGetSet);
    }
}
