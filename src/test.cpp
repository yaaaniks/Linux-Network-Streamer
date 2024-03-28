// #include "bytestorm_boost.hpp"

// using namespace ByteStorm;

// TcpConnectionHandler::TcpConnectionHandler(std::string log_prefix,
//                                            boost::asio::io_service &io_service,
//                                            boost::function<void(std::string &)> received_message_callback)
//     : io_service_(io_service),
//       strand_(io_service_),
//       socket_(io_service),
//       outbox_()
// {
//     log_prefix_ = log_prefix;
//     received_message_callback_ = received_message_callback;
// }

// boost::asio::ip::tcp::socket &TcpConnectionHandler::socket()
// {
//     return socket_;
// }

// void TcpConnectionHandler::start()
// {
//     async_read_until(socket_,
//                      message_,
//                      "\r\n",
//                      boost::bind(&TcpConnectionHandler::handle_read,
//                                  shared_from_this(),
//                                  boost::asio::placeholders::error,
//                                  boost::asio::placeholders::bytes_transferred));
// }

// void TcpConnectionHandler::write(const std::string &message)
// {
//     strand_.post(boost::bind(&TcpConnectionHandler::writeImpl, this, message));
// }

// void TcpConnectionHandler::writeImpl(const std::string &message)
// {
//     outbox_.push_back(message);
//     if (outbox_.size() > 1)
//     {
//         // outstanding async_write
//         return;
//     }

//     this->write();
// }

// void TcpConnectionHandler::write()
// {
//     const std::string &message = outbox_[0];
//     boost::asio::async_write(socket_,
//                              boost::asio::buffer(message.c_str(), message.size()),
//                              strand_.wrap(boost::bind(&TcpConnectionHandler::handle_write,
//                                                       this,
//                                                       boost::asio::placeholders::error,
//                                                       boost::asio::placeholders::bytes_transferred)));
// }

// void TcpConnectionHandler::handle_read(const boost::system::error_code &error, size_t bytes_transferred)
// {
//     // Check for client disconnection
//     if ((boost::asio::error::eof == error) || (boost::asio::error::connection_reset == error))
//     {
//         // LOG(ERROR) << log_prefix_ << " TCP/IP client disconnected!";
//         return;
//     }

//     // Convert stream to string
//     std::istream stream(&message_);
//     std::istreambuf_iterator<char> eos;
//     std::string message_str(std::istreambuf_iterator<char>(stream), eos);

//     // LOG(DEBUG) << log_prefix_ << " communication object received message: " << getPrintableMessage(message_str);

//     std::istringstream iss(message_str);

//     std::string msg;
//     std::getline(iss, msg, '\r'); // Consumes from the streambuf.

//     // Discard the rest of the message from buffer
//     message_.consume(message_.size());

//     if (!error)
//     {
//         received_message_callback_(msg);
//         start();
//     }
//     else
//     {
//         // TODO: Handler here the error
//     }
// }

// void TcpConnectionHandler::handle_write(const boost::system::error_code &error, size_t bytes_transferred)
// {
//     outbox_.pop_front();

//     if (error)
//     {
//         std::cerr << "could not write: " << boost::system::system_error(error).what() << std::endl;
//         return;
//     }

//     if (!outbox_.empty())
//     {
//         // more messages to send
//         this->write();
//     }
// }

// // TcpServer

// TcpServer::TcpServer(std::string log_prefix,
//                      unsigned int port,
//                      boost::function<void(std::string &)> received_message_callback)
//     : acceptor_(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
//       client_connected_(false),
//       io_thread_(NULL)
// {

//     log_prefix_ = log_prefix;
//     received_message_callback_ = received_message_callback;

//     start_accept();

//     // Run io_service in secondary thread
//     io_thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
// }

// TcpServer::~TcpServer()
// {
//     if (io_thread_)
//     {
//         io_service_.stop();
//         io_thread_->interrupt();
//         io_thread_->join();
//         delete io_thread_;
//     }
// }

// void TcpServer::start()
// {
//     // Wait until client is connected to our TCP server. (condition variable)
//     boost::unique_lock<boost::mutex> lock(connection_mutex_);

//     while (!client_connected_)
//     {
//         // LOG(INFO) << "Waiting for " << log_prefix_ << " client to establish connection...";

//         connection_cond_.wait(lock);
//     }

//     // LOG(INFO) << log_prefix_ << " client successfully connected.";
// }

// void TcpServer::write(std::string content)
// {
//     connection_->write(content);
// }

