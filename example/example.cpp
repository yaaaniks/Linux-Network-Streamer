#include "bytestorm_boost_test.hpp"
#include "handler_base.hpp"
#include <iostream>

using namespace ByteStorm;
class TestServer : public HandlerBase<TCPConnection>
{
private:
    /* data */
public:
    TestServer(std::uint8_t a) : HandlerBase<TCPConnection>(a) {}
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
    ByteStormBoost bs(8081, 21);
    auto netServer = bs.create(21, &serv);
    bs.acc->async_accept(*netServer->getSocket(),
                         boost::bind(&ByteStormBoost::handle_accept, &bs, netServer, _1, &serv));
    bs.service.run();
    while (true) {}
    return 0;
}
