#ifndef NETLINUX_H
#define NETLINUX_H

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <stdint.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>

#include "SocketData.h"
#include "NetHandlerInterface.hpp"

namespace NetLinux
{

    using std::vector;
    using std::string;
    using std::thread;
    using std::mutex;
    using std::atomic_bool;
    using std::size_t;

    enum StatusReturn {
        Success,
        NoData,
        ConnectionError,
        SocketError,
        NetworkError,
        DataFormatError,
        TimeoutError,
        ResourceError,
        NetConfigurationStatusReturn
    };
    
    /********************************************Client********************************************/
    template<int bufferSize>
    class NetworkClient
    {
    private:
        int m_status, m_valRead, m_port, m_fdClient = 0;
        std::string m_ip;
        struct sockaddr_in m_servAddr;
        uint8_t m_rxBuffer[sizeof(StandDataPacket)];
        std::mutex m_bufferMutex;
        
        void run()
        {

        }

        StatusReturn receiveData(uint8_t *userBuffer, size_t bytesToReceive)
        {
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            int valRead = read(m_fdClient, m_rxBuffer, bytesToReceive); //recv
            if (valRead > 0) {
                memcpy(userBuffer, m_rxBuffer, bytesToReceive);
                return StatusReturn::Success;
            }
            return StatusReturn::Success;
        }
    public:
        explicit NetworkClient(const std::string& ip, int port) : m_ip(ip), m_port(port)
        {
            if ((m_fdClient = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                throw std::runtime_error("Failed to open socket!");

            m_servAddr.sin_family = AF_INET;
            m_servAddr.sin_port = htons(m_port);
            if (inet_pton(AF_INET, m_ip.c_str(), &m_servAddr.sin_addr) <= 0)
                throw std::runtime_error("\nInvalid address / Address not supported \n");
        }
        
        ~NetworkClient()
        {
            this->disconnect();
        }
        
        StatusReturn connectToServer()
        {
            if (connect(m_fdClient, (struct sockaddr*)&m_servAddr, sizeof(m_servAddr)) < 0) 
                return StatusReturn::ConnectionError;
            return StatusReturn::Success;
        }

        void disconnect()
        {
            if (m_fdClient)
                close(m_fdClient);
        }
        
        void startRx()
        {
            this->start();
        }

        void stopRx()
        {
            this->stop();
        }

        StatusReturn sendData(const void *data, size_t dataSize)
        {
            if (send(m_fdClient, data, dataSize, 0) < 0)
                return StatusReturn::NetworkError;
            return StatusReturn::Success;

        }

        StatusReturn getReceivedData(uint8_t *userBuffer);

    protected:
        void start() 
        {
            if (!m_isRunning) {
                m_thread = std::thread(&NetworkServer<bufferSize>::run, this);
                m_isRunning = true;
            }
        }

        
        void stop() 
        {
            if (m_isRunning) {
                m_thread.join();
                m_isRunning = false;
            }
        }
    };

    /********************************************Server********************************************/
    template<int bufferSize>
    class NetworkServer 
    {
    private:
        int m_port, m_serverSd, m_newSd, m_fdClient {0};
        struct sockaddr_in m_servAddr;
        NetHandlerInterface *m_neth;
        uint8_t m_rxBuffer[sizeof(StandDataPacket)];

        std::string m_ip;
        std::mutex m_bufferMutex;
        std::thread m_thread;
        std::atomic_bool m_isRunning {false};

        StatusReturn receivePacket()
        {
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            ssize_t bytesRead = recv(m_newSd, m_rxBuffer, sizeof(StandDataPacket), 0);
            if (bytesRead < 0) {
                return StatusReturn::NetworkError;
            } 
            switch (reinterpret_cast<StandDataPacket*>(m_rxBuffer)->common.message)
            {
                case DATA_EXIST:
                {
                    m_neth->rxHandle(m_rxBuffer, sizeof(m_rxBuffer));
                    break;
                }
                case NO_DATA_EXIST:
                {
                    return StatusReturn::NoData;
                }
            }
            return StatusReturn::Success;
        }

        
        StatusReturn listenPorts(int countRequests)
        {
            listen(m_serverSd, countRequests);
            sockaddr_in newSockAddr;
            socklen_t newSockAddrSize = sizeof(newSockAddr);
            m_newSd = accept(m_serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
            if (m_newSd < 0)
                return StatusReturn::SocketError;
            return StatusReturn::Success;
        }

        
        StatusReturn bindServer()
        {
            int bindStatus = bind(m_serverSd, (struct sockaddr*) &m_servAddr, sizeof(m_servAddr));
            if (bindStatus < 0)
                return StatusReturn::SocketError;
            return StatusReturn::Success;
        }

        
        void run() 
        {
            while (m_isRunning) {
                StatusReturn status = this->receivePacket();
                if (StatusReturn::Success == status)
                    m_neth->txHandle(reinterpret_cast<uint8_t*>(&status));
                else 
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    public:
        NetworkServer(NetHandlerInterface *neth, int port) : m_neth(neth), m_port(port)
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

        
        ~NetworkServer()
        {
            if (this->isRunning())
                this->stop();
            if (m_newSd)
                close(m_newSd);
            if (m_serverSd)
                close(m_serverSd);
        }

        
        void startRx(void)
        {
            this->start();
        }

        
        void stopRx(void)
        {
            this->stop();
        }

        
        StatusReturn sendData(const void *data, size_t dataSize)
        {
            ssize_t bytesWritten = send(m_newSd, data, dataSize, 0);
            if (bytesWritten < 0)
                return StatusReturn::ResourceError;
            return StatusReturn::Success;
        }

        bool isRunning() const { return m_isRunning; }

        StatusReturn getReceivedData(uint8_t *userBuffer);
    protected:
        
        void start() 
        {
            if (!m_isRunning) {
                m_thread = std::thread(&NetworkServer<bufferSize>::run, this);
                m_isRunning = true;
            }
        }

        
        void stop() 
        {
            if (m_isRunning) {
                m_thread.join();
                m_isRunning = false;
            }
        }
    };
};


#endif // NETLINUX_H
