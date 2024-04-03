#include "bytestorm.hpp"
#include <iostream>

using namespace ByteStorm;

class TestServer : public Processor
{
private:
    /* data */
public:
    TestServer() {}
    ~TestServer() {};

protected:
    [[maybe_unused]] void process(std::unique_ptr<std::uint8_t[]> &p, const size_t size) override
    {
        std::cout << "Received " << size;
        auto p_data = std::make_unique<std::uint8_t[]>(71);
        // ptr->send(p_data, 71);
    }
};

int main(int argc, char *argv[])
{
    TestServer serv;
    ByteStormBoost bs(8081, 21);
    auto netServer = bs.create(21, &serv);
    bs.acc->async_accept(netServer->sock,
                         boost::bind(&ByteStormBoost::handle_accept, &bs, netServer, _1, &serv));
    bs.service.run();
    while (true) {}
    return 0;
}
