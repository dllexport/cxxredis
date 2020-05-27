//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include "Session.h"

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
        auto session = boost::intrusive_ptr<Session>(new Session(this->io_context));
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
