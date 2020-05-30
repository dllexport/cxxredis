//
// Created by Mario on 2020/5/31.
//

#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <boost/thread.hpp>
#include "../Utils/Singleton.h"
#include "../Utils/GetConcurrency.h"

class IOExecutor : public Singleton<IOExecutor> {

    using IOContext = boost::asio::io_context;
    using Worker = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

public:

    IOExecutor() : io_contexts(GetConcurrency()) {
        // create worker for each io_context
        for (int i = 0; i < this->io_contexts.size(); ++i) {
            this->workers.emplace_back(boost::asio::make_work_guard(io_contexts[i]));
        }
    }

    void Run()
    {
        for (int i = 0; i < io_contexts.size(); ++i) {
            this->thread_group.create_thread([this, i](){
                this->io_contexts[i].run();
            });
        }
        this->thread_group.join_all();
    }

    std::vector<IOContext>& GetContexts() {
        return this->io_contexts;
    }

private:


    boost::thread_group thread_group;
    std::vector<IOContext> io_contexts;
    std::vector<Worker> workers;
};
