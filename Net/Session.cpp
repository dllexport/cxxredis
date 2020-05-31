//
// Created by Mario on 2020/5/27.
//

#include "Session.h"
#include "../Primitives/String.h"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include "../Persistence/Dump.h"

#include "../Protocol/BProto.pb.h"

#include "../Command/CommandDispatch.h"

#include "IOTransfer.h"

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
    header->payload_cmd = universal_command::OK;
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
    universal_command::INT_REPLY1 reply;
    reply.set_value(code);
    header->payload_len = reply.ByteSizeLong();
    header->payload_cmd = universal_command::INT_OK;
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
    universal_command::REPLY1 reply;
    reply.set_value(str);
    header->payload_len = reply.ByteSizeLong();
    header->payload_cmd = universal_command::STRING_OK;
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
    universal_command::REPLY_ANY reply;
    for (int i = 0; i < strs.size(); ++i) {
        reply.add_value(std::move(strs[i]));
    }
    header->payload_len = reply.ByteSizeLong();
    header->payload_cmd = universal_command::REPEATED_STRING_OK;
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
    universal_command::INT_REPLY1 reply;
    reply.set_value(code);
    header->payload_len = 0;
    header->payload_cmd = (universal_command::COMMAND)code;
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
                                        auto buffer_size = buff.capacity();
                                        if (header->payload_len > buffer_size) {
                                            return;
                                        }
                                        this->readPayload(header->payload_len, header->payload_cmd);
                                    }));
}

void Session::readPayload(uint32_t size, int command_code)
{
    auto self = this->self();
    boost::asio::async_read(this->peer,
                            boost::asio::buffer(&buff[0], size),
                            make_custom_alloc_handler(
                                    this->handler_memory_,
                                    [self, this, command_code](const boost::system::error_code &ec,
                                                     std::size_t bytes_transferred) {
                                        if (ec)
                                        {
                                            std::cout << ec.message();
                                            fflush(stdout);
                                            return;
                                        }

                                        this->buff.resize(bytes_transferred);

                                        auto self = this->self();
                                        CommandDispatch::GetInstance()->Dispatch(command_code, self);

                                        readHeader();
                                    }));
}
