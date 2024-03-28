#ifndef BYTE_STORM_HPP
#define BYTE_STORM_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <cstdint>

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

using Handler = HandlerBase<ByteStormBoost>;

class TCPConnection : public ByteStormBase<ByteStormBoost>, public boost::enable_shared_from_this<Connection>, boost::noncopyable
{
public:
    TCPConnection();
    ~TCPConnection();
    
    void read();
    void on_read();
    
    void write();
    void on_write();
    
    template <typename Fn, typename... Args>
    auto bind(Fn &&fn, Args &&...args)
    {
        return boost::bind(std::forward<Fn>(fn), shared_from_this(), std::forward<Args>(args)...);
    }

private:
    std::string ip;
    Endpoint remote;
}; // class TCPConnection

class ByteStormBoost : public ByteStormBase<ByteStormBoost>
{
public:
    ByteStormBoost();
    ~ByteStormBoost();
    
    void start();
    void stop();

    static void create(Handler *h = nullptr);
    
    void send(const void *data, const size_t size);
    // {
        // if (!started() || size == 0) return;
        // std::memcpy(m_txBuffer.data(), data, size);
        // m_socket.async_write_some(buffer(m_txBuffer, size), bindMemberFunction(&ByteStormBoost::onWrite_, _1, _2));
        // this->flush_socket();
    // }
private:
    void handle_accept(Connection connection, const Error &err, Handler *h);
    io_service service;
    Acceptor acc;
    Socket socket;

    Handler *h{nullptr};
    Connections connections;
    
}; // class ByteStormBoost
    

// public:
//     ByteStormBoost(HandlerBase<client_ptr> *h, int p, io_service &service);
//     ~ByteStormBoost();

//     inline Socket &sock()
//     {
//         return m_socket;
//     }

//     inline HandlerBase<client_ptr> *netHandler()
//     {
//         return m_neth;
//     }

//     void set_clients_changed()
//     {
//         clientListChanged = true;
//     }

// private:
//     friend void handle_accept(client_ptr client, const boost::system::error_code &err, HandlerBase<client_ptr> *neth); 
//     void onRead_(const error_code &err, size_t bytes); void onConnect_(); inline void onPing_()
//     {
//         clientListChanged = false;
//     }

//     void ping_();
//     void onCheckPing_();
//     void postCheckPing_();

//     void onWrite_(const error_code &err, size_t bytes)
//     {
//         read_();
//     }

//     void read_()
//     {
//         async_read(m_socket,
//                    buffer(m_rxBuffer),
//                    bindMemberFunction(&ByteStormBoost::read_complete, _1, _2),
//                    bindMemberFunction(&ByteStormBoost::onRead_, _1, _2));
//         postCheckPing_();
//     }

//     void flush_socket()
//     {
//         boost::asio::streambuf b;
//         boost::asio::streambuf::mutable_buffers_type bufs = b.prepare(1024);
//         std::size_t bytes = m_socket.receive(bufs); // !!! This will block until some data becomes available
//         b.commit(bytes);
//         boost::asio::socket_base::bytes_readable command(true);
//         m_socket.io_control(command);

//         while (command.get())
//         {
//             bufs = b.prepare(1024);
//             bytes = m_socket.receive(bufs);
//             b.commit(bytes);
//             m_socket.io_control(command); // reset for bytes pending
//         }
//         return;
//     }

//     size_t read_complete(const boost::system::error_code &err, size_t bytes)
//     {
//         if (err) return 0;
//         bool found = std::find(m_rxBuffer.begin(), m_rxBuffer.begin() + bytes, '\n') != m_rxBuffer.begin() + bytes;
//         // we read one-by-one until we get to enter, no buffering
//         return found ? 0 : 1;
//     }



// inline void handle_accept(client_ptr client, const boost::system::error_code &err,
// HandlerBase<typename<client_ptr>
// *neth)
// {
//     std::cout << "[info]: New client is accepted with IP " << client->clientIP;
//     client->start();
//     neth->set(client);
//     client_ptr new_client = ByteStorm::new_(neth);
//     // new_client->sock() = ip::tcp::socket(service);
//     acceptor.async_accept(new_client->m_socket, boost::bind(handle_accept, new_client, _1, neth));
// }
}; // namespace ByteStorm

#endif // BYTE_STORM_HPP
