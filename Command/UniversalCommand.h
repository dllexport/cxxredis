//
// Created by root on 2020/5/31.
//

#pragma once

#include <boost/intrusive_ptr.hpp>
#include "../Net/Session.h"
#include "../Protocol/BProto.pb.h"
#include "CommandDispatch.h"
#include <boost/lexical_cast.hpp>
#include "../Net/IOTransfer.h"
#include "../Persistence/Dump.h"

namespace command {

    const auto SELECT = [](const boost::intrusive_ptr<Session>& session, int payload_len) {
        universal_command::REQ1 req;
        req.ParseFromArray(&session->buff[0], payload_len);
        uint32_t db_index = 0;
        try {
            db_index = boost::lexical_cast<uint32_t>(req.value());
        }catch (...) {
            session->replyErr(universal_command::PARAM_ERR);
            return;
        }
        // session is revoke if doTransfer return true
        IOTransfer::GetInstance()->doTransfer(session, db_index);
    };


    const auto SAVE = [](const boost::intrusive_ptr<Session>& session, int) {
        Dump::SAVE();
        session->replyOK();
    };

    const auto BG_SAVE = [](const boost::intrusive_ptr<Session>& session, int) {
        Dump::BGSAVE();
        session->replyOK();
    };

    const auto KEYS = [](const boost::intrusive_ptr<Session>& session, int) {
        auto db = Database::GetInstance();
        session->replyRepeatedStringOK(db->KEYS(session->db_index));
    };

}