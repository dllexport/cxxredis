//
// Created by Mario on 2020/5/31.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include "../Utils/Singleton.h"
using CommandSerilizer = std::function<int64_t(std::vector<std::string>& command_parts, std::vector<char> &buff)>;
constexpr unsigned int str2int(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}
class CommandMap : public Singleton<CommandMap> {
public:
    void Register(int command_code, CommandSerilizer handler) {
        if (this->serialization_map.find(command_code) != this->serialization_map.end()) {
            throw std::runtime_error("[CommandMap::Register] command_code collision");
        }
        this->serialization_map.insert({command_code, handler});
    }

    int64_t Dispatch(std::vector<std::string>& command_parts, std::vector<char> &buff) {
        auto key = str2int(command_parts[0].c_str());
        return this->serialization_map[key](command_parts, buff);
    }
private:
    std::unordered_map<int, CommandSerilizer> serialization_map;
};


