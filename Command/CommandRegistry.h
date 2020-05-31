//
// Created by root on 2020/5/31.
//

#pragma once

#include "CommandDispatch.h"
#include "StringCommand.h"
#include "UniversalCommand.h"

namespace command {
    void RegisterAll() {
        auto dispatcher = CommandDispatch::GetInstance();

        dispatcher->Register(string_command::GET, command::GET);
        dispatcher->Register(string_command::INCR, command::INCR);
        dispatcher->Register(string_command::DECR, command::DECR);
        dispatcher->Register(string_command::STRLEN, command::STRLEN);

        dispatcher->Register(string_command::GETSET, command::GETSET);
        dispatcher->Register(string_command::SET, command::SET);
        dispatcher->Register(string_command::SETNX, command::SETNX);
        dispatcher->Register(string_command::APPEND, command::APPEND);
        dispatcher->Register(string_command::INCRBY, command::INCRBY);
        dispatcher->Register(string_command::DECRBY, command::DECRBY);

        dispatcher->Register(string_command::PSETEX, command::PSETEX);
        dispatcher->Register(string_command::SETEX, command::SETEX);
        dispatcher->Register(string_command::GETRANGE, command::GETRANGE);

        dispatcher->Register(universal_command::SELECT, command::SELECT);
        dispatcher->Register(universal_command::SAVE, command::SAVE);
        dispatcher->Register(universal_command::BG_SAVE, command::BG_SAVE);
        dispatcher->Register(universal_command::KEYS, command::KEYS);

    }
}
