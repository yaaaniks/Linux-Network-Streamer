#include "bytestorm_unix.hpp"

#include <iostream>

using namespace ByteStorm;

ByteStormUnix::ByteStormUnix(ProcessorBase<ByteStormUnix> *processor, int p, int bufferSize)
    : ByteStormBase<ByteStormUnix>(processor)
{
    if (bufferSize <= 0)
    {
        bufferSize = kDefaultBufferSize;
        std::cout << "[info] | ByteStormUnix: Setting default buffer size - " << kDefaultBufferSize;
    }

    m_servAddr.sin_family = AF_INET;
    m_servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_servAddr.sin_port = htons(p);
    serverDescription = socket(AF_INET, SOCK_STREAM, 0);

    if (serverDescription < 0) { throw std::runtime_error("Error establishing the server socket"); }
    buffer.resize(bufferSize);
}

ByteStormUnix::~ByteStormUnix() {}

Status ByteStormUnix::send(std::unique_ptr<std::uint8_t[]> &data, const size_t size)
{
    ssize_t bytesWritten = ::send(clientDescription, data.release(), size, 0);
    if (bytesWritten < 0) { return Status::ErrorOnTx; }
    return Status::OK;
}

void ByteStormUnix::flush() {}

Status ByteStormUnix::receive(size_t size)
{
    std::lock_guard<std::mutex> lock(mutex);
    bytesRx = recv(clientDescription, &buffer[0], size, 0);
    if (bytesRx <= 0) { return Status::ErrorOnRx; }
    return Status::OK;
}

Status ByteStormUnix::listen(int count)
{
    int ret = ::listen(serverDescription, count);
    if (!ret) {} // TODO: add handling
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    clientDescription = ::accept(clientDescription, reinterpret_cast<sockaddr *>(&newSockAddr), &newSockAddrSize);
    if (clientDescription < 0) { return Status::ErrorOnBind; }
    return Status::OK;
}

Status ByteStormUnix::bind()
{
    int flag = 1;
    int status = (::bind(serverDescription, reinterpret_cast<sockaddr *>(&m_servAddr), sizeof(m_servAddr)) &&
                  setsockopt(serverDescription, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)));
    if (status < 0) { return Status::ErrorOnBind; }
    return Status::OK;
}

Status ByteStormUnix::enableKeepAlive()
{
    int flag = 1;
    if (setsockopt(serverDescription, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) == -1)
    {
        return Status::ErrorOnBind;
    }
    if (setsockopt(serverDescription, IPPROTO_TCP, TCP_KEEPIDLE, &kaConfig.ka_idle, sizeof(kaConfig.ka_idle)) == -1)
    {
        return Status::ErrorOnBind;
    }
    if (setsockopt(serverDescription, IPPROTO_TCP, TCP_KEEPINTVL, &kaConfig.ka_intvl, sizeof(kaConfig.ka_intvl)) == -1)
    {
        return Status::ErrorOnBind;
    }
    if (setsockopt(serverDescription, IPPROTO_TCP, TCP_KEEPCNT, &kaConfig.ka_cnt, sizeof(kaConfig.ka_cnt)) == -1)
    {
        return Status::ErrorOnBind;
    }
    return Status::OK;
}
