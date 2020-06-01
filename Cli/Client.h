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
#include <boost/lexical_cast.hpp>
#include "CommandMap.h"
#include "CommandRegistry.h"

#include <regex>
#define Q(x) #x
#define QUOTE(x) Q(x)
#define GENSTRCASE_COMMAND(key) string_command::key
#define GenStringCase2(key) \
    case str2int(QUOTE(key)): \
    { \
        if (parts.size() != 2) \
        return false; \
        universal_command::REQ1 req; \
        req.set_value(parts[1]); \
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
        universal_command::REQ2 req; \
        req.set_value1(parts[1]); \
        req.set_value2(parts[2]); \
        auto serial_size = req.ByteSizeLong(); \
        if (serial_size > buff.capacity()) \
        return -1; \
        req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
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
        universal_command::REQ3 req; \
        req.set_value1(parts[1]); \
        req.set_value2(parts[2]); \
        req.set_value3(parts[3]); \
        auto serial_size = req.ByteSizeLong(); \
        if (serial_size > buff.capacity()) \
        return -1; \
        req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size); \
        auto header = (BProtoHeader *)&buff[0]; \
        header->payload_len = serial_size; \
        header->payload_cmd = GENSTRCASE_COMMAND(key); \
        return BProtoHeaderSize + serial_size; \
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
    Client() : io_context(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE), socket(io_context) {
        client_command::RegisterAll();
    }

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

            auto remote_ep = this->socket.remote_endpoint();
            while(1) {
                auto prompt = remote_ep.address().to_string() + ":" + std::to_string(remote_ep.port()) + "[" + std::to_string(db_index) + "]> ";
                line = linenoise(prompt.c_str());
                if (!line)
                    return;
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
                    if (payload_type == universal_command::OK && payload_len == 0) {
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
    uint32_t db_index = 0;

    int64_t serialize(std::string &command, std::vector<char> &buff)
    {
        std::vector<std::string> parts;
        boost::split(parts, command, boost::is_any_of(" "));
        if (parts.size() == 0)
            return -1;
        parts[0] = boost::to_upper_copy<std::string>(parts[0]);
        return CommandMap::GetInstance()->Dispatch(parts, buff);
        switch (str2int(parts[0].c_str()))
        {
            case str2int("KEYS"): {
                if (parts.size() != 1) return false;
                auto header = (BProtoHeader *) &buff[0];
                header->payload_len = 0;
                header->payload_cmd = universal_command::KEYS;
                return BProtoHeader::Size();
                break;
            }
            case str2int("SAVE"): {
                if (parts.size() != 1) return false;
                auto header = (BProtoHeader *) &buff[0];
                header->payload_len = 0;
                header->payload_cmd = universal_command::SAVE;
                return BProtoHeader::Size();
                break;
            }
            case str2int("BGSAVE"): {
                if (parts.size() != 1) return false;
                auto header = (BProtoHeader *) &buff[0];
                header->payload_len = 0;
                header->payload_cmd = universal_command::BG_SAVE;
                return BProtoHeader::Size();
                break;
            }
            case str2int("SELECT"): {
                if (parts.size() != 2)return false;
                universal_command::REQ1 req;
                req.set_value(parts[1]);
                auto serial_size = req.ByteSizeLong();
                if (serial_size > buff.capacity())return false;
                req.SerializeToArray(&buff[0] + BProtoHeader::Size(), serial_size);
                auto header = (BProtoHeader *) &buff[0];
                header->payload_len = serial_size;
                header->payload_cmd = universal_command::SELECT;
                return BProtoHeader::Size() + serial_size;
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
            GenStringCase4(PSETEX)
            GenStringCase4(SETEX)
            GenStringCase4(GETRANGE)
        default:
        {
            return -1;
        }
        };
    }

    std::vector<std::string> deserialize(std::vector<char> &buff, uint32_t size, int cmd_type)
    {
        switch (cmd_type)
        {
            case universal_command::PARAM_ERR:{
                return {"Param error"};
            }
            case universal_command::EXIST_ERR:{
                return {"Key already exist"};
            }
            case universal_command::NOT_EXIST_ERR:{
                return {"Key not exist"};
            }
            case universal_command::NOT_MATCH_ERR:{
                return {"Key type not match"};
            }
            case universal_command::STRING_OK:
            {
                universal_command::REPLY1 reply;
                reply.ParseFromArray(&buff[0], size);
                return {reply.value()};
            }
            case universal_command::INT_OK:
            {
                universal_command::INT_REPLY1 reply;
                reply.ParseFromArray(&buff[0], size);
                return {std::to_string(reply.value())};
            }
            case universal_command::SELECT_OK:
            {
                universal_command::INT_REPLY1 reply;
                reply.ParseFromArray(&buff[0], size);
                this->db_index = reply.value();
                return {"OK"};
            }
            case universal_command::REPEATED_STRING_OK:
            {
                universal_command::REPLY_ANY reply;
                reply.ParseFromArray(&buff[0], size);
                return std::vector<std::string>(reply.value().begin(), reply.value().end());
            }
        }
        return {};
    }
};
