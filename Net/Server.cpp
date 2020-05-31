//
// Created by Mario on 2020/5/17.
//

#include "Server.h"
#include "Session.h"
#include "IOTransfer.h"
#include "IOExecutor.h"
void Server::Init() {

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

        /*
         *
         * SO_REUSEPORT works on linux only
         * it enables load balance for tcp accept()
         *
         */
        setsockopt(acceptors.back().native_handle(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

        acceptors.back().bind(bind_ep, ec);
        if (ec)
        {
            return;
        }

        acceptors.back().listen(SOMAXCONN, ec);

        runAccept(i);
    }

    // enable IOTransfer for db selection
    IOTransfer::GetInstance()->Init();
}

void Server::runAccept(int which)
{

    auto session = boost::intrusive_ptr<Session>(new Session(IOExecutor::GetInstance()->GetContextAt(which)));
    this->acceptors[which].async_accept(session->peer,
                                    [this, session, which](const boost::system::error_code &ec) {
                                        boost::asio::ip::tcp::no_delay option(true);
                                        session->peer.set_option(option);
                                        session->WaitProcess();
                                        this->runAccept(which);
                                    });
}