// void TcpServer::start_accept()
// {
//     // Create a new connection handler
//     connection_.reset(new TcpConnectionHandler(log_prefix_, acceptor_.get_io_service(), received_message_callback_));

//     // Asynchronous accept operation and wait for a new connection.
//     acceptor_.async_accept(connection_->socket(),
//                            boost::bind(&TcpServer::handle_accept, this, connection_,
//                            boost::asio::placeholders::error));

//     // LOG(DEBUG) << log_prefix_ << " communication object started asynchronous TCP/IP connection acceptance.";
// }

// void TcpServer::handle_accept(boost::shared_ptr<TcpConnectionHandler> connection,
//                               const boost::system::error_code &error)
// {
//     if (!error)
//     {
//         // LOG(INFO) << log_prefix_ << " client connected!";
//         connection->start();
//         boost::mutex::scoped_lock lock(connection_mutex_);
//         client_connected_ = true;
//         connection_cond_.notify_one();
//         // LOG(INFO) << log_prefix_ << " client connection accepted";
//     }

//     start_accept();
// }

// client_ptr create(Handler *h, int p)
// {
//     std::cout << "[info] | ByteStorm: Creating new server for client on port " << p << std::endl;
//     client_ptr new_(new ByteStormBoost(h, p));
//     return new_;
// }

// Acceptor ByteStormBoost::acc{ service, ip::tcp::endpoint(ip::tcp::v4(), kDefaultPort) };

// ByteStormBoost::ByteStormBoost(Handler *h, int p) : ByteStormBase(h, p), m_socket(service), timer(service) {}

// ByteStormBoost::~ByteStormBoost() {}

// Status ByteStormBoost::send(std::unique_ptr<std::uint8_t> data, const size_t size)
// {
//     if (size == 0) return Status::ErrorOnTx;
//     std::memcpy(txBuf.data(), data.get(), size);
//     m_socket.async_write_some(buffer(txBuf, size), bind(&ByteStormBoost::on_write, _1, _2));
//     this->flush();
//     return Status::OK;
// }

// void ByteStormBoost::read()
// {
//     async_read(m_socket, buffer(rxBuf), bind(&ByteStormBoost::on_read, _1, _2));
// }

// void ByteStormBoost::flush()
// {
//     boost::asio::streambuf b;
//     boost::asio::streambuf::mutable_buffers_type bufs = b.prepare(1024);
//     std::size_t bytes = m_socket.receive(bufs); // !!! This will block until some data becomes available
//     b.commit(bytes);
//     boost::asio::socket_base::bytes_readable command(true);
//     m_socket.io_control(command);

//     while (command.get())
//     {
//         bufs = b.prepare(1024);
//         bytes = m_socket.receive(bufs);
//         b.commit(bytes);
//         m_socket.io_control(command); // reset for bytes pending
//     }
//     return;
// }

// void ByteStormBoost::on_read(const error_code &err, size_t bytes)
// {
//     if (err)
//     {
//         std::cerr << "[info] | ByteStormBoost: Error is occured while reading with code: " << err << std::endl;
//         std::cerr << "[info] | ByteStormBoost: Stop this client with IP " << ip << std::endl;
//     }

//     if (bytes <= 0)
//     {
//         std::cerr << "[info] | ByteStormBoost: There is nothing to receive." << std::endl;
//         return;
//     }
//     if (handler == nullptr)
//     {
//         std::cerr << "[info] | ByteStormBoost: Handler is not set. Skipped query." << std::endl;
//         return;
//     }

//     std::cout << "[info] | ByteStormBoost: Bytes received: " << bytes << std::endl;

//     auto query = std::make_unique<std::uint8_t[]>(bytes);
//     memcpy(query.get(), rxBuf.data(), bytes);
//     handler->handle(query, bytes);
// }

// size_t ByteStormBoost::read_complete(const boost::system::error_code &err, size_t bytes)
// {
//     if (err) return 0;
//     bool found = std::find(rxBuf.begin(), rxBuf.begin() + bytes, '\n') != rxBuf.begin() + bytes;
//     // we read one-by-one until we get to enter, no buffering
//     return found ? 0 : 1;
// }

// void ByteStormBoost::on_connect()
// {
//     ip = m_socket.remote_endpoint().address().to_string();
//     std::cout << ">>[info] Client with IP " << ip << " is connected." << std::endl;
//     last = boost::posix_time::microsec_clock::local_time();
// }
