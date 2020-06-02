//
// Created by Mario on 2020/5/31.
//

#include "CommandRegistry.h"

#define GenClientHandler1(HandlerName, command_type) \
CommandSerilizer HandlerName = [](std::vector<std::string> parts, std::vector<char> &buff) { \
    universal_command::REQ1 req; \
    req.set_value(parts[1]); \
    auto serial_size = req.ByteSizeLong(); \
    if (serial_size > buff.capacity()) \
        return size_t(-1); \
    req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
    auto header = (BProtoHeader *) &buff[0]; \
    header->payload_len = serial_size; \
    header->payload_cmd = command_type::HandlerName; \
    return size_t(BProtoHeaderSize + serial_size); \
};

#define GenClientHandler2(HandlerName, command_type) \
    CommandSerilizer HandlerName = [](std::vector<std::string> parts, std::vector<char> &buff) { \
        universal_command::REQ2 req; \
        req.set_value1(parts[1]); \
        req.set_value2(parts[2]); \
        auto serial_size = req.ByteSizeLong(); \
        if (serial_size > buff.capacity()) \
            return size_t(-1); \
        req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
        auto header = (BProtoHeader *) &buff[0]; \
        header->payload_len = serial_size; \
        header->payload_cmd = command_type::HandlerName; \
            return size_t(BProtoHeaderSize + serial_size); \
    };\

#define GenClientHandler3(HandlerName, command_type) \
    CommandSerilizer HandlerName = [](std::vector<std::string> parts, std::vector<char> &buff) { \
        universal_command::REQ3 req; \
        req.set_value1(parts[1]); \
        req.set_value2(parts[2]); \
        req.set_value3(parts[3]); \
        auto serial_size = req.ByteSizeLong(); \
        if (serial_size > buff.capacity()) \
            return size_t(-1); \
        req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
        auto header = (BProtoHeader *) &buff[0]; \
        header->payload_len = serial_size; \
        header->payload_cmd = command_type::HandlerName; \
            return size_t(BProtoHeaderSize + serial_size); \
    };

#define Q(x) #x
#define QUOTE(x) Q(x)

#define RegisterOne(command_name) \
        command_map->Register(Q(command_name), command_name);


namespace client_command {
    void RegisterAll() {
        auto command_map = CommandMap::GetInstance();

        GenClientHandler1(SELECT, universal_command)
        GenClientHandler1(KEYS, universal_command)

        GenClientHandler1(GET, string_command)
        GenClientHandler1(INCR, string_command)
        GenClientHandler1(DECR, string_command)
        GenClientHandler1(STRLEN, string_command)

        GenClientHandler2(SET, string_command)
        GenClientHandler2(GETSET, string_command)
        GenClientHandler2(INCRBY, string_command)
        GenClientHandler2(DECRBY, string_command)
        GenClientHandler2(APPEND, string_command)

        GenClientHandler3(SETEX, string_command)
        GenClientHandler3(SETNX, string_command)
        GenClientHandler3(PSETEX, string_command)
        GenClientHandler3(GETRANGE, string_command)

        RegisterOne(SELECT)
        RegisterOne(KEYS)

        RegisterOne(SET)
        RegisterOne(GET)
        RegisterOne(INCR)
        RegisterOne(DECR)
        RegisterOne(STRLEN)

        RegisterOne(GETSET)
        RegisterOne(INCRBY)
        RegisterOne(DECRBY)
        RegisterOne(APPEND)

        RegisterOne(SETEX)
        RegisterOne(SETNX)
        RegisterOne(PSETEX)
        RegisterOne(GETRANGE)

    }
}