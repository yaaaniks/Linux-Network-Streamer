// #include <iostream>
// #include <stdint.h>
// #include <boost/noncopyable.hpp>
// #include <boost/bind/bind.hpp>
// #include <boost/asio.hpp>
// #include <boost/array.hpp>
// #include <arpa/inet.h>
// #include <boost/shared_ptr.hpp>
// #include <boost/enable_shared_from_this.hpp>

// #include "NetHandlerInterface.hpp"

// enum class ServerCommand : uint8_t {
//     SetFSR = 0,
//     SetSPS,
//     SetADCMode,
//     SetAzimuth,
//     SetElevation,
//     ResetAzimuth,
//     ResetElevation,
//     AutoCalibration,
//     GetData,
//     EndPacket
// };

// enum class Message : uint32_t {
//     ConnectionAccepted = 0,
//     Start,
//     Ping,
//     DataExist = 0xAA,
//     NoDataExist = 0xFF,
//     ErrorOccur = 0xBB,
// };

// typedef struct {
//     uint32_t header;
//     Message message;
//     uint32_t err;
//     uint32_t timeStamp;
// } CommonPacket;

// using namespace boost::asio;
// using namespace boost::posix_time;
// using namespace boost::placeholders;

// namespace ByteStorm 
// {
//     io_service service;
//     ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8050));

//     class ByteStormServer;
//     void updateClientList();

//     template <typename Server_t>
//     void handleAccept(Server_t client, const boost::system::error_code &err, NetHandlerInterface<Server_t> *neth); 

//     typedef boost::shared_ptr<ByteStormServer> client_ptr;
//     typedef std::vector<client_ptr> ClientVector;
//     ClientVector clients;

//     class ByteStormServer : public boost::enable_shared_from_this<ByteStormServer>, boost::noncopyable 
//     {
//         typedef ByteStormServer me;
//         typedef boost::system::error_code error_code;
//     public:
//         ByteStormServer(NetHandlerInterface<client_ptr> *neth, size_t bufferSize) : 
//             m_neth(neth), 
//             m_bufferSize(bufferSize),
//             m_socket(service), 
//             m_started(false), 
//             m_deadlineTimer(service), 
//             clientListChanged(false) 
//         {

//         }

//         ByteStormServer(NetHandlerInterface<client_ptr> *neth) :  
//             m_neth(neth),
//             // m_bufferSize(bufferSize), 
//             m_socket(service), 
//             m_started(false), 
//             m_deadlineTimer(service), 
//             clientListChanged(false) 
//         {
//         }

//         ~ByteStormServer()
//         {
//             // delete m_rxBuffer;
//         }

//         template<typename Server_t>
//         friend void handleAccept(Server_t client, const boost::system::error_code &err, NetHandlerInterface<client_ptr> *neth);

//         void start() {
//             m_started = true;
//             clients.push_back(shared_from_this());
//             m_lastPing = boost::posix_time::microsec_clock::local_time();
//             // first, we wait for client to login
//             read_();
//         }

//         static client_ptr new_(NetHandlerInterface<client_ptr> *m_neth) {
//             client_ptr new_(new ByteStormServer(m_neth));
//             return new_;
//         }
        
//         void stop() {
//             if (!m_started) 
//                 return;
//             m_started = false;
//             m_socket.close();

//             client_ptr self = shared_from_this();
//             ClientVector::iterator it = std::find(clients.begin(), clients.end(), self);
//             clients.erase(it);
//             updateClientList();
//         }

//         void sendData(const void *data, const size_t size)
//         {
//             if (data)
//                 this->write_(data, size);
//         }

//         bool started() const __attribute__((warn_unused_result)) { return m_started; }
//         ip::tcp::socket &sock() { return m_socket; }
//         NetHandlerInterface<client_ptr> *netHandler() { return m_neth; }
//         void set_clients_changed() { clientListChanged = true; }
//     private:
//         template <typename Fn, typename... Args>
//         auto bindMemberFunction(Fn&& fn, Args&&... args) {
//             return boost::bind(std::forward<Fn>(fn), shared_from_this(), std::forward<Args>(args)...);
//         }

//         void onRead_(const error_code & err, size_t bytes) {
//             if (err) { 
//                 std::cout << ">>[info] Error is occured while reading with code: " << err << std::endl;
//                 std::cout << ">>[info] Stop this client with IP " << clientIP << std::endl;
//                 stop();
//             }
//             if (!started()) { 
//                 return;
//             }

//             if (bytes < 0) {
//                 std::cout << ">>[info] Bytes received: " << bytes << std::endl;
//                 return;
//             }

