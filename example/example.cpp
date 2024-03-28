#include "bytestorm.hpp"
#include "handler_base.hpp"
#include <iostream>

using namespace ByteStorm;
class TestServer : public HandlerBase<ByteStormUnix>
{
private:
    /* data */
public:
    TestServer(std::uint8_t a) : HandlerBase<ByteStormUnix>(a) {}
    ~TestServer() {};

protected:
    [[maybe_unused]] void handle(std::unique_ptr<std::uint8_t[]> data, const size_t size) override
    {
        std::cout << "Received " << size;
    }
};

int main(int argc, char *argv[])
{
    TestServer testServer(5);
    ByteStorm::ByteStormUnix bs(nullptr, 8095, 21);
    bs.bind();
    bs.listen(5);
    bs.enableKeepAlive();
    uint8_t buf[21];
    while (true) {
        bs.receive(buf, 21);
    }
    return 0;
}
