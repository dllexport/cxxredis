//
// Created by root on 2020/5/31.
//

#pragma once

#include <boost/intrusive_ptr.hpp>
#include "../Net/Session.h"
#include "../Protocol/BProto.pb.h"
#include "../Primitives/String.h"
#include "CommandDispatch.h"

#define GenHandler3(HandlerName, req_name, command_name, reply_handler) \
    const auto HandlerName = [](const boost::intrusive_ptr<Session>& session) { \
        universal_command::req_name req; \
        req.ParseFromArray(&session->buff[0], session->buff.size()); \
        auto pair = String::command_name(0, req.value1(), std::move(*req.mutable_value2()), std::move(*req.mutable_value3())); \
        if (pair.first == universal_command::OK) \
            session->reply_handler(pair.second); \
        else \
            session->replyErr(pair.first); \
    };

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

#define GenHandler1(HandlerName, req_name, command_name, reply_handler) \
    const auto HandlerName = [](const boost::intrusive_ptr<Session>& session) { \
        universal_command::req_name req; \
        req.ParseFromArray(&session->buff[0], session->buff.size()); \
        auto pair = String::command_name(0, req.value()); \
        if (pair.first == universal_command::OK) \
            session->reply_handler(pair.second); \
        else \
            session->replyErr(pair.first); \
    };

namespace command {

    GenHandler1(GET, REQ1, GET, replyStringOK)
    GenHandler1(INCR, REQ1, INCR, replyStringOK)
    GenHandler1(DECR, REQ1, DECR, replyStringOK)
    GenHandler1(STRLEN, REQ1, STRLEN, replyIntOK)

    GenHandler2(GETSET, REQ2, GETSET, replyStringOK)
    GenHandler2(SET, REQ2, SET, replyOK)
    GenHandler2(SETNX, REQ2, SETNX, replyIntOK)
//    GenHandler2(APPEND, REQ2, APPEND, replyIntOK)
    GenHandler2(INCRBY, REQ2, INCRBY, replyStringOK)
    GenHandler2(DECRBY, REQ2, DECRBY, replyStringOK)

    const auto APPEND = [](const boost::intrusive_ptr<Session>& session) {
        universal_command::REQ2 req;
        req.ParseFromArray(&session->buff[0], session->buff.size());
        auto pair = String::APPEND(0, req.value1(), std::move(*req.mutable_value2()));
        if (pair.first == universal_command::OK)
            session->replyIntOK(pair.second);
        else
            session->replyErr(pair.first);
    };

    GenHandler3(PSETEX, REQ3, PSETEX, replyIntOK)
    GenHandler3(SETEX, REQ3, SETEX, replyIntOK)
    GenHandler3(GETRANGE, REQ3, GETRANGE, replyStringOK)

}