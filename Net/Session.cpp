//
// Created by Mario on 2020/5/27.
//

#include "Session.h"
#include "../Primitives/String.h"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include "../Persistence/Dump.h"

#include "../Protocol/BProto.pb.h"

#define Q(x) #x
#define QUOTE(x) Q(x)
#define GENSTRCASE_COMMAND(key) Command::key

// gen case that takes 3 value
#define GenStringCase3(key_name, reply_type) \
    case GENSTRCASE_COMMAND(key_name): \
    { \
        CMD_##key_name##_REQ req; \
        req.ParseFromArray(&buff[0], bytes_transferred); \
        auto pair = String::key_name(0, std::string(req.key()), std::move(*req.mutable_value()), std::move(*req.mutable_value2())); \
        if (pair.first == Command::OK) \
            reply_type(pair.second); \
        else \
            replyErr(pair.first); \
        break; \
    }

// gen case that takes 2 value
#define GenStringCase2(key_name, reply_type) \
    case GENSTRCASE_COMMAND(key_name): \
    { \
        CMD_##key_name##_REQ req; \
        req.ParseFromArray(&buff[0], bytes_transferred); \
        auto pair = String::key_name(0, std::string(req.key()), std::move(*req.mutable_value())); \
        if (pair.first == (int)Command::OK) \
            reply_type(pair.second); \
        else \
            replyErr((int)pair.first); \
        break; \
    }

// gen case that takes 1 value
#define GenStringCase1(key_name, reply_type) \
    case GENSTRCASE_COMMAND(key_name): \
    { \
        CMD_##key_name##_REQ req; \
        req.ParseFromArray(&buff[0], bytes_transferred); \
        auto pair = String::key_name(0, std::string(req.key())); \
        if (pair.first == (int)Command::OK) \
            reply_type(pair.second); \
        else \
            replyErr((int)pair.first); \
        break; \
    }


void Session::replyOK(...)
{
    auto header = (BProtoHeader *)&buff[0];
    header->payload_len = 0;
    header->payload_cmd = Command::OK;
    boost::asio::async_write(this->peer,
                             boost::asio::buffer(&buff[0], BProtoHeaderSize),
                             [this](const boost::system::error_code &ec,
                                    std::size_t bytes_transferred) {
                                 if (ec)
                                 {
                                     return;
                                 }
                             });
}
void Session::replyIntOK(int code)
{
    auto header = (BProtoHeader *)&buff[0];
    INT_OK_REPLY reply;
    reply.set_value(code);
    header->payload_len = reply.ByteSizeLong();
    header->payload_cmd = Command::INT_OK;
    reply.SerializePartialToArray(&buff[BProtoHeaderSize], reply.ByteSizeLong());
    boost::asio::async_write(this->peer,
                             boost::asio::buffer(&buff[0], BProtoHeaderSize + header->payload_len),
                             [this](const boost::system::error_code &ec,
                                    std::size_t bytes_transferred) {
                                 if (ec)
                                 {
                                     return;
                                 }
                             });
}
void Session::replyStringOK(const std::string &str)
{
    auto header = (BProtoHeader *)&buff[0];
    STRING_OK_REPLY reply;
    reply.set_value(str);
    header->payload_len = reply.ByteSizeLong();
    header->payload_cmd = Command::STRING_OK;
    reply.SerializeToArray(BProtoHeaderOffset(&buff[0]), header->payload_len);
    boost::asio::async_write(this->peer,
                             boost::asio::buffer(&buff[0], BProtoHeaderSize + header->payload_len),
                             [this](const boost::system::error_code &ec,
                                    std::size_t bytes_transferred) {
                                 if (ec)
                                 {
                                     return;
                                 }
                             });
}
void Session::replyRepeatedStringOK(std::vector<std::string> &&strs)
{
    auto header = (BProtoHeader *)&buff[0];
    REPEATED_STRING_OK_REPLY reply;
    for (int i = 0; i < strs.size(); ++i) {
        reply.add_value(std::move(strs[i]));
    }
    header->payload_len = reply.ByteSizeLong();
    header->payload_cmd = Command::REPEATED_STRING_OK;
    reply.SerializeToArray(BProtoHeaderOffset(&buff[0]), header->payload_len);
    boost::asio::async_write(this->peer,
                             boost::asio::buffer(&buff[0], BProtoHeaderSize + header->payload_len),
                             [this](const boost::system::error_code &ec,
                                    std::size_t bytes_transferred) {
                                 if (ec)
                                 {
                                     return;
                                 }
                             });
}
void Session::replyErr(int code)
{
    auto header = (BProtoHeader *)&buff[0];
    INT_OK_REPLY reply;
    reply.set_value(code);
    header->payload_len = 0;
    header->payload_cmd = (Command)code;
    reply.SerializePartialToArray(&buff[BProtoHeaderSize], reply.ByteSizeLong());
    boost::asio::async_write(this->peer,
                             boost::asio::buffer(&buff[0], BProtoHeaderSize + header->payload_len),
                             [this](const boost::system::error_code &ec,
                                    std::size_t bytes_transferred) {
                                 if (ec)
                                 {
                                     return;
                                 }
                             });
}

