/**
 * @file bytestorm_boost.hpp
 * @author Semikozov Ian (y.semikozov@geoscan.ru)
 * @brief
 * @version 0.1
 * @date 03.04.2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef BYTESTORM_BOOST_HPP_INCLUDED
#define BYTESTORM_BOOST_HPP_INCLUDED

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/multi_array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <cstdint>
#include <vector>

#include "bytestorm_base.hpp"
#include "processor_base.hpp"

namespace ByteStorm
{

using namespace boost::asio;
using namespace boost::posix_time;
using namespace boost::placeholders;

using Error = boost::system::error_code;
using Acceptor = ip::tcp::acceptor;
using Socket = ip::tcp::socket;
using Endpoint = ip::tcp::endpoint;

class ByteStormBoost;
class TCPConnection;

using Connection = boost::shared_ptr<TCPConnection>;
using Connections = std::vector<Connection>;

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>,
                      boost::noncopyable,
                      public ByteStormBase<TCPConnection>
{
    friend class ByteStormBoost;

public:
    TCPConnection(io_service &service, size_t transfer_size, ProcessorBase<TCPConnection> *h = nullptr);
    ~TCPConnection();

    void flush() override;

    Status send(std::unique_ptr<std::uint8_t[]> &data, const size_t size) override;
    void read();
    void read_complete(const Error &err, size_t bytes_transferred);

    void write(std::unique_ptr<std::uint8_t[]> &data, const size_t size);
    void write_complete(const Error &err, size_t bytes_transferred);

    void on_connect();

    template <typename Fn, typename... Args>
    auto bind(Fn &&fn, Args &&...args)
    {
        return boost::bind(std::forward<Fn>(fn), shared_from_this(), std::forward<Args>(args)...);
    }

private:
    void check();
    void on_check();

public:
    Socket sock;

private:
    boost::signals2::signal<void(Connection)> close;

private:
    Endpoint remote;
    deadline_timer timer;
    boost::asio::streambuf read_buffer, write_buffer;
    boost::posix_time::ptime last_ping;
    std::string ip;
    std::size_t transfer_size;
}; // class TCPConnection

class ByteStormBoost
{
    static constexpr size_t kDefaultBufferSize{ 70 };
    static constexpr int kDefaultPort{ 8081 };

public:
    ByteStormBoost(int port, size_t buffer_size = kDefaultBufferSize);
    ~ByteStormBoost();

    void start();
    void stop();

    Connection create(size_t buffer_size, ProcessorBase<TCPConnection> *processor = nullptr);
    void handle_accept(Connection &connection, const Error &err, ProcessorBase<TCPConnection> *processor = nullptr);
    void on_delete_connection(Connection &connection);

public:
    io_service service;

    Acceptor *acc;
    Connections connections;

    ProcessorBase<TCPConnection> *processor{ nullptr };

    size_t buffer_size;
    int port;
}; // class ByteStormBoost

}; // namespace ByteStorm

#endif // BYTESTORM_BOOST_HPP_INCLUDED
