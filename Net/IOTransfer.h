//
// Created by Mario on 2020/5/30.
//

#pragma once

#include <unordered_map>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include "../Utils/Singleton.h"
#include "Session.h"

class IOTransfer : public Singleton<IOTransfer> {

    using UnixSocket = boost::asio::local::datagram_protocol::socket;

    struct transfer_state {
        std::unique_ptr<UnixSocket> low_socket;
        std::unique_ptr<UnixSocket> high_socket;
        uint32_t low_fd = 0;
        uint32_t high_fd = 0;
    };

    int genTransferKey(int from, int to) {
        return (from + 1) * 1000 + to;
    }

public:
    IOTransfer() {}

    void Init(std::vector<boost::asio::io_context>& io_contexts) {

        for (int i = 0 ; i <= io_contexts.size() - 1; ++i) {
            this->transfer_index_map.insert({&io_contexts[i], i});
        }

        for (int i = 0; i <= io_contexts.size() - 2; ++i) {
            for (int j = i + 1; j <= io_contexts.size() - 1; ++j) {
                auto state = std::make_unique<transfer_state>();
                state->low_socket = std::make_unique<UnixSocket>(io_contexts[i]);
                state->high_socket = std::make_unique<UnixSocket>(io_contexts[j]);
                boost::asio::local::connect_pair(*state->low_socket, *state->high_socket);

                this->transfer_map.insert({
                    genTransferKey(i, j),
                    std::move(state)
                });

                // read from low_fd
                // send from high idx to low idx, from j to i
                boost::asio::spawn([&, this](boost::asio::yield_context yield){
                    boost::system::error_code ec;
                    auto& state = this->transfer_map[genTransferKey(i, j)];
                    while(1) {
                        state->low_socket->async_receive(boost::asio::buffer(&state->low_fd, sizeof(uint32_t)), yield[ec]);
                        auto session = boost::intrusive_ptr<Session>(new Session(this->io_contexts[i]));

                    }
                });

                // read from high idx
                // send from low idx to high idx, from i to j
                boost::asio::spawn([&, this](boost::asio::yield_context yield){
                    boost::system::error_code ec;
                    auto& state = this->transfer_map[genTransferKey(i, j)];
                    state->high_socket->async_receive(boost::asio::buffer(&state->high_fd, sizeof(uint32_t)), yield[ec]);
                });
            }
        }

    }

private:
    // we build an idx map for io_context so that we could find where the ctx is located
    std::unordered_map<boost::asio::io_context*, int> transfer_index_map;
    std::unordered_map<int, std::unique_ptr<transfer_state>> transfer_map;
};


