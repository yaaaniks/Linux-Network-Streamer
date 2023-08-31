#ifndef CLIENT_H
#define CLIENT_H

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

#define FIFOSIZE    1024

using std::vector;

namespace NetworkStreamer 
{
    enum StatusReturn {
        Success,
        ConnectionError,
        SocketError,
        NetworkError,
        DataFormatError,
        TimeoutError,
        ResourceError,
        NetConfigurationError
    };
    
    // template<int fifoSize>
    class TCPStreamer
    {
    private:
        int m_status, m_valRead, m_port, m_fdClient = 0;
        std::string m_ip;
        struct sockaddr_in m_servAddr;
        /*std::vector<uint8_t> fifoBuffer*/ uint8_t m_buffer[FIFOSIZE];
        std::mutex m_bufferMutex;

        StatusReturn decodeData();
    public:
        explicit TCPStreamer(const std::string& ip, int port);
        ~TCPStreamer();
        
        StatusReturn connectToServer();
        void disconnect();
        StatusReturn sendData(const void *data, size_t dataSize);
        StatusReturn receiveData(uint8_t *userBuffer, size_t bytesToReceive);
        void run();
        StatusReturn getReceivedData(uint8_t *userBuffer);
    };
}


#endif // CLIENT_H