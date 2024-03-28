#include "bytestorm_boost.hpp"
#include "handler_base.hpp"
#include <iostream>

using namespace ByteStorm;
class TestServer : public HandlerBase<client_ptr>
{
private:
    /* data */
public:
    TestServer(std::uint8_t a) : HandlerBase<client_ptr>(a) {}
    ~TestServer() {};

protected:
    [[maybe_unused]] virtual void handle(std::unique_ptr<std::uint8_t[]> &p, const size_t size) override
    {
        std::cout << "Received " << size;
    }
};

int main(int argc, char *argv[])
{
    TestServer serv(2);
    ByteStormServer bs(&serv, 8090);
    while (true) {}
    return 0;
}
