//
// Created by Mario on 2020/5/17.
//

#include "Server.h"
#include "IOExecutor.h"

void Server::runAccept(int which)
{
    auto& io_contexts = IOExecutor::GetInstance()->GetContexts();

    auto session = boost::intrusive_ptr<Session>(new Session(io_contexts[which]));
    this->acceptors[which].async_accept(session->peer,
                                    [this, session, which](const boost::system::error_code &ec) {
                                        boost::asio::ip::tcp::no_delay option(true);
                                        session->peer.set_option(option);
                                        session->WaitProcess();
                                        this->runAccept(which);
                                    });
}

