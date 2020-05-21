//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include "../Protocol/bproto.h"
#include "../String.h"
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
                                    this->readPayload(header->payload_len, header->payload_cmd);
                                });
    }

    void readPayload(uint32_t size, Command cmd_type)
    {
        boost::asio::async_read(this->peer,
                                boost::asio::buffer(&buff[0], size),
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
                                        String::Set(0, std::string(req.key()), std::move(*req.mutable_value()));
                                        replyOK();
                                        break;
                                    }
                                    case Command::GET:
                                    {
                                        CMD_GET_REQ req;
                                        req.ParseFromArray(&buff[0], bytes_transferred);
                                        auto &str = String::Get(0, std::string(req.key()));
                                        replyGet(str);
                                        break;
                                    }
                                    default:
                                    {
                                    }
                                    };
                                    readHeader();
                                });
    }

    void replyOK()
    {
        auto header = (BProtoHeader *)&buff[0];
        header->payload_len = 0;
        header->payload_cmd = Command::OK;
        boost::asio::async_write(this->peer,
                                 boost::asio::buffer(&buff[0], BProtoHeaderSize),
                                 [this](const boost::system::error_code &ec,
                                        std::size_t bytes_transferred) {
                                     if (ec)
                                     {
                                         return;
                                     }
                                 });
    }

    void replyGet(const std::string &str)
    {
        auto header = (BProtoHeader *)&buff[0];
        CMD_GET_REPLY reply;
        reply.set_value(str);
        header->payload_len = reply.ByteSizeLong();
        header->payload_cmd = Command::GET_OK;
        reply.SerializeToArray(BProtoHeaderOffset(&buff[0]), header->payload_len);
        boost::asio::async_write(this->peer,
                                 boost::asio::buffer(&buff[0], BProtoHeaderSize + header->payload_len),
                                 [this](const boost::system::error_code &ec,
                                        std::size_t bytes_transferred) {
                                     if (ec)
                                     {
                                         return;
                                     }
                                 });
    }

    std::vector<uint8_t> buff;
    boost::asio::ip::tcp::socket peer;
};
class Server
{
public:
    Server() : acceptor(io_context)
    {
    }

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
                session->WaitProcess();
                this->runAccept();
            });
    }

    void handleOnAccept(boost::system::error_code ec)
    {
    }

private:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor;
};
