// #ifndef BYTE_STORM_HPP
// #define BYTE_STORM_HPP

// #include <boost/array.hpp>
// #include <boost/asio.hpp>
// #include <boost/bind/bind.hpp>
// #include <boost/enable_shared_from_this.hpp>
// #include <boost/function.hpp>
// #include <boost/noncopyable.hpp>
// #include <boost/shared_ptr.hpp>
// #include <boost/thread.hpp>

// #include <cstdint>
// #include <deque>
// #include <iostream>

// #include "bytestorm_base.hpp"
// #include "handler_base.hpp"

// namespace ByteStorm
// {

// using namespace boost::asio;
// using namespace boost::posix_time;
// using namespace boost::placeholders;

// class ByteStormBoost;

// using Handler = HandlerBase<ByteStormBoost>;

// using Socket = ip::tcp::socket;
// using Acceptor = ip::tcp::acceptor;
// using Endpoint = ip::tcp::endpoint;

// using client_ptr = boost::shared_ptr<ByteStormBoost>;
// using error_code = boost::system::error_code;

// client_ptr create(Handler *h = nullptr);

// class TcpConnectionHandler : public boost::enable_shared_from_this<TcpConnectionHandler>
// {
// public:
//     TcpConnectionHandler(std::string log_prefix,
//                          boost::asio::io_service &io_service,
//                          boost::function<void(std::string &)> received_message_callback);

//     boost::asio::ip::tcp::socket &socket();

//     void start();
//     void write(const std::string &message);

// private:
//     void writeImpl(const std::string &message);

//     void write();

//     void handle_read(const boost::system::error_code &error, size_t bytes_transferred);

//     void handle_write(const boost::system::error_code &error, size_t bytes_transferred);

//     boost::asio::ip::tcp::socket socket_;
//     boost::asio::streambuf message_;
//     std::string log_prefix_;

//     std::deque<std::string> outbox_;
//     boost::asio::io_service &io_service_;
//     boost::asio::io_service::strand strand_;
// };

// class TcpServer
// {
// public:
//     TcpServer(std::string log_prefix, unsigned int port, Handler *h);
//     ~TcpServer();

//     void start();

//     void write(std::string content);

// private:
//     void start_accept();

//     void handle_accept(boost::shared_ptr<TcpConnectionHandler> connection, const boost::system::error_code &error);

//     boost::shared_ptr<TcpConnectionHandler> connection_;
//     boost::asio::io_service io_service_;
//     boost::asio::ip::tcp::acceptor acceptor_;
//     std::string log_prefix_;
//     boost::function<void(std::string &)> received_message_callback_;
//     boost::condition_variable connection_cond_;
//     boost::mutex connection_mutex_;
//     bool client_connected_;
//     boost::thread *io_thread_; /**< Thread to run boost.asio io_service. */
// };

// }; // namespace ByteStorm

// #endif // BYTE_STORM_HPP
