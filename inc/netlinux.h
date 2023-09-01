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

using std::vector;
using std::thread;
using std::mutex;
using std::atomic_bool;

namespace NetLinux
{
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
    
    template<int bufferSize>
    class NetworkClient
    {
    private:
        int m_status, m_valRead, m_port, m_fdClient = 0;
        std::string m_ip;
        struct sockaddr_in m_servAddr;
        uint8_t m_rxBuffer[bufferSize];
        std::mutex m_bufferMutex;
    public:
        explicit NetworkClient(const std::string& ip, int port);
        ~NetworkClient();
        
        StatusReturn connectToServer();
        void disconnect();
        StatusReturn sendData(const void *data, size_t dataSize);
        StatusReturn receiveData(uint8_t *userBuffer, size_t bytesToReceive);
        void run();
        StatusReturn getReceivedData(uint8_t *userBuffer);
    };

    template<int bufferSize>
    class NetworkServer 
    {
    private:
        int m_port, m_serverSd, m_newSd, m_fdClient {0};
        struct sockaddr_in m_servAddr;
        NetHandlerInterface *m_neth;
        uint8_t m_rxBuffer[bufferSize];

        std::string m_ip;
        std::mutex m_bufferMutex;
        std::thread m_thread;
        std::atomic_bool m_isRunning {false};

        StatusReturn receivePacket();
        StatusReturn listenPorts(int countRequests);
        StatusReturn bindServer();

        void run();
    public:
        explicit NetworkServer(NetHandlerInterface *neth, int port);
        ~NetworkServer();
        void startRx(void);
        void stopRx(void);

        StatusReturn sendData(const void *data, size_t dataSize);
        StatusReturn getReceivedData(uint8_t *userBuffer);
    protected:
        void start();
        void stop();
        bool isRunning() const { return m_isRunning; }
    };
};


#endif // NETLINUX_H
