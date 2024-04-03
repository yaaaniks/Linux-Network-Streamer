#ifndef NET_LINUX_H
#define NET_LINUX_H

#include "bytestorm_base.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <valarray>

namespace ByteStorm
{

class ByteStormUnix;

struct KeepAliveConfiguration
{
    static constexpr int kDefaultIdle{ 120 };
    static constexpr int kDefaultIntvl{ 3 };
    static constexpr int kDefaultCnt{ 5 };

    int ka_idle{ kDefaultIdle };
    int ka_intvl{ kDefaultIntvl };
    int ka_cnt{ kDefaultCnt };
};

class ByteStormUnix : public ByteStormBase<ByteStormUnix>
{
    static constexpr int kMaxCountListen{ 5 };
    static constexpr size_t kDefaultBufferSize{ 1024 };

private:
    int serverDescription, clientDescription;
    ssize_t bytesRx{ 0 };

    std::valarray<uint8_t> buffer{};
    std::mutex mutex;
    std::thread t;
    std::atomic_bool enable{ false };
    std::shared_ptr<std::valarray<uint8_t>> pBuffer;

    struct sockaddr_in m_servAddr;
    KeepAliveConfiguration kaConfig;

public:
    ByteStormUnix() = default;
    ByteStormUnix(ProcessorBase<ByteStormUnix> *processor, int p, int bufferSize = kDefaultBufferSize);
    ~ByteStormUnix();

    void flush() override;

    Status send(std::unique_ptr<std::uint8_t[]> &data, const size_t size) override;
    Status receive(size_t size);
    Status listen(int count = kMaxCountListen);
    Status bind();
    Status enableKeepAlive();

protected:
    void run();
}; // class ByteStormUnix

}; // namespace ByteStorm

#endif // NETLINUX_H
