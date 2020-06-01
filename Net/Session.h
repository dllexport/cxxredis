//
// Created by Mario on 2020/5/27.
//

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "../Protocol/bproto.h"
#include "../Utils/SmartPtr.h"
#include "HandleAlloc.h"

class Session : public SmartPtr<Session>
{
public:
    Session(boost::asio::io_context &io) : buff(1024), peer(io) { }

    ~Session() {
        std::cout << "session die " << &this->peer.get_executor().context() << std::endl;
        fflush(stdout);
    }

    void WaitProcess();

    void readHeader();

    void readPayload(uint32_t size, int cmd_type);

    void replyOK(...);

    void replyIntOK(int code);

    void replySelectOK(int db_index);

    void replyStringOK(const std::string &str);

    void replyRepeatedStringOK(std::vector<std::string> &&strs);

    void replyErr(int code);

    friend class IOTransfer;

    std::vector<uint8_t> buff;
    boost::asio::ip::tcp::socket peer;
    handler_memory handler_memory_;
    uint32_t db_index = 0;
};


