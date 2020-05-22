//
// Created by Mario on 2020/5/17.
//

#include "Server.h"
void Session::replyOK()
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