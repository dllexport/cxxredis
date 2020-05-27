//
// Created by Mario on 2020/5/27.
//

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "../Protocol/bproto.h"
#include "HandleAlloc.h"
#include "../Utils/SmartPtr.h"

class Session : public SmartPtr<Session>
{
public:
    Session(boost::asio::io_context &io) : buff(1024), peer(io) {}

    ~Session() {}

    void WaitProcess();

    void readHeader();

    void readPayload(uint32_t size, Command cmd_type);

    void replyOK(...);

    void replyIntOK(int code);

    void replyStringOK(const std::string &str);

    void replyRepeatedStringOK(std::vector<std::string> &&strs);

    void replyErr(int code);

    std::vector<uint8_t> buff;
    boost::asio::ip::tcp::socket peer;
    handler_memory handler_memory_;

};


