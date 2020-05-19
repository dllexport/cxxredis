//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
class Session {
public:
    Session(boost::asio::io_context& io) : peer(io) {}
    boost::asio::ip::tcp::socket peer;
};
class Server {
public:
    Server() : acceptor(io_context) {

    }

    void Run() {
        auto bind_ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 6666);
        acceptor.open(bind_ep.protocol());
        acceptor.bind(bind_ep);
        this->runAccept();
    }

    void runAccept() {
        auto session = std::make_unique<Session>(this->io_context);
        acceptor.async_accept(
                session->peer,
                [session = std::move(session), this](const boost::system::error_code& ec) {
                    session->peer.close();
                    runAccept();
                });
    }

    void handleOnAccept(boost::system::error_code ec) {

    }

private:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor;
};


