//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <iostream>
#include "../Protocol/bproto.h"
#include "../Utils/linenoise.h"
#include <regex>
#define Q(x) #x
#define QUOTE(x) Q(x)
#define GENSTRCASE_COMMAND(key) Command::key
#define GenStringCase2(key) \
    case str2int(QUOTE(key)): \
    { \
        if (parts.size() != 2) \
        return false; \
        CMD_##key##_REQ req; \
        req.set_key(parts[1]); \
        auto serial_size = req.ByteSizeLong(); \
        if (serial_size > buff.capacity()) \
        return false; \
        req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
        auto header = (BProtoHeader *)&buff[0]; \
        header->payload_len = serial_size; \
        header->payload_cmd = GENSTRCASE_COMMAND(key); \
        return BProtoHeaderSize + serial_size; \
    } \

#define GenStringCase3(key) \
    case str2int(QUOTE(key)): \
    { \
        if (parts.size() != 3) \
        return -1; \
        CMD_##key##_REQ cs; \
        cs.set_key(parts[1]); \
        cs.set_value(parts[2]); \
        auto serial_size = cs.ByteSizeLong(); \
        if (serial_size > buff.capacity()) \
        return -1; \
        cs.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
        auto header = (BProtoHeader *)&buff[0]; \
        header->payload_len = serial_size; \
        header->payload_cmd = GENSTRCASE_COMMAND(key); \
        return BProtoHeaderSize + serial_size; \
    }
#define GenStringCase4(key) \
    case str2int(QUOTE(key)): \
    { \
        if (parts.size() != 4) \
        return -1; \
        CMD_##key##_REQ cs; \
        cs.set_key(parts[1]); \
        cs.set_value(parts[2]); \
        cs.set_value2(parts[3]); \
        auto serial_size = cs.ByteSizeLong(); \
        if (serial_size > buff.capacity()) \
        return -1; \
        cs.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
        auto header = (BProtoHeader *)&buff[0]; \
        header->payload_len = serial_size; \
        header->payload_cmd = GENSTRCASE_COMMAND(key); \
        return BProtoHeaderSize + serial_size; \
    }
