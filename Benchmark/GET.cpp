//
// Created by Mario on 2020/5/31.
//

//
// Created by Mario on 2020/5/17.
//

#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <iostream>
#include <vector>
#include "../Protocol/bproto.h"
#include <boost/lexical_cast.hpp>
#include "../Cli/CommandMap.h"
#include "../Cli/CommandRegistry.h"

class BenchmarkSet
{
public:
    BenchmarkSet(int db_index) : db_index(db_index), io_context(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE), buff(4096), socket(io_context) {}

    void Run()
    {
        socket.open(boost::asio::ip::tcp::v4());
        boost::asio::ip::tcp::acceptor::reuse_address reuse_address(true);
        socket.set_option(reuse_address);
        boost::asio::ip::tcp::no_delay option(true);
        socket.set_option(option);
        boost::asio::spawn(this->io_context, [this](boost::asio::yield_context yield) {
            boost::system::error_code ec;
            auto connect_ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 6666);
            this->socket.async_connect(connect_ep, yield[ec]);
            if (ec)
            {
                return;
            }

            auto select_res = selectDB(this->db_index, yield);
            if (!select_res) return;


            for(int i = 0; i < 1000000; ++i) {

                testSET(yield);

            }

        });
        this->io_context.run();
    }

private:
    uint32_t db_index = 0;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;
    std::vector<char> buff;

    bool selectDB(int db_index, boost::asio::yield_context yield) {
        std::vector<std::string> select_parts;
        select_parts.push_back("SELECT");
        select_parts.push_back(std::to_string(db_index));
        auto bytes_to_send = CommandMap::GetInstance()->Dispatch(select_parts, this->buff);
        if (bytes_to_send <= 0) {
            return false;
        }
        auto send_res = sendCommand(bytes_to_send, yield);
        if (!send_res) return false;

        auto read_header_res = readHeader(yield);
        if (!read_header_res.first) return false;

        auto header = (BProtoHeader *)&buff[0];
        if (header->payload_len > buff.size())
        {
            return false;
        }
        auto payload_len = header->payload_len;
        auto payload_type = header->payload_cmd;
        if (payload_type == universal_command::SELECT_OK) {
            if (readPayload(payload_len, yield))
                return true;
        }
        return false;
    }

    bool sendCommand(size_t bytes_to_send, boost::asio::yield_context yield) {
        boost::system::error_code ec;
        // ready to send
        auto bytes_send = boost::asio::async_write(this->socket, boost::asio::buffer(&buff[0], bytes_to_send), yield[ec]);
        if (ec)
        {
            return false;
        }
        return true;
    }

    std::pair<bool, size_t> readHeader(boost::asio::yield_context yield) {
        boost::system::error_code ec;
        // ready to send
        auto bytes_read = boost::asio::async_read(this->socket, boost::asio::buffer(buff, BProtoHeaderSize), yield[ec]);
        if (ec)
        {
            return {false, 0};
        }
        return {true, bytes_read};
    }

    bool readPayload(size_t bytes_to_read, boost::asio::yield_context yield) {
        boost::system::error_code ec;
        // ready to send
        auto bytes_read = boost::asio::async_read(this->socket, boost::asio::buffer(&buff[0], bytes_to_read), yield[ec]);
        if (ec)
        {
            return false;
        }
        return true;
    }

    bool testGET(boost::asio::yield_context yield)
    {
        static std::vector<std::string> parts = {"GET", "BENCHMARK_SET"};

        auto bytes_to_send = CommandMap::GetInstance()->Dispatch(parts, this->buff);
        if (bytes_to_send <= 0) {
            return false;
        }
        auto send_res = sendCommand(bytes_to_send, yield);
        if (!send_res) return false;

        auto read_header_res = readHeader(yield);
        if (!read_header_res.first) return false;

        auto header = (BProtoHeader *)&buff[0];
        if (header->payload_len > buff.size())
        {
            return false;
        }
        auto payload_len = header->payload_len;
        auto payload_type = header->payload_cmd;
        if (payload_type == universal_command::STRING_OK) {
            if (readPayload(payload_len, yield))
                return true;
        }
        return false;
    }

    bool testSET(boost::asio::yield_context yield)
    {
        static std::vector<std::string> parts = {"SET", "BENCHMARK_SET", "BENCHMARK_SET_VALUE"};

        auto bytes_to_send = CommandMap::GetInstance()->Dispatch(parts, this->buff);
        if (bytes_to_send <= 0) {
            return false;
        }
        auto send_res = sendCommand(bytes_to_send, yield);
        if (!send_res) return false;

        auto read_header_res = readHeader(yield);
        if (!read_header_res.first) return false;

        auto header = (BProtoHeader *)&buff[0];
        if (header->payload_len > buff.size())
        {
            return false;
        }
        auto payload_len = header->payload_len;
        auto payload_type = header->payload_cmd;
        if (payload_type == universal_command::OK) {
            return true;
        }
        return false;
    }

};
#include <boost/thread.hpp>
int main() {
    int core = 128;
    client_command::RegisterAll();
    std::atomic<uint32_t> time_sum;
    boost::thread_group tg;
    for (int i = 0; i < core; ++i) {
        tg.create_thread([&, i](){
            auto t1 = std::chrono::high_resolution_clock::now();
            BenchmarkSet(i).Run();
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            time_sum += duration;
        });
    }
    tg.join_all();

    printf("timesum: %d\n", time_sum.load());
    printf("avg time: %lf\n", time_sum.load() / double(core));
    std::cout << "QPS:" << (1000000 * core) / (time_sum / core)  << "\n";
    fflush(stdout);
}