//
// Created by root on 2020/5/31.
//

#pragma once

#include <boost/intrusive_ptr.hpp>
#include "../Net/Session.h"
#include "../Protocol/BProto.pb.h"
#include "../Primitives/String.h"
#include "CommandDispatch.h"

#define GenHandler2(HandlerName, req_name, command_name, reply_handler) \
    const auto HandlerName = [](const boost::intrusive_ptr<Session>& session) { \
        universal_command::req_name req; \
        req.ParseFromArray(&session->buff[0], session->buff.size()); \
        auto pair = String::command_name(0, req.value1(), std::move(*req.mutable_value2())); \
        if (pair.first == universal_command::OK) \
            session->reply_handler(pair.second); \
        else \
            session->replyErr(pair.first); \
    };

namespace command {

    const auto StringSet = [](boost::intrusive_ptr<Session> session) {
        universal_command::REQ2 req;
        req.ParseFromArray(&session->buff[0], session->buff.size());
        auto pair = String::SET(0, std::string(req.value1()), std::move(*req.mutable_value2()));
        if (pair.first == universal_command::OK)
            session->replyOK();
        else
            session->replyErr(pair.first);
    };

    const auto StringGet = [](boost::intrusive_ptr<Session> session) {
        universal_command::REQ1 req;
        req.ParseFromArray(&session->buff[0], session->buff.size());
        auto pair = String::GET(0, req.value());
        if (pair.first == universal_command::OK)
            session->replyStringOK(pair.second);
        else
            session->replyErr(pair.first);
    };

    GenHandler2(StringGetSet, REQ2, GETSET, replyStringOK)


}