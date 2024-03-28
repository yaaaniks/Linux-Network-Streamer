// #include "bytestorm_boost.hpp"

// using namespace ByteStorm;

// client_ptr create(HandlerInterface<client_ptr> *neth)
// {
//     std::cout << "[info]: Creating server for new client" << std::endl;
//     client_ptr new_cp(new ByteStorm(neth));

//     return new_cp;
// }

// ByteStormBoost::ByteStormBoost(HandlerInterface<client_ptr> *h, int p, io_service &service)
//     : ByteStormBase(h, p),
//       m_socket(service),
//       m_deadlineTimer(service),
//       acceptor( service, ip::tcp::endpoint(ip::tcp::v4(), kPortNumber) )
// {
//     m_started = false;
//     clientListChanged = false;
// }

// ByteStorm::~ByteStorm() {}

// void ByteStorm::start()
// {
//     m_started = true;
//     clients.push_back(shared_from_this());
//     this->onConnect_();
//     m_lastPing = boost::posix_time::microsec_clock::local_time();
//     // first, we wait for client to login
//     read_();
// }

// void ByteStorm::stop()
// {
//     if (!m_started) return;
//     m_started = false;
//     m_socket.close();

//     client_ptr self = shared_from_this();
//     client_vector::iterator it = std::find(clients.begin(), clients.end(), self);
//     clients.erase(it);
//     updateClientList();
// }

// void ByteStorm::onRead_(const error_code &err, size_t bytes)
// {
//     if (err)
//     {
//         std::cerr << ">>[info] Error is occured while reading with code: " << err << std::endl;
//         std::cerr << ">>[info] Stop this client with IP " << clientIP << std::endl;
//         stop();
//     }
//     if (!started()) { return; }

//     if (bytes <= 0)
//     {
//         std::cerr << ">>[info] There is nothing to receive." << std::endl;
//         return;
//     }
//     if (m_neth == nullptr)
//     {
//         std::cerr << ">>[info] NetHandler is not set. Skipped query." << std::endl;
//         return;
//     }

//     std::cout << ">>[info] Bytes received: " << bytes << std::endl;
//     m_neth->handle(reinterpret_cast<uint8_t *>(m_rxBuffer.data()), bytes);
// }

// void ByteStorm::onConnect_()
// {
//     clientIP = m_socket.remote_endpoint().address().to_string();
//     std::cout << ">>[info] Client with IP " << clientIP << " is connected." << std::endl;
//     m_lastPing = boost::posix_time::microsec_clock::local_time();
//     updateClientList();
// }

// void ByteStorm::onCheckPing_()
// {
//     boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
//     if ((now - m_lastPing).total_milliseconds() > 15000)
//     {
//         std::cout << ">>[INFO] Stopping " << clientIP
//                   << " - no ping in time. Difference between pings: " << (now - m_lastPing).total_milliseconds()
//                   << std::endl;
//         stop();
//     }
//     m_lastPing = boost::posix_time::microsec_clock::local_time();
// }

// void ByteStorm::postCheckPing_()
// {
//     m_deadlineTimer.expires_from_now(boost::posix_time::millisec(5));
//     m_deadlineTimer.async_wait(bindMemberFunction(&me::onCheckPing_));
// }
