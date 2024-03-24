#ifndef BYTE_STORM_HPP
#define BYTE_STORM_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <stdint.h>

#include "NetHandlerInterface.hpp"

namespace ByteStorm
{

using namespace boost::asio;
using namespace boost::posix_time;
using namespace boost::placeholders;

static constexpr int kPortNumber{ 8081 };

static io_service service;
static ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), kPortNumber));

static void updateClientList();

class ByteStorm;

using client_ptr = boost::shared_ptr<ByteStorm>;
using client_vector = std::vector<client_ptr>;
using error_code = boost::system::error_code;

static client_vector clients;

class ByteStorm : public boost::enable_shared_from_this<ByteStorm>, boost::noncopyable
{
    using me = ByteStorm;

public:
    template <typename Fn, typename... Args>
    auto bindMemberFunction(Fn &&fn, Args &&...args)
    {
        return boost::bind(std::forward<Fn>(fn), shared_from_this(), std::forward<Args>(args)...);
    }

    ByteStorm(NetHandlerInterface<client_ptr> *neth, size_t bufferSize)
        : m_neth(neth),
          m_bufferSize(bufferSize),
          m_socket(service),
          m_started(false),
          m_deadlineTimer(service),
          clientListChanged(false)
    {}

    ByteStorm(NetHandlerInterface<client_ptr> *neth)
        : m_neth(neth),
          m_socket(service),
          m_started(false),
          m_deadlineTimer(service),
          clientListChanged(false)
    {}

    ~ByteStorm()
    {
        // delete m_rxBuffer;
    }
    void start()
    {
        m_started = true;
        clients.push_back(shared_from_this());
        this->onConnect_();
        m_lastPing = boost::posix_time::microsec_clock::local_time();
        // first, we wait for client to login
        read_();
    }
    static client_ptr new_(NetHandlerInterface<client_ptr> *m_neth)
    {
        client_ptr new_(new ByteStorm(m_neth));
        return new_;
    }
    void stop()
    {
        if (!m_started) return;
        m_started = false;
        m_socket.close();

        client_ptr self = shared_from_this();
        client_vector::iterator it = std::find(clients.begin(), clients.end(), self);
        clients.erase(it);
        updateClientList();
    }

    void sendData(const void *data, const size_t size)
    {
        this->write_(data, size);
    }

    bool started() const __attribute__((warn_unused_result))
    {
        return m_started;
    }
    ip::tcp::socket &sock()
    {
        return m_socket;
    }
    NetHandlerInterface<client_ptr> *netHandler()
    {
        return m_neth;
    }
    void set_clients_changed()
    {
        clientListChanged = true;
    }

private:
    // friend void handle_accept(ByteStorm::ptr client, const boost::system::error_code &err, Test<client_ptr>
    // *neth);
    void onRead_(const error_code &err, size_t bytes)
    {
        if (err)
        {
            std::cerr << ">>[info] Error is occured while reading with code: " << err << std::endl;
            std::cerr << ">>[info] Stop this client with IP " << clientIP << std::endl;
            stop();
        }
        if (!started()) { return; }

        if (bytes <= 0)
        {
            std::cerr << ">>[info] There is nothing to receive." << std::endl;
            return;
        }
        if (m_neth == nullptr)
        {
            std::cerr << ">>[info] NetHandler is not set. Skipped query." << std::endl;
            return;
        }

        std::cout << ">>[info] Bytes received: " << bytes << std::endl;
        m_neth->handle(reinterpret_cast<uint8_t *>(m_rxBuffer.data()), bytes);
    }

    void onConnect_()
    {
        clientIP = m_socket.remote_endpoint().address().to_string();
        std::cout << ">>[info] Client with IP " << clientIP << " is connected." << std::endl;
        m_lastPing = boost::posix_time::microsec_clock::local_time();
        updateClientList();
    }
    void onPing_()
    {
        clientListChanged = false;
    }

    void ping_() {}

    void onCheckPing_()
    {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        if ((now - m_lastPing).total_milliseconds() > 15000)
        {
            std::cout << ">>[INFO] Stopping " << clientIP
                      << " - no ping in time. Difference between pings: " << (now - m_lastPing).total_milliseconds()
                      << std::endl;
            stop();
        }
        m_lastPing = boost::posix_time::microsec_clock::local_time();
    }

    void postCheckPing_()
    {
        m_deadlineTimer.expires_from_now(boost::posix_time::millisec(5));
        m_deadlineTimer.async_wait(bindMemberFunction(&me::onCheckPing_));
    }

    void onWrite_(const error_code &err, size_t bytes)
    {
        read_();
    }

    void read_()
    {
        async_read(m_socket,
                   buffer(m_rxBuffer),
                   bindMemberFunction(&me::read_complete, _1, _2),
                   bindMemberFunction(&me::onRead_, _1, _2));
        postCheckPing_();
    }
    void write_(const void *data, const uint32_t size)
    {
        if (!started() || size == 0) return;
        std::memcpy(m_txBuffer.data(), data, size);
        m_socket.async_write_some(buffer(m_txBuffer, size), bindMemberFunction(&me::onWrite_, _1, _2));
        this->flush_socket();
    }

    void flush_socket()
    {
        boost::asio::streambuf b;
        boost::asio::streambuf::mutable_buffers_type bufs = b.prepare(1024);
        std::size_t bytes = m_socket.receive(bufs); // !!! This will block until some data becomes available
        b.commit(bytes);
        boost::asio::socket_base::bytes_readable command(true);
        m_socket.io_control(command);

        while (command.get())
        {
            bufs = b.prepare(1024);
            bytes = m_socket.receive(bufs);
            b.commit(bytes);
            m_socket.io_control(command); // reset for bytes pending
        }
        return;
    }

    size_t read_complete(const boost::system::error_code &err, size_t bytes)
    {
        if (err) return 0;
        bool found = std::find(m_rxBuffer.begin(), m_rxBuffer.begin() + bytes, '\n') != m_rxBuffer.begin() + bytes;
        // we read one-by-one until we get to enter, no buffering
        return found ? 0 : 1;
    }

private:
    std::string clientIP;
    uint32_t m_netHeader{ 0 };
    NetHandlerInterface<client_ptr> *m_neth;
    ip::tcp::socket m_socket;
    int m_bufferSize{ 0 };
    boost::array<uint8_t, 1024> m_rxBuffer;
    boost::array<uint8_t, 1024> m_txBuffer;
    std::atomic<bool> m_started{ false };
    std::string username_;
    deadline_timer m_deadlineTimer;
    boost::posix_time::ptime m_lastPing;
    bool clientListChanged;
};

void updateClientList()
{
    for (auto client : clients) { client->set_clients_changed(); }
}

static void
handle_accept(client_ptr client, const boost::system::error_code &err, NetHandlerInterface<client_ptr> *neth)
{
    client->start();
    neth->set(client);
    client_ptr new_client = ByteStorm::new_(neth);
    // new_client->sock() = ip::tcp::socket(service);
    acceptor.async_accept(new_client->sock(), boost::bind(handle_accept, new_client, _1, neth));
}
}; // namespace ByteStorm

#endif // BYTE_STORM_HPP
