#include "netlinux.h"

namespace NetLinux
{
    /********************************************Client********************************************/
    template<int bufferSize>
    NetworkClient<bufferSize>::NetworkClient(const std::string& ip, int port) : m_ip(ip), m_port(port)
    {
        if ((m_fdClient = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::runtime_error("Failed to open socket!");

        m_servAddr.sin_family = AF_INET;
        m_servAddr.sin_port = htons(m_port);
        if (inet_pton(AF_INET, m_ip.c_str(), &m_servAddr.sin_addr) <= 0)
            throw std::runtime_error("\nInvalid address / Address not supported \n");
    }

    template<int bufferSize>
    NetworkClient<bufferSize>::~NetworkClient()
    {
        this->disconnect();
    }

    template<int bufferSize>
    StatusReturn NetworkClient<bufferSize>::connectToServer()
    {
        if (connect(m_fdClient, (struct sockaddr*)&m_servAddr, sizeof(m_servAddr)) < 0) 
            return StatusReturn::ConnectionError;
        return StatusReturn::Success;
    }

    template<int bufferSize>
    void NetworkClient<bufferSize>::disconnect()
    {
        if (m_fdClient)
            close(m_fdClient);
    }

    template<int bufferSize>
    StatusReturn NetworkClient<bufferSize>::sendData(const void *data, size_t dataSize)
    {
        if (send(m_fdClient, data, dataSize, 0) < 0)
            return StatusReturn::NetworkError;
        return StatusReturn::Success;
        
    }

    template<int bufferSize>
    StatusReturn NetworkClient<bufferSize>::receiveData(uint8_t *userBuffer, size_t bytesToReceive)
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        int valRead = read(m_fdClient, m_rxBuffer, bytesToReceive); //recv
        if (valRead > 0) {
            memcpy(userBuffer, m_rxBuffer, bytesToReceive);
            return StatusReturn::Success;
        }
        return StatusReturn::Success;
    }

    /********************************************Server********************************************/

    template <int bufferSize>
    NetworkServer<bufferSize>::NetworkServer(NetHandlerInterface *neth, int port) : m_neth(neth), m_port(port)
    {
        m_servAddr.sin_family = AF_INET;
        m_servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_servAddr.sin_port = htons(port);
        m_serverSd = socket(AF_INET, SOCK_STREAM, 0);

        if (m_serverSd < 0)
            throw std::runtime_error("StatusReturn establishing the server socket");

        this->bindServer();
        this->listenPorts(1);
    }

    template<int bufferSize>
    NetworkServer<bufferSize>::~NetworkServer()
    {
        if (this->isRunning())
            this->join();
        if (m_newSd)
            close(m_newSd);
        if (m_serverSd)
            close(m_serverSd);
    }

    template <int bufferSize>
    StatusReturn NetworkServer<bufferSize>::receivePacket()
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        ssize_t bytesRead = recv(m_newSd, m_rxBuffer, sizeof(StandDataPacket), 0);
        if (bytesRead < 0) {
            return StatusReturn::NetworkError;
        } else {
            switch (reinterpret_cast<StandDataPacket*>(m_rxBuffer)->common.message)
            {
                case DATA_EXIST:
                {
                    m_neth->rxHandle(&m_rxBuffer, sizeof(m_rxBuffer));
                    return StatusReturn::Success;
                }
                case NO_DATA_EXIST:
                {
                    return StatusReturn::NoData;
                }
            }
        }
    }

    template<int bufferSize>
    void NetworkServer<bufferSize>::run() 
    {
        while (m_isRunning) {
            StatusReturn status = this->receivePacket();
            if (StatusReturn::Success == status)
                m_neth->txHandle(reinterpret_cast<uint8_t*>(&status));
            else 
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    template<int bufferSize>
    void NetworkServer<bufferSize>::startRx(void)
    {
        this->start();
    }
    
    template<int bufferSize>
    void NetworkServer<bufferSize>::stopRx(void)
    {
        this->stop();
    }

    template<int bufferSize>
    StatusReturn NetworkServer<bufferSize>::listenPorts(int countRequests)
    {
        listen(m_serverSd, countRequests);
        sockaddr_in newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);
        m_newSd = accept(m_serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
        if (m_newSd < 0)
            return StatusReturn::SocketError;
        return StatusReturn::Success;
    }

    template<int bufferSize>
    StatusReturn NetworkServer<bufferSize>::bindServer()
    {
        int bindStatus = bind(m_serverSd, (struct sockaddr*) &m_servAddr, sizeof(m_servAddr));
        if (bindStatus < 0)
            return StatusReturn::SocketError;
        return StatusReturn::Success;
    }

    template<int bufferSize>
    StatusReturn NetworkServer<bufferSize>::sendData(const void *data, size_t dataSize)
    {
        ssize_t bytesWritten = send(m_newSd, data, dataSize, 0);
        if (bytesWritten < 0)
            return StatusReturn::ResourceError;
    }

    template<int bufferSize>
    void NetworkServer<bufferSize>::start() 
    {
        if (!m_isRunning) {
            m_thread = std::thread(&NetworkServer::run, this);
            m_isRunning = true;
        }
    }

    template<int bufferSize>
    void NetworkServer<bufferSize>::stop() 
    {
        if (m_isRunning) {
            m_thread.join();
            m_isRunning = false;
        }
    }
}