void Session::WaitProcess()
{
    readHeader();
}

void Session::readHeader()
{
    auto self = this->self();
    boost::asio::async_read(this->peer,
                            boost::asio::buffer(&buff[0], BProtoHeaderSize),
                            make_custom_alloc_handler(
                                    this->handler_memory_,
                                    [self, this](const boost::system::error_code &ec,
                                           std::size_t bytes_transferred) {
                                        if (ec)
                                        {
                                            std::cout << ec.message();
                                            fflush(stdout);
                                            return;
                                        }
                                        auto header = (BProtoHeader *)&buff[0];
                                        auto buffer_size = buff.size();
                                        if (header->payload_len > buffer_size) {
                                            return;
                                        }
                                        this->readPayload(header->payload_len, header->payload_cmd);
                                    }));
}

void Session::readPayload(uint32_t size, Command cmd_type)
{
    auto self = this->self();
    boost::asio::async_read(this->peer,
                            boost::asio::buffer(&buff[0], size),
                            make_custom_alloc_handler(
                                    this->handler_memory_,
                                    [self, this, cmd_type](const boost::system::error_code &ec,
                                                     std::size_t bytes_transferred) {
                                        if (ec)
                                        {
                                            std::cout << ec.message();
                                            fflush(stdout);
                                            return;
                                        }
                                        switch ((int)cmd_type)
                                        {
                                            case universal_command::SELECT:
                                            {
                                                universal_command::SELECT_REQ req;
                                                req.ParseFromArray(&buff[0], bytes_transferred);
                                                break;
                                            }
                                            case Command::SAVE:
                                            {
                                                Dump::SAVE();
                                                replyOK();
                                                break;
                                            }
                                            case Command::BGSAVE:
                                            {
                                                Dump::BGSAVE();
                                                replyOK();
                                                break;
                                            }
                                            case Command::KEYS:
                                            {
                                                auto db = Database::GetInstance();
                                                replyRepeatedStringOK(db->KEYS(0));
                                                break;
                                            }
                                            GenStringCase2(SET, replyOK)
                                            GenStringCase1(GET, replyStringOK)
                                            GenStringCase3(PSETEX, replyIntOK)
                                            GenStringCase3(SETEX, replyIntOK)
                                            GenStringCase3(GETRANGE, replyStringOK)
                                            GenStringCase2(SETNX, replyIntOK)
                                            GenStringCase2(GETSET, replyStringOK)
                                            GenStringCase1(INCR, replyStringOK)
                                            GenStringCase1(DECR, replyStringOK)
                                            GenStringCase1(STRLEN, replyIntOK)
                                            GenStringCase2(APPEND, replyIntOK)
                                            GenStringCase2(INCRBY, replyStringOK)
                                            GenStringCase2(DECRBY, replyStringOK)
                                            default:
                                            {
                                            }
                                        };
                                        readHeader();
                                    }));
}


void Session::selectDatabase(int which) {

    this->peer.release();
}