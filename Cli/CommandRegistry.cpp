//
// Created by Mario on 2020/5/31.
//

#include "CommandRegistry.h"
namespace client_command {
    void RegisterAll() {
        auto command_map = CommandMap::GetInstance();

        CommandSerilizer SET = [](std::vector<std::string> &parts, std::vector<char> &buff) {
            universal_command::REQ2 req;
            req.set_value1(parts[1]);
            req.set_value2(parts[2]);
            auto serial_size = req.ByteSizeLong();
            if (serial_size > buff.capacity())
                return size_t(-1);
            req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size);
            auto header = (BProtoHeader *) &buff[0];
            header->payload_len = serial_size;
            header->payload_cmd = string_command::SET;
            return size_t(BProtoHeaderSize + serial_size);
        };

        CommandSerilizer SELECT = [](std::vector<std::string> &parts, std::vector<char> &buff) {
            universal_command::REQ1 req;
            req.set_value(parts[1]);
            auto serial_size = req.ByteSizeLong();
            if (serial_size > buff.capacity())
                return size_t(-1);
            req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size);
            auto header = (BProtoHeader *) &buff[0];
            header->payload_len = serial_size;
            header->payload_cmd = universal_command::SELECT;
            return size_t(BProtoHeaderSize + serial_size);
        };

        CommandSerilizer GET = [](std::vector<std::string> &parts, std::vector<char> &buff) {
            universal_command::REQ1 req;
            req.set_value(parts[1]);
            auto serial_size = req.ByteSizeLong();
            if (serial_size > buff.capacity())
                return size_t(-1);
            req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size);
            auto header = (BProtoHeader *) &buff[0];
            header->payload_len = serial_size;
            header->payload_cmd = string_command::GET;
            return size_t(BProtoHeaderSize + serial_size);
        };

        CommandSerilizer KEYS = [](std::vector<std::string> &parts, std::vector<char> &buff) {
            universal_command::REQ1 req;
            req.set_value("KEYS");
            auto serial_size = req.ByteSizeLong();
            if (serial_size > buff.capacity())
                return size_t(-1);
            req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size);
            auto header = (BProtoHeader *) &buff[0];
            header->payload_len = serial_size;
            header->payload_cmd = universal_command::KEYS;
            return size_t(BProtoHeaderSize + serial_size);
        };

        command_map->Register(str2int("GET"), GET);
        command_map->Register(str2int("SET"), SET);
        command_map->Register(str2int("SELECT"), SELECT);
        command_map->Register(str2int("KEYS"), KEYS);
    }
}