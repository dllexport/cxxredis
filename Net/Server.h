//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include "Session.h"
#include "../Utils/Singleton.h"
#include "IOTransfer.h"
#include "IOExecutor.h"

class Server : public Singleton<Server>
{
public:
    Server() {}

    void Init() {

        auto io_executor = IOExecutor::GetInstance();

        auto bind_ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 6666);
        boost::system::error_code ec;

        for (int i = 0; i < io_executor->GetContextCount(); ++i) {

            int opt = 1;

            acceptors.emplace_back(Acceptor(io_executor->GetContextAt(i)));
            acceptors.back().open(bind_ep.protocol(), ec);
            if (ec)
            {
                return;
            }

            setsockopt(acceptors.back().native_handle(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

            acceptors.back().bind(bind_ep, ec);
            if (ec)
            {
                return;
            }

            acceptors.back().listen(SOMAXCONN, ec);

            runAccept(i);
        }

        IOTransfer::GetInstance()->Init();
    }

private:
    using Acceptor = boost::asio::ip::tcp::acceptor;

    std::vector<Acceptor> acceptors;

    void runAccept(int which);
};
