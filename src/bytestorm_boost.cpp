#include "bytestorm_boost.hpp"

#include <iostream>
#include <memory>

using namespace ByteStorm;

TCPConnection::TCPConnection(io_service &service, size_t transfer_size, ProcessorBase<TCPConnection> *processor)
    : ByteStormBase(processor),
      timer(service),
      sock(service),
      transfer_size(transfer_size)
{}

TCPConnection::~TCPConnection()
{
    sock.close();
}

void TCPConnection::read()
{
    boost::asio::socket_base::bytes_readable command(true);
    sock.io_control(command);
    std::size_t bytes_readable = command.get();
    async_read(sock, read_buffer, transfer_at_least(transfer_size), this->bind(&TCPConnection::read_complete, _1, _2));
}

void TCPConnection::read_complete(const Error &err, size_t bytes_transferred)
{
    auto ptr = std::make_unique<std::uint8_t[]>(bytes_transferred);
    std::memcpy(ptr.get(), read_buffer.data().data(), bytes_transferred);

    std::cout << "[info] | TCPConnection: Bytes received: " << bytes_transferred << std::endl;
    if (processor) { processor->process(ptr, bytes_transferred); }

    this->check();
    read_buffer.consume(bytes_transferred);

    read();
}

void TCPConnection::write(std::unique_ptr<std::uint8_t[]> &data, const size_t size)
{
    if (!connected || size == 0) { return; }
    std::cout << "[info] | TCPConnection: Preparing sending query with size: " << size << std::endl;
    std::ostream os{ &write_buffer }; 
    
    for (int i = 0; i < size; i++) { os << data[i]; }   
    data.release();
    
    async_write(sock, write_buffer, transfer_exactly(size), this->bind(&TCPConnection::write_complete, _1, _2));
}

void TCPConnection::write_complete(const Error &err, size_t bytes_transferred)
{
    std::cout << "[info] | TCPConnection: Bytes sent: " << bytes_transferred << std::endl;
    if (bytes_transferred) { write_buffer.consume(bytes_transferred); }
    this->flush();
}

Status TCPConnection::send(std::unique_ptr<std::uint8_t[]> &data, const size_t size)
{
    this->write(data, size);
    return Status::OK;
}

void TCPConnection::check()
{
    // timer.expires_from_now(boost::posix_time::millisec(5));
    // timer.async_wait(this->bind(&TCPConnection::on_check));
}

void TCPConnection::on_check()
{
    // boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    // if ((now - last_ping).total_milliseconds() > 15000)
    // {
    //     std::cout << "[warn] | TCPConnection: On " << ip
    //               << " - no ping in time. Difference: " << (now - last_ping).total_milliseconds() << std::endl;
    // }
    // last_ping = boost::posix_time::microsec_clock::local_time();
}

void TCPConnection::on_connect()
{
    connected = true;
    ip = sock.remote_endpoint().address().to_string();
    std::cout << "[info] | TCPConnection: Client with IP " << ip << " is connected." << std::endl;
    this->last_ping = boost::posix_time::microsec_clock::local_time();
    this->read();
}

// TODO: add method
void TCPConnection::flush() {}

ByteStormBoost::ByteStormBoost(int port, size_t buffer_size) : port(port), buffer_size(buffer_size)
{
    acc = new Acceptor(service, Endpoint(ip::tcp::v4(), port));
}

ByteStormBoost::~ByteStormBoost()
{
    delete acc;
}

Connection ByteStormBoost::create(size_t buffer_size, ProcessorBase<TCPConnection> *processor)
{
    Connection new_(new TCPConnection(service, buffer_size, processor));
    return new_;
}

void ByteStormBoost::on_delete_connection(Connection &connection)
{
    std::cout << "[info] | ByteStromServer: Closing connection with IP " << connection->ip << std::endl;
    Connections::iterator it = std::find(ByteStormBoost::connections.begin(), connections.end(), connection);
    connections.erase(it);
}

void ByteStormBoost::handle_accept(Connection &connection, const Error &err, ProcessorBase<TCPConnection> *processor)
{
    connection->on_connect();

    connections.push_back(connection);

    auto ptr = connection.get();

    if (processor) { processor->set(ptr); }

    // connection->close.connect(boost::bind(&ByteStormBoost::on_delete_connection, this, connection));

    auto new_client = ByteStormBoost::create(buffer_size, processor);

    acc->async_accept(new_client->sock, boost::bind(&ByteStormBoost::handle_accept, this, new_client, _1, processor));
}
