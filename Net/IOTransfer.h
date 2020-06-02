//
// Created by Mario on 2020/5/30.
//

#pragma once

#include <unordered_map>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include "../Utils/Singleton.h"
#include "../Database.h"
#include "Session.h"
#include "IOExecutor.h"

/*
 * IOTransfer is an mechanism to avoid locking
 * each io_context is responsible for a set of dbs, their index is simply a mod calculation
 * for instance:
 *      on a system with 4 core(which means 4 io_contexts) 16 dbs
 *      db 0,4,8,12 is managed by io_contexts[0]
 *      db 1,5,9,13 is managed by io_contexts[1] ... so forth
 *
 * with SO_REUSEPORT enalbed, each io_context has it's own socket, but they all bind to same endpoint
 * let's say the default db is db[0] for every new connection, which is handle by io_contexts[0]
 * when client connect, the socket might be handled by any of the io_contexts
 * client socket in io_conetxt[1] must not access db [0,4,8,12], db [2,6,10,14] and db [3,7,11,15]
 * there's a race condition here, but client should be access any db anyway
 * adding mutex is an easy approach but there's another way
 * we use UnixSocket to solve the issue
 *
 * the idea is simple, below are some steps of execution when Session in io_context[1] wants to access db[0]
 * 1. save the corresponding fd
 * 2. release asio::ip::tcp::socket, keep the underling fd opened
 * 3. send fd to io_context[0] via socket pair
 * 4. build another session in io_context[0]
 *
 *  that's it, no mutex is needed.
 *
 */
class IOTransfer : public Singleton<IOTransfer> {

    using UnixSocket = boost::asio::local::datagram_protocol::socket;

    struct transfer_state {
        std::unique_ptr<UnixSocket> low_socket;
        std::unique_ptr<UnixSocket> high_socket;
    };

    struct transfer_context {
        int fd;
        int db_index;
    };

    int genTransferKey(int from, int to) {
        return (from + 1) * 1000 + to;
    }

public:
    IOTransfer() {}

    void Init() {

        auto io_executor = IOExecutor::GetInstance();

        auto context_count = io_executor->GetContextCount();

        for (int i = 0 ; i <= context_count - 1; ++i) {
            this->transfer_index_map.insert({&io_executor->GetContextAt(i), i});
        }

        for (int i = 0; i <= context_count - 2; ++i) {
            for (int j = i + 1; j <= context_count - 1; ++j) {
                auto state = std::make_shared<transfer_state>();
                state->low_socket = std::make_unique<UnixSocket>(io_executor->GetContextAt(i));
                state->high_socket = std::make_unique<UnixSocket>(io_executor->GetContextAt(j));
                boost::asio::local::connect_pair(*state->low_socket, *state->high_socket);

                auto res = state->low_socket->is_open();

                this->transfer_map.insert({
                    genTransferKey(i, j),
                    std::move(state)
                });

                // read from low_fd
                // send from high idx to low idx, from j to i
                boost::asio::spawn(io_executor->GetContextAt(i), [i, j, io_executor, this](boost::asio::yield_context yield){
                    boost::system::error_code ec;
                    auto& state = this->transfer_map[genTransferKey(i, j)];
                    transfer_context ctx{};
                    while(1) {
                        state->low_socket->async_receive(boost::asio::buffer(&ctx, sizeof(transfer_context)), yield[ec]);
                        if (ec) {
                            return;
                        }
                        auto session = boost::intrusive_ptr<Session>(new Session(io_executor->GetContextAt(i)));
//                        std::cout << "asign to " << &io_executor->GetContextAt(i) << std::endl;
//                        fflush(stdout);
                        session->peer.assign(boost::asio::ip::tcp::v4(), ctx.fd);
                        session->db_index = ctx.db_index;
                        session->replySelectOK(ctx.db_index);
                        session->WaitProcess();
                    }
                });

                // read from high idx
                // send from low idx to high idx, from i to j
                boost::asio::spawn(io_executor->GetContextAt(j), [i, j, io_executor,this](boost::asio::yield_context yield){
                    boost::system::error_code ec;
                    auto& state = this->transfer_map[genTransferKey(i, j)];
                    transfer_context ctx{};
                    while(1) {
                        state->high_socket->async_receive(boost::asio::buffer(&ctx, sizeof(transfer_context)), yield[ec]);
                        if (ec) {
                            return;
                        }
                        auto session = boost::intrusive_ptr<Session>(new Session(io_executor->GetContextAt(j)));
//                        std::cout << "session assign " << &io_executor->GetContextAt(j) << std::endl;
//                        fflush(stdout);
                        session->peer.assign(boost::asio::ip::tcp::v4(), ctx.fd);
                        session->db_index = ctx.db_index;
                        session->replySelectOK(ctx.db_index);
                        session->WaitProcess();
                    }
                });
            }
        }

    }

    /*
     * return true if transfer complete
     * return false if transfer is not needed
     */
    bool doTransfer(const boost::intrusive_ptr<Session>& session, uint32_t target_db_idx) {

        auto io_executor = IOExecutor::GetInstance();

        auto max_db_index = Database::GetInstance()->DBCount() - 1;

        if (target_db_idx > max_db_index)
        {
            session->replyErr(universal_command::PARAM_ERR);
            return false;
        }

        // there's nothing to transfer on non-linux system
        #ifndef __linux__
                session->db_index = target_db_idx;
                session->replySelectOK(target_db_idx);
                return false;
        #endif

        uint32_t to_idx = target_db_idx % io_executor->GetContextCount();
        auto io_context = static_cast<boost::asio::io_context*>(&session->peer.get_executor().context());
        uint32_t from_idx = transfer_index_map[io_context];

        if (from_idx == to_idx) {
            // the default db_index is 0
            // which must be updated to the right idx
            session->db_index = to_idx;
            session->replySelectOK(target_db_idx);
            return false;
        }

        auto low = std::min(from_idx, to_idx);
        auto high = std::max(from_idx, to_idx);

        auto transfer_state = this->transfer_map[genTransferKey(low, high)];

//        std::cout << "release\n";
//        fflush(stdout);
        auto fd = session->peer.release();
        *(int*)session->buff.data() = fd;

        auto ctx = (transfer_context*)session->buff.data();
        ctx->fd = fd;
        ctx->db_index = target_db_idx;

        // if we are transfering to high
        if (low == from_idx) {
            transfer_state->low_socket->async_send(boost::asio::buffer((void*)ctx, sizeof(transfer_context)), [session](...){});
        }else if (low == to_idx) {
            transfer_state->high_socket->async_send(boost::asio::buffer((void*)ctx, sizeof(transfer_context)), [session](...){});
        }else {
            printf("should not reach this line\n");
            exit(-1);
        }

        return true;
    }

private:

    // we build an idx map for io_context so that we could find where the ctx is located
    std::unordered_map<boost::asio::io_context*, int> transfer_index_map;
    std::unordered_map<int, std::shared_ptr<transfer_state>> transfer_map;
};


