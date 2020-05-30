//
// Created by Mario on 2020/5/17.
//

#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include "Session.h"
#include "../Utils/GetConcurrency.h"
#include "../Utils/Singleton.h"
#include "IOTransfer.h"

class Server : public Singleton<Server>
{
public:
    Server() : io_contexts(GetConcurrency()) {

        // create worker for each io_context
        for (int i = 0; i < io_contexts.size(); ++i) {
            this->workers.emplace_back(boost::asio::make_work_guard(io_contexts[i]));
        }

        auto bind_ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 6666);
        boost::system::error_code ec;

        for (int i = 0; i < io_contexts.size(); ++i) {

            int opt = 1;

            acceptors.emplace_back(Acceptor(io_contexts[i]));
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
        }

        IOTransfer::GetInstance()->Init(this->io_contexts);
    }

    void Run()
    {
        for (int i = 0; i < io_contexts.size(); ++i) {
            this->thread_group.create_thread([this, i](){
                runAccept(i);
                this->io_contexts[i].run();
            });
        }
        this->thread_group.join_all();
    }

private:
    using IOContext = boost::asio::io_context;
    using Worker = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using Acceptor = boost::asio::ip::tcp::acceptor;

    boost::thread_group thread_group;
    std::vector<IOContext> io_contexts;
    std::vector<Worker> workers;
    std::vector<Acceptor> acceptors;

    void runAccept(int i);
};