//             std::shared_ptr<CommonPacket> common = std::make_shared<CommonPacket>();
//             std::memcpy(common.get(), m_rxBuffer.data(), sizeof(CommonPacket));

//             if (common->message == Message::ConnectionAccepted)
//                 this->onConnect_(common->header);

//             if ((common->message == Message::DataExist || 
//                  common->message == Message::NoDataExist) && m_neth != nullptr) {
//                 m_neth->rxHandle(reinterpret_cast<uint8_t*>(common.get()), sizeof(CommonPacket));
//             }
            
//             else if (common->message == Message::ErrorOccur)
//                 std::cout << ">>[info] Error on client side is occured. Error code: " << std::hex << common->err << std::endl;

//             else if (common->message == Message::Ping)
//                 this->onPing_();

//             else 
//                 std::cerr << ">>[info] Invalid message: " << static_cast<uint32_t>(common->message) << " or NetHandler is not set. Skipped query." <<  std::endl;
//         }


//         void onConnect_(uint32_t header) {
//             clientIP = m_socket.remote_endpoint().address().to_string();
//             std::cout <<  clientIP << " is connected." << std::endl;
//             std::shared_ptr<CommonPacket> requestAnswer = std::make_shared<CommonPacket>();
//             m_lastPing = boost::posix_time::microsec_clock::local_time();;
//             m_netHeader = requestAnswer->header = header;
//             requestAnswer->err = 0;
//             requestAnswer->message = Message::Start;
//             write_(&requestAnswer, sizeof(CommonPacket));
//             updateClientList();
//         }

//         void onPing_() {
//             clientListChanged = false;
//         }

//         void ping_() {
//             auto common = new CommonPacket;
//             common->err = 0;
//             common->header = 0xABCDEFFF;
//             common->message = Message::Ping;
//             write_(common, sizeof(CommonPacket));
//             delete common;
//         }

//         void onCheckPing_() {
//             boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
//             if ((now - m_lastPing).total_milliseconds() > 6000) {
//                 std::cout << ">>[info] Stopping " << clientIP << " - no ping in time. Difference ping: " << (now - m_lastPing).total_milliseconds() << std::endl;
//                 stop();
//             }
//             m_lastPing = boost::posix_time::microsec_clock::local_time();
//         }

//         void postCheckPing_() {
//             m_deadlineTimer.expires_from_now(boost::posix_time::millisec(5));
//             m_deadlineTimer.async_wait(bindMemberFunction(&me::onCheckPing_));
//         }

//         void onWrite(const error_code & err, size_t bytes) {
//             read_();
//         }

//         void read_() {
//             async_read(m_socket, buffer(m_rxBuffer), bindMemberFunction(&me::readComplete_,_1,_2), bindMemberFunction(&me::onRead_,_1,_2));
//             postCheckPing_();
//         }
//         void write_(const void *data, const uint32_t size) {
//             if (!started() || size == 0) 
//                 return;
//             std::memcpy(m_txBuffer.data(), data, size);
//             m_socket.async_write_some(buffer(m_txBuffer, size), bindMemberFunction(&me::onWrite,_1,_2));
//             this->flush_socket();
//         }

//         void flush_socket()
//         {
//             boost::asio::streambuf b;
//             boost::asio::streambuf::mutable_buffers_type bufs = b.prepare(1024);
//             std::size_t bytes = m_socket.receive(bufs); // !!! This will block until some data becomes available
//             b.commit(bytes);
//             boost::asio::socket_base::bytes_readable command(true);
//             m_socket.io_control(command); 

//             while(command.get())
//             {
//                 bufs = b.prepare(1024);
//                 bytes = m_socket.receive(bufs);
//                 b.commit(bytes);
//                 m_socket.io_control(command); // reset for bytes pending
//             }
//             return;
//         }

//         size_t readComplete_(const boost::system::error_code &err, size_t bytes) {
//             if (err) 
//                 return 0;
//             bool found = std::find(m_rxBuffer.begin(), m_rxBuffer.begin() + bytes, '\n') != m_rxBuffer.begin() + bytes;
//             // we read one-by-one until we get to enter, no buffering
//             return found ? 0 : 1;
//         }
//     private:
//         std::string clientIP;
//         ip::tcp::socket m_socket;
//         std::atomic<bool> m_started {false};
//         deadline_timer m_deadlineTimer;
//         boost::posix_time::ptime m_lastPing;
//         boost::array<uint8_t, 1024> m_rxBuffer, m_txBuffer;
//         NetHandlerInterface<client_ptr> *m_neth;
//         uint32_t m_netHeader {0};
//         int m_bufferSize {0};
//         bool clientListChanged;
//     };

