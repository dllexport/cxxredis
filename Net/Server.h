//
// Created by Mario on 2020/5/17.
//

#pragma once

#include <boost/asio/ip/tcp.hpp>
#include "../Utils/Singleton.h"

/*
 * impl socket related services
 */
class Server : public Singleton<Server>
{
public:
    Server() {}

    void Init();

private:
    using Acceptor = boost::asio::ip::tcp::acceptor;

    std::vector<Acceptor> acceptors;

    void runAccept(int which);
};
