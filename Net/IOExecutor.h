//
// Created by Mario on 2020/5/31.
//

#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <boost/thread.hpp>
#include "../Utils/Singleton.h"
#include "../Utils/GetConcurrency.h"

/*
 * simple wrapper for asio executor
 */
class IOExecutor : public Singleton<IOExecutor> {

    using IOContext = boost::asio::io_context;
    using Worker = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

public:

    IOExecutor() {
        for (int i = 0; i < GetConcurrency(); ++i) {
            // disable all mutex in io_context
            // and each io_context has only one corresponding thread
            this->io_contexts.emplace_back(std::make_unique<IOContext>(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE));
        }
        // create worker for each io_context
        for (int i = 0; i < this->io_contexts.size(); ++i) {
            this->workers.emplace_back(boost::asio::make_work_guard(*io_contexts[i]));
        }
    }

    void Run()
    {
        for (int i = 0; i < io_contexts.size(); ++i) {
            this->thread_group.create_thread([this, i](){
                this->io_contexts[i]->run();
            });
        }
        this->thread_group.join_all();
    }

    std::vector<Worker>& GetWorkers() {
        return this->workers;
    }

    IOContext& GetContextAt(int i) {
        return *this->io_contexts[i];
    }

    int GetContextCount() {
        return this->io_contexts.size();
    }

private:
    boost::thread_group thread_group;
    std::vector<std::unique_ptr<IOContext>> io_contexts;
    std::vector<Worker> workers;
};