//     void updateClientList() {
//         for( ClientVector::iterator b = clients.begin(), e = clients.end(); b != e; ++b)
//             (*b)->set_clients_changed();
//     }

//     template <typename Server_t>
//     void handleAccept(Server_t client, const boost::system::error_code &err, NetHandlerInterface<Server_t> *neth) {
//         if (!err) {
//             std::cout << ">>[info] Client connection is accepted with IP " << client->clientIP << ". Receiving is starting" << std::endl;
//             client->start();
//             neth->setServer(client);
//         } else {

//         }
//         ByteStorm::client_ptr new_client = ByteStormServer::new_(neth);
//         // new_client->sock() = ip::tcp::socket(service);
//         acceptor.async_accept(new_client->sock(), boost::bind(ByteStorm::handleAccept<Server_t>, new_client, _1, neth));
//     }
// };

#include <iostream>
#include <stdint.h>
#include <boost/noncopyable.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <arpa/inet.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "NetHandlerInterface.hpp"

#define ADC_CHANNEL_COUNT   4

using namespace boost::asio;
using namespace boost::posix_time;
using namespace boost::placeholders;

namespace ByteStorm 
{
    io_service service;

    class ByteStormServer;
    
    template <typename ServerType>
    class Test;
    void update_clients_changed();

    typedef boost::shared_ptr<ByteStormServer> client_ptr;
    typedef std::vector<client_ptr> ClientVector;
    
    enum class Message : uint32_t {
        ConnectionAccepted = 0,
        Start,
        Ping,
        DataExist = 0xAA,
        NoDataExist = 0xFF,
        ErrorOccur = 0xBB,
    };

    typedef struct {
        uint32_t header;
        Message message;
        uint32_t err;
        uint32_t timeStamp;
    } CommonPacket;

    ClientVector clients;

    class ByteStormServer : public boost::enable_shared_from_this<ByteStormServer>, boost::noncopyable 
    {
        typedef ByteStormServer me;
    public:
        typedef boost::system::error_code error_code;
        typedef boost::shared_ptr<ByteStormServer> ptr;

        template <typename Fn, typename... Args>
        auto bindMemberFunction(Fn&& fn, Args&&... args) {
            return boost::bind(std::forward<Fn>(fn), shared_from_this(), std::forward<Args>(args)...);
        }

        ByteStormServer(NetHandlerInterface<client_ptr> *neth, size_t bufferSize) : 
            m_neth(neth), 
            m_bufferSize(bufferSize),
            m_socket(service), 
            m_started(false), 
            m_deadlineTimer(service), 
            clientListChanged(false) 
        {
        }

        ByteStormServer(NetHandlerInterface<client_ptr> *neth) :  
            m_neth(neth),
            // m_bufferSize(bufferSize), 
            m_socket(service), 
            m_started(false), 
            m_deadlineTimer(service), 
            clientListChanged(false) 
        {
        }

        ~ByteStormServer()
        {
            // delete m_rxBuffer;
        }
        void start() {
            m_started = true;
            clients.push_back( shared_from_this());
            m_lastPing = boost::posix_time::microsec_clock::local_time();
            // first, we wait for client to login
            read_();
        }
        static client_ptr new_(NetHandlerInterface<client_ptr> *m_neth) {
            client_ptr new_(new ByteStormServer(m_neth));
            return new_;
        }
        void stop() {
            if (!m_started) 
                return;
            m_started = false;
            m_socket.close();

            client_ptr self = shared_from_this();
            ClientVector::iterator it = std::find(clients.begin(), clients.end(), self);
            clients.erase(it);
            update_clients_changed();
        }

        void sendData(const void *data, const size_t size)
        {
            this->write_(data, size);
        }

        bool started() const __attribute__((warn_unused_result)) { return m_started; }
        ip::tcp::socket &sock() { return m_socket; }
        NetHandlerInterface<client_ptr> *netHandler() { return m_neth; }
        void set_clients_changed() { clientListChanged = true; }
    private:
        // friend void handle_accept(ByteStormServer::ptr client, const boost::system::error_code &err, Test<client_ptr> *neth);
        void onRead_(const error_code & err, size_t bytes) {
            if (err) { 
                std::cout << ">>[info] Error is occured while reading with code: " << err << std::endl;
                std::cout << ">>[info] Stop this client with IP " << clientIP << std::endl;
                stop();
            }
            if (!started()) { 
                return;
            }

            if (bytes < 0) {
                std::cout << ">>[info] Bytes received: " << bytes << std::endl;
                return;
            }

            CommonPacket common;
            std::memcpy(&common, m_rxBuffer.data(), sizeof(CommonPacket));

            if (common.message == Message::ConnectionAccepted)
                this->onConnect_(common.header);

            if ((common.message == Message::DataExist || 
                 common.message == Message::NoDataExist) && m_neth != nullptr) {
                m_neth->rxHandle(reinterpret_cast<uint8_t*>(&common), sizeof(CommonPacket));
            }
            
            else if (common.message == Message::ErrorOccur)
                std::cout << ">>[info] Error on client side is occured. Error code: " << std::hex << common.err << std::endl;

            else if (common.message == Message::Ping)
                this->onPing_();

            else 
                std::cerr << ">>[info] Invalid message: " << static_cast<uint32_t>(common.message) << " or NetHandler is not set. Skipped query." <<  std::endl;
        }

