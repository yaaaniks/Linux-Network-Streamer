#include "bytestorm_boost_test.hpp"

#include <memory>

using namespace ByteStorm;

TCPConnection::TCPConnection(io_service &service, size_t buffer_size, Handler *h) : ByteStormBase(h), timer(service) {}

TCPConnection::~TCPConnection()
{
    sock->close();
    delete sock;
    // Connections::iterator it = std::find(ByteStormBoost::connections.begin(), connections.end(), self);
    // clients.erase(it);
}

void TCPConnection::read()
{
    async_read(*sock,
               buffer(rx_buffer),
               boost::bind(&TCPConnection::read_complete, this, _1, _2),
               boost::bind(&TCPConnection::on_read, this, _1, _2));
}

size_t TCPConnection::read_complete(const Error &err, size_t bytes)
{
    if (err) return 0;
    bool found = std::find(rx_buffer.begin(), rx_buffer.begin() + bytes, '\n') != rx_buffer.begin() + bytes;
    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}

void TCPConnection::on_read(const Error &err, size_t bytes)
{
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    if ((now - last_ping).total_milliseconds() > 15000)
    {
        std::cout << "[warn] | TCPConnection: On " << ip
                  << " - no ping in time. Difference: " << (now - last_ping).total_milliseconds() << std::endl;
    }
    last_ping = boost::posix_time::microsec_clock::local_time();
    // this->onCheckPing();
}

void TCPConnection::onCheckPing()
{
    // timer.expires_from_now(boost::posix_time::millisec(5));
    // timer.async_wait(this->bind(&TCPConnection::onCheckPing));
}

void TCPConnection::write(std::unique_ptr<std::uint8_t> data, const size_t size)
{
    if (!connected || size == 0) return;
    std::memcpy(tx_buffer.data(), data.get(), size);
    sock->async_write_some(buffer(tx_buffer, size), boost::bind(&TCPConnection::on_write, this, _1, _2));
}

void TCPConnection::on_write(const Error &err, size_t bytes)
{
    this->flush();
}

void TCPConnection::on_connect()
{
    connected = true;
    ip = sock->remote_endpoint().address().to_string();
    std::cout << "[info] | TCPConnection: Client with IP " << ip << " is connected." << std::endl;
    this->last_ping = boost::posix_time::microsec_clock::local_time();
    this->read();
}

void TCPConnection::flush() {}
Status TCPConnection::send(std::unique_ptr<std::uint8_t> data, const size_t size) {}

ByteStormBoost::ByteStormBoost(int port, size_t buffer_size) : port(port)
{
    acc = new Acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), port));
}

ByteStormBoost::~ByteStormBoost()
{
    delete acc;
}

Connection ByteStormBoost::create(size_t buffer_size, Handler *h)
{
    Connection new_(new TCPConnection(service, buffer_size, h));
    return new_;
}

void ByteStormBoost::handle_accept(Connection connection, const Error &err, Handler *h)
{
    connection->on_connect();
    connections.push_back(connection);
    auto ptr = connection.get();
    if (h) { h->set(ptr); }
    auto new_client = ByteStormBoost::create(buffer_size, h);

    auto sock = new_client->sock = connection->getSocket();

    acc->async_accept(*sock, boost::bind(&ByteStormBoost::handle_accept, this, new_client, _1, h));
}
