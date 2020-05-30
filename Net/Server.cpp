//
// Created by Mario on 2020/5/17.
//

#include "Server.h"

void Server::runAccept(int i)
{
    for (int i = 0; i < io_contexts.size(); ++i) {
        auto session = boost::intrusive_ptr<Session>(new Session(this->io_contexts[i]));
        this->acceptors[i].async_accept(session->peer,
                                        [this, session, i](const boost::system::error_code &ec) {
                                            boost::asio::ip::tcp::no_delay option(true);
                                            session->peer.set_option(option);
                                            session->WaitProcess();
                                            this->runAccept(i);
                                        });
    }
}