constexpr unsigned int str2int(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

static char *hints(const char *buf, int *color, int *bold) {
    std::regex kv_reg("(SET|SETNX|INCRBY|DECRBY|APPEND)\\s*", std::regex::icase);
    std::regex kvv_reg("(PSETEX|SETEX)\\s*", std::regex::icase);
//    std::regex vv_reg("(PSETEX|SETEX)\\s*\\w+\\s*", std::regex::icase);
    std::regex v_reg("(SET|SETNX|INCRBY|DECRBY|APPEND)\\s*\\w+\\s*", std::regex::icase);
    std::regex k_reg("(GET|INCR|DECR|STRLEN)\\s*", std::regex::icase);
    auto str = std::string(buf);
    if (std::regex_match(str, kvv_reg)){
        *color = 35;
        *bold = 0;
        return " [key] [value] [expire_time]";
    }
    if (std::regex_match(str, kv_reg)){
        *color = 35;
        *bold = 0;
        return " [key] [value]";
    }
    if (std::regex_match(str, v_reg)){
        *color = 35;
        *bold = 0;
        return " [value]";
    }
    if (std::regex_match(str, k_reg)){
        *color = 35;
        *bold = 0;
        return " [key]";
    }
    return nullptr;
}

class Client
{
public:
    Client() : io_context(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE), socket(io_context) {}

    void Run()
    {
        socket.open(boost::asio::ip::tcp::v4());
        boost::asio::ip::tcp::acceptor::reuse_address reuse_address(true);
        socket.set_option(reuse_address);
        boost::asio::ip::tcp::no_delay option(true);
        socket.set_option(option);
        boost::asio::spawn(this->io_context, [this](boost::asio::yield_context yield) {
            boost::system::error_code ec;
            auto connect_ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 6666);
            this->socket.async_connect(connect_ep, yield[ec]);
            if (ec)
            {
                return;
            }
            constexpr const size_t max_buff_size = 4096;
            std::vector<char> buff(max_buff_size);

            linenoiseSetHintsCallback(hints);

            char *line;
//            std::vector<char> input(4096);
            auto t1 = std::chrono::high_resolution_clock::now();



            while((line = linenoise("cxxredis> ")) != NULL) {
//            while(1) {
//                std::cin.getline(&input.at(0), 4096);
//                line = &input[0];
                linenoiseHistoryAdd(line); /* Add to the history. */
                auto line_str = std::string(line);
                boost::trim_all(line_str);
                if (line[0] != '\0' && line[0] != '/') {
                    auto bytes_to_send = serialize(line_str, buff);
                    if (bytes_to_send <= 0) {
                        continue;
                    }
                    // ready to send
                    auto bytes_send = boost::asio::async_write(this->socket, boost::asio::buffer(&buff[0], bytes_to_send), yield[ec]);
                    if (ec)
                    {
                        return;
                    }
                    auto bytes_read = boost::asio::async_read(this->socket, boost::asio::buffer(buff, BProtoHeaderSize), yield[ec]);
                    if (ec)
                    {
                        return;
                    }
                    auto header = (BProtoHeader *)&buff[0];
                    if (header->payload_len > max_buff_size)
                    {
                        return;
                    }
                    auto payload_len = header->payload_len;
                    auto payload_type = header->payload_cmd;
                    if (payload_type == Command::OK && payload_len == 0) {
                        std::cout << "OK\n";
                        continue;
                    }
                    bytes_read = boost::asio::async_read(this->socket, boost::asio::buffer(&buff[0], header->payload_len), yield[ec]);
                    if (ec)
                    {
                        return;
                    }
                    auto rs = deserialize(buff, payload_len, payload_type);
                    for (auto &item : rs) {
                        std::cout << item << std::endl;
                        fflush(stdout);
                    }

                } else if (line[0] == '/') {
                    printf("Unreconized command: %s\n", line);
                }
                free(line);
            }

            auto t2 = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

            std::cout << duration << "\n";
            fflush(stdout);
        });
        this->io_context.run();
    }

private:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

    int64_t serialize(std::string &command, std::vector<char> &buff)
    {
        std::vector<std::string> parts;
        boost::split(parts, command, boost::is_any_of(" "));
        if (parts.size() == 0)
            return -1;
        parts[0] = boost::to_upper_copy<std::string>(parts[0]);
        switch (str2int(parts[0].c_str()))
        {
            case str2int("KEYS"): {
                if (parts.size() != 1) return false;
                auto header = (BProtoHeader *) &buff[0];
                header->payload_len = 0;
                header->payload_cmd = Command::KEYS;
                return BProtoHeader::Size();
                break;
            }
            GenStringCase2(GET)
            GenStringCase2(STRLEN)
            GenStringCase3(SET)
            GenStringCase3(GETSET)
            GenStringCase3(APPEND)
            GenStringCase2(INCR)
            GenStringCase2(DECR)
            GenStringCase3(DECRBY)
            GenStringCase3(INCRBY)
            GenStringCase3(SETNX)
            GenStringCase3(PSETEX)
            GenStringCase4(SETEX)
            GenStringCase4(GETRANGE)
        default:
        {
            return -1;
        }
        };
    }

    std::vector<std::string> deserialize(std::vector<char> &buff, uint32_t size, Command cmd_type)
    {
        switch (cmd_type)
        {
            case Command::PARAM_ERR:{
                return {"Param error"};
            }
            case Command::EXIST_ERR:{
                return {"Key already exist"};
            }
            case Command::NOTEXIST_ERR:{
                return {"Key not exist"};
            }
            case Command::NOTMATCH_ERR:{
                return {"Key type not match"};
            }
            case Command::STRING_OK:
            {
                STRING_OK_REPLY reply;
                reply.ParseFromArray(&buff[0], size);
                return {reply.value()};
            }
            case Command::INT_OK:
            {
                INT_OK_REPLY reply;
                reply.ParseFromArray(&buff[0], size);
                return {std::to_string(reply.value())};
            }
            case Command::REPEATED_STRING_OK:
            {
                REPEATED_STRING_OK_REPLY reply;
                reply.ParseFromArray(&buff[0], size);
                return std::vector<std::string>(reply.value().begin(), reply.value().end());
            }
        }
        return {};
    }
};
