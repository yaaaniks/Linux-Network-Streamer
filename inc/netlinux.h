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

#include "NetHandlerInterface.hpp"

#define COUNT_REQUEST 5

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

    enum Protocol {
        TCP,
        UDP
    };
    
    /********************************************Client********************************************/
    template<enum Protocol>
    class NetworkClient
    {
    private:
        int m_status, m_valRead, m_port, m_bufferSize, m_fdClient = 0;
        struct sockaddr_in m_servAddr;
        uint8_t *m_rxBuffer;
        NetHandlerInterface *m_neth;

        std::string m_ip;
        std::mutex m_bufferMutex;
        std::thread m_thread;
        std::atomic_bool m_isRunning {false}, m_isConnected {false};

        void start() 
        {
            if (!m_isRunning) {
                m_thread = std::thread(&NetworkClient::run, this);
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

    public:
        explicit NetworkClient(NetHandlerInterface *neth, const std::string &ip, int port, int bufferSize) : 
            m_neth(neth), m_ip(ip), m_port(port), m_bufferSize(bufferSize)
        {
            if (m_bufferSize > 0)
                m_rxBuffer = new uint8_t[m_bufferSize];
            else 
                throw std::runtime_error("Invalid buffer size");
                
            if ((m_fdClient = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                throw std::runtime_error("Failed to open socket!");

            m_servAddr.sin_family = AF_INET;
            m_servAddr.sin_port = htons(m_port);
            if (inet_pton(AF_INET, m_ip.c_str(), &m_servAddr.sin_addr) <= 0)
                throw std::runtime_error("Invalid address / Address not supported");
        }
        
        ~NetworkClient()
        {
            this->disconnect();
            delete[] m_rxBuffer;
        }
        
        StatusReturn receiveData(uint8_t *buffer, size_t bytesToReceive)
        {
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            uint8_t *tempBuffer = new uint8_t[bytesToReceive];
            m_valRead = recv(m_fdClient, tempBuffer, bytesToReceive, 0); //recv
            if (m_valRead > 0) {
                memcpy(buffer, tempBuffer, m_valRead);
                delete[] tempBuffer;
                return StatusReturn::Success;
            }
            delete[] tempBuffer;
            return StatusReturn::NoData;
        }

        StatusReturn connectToServer()
        {
            uint8_t i = 0;
            while (++i < COUNT_REQUEST) {
                if (connect(m_fdClient, (struct sockaddr*)&m_servAddr, sizeof(m_servAddr)) < 0) {
                    usleep(10000);
                    continue;
                } else {
                    m_isConnected = true;
                    return StatusReturn::Success;
                }
            } 
            return StatusReturn::ConnectionError;
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

        bool isRunning() const { return m_isRunning; }
        bool isConnected() const { return m_isConnected; }
    protected:

        void run()
        {
            while (m_isRunning) {
                StatusReturn status = this->receiveData(m_rxBuffer, m_bufferSize);
                if (StatusReturn::Success == status)
                    m_neth->rxHandle(m_rxBuffer, m_valRead);
                else 
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    };

    /********************************************Server********************************************/
    template<enum Protocol>
    class NetworkServer 
    {
    private:
        int m_port, m_serverSd, m_newSd, m_valRead, m_bufferSize, m_fdClient {0};
        struct sockaddr_in m_servAddr;
        NetHandlerInterface *m_neth;
        uint8_t *m_rxBuffer;

        std::mutex m_bufferMutex;
        std::thread m_thread;
        std::atomic_bool m_isRunning {false};

        void start() 
        {
            if (!m_isRunning) {
                m_thread = std::thread(&run, this);
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

    public:
        NetworkServer(NetHandlerInterface *neth, int port, int bufferSize) : 
            m_neth(neth), m_port(port), m_bufferSize(bufferSize)
        {
            if (m_bufferSize > 0)
                m_rxBuffer = new uint8_t[m_bufferSize];
            else 
                throw std::runtime_error("Invalid buffer size");
            m_servAddr.sin_family = AF_INET;
            m_servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
            m_servAddr.sin_port = htons(port);
            m_serverSd = socket(AF_INET, SOCK_STREAM, 0);

            if (m_serverSd < 0)
                throw std::runtime_error("Error establishing the server socket");

            this->bindServer();
            this->listenPorts(1);
        }
      
        ~NetworkServer()
        {
            this->stop();
            if (m_newSd)
                close(m_newSd);
            if (m_serverSd)
                close(m_serverSd);
            delete[] m_rxBuffer;
        }

        void startRx(void)
        {
            this->start();
        }

        
        void stopRx(void)
        {
            this->stop();
        }

        StatusReturn receiveData(uint8_t *buffer, size_t bytesToReceive)
        {
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            uint8_t *tempBuffer = new uint8_t[bytesToReceive];
            m_valRead = recv(m_fdClient, tempBuffer, bytesToReceive, 0); //recv
            if (m_valRead > 0) {
                memcpy(buffer, tempBuffer, m_valRead);
                delete[] tempBuffer;
                return StatusReturn::Success;
            }
            delete[] tempBuffer;
            return StatusReturn::NoData;
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

        StatusReturn sendData(const void *data, size_t dataSize)
        {
            ssize_t bytesWritten = send(m_newSd, data, dataSize, 0);
            if (bytesWritten < 0)
                return StatusReturn::ResourceError;
            return StatusReturn::Success;
        }

        bool isRunning() const { return m_isRunning; }
    protected:
        
        void run()
        {
            while (m_isRunning) {
                StatusReturn status = this->receiveData(m_rxBuffer, m_bufferSize);
                if (StatusReturn::Success == status)
                    m_neth->rxHandle(m_rxBuffer, m_valRead);
                else 
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    };
};


#endif // NETLINUX_H