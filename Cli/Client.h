//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include "../Protocol/bproto.h"
#include "../Utils/linenoise.h"
constexpr unsigned int str2int(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

class Client
{
public:
    Client() : socket(io_context)
    {
    }

    void Run()
    {
        socket.open(boost::asio::ip::tcp::v4());
        boost::asio::ip::tcp::acceptor::reuse_address reuse_address(true);
        socket.set_option(reuse_address);
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
            char *line;
//            while((line = linenoise("cxxredis> ")) != NULL) {
            while(1) {
                std::vector<char> input(4096);
                std::cin.getline(&input.at(0), 4096);
                line = &input[0];
                if (line[0] != '\0' && line[0] != '/') {
                    linenoiseHistoryAdd(line);
                    auto bytes_to_send = serialize(input, buff);
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
                    bytes_read = boost::asio::async_read(this->socket, boost::asio::buffer(buff, header->payload_len), yield[ec]);
                    if (ec)
                    {
                        return;
                    }
                    std::cout << deserialize(buff, payload_len, payload_type) << std::endl;
                    fflush(stdout);
                } else if (line[0] == '/') {
                    printf("Unreconized command: %s\n", line);
                }
//                free(line);
            }

        });
        this->io_context.run();
    }

private:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

    int64_t serialize(std::vector<char> &cmd, std::vector<char> &buff)
    {
        auto command = std::string(&cmd[0]);
        std::vector<std::string> parts;
        boost::split(parts, command, boost::is_any_of(" "));
        if (parts.size() == 0)
            return -1;
        parts[0] = boost::to_upper_copy<std::string>(parts[0]);
        switch (str2int(parts[0].c_str()))
        {
            case str2int("SET"):
            {
                if (parts.size() != 3)
                    return -1;
                CMD_SET_REQ cs;
                cs.set_key(parts[1]);
                cs.set_value(parts[2]);
                auto serial_size = cs.ByteSizeLong();
                if (serial_size > buff.capacity())
                    return -1;
                cs.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size);
                auto header = (BProtoHeader *)&buff[0];
                header->payload_len = serial_size;
                header->payload_cmd = Command::SET;
                return BProtoHeaderSize + serial_size;
            }
            case str2int("GET"):
            {
                if (parts.size() != 2)
                    return false;
                CMD_GET_REQ req;
                req.set_key(parts[1]);
                auto serial_size = req.ByteSizeLong();
                if (serial_size > buff.capacity())
                    return false;
                req.SerializeToArray(BProtoHeaderOffset(&buff[0]), serial_size);
                auto header = (BProtoHeader *)&buff[0];
                header->payload_len = serial_size;
                header->payload_cmd = Command::GET;
                return BProtoHeaderSize + serial_size;
            }
        default:
        {
            return -1;
        }
        };
    }

    std::string deserialize(std::vector<char> &buff, uint32_t size, Command cmd_type)
    {
        switch (cmd_type)
        {
            case Command::GET_OK:
            {
                CMD_GET_REPLY reply;
                reply.ParseFromArray(&buff[0], size);
                return reply.value();
            }
        }
        return "";
    }
};