        void onConnect_(uint32_t header) {
            clientIP = m_socket.remote_endpoint().address().to_string();
            std::cout <<  clientIP << " is connected." << std::endl;
            auto serverPacket = new CommonPacket;
            m_lastPing = boost::posix_time::microsec_clock::local_time();;
            m_netHeader = 0xABCDEFFF; 
            serverPacket->header = 0xABCDEFFF;
            serverPacket->err = 0;
            serverPacket->message = Message::Start;
            write_(&serverPacket, sizeof(CommonPacket));
            update_clients_changed();
        }
        void onPing_() {
            clientListChanged = false;
        }

        void ping_() {
            auto netPacket = new CommonPacket;
            netPacket->err = 0;
            netPacket->header = 0xABCDEFFF;
            netPacket->message = Message::Ping;
            write_(netPacket, sizeof(CommonPacket));
            delete netPacket;
        }

        void onCheckPing_() {
            boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
            if ((now - m_lastPing).total_milliseconds() > 6000) {
                std::cout << ">>[INFO] Stopping " << clientIP << " - no ping in time. Difference ping: " << (now - m_lastPing).total_milliseconds() << std::endl;
                stop();
            }
            m_lastPing = boost::posix_time::microsec_clock::local_time();
        }

        void postCheckPing_() {
            m_deadlineTimer.expires_from_now(boost::posix_time::millisec(5));
            m_deadlineTimer.async_wait(bindMemberFunction(&me::onCheckPing_));
        }

        void onWrite_(const error_code &err, size_t bytes) {
            read_();
        }

        void read_() {
            async_read(m_socket, buffer(m_rxBuffer), bindMemberFunction(&me::read_complete,_1,_2), bindMemberFunction(&me::onRead_,_1,_2));
            postCheckPing_();
        }
        void write_(const void *data, const uint32_t size) {
            if (!started() || size == 0) 
                return;
            std::memcpy(m_txBuffer.data(), data, size);
            m_socket.async_write_some(buffer(m_txBuffer, size), bindMemberFunction(&me::onWrite_,_1,_2));
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

            while(command.get())
            {
                bufs = b.prepare(1024);
                bytes = m_socket.receive(bufs);
                b.commit(bytes);
                m_socket.io_control(command); // reset for bytes pending
            }
            return;
        }

        size_t read_complete(const boost::system::error_code &err, size_t bytes) {
            if (err) 
                return 0;
            bool found = std::find(m_rxBuffer.begin(), m_rxBuffer.begin() + bytes, '\n') != m_rxBuffer.begin() + bytes;
            // we read one-by-one until we get to enter, no buffering
            return found ? 0 : 1;
        }
    private:
        std::string clientIP;
        uint32_t m_netHeader {0};
        NetHandlerInterface<client_ptr> *m_neth;
        ip::tcp::socket m_socket;
        int m_bufferSize {0};
        boost::array<uint8_t, 1024> m_rxBuffer;
        boost::array<uint8_t, 1024> m_txBuffer;
        std::atomic<bool> m_started {false};
        std::string username_;
        deadline_timer m_deadlineTimer;
        boost::posix_time::ptime m_lastPing;
        bool clientListChanged;
    };

    void update_clients_changed() {
        for( ClientVector::iterator b = clients.begin(), e = clients.end(); b != e; ++b)
            (*b)->set_clients_changed();
    }

    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8055));

    template <typename ServerType>
    void handle_accept(ServerType client, const boost::system::error_code &err, NetHandlerInterface<ServerType> *neth) {
        client->start();
        neth->setServer(client);
        ByteStormServer::ptr new_client = ByteStormServer::new_(neth);
        // new_client->sock() = ip::tcp::socket(service);
        acceptor.async_accept(new_client->sock(), boost::bind(ByteStorm::handle_accept<ServerType>, new_client, _1, neth));
    }
}; // ByteStorm