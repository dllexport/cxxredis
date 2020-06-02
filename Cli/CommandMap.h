//
// Created by Mario on 2020/5/31.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>
#include "../Utils/Singleton.h"
using CommandSerilizer = std::function<int64_t(std::vector<std::string> command_parts, std::vector<char> &buff)>;
constexpr unsigned int str2int(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}
class CommandMap : public Singleton<CommandMap> {
public:
    CommandMap() {
        this->serialization_map.reserve(65535);
    }
    void Register(std::string&& command_code_str, CommandSerilizer handler) {
        if (this->serialization_map.find(command_code_str) != this->serialization_map.end()) {
            throw std::runtime_error("[CommandMap::Register] command_code collision");
        }
        this->serialization_map.insert({command_code_str, handler});
    }

    int64_t Dispatch(std::vector<std::string>& command_parts, std::vector<char> &buff) {
        if (this->serialization_map.find(command_parts[0]) == this->serialization_map.end()) {
            std::cout << "unknown command\n";
            return -1;
        }
        return this->serialization_map[command_parts[0]](command_parts, buff);
    }
private:
    std::unordered_map<std::string, CommandSerilizer> serialization_map;
};


