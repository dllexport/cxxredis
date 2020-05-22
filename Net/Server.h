//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include "../Protocol/bproto.h"
#include "../String.h"
#include "HandleAlloc.h"

#define Q(x) #x
#define QUOTE(x) Q(x)
#define GENSTRCASE_COMMAND(key) Command::key
#define GenStringCase3(key_name, reply_type) \
    case GENSTRCASE_COMMAND(key_name): \
    { \
        CMD_##key_name##_REQ req; \
        req.ParseFromArray(&buff[0], bytes_transferred); \
        auto pair = String::key_name(0, std::string(req.key()), std::move(*req.mutable_value())); \
        if (pair.first == (int)Command::OK) \
            reply_type(pair.second); \
        else \
            replyErr((int)pair.first); \
        break; \
    }

#define GenStringCase2(key_name, reply_type) \
    case GENSTRCASE_COMMAND(key_name): \
    { \
        CMD_##key_name##_REQ req; \
        req.ParseFromArray(&buff[0], bytes_transferred); \
        auto pair = String::key_name(0, std::string(req.key())); \
        if (pair.first == (int)Command::OK) \
            reply_type(pair.second); \
        else \
            replyErr((int)pair.first); \
        break; \
    }
class Session
{
public:
    Session(boost::asio::io_context &io) : buff(1024), peer(io) {}

    void WaitProcess()
    {
        readHeader();
    }

    void readHeader()
    {
        boost::asio::async_read(this->peer,
                                boost::asio::buffer(&buff[0], BProtoHeaderSize),
                                make_custom_alloc_handler(
                                    this->handler_memory_,
                                    [this](const boost::system::error_code &ec,
                                           std::size_t bytes_transferred) {
                                        if (ec)
                                        {
                                            std::cout << ec.message();
                                            fflush(stdout);
                                            return;
                                        }
                                        auto header = (BProtoHeader *)&buff[0];
                                        auto buffer_size = buff.size();
                                        if (header->payload_len > buffer_size) {
                                            return;
                                        }
                                        this->readPayload(header->payload_len, header->payload_cmd);
                                    }));
    }

    void readPayload(uint32_t size, Command cmd_type)
    {
        boost::asio::async_read(this->peer,
                                boost::asio::buffer(&buff[0], size),
                                make_custom_alloc_handler(
                                    this->handler_memory_,
                                    [this, cmd_type](const boost::system::error_code &ec,
                                                     std::size_t bytes_transferred) {
                                        if (ec)
                                        {
                                            std::cout << ec.message();
                                            fflush(stdout);
                                            return;
                                        }
                                        switch (cmd_type)
                                        {
                                            case Command::SET:
                                            {
                                                CMD_SET_REQ req;
                                                req.ParseFromArray(&buff[0], bytes_transferred);
                                                String::SET(0, std::string(req.key()), std::move(*req.mutable_value()));
                                                replyOK();
                                                break;
                                            }
                                            case Command::SETEX:
                                            {
                                                CMD_SETEX_REQ req;
                                                req.ParseFromArray(&buff[0], bytes_transferred);
                                                auto pair = String::SETEX(0, std::string(req.key()), std::move(*req.mutable_value()), std::move(*req.mutable_expire_time()));
                                                if (pair.first == (uint32_t)Command::OK)
                                                    replyIntOK(pair.second);
                                                else
                                                    replyErr((uint32_t)pair.first);
                                                break;
                                            }
                                            case Command::GET:
                                        {
                                            CMD_GET_REQ req;
                                            req.ParseFromArray(&buff[0], bytes_transferred);
                                            auto &str = String::GET(0, std::string(req.key()));
                                            replyStringOK(str);
                                            break;
                                        }
                                            GenStringCase3(SETNX, replyIntOK)
                                            GenStringCase3(GETSET, replyStringOK)
                                            GenStringCase2(INCR,replyStringOK)
                                            GenStringCase2(DECR,replyStringOK)
                                            GenStringCase2(STRLEN,replyIntOK)
                                            GenStringCase3(APPEND,replyStringOK)
                                            GenStringCase3(INCRBY,replyStringOK)
                                            GenStringCase3(DECRBY,replyStringOK)

                                            default:
                                            {
                                            }
                                        };
                                        readHeader();
                                    }));
    }

    void replyOK();

    void replyIntOK(int code);

    void replyStringOK(const std::string &str);

    void replyErr(int code);

    std::vector<uint8_t> buff;
    boost::asio::ip::tcp::socket peer;
    // The memory to use for handler-based custom memory allocation.
    handler_memory handler_memory_;
};

class Server
{
public:
    Server() : io_context(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE), acceptor(io_context) {}

    void Run()
    {
        auto bind_ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 6666);
        boost::system::error_code ec;
        acceptor.open(bind_ep.protocol(), ec);
        boost::asio::ip::tcp::acceptor::reuse_address reuse_address(true);
        acceptor.set_option(reuse_address);
        acceptor.bind(bind_ep, ec);
        if (ec)
        {
            std::cout << ec.message() << std::endl;
            fflush(stdout);
            return;
        }
        acceptor.listen();
        if (ec)
        {
            std::cout << ec.message() << std::endl;
            fflush(stdout);
            return;
        }
        this->runAccept();
        this->io_context.run();
    }

    void runAccept()
    {
        auto session = new Session(this->io_context);
        acceptor.async_accept(
            session->peer,
            [session, this](const boost::system::error_code &ec) {
                boost::asio::ip::tcp::no_delay option(true);
                session->peer.set_option(option);
                session->WaitProcess();
                this->runAccept();
            });
    }

private:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor;
};
