#include "client.h"

namespace NetworkStreamer
{
    TCPStreamer::TCPStreamer(const std::string& ip, int port) : m_ip(ip), m_port(port)
    {
        if ((m_fdClient = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::runtime_error("Failed to open socket!");

        m_servAddr.sin_family = AF_INET;
        m_servAddr.sin_port = htons(m_port);
        if (inet_pton(AF_INET, m_ip.c_str(), &m_servAddr.sin_addr) <= 0)
            throw std::runtime_error("\nInvalid address / Address not supported \n");
    }

    TCPStreamer::~TCPStreamer()
    {
        this->disconnect();
    }

    StatusReturn TCPStreamer::connectToServer()
    {
        if ((m_status = connect(m_fdClient, (struct sockaddr*)&m_servAddr, sizeof(m_servAddr))) < 0) 
            return StatusReturn::ConnectionError;
        return StatusReturn::Success;
    }

    void TCPStreamer::disconnect()
    {
        if (m_fdClient)
            close(m_fdClient);
    }

    StatusReturn TCPStreamer::sendData(const void *data, size_t dataSize)
    {
        if (send(m_fdClient, data, dataSize, 0) < 0)
            return StatusReturn::NetworkError;
        return StatusReturn::Success;
        
    }

    StatusReturn TCPStreamer::receiveData(uint8_t *userBuffer, size_t bytesToReceive)
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        m_valRead = read(m_fdClient, m_buffer, bytesToReceive); //recv
        if (m_valRead > 0) {
            memcpy(userBuffer, m_buffer, bytesToReceive);
            return StatusReturn::Success;
        }
        return StatusReturn::Success;
    }

    StatusReturn TCPStreamer::decodeData() 
    {
        return StatusReturn::Success;
    }

    StatusReturn TCPStreamer::getReceivedData(uint8_t *userBuffer)
    {
        // std::lock_guard<std::mutex> lock(m_bufferMutex);
        // // size = fifoBuffer.getDataSize()
        // // fifoBuffer.getData(userBuffer, size);
        // memcpy(userBuffer, m_buffer, sizeof(m_buffer));
        return StatusReturn::Success;
    }
}