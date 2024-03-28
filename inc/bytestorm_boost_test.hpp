/**
 * @file bytestorm_boost.hpp
 * @author Semikozov Ian (semikozov.yal@yandex.ru)
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
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <cstdint>
#include <iostream>

#include "bytestorm_base.hpp"
#include "handler_base.hpp"

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

using Handler = HandlerBase<TCPConnection>;

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>,
                      boost::noncopyable,
                      public ByteStormBase<TCPConnection>
{
    friend class ByteStormBoost;

public:
    TCPConnection(io_service &service, size_t buffer_size, Handler *h = nullptr);
    ~TCPConnection();

    void flush() override;

    Status send(std::unique_ptr<std::uint8_t> data, const size_t size) override;
    inline Socket *getSocket() const
    {
        return sock;
    }
    void read();
    void on_read(const Error &err, size_t bytes);
    size_t read_complete(const Error &err, size_t bytes);

    void write(std::unique_ptr<std::uint8_t> data, const size_t size);
    void on_write(const Error &err, size_t bytes);

    void on_connect();

    template <typename Fn, typename... Args>
    auto bind(Fn &&fn, Args &&...args)
    {
        return boost::bind(std::forward<Fn>(fn), shared_from_this(), std::forward<Args>(args)...);
    }

private:
    void onCheckPing();

private:
    Handler *h;
    Endpoint remote;
    Socket *sock;
    deadline_timer timer;
    boost::array<uint8_t, 21> rx_buffer, tx_buffer;
    std::string ip;
    boost::posix_time::ptime last_ping;
}; // class TCPConnection

class ByteStormBoost
{
    static constexpr size_t kDefaultBufferSize{ 21 };
    static constexpr int kDefaultPort{ 8081 };

public:
    ByteStormBoost(int port, size_t buffer_size = kDefaultBufferSize);
    ~ByteStormBoost();

    void start();
    void stop();

    Connection create(size_t buffer_size, Handler *h = nullptr);
    void setPort(int port);

    void handle_accept(Connection connection, const Error &err, Handler *h = nullptr);

public:
    io_service service;
    Acceptor *acc;
    Handler *h{ nullptr };
    Connections connections;
    size_t buffer_size;
    int port;
}; // class ByteStormBoost

}; // namespace ByteStorm

#endif // BYTESTORM_BOOST_HPP_INCLUDED
