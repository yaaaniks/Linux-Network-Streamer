#include "bytestorm_boost.hpp"
#include "handler_base.hpp"
#include <iostream>

using namespace ByteStorm;
class TestServer : public HandlerBase<client_ptr>
{
private:
    /* data */
public:
    TestServer(std::uint8_t a) : HandlerBase<client_ptr>(a) 
    {
        auto netServer = ByteStorm::ByteStormServer::new_(this);
        ByteStorm::acceptor.async_accept(netServer->sock(), boost::bind(ByteStorm::handle_accept<ByteStorm::client_ptr>, netServer, _1, this));
        ByteStorm::service.run();
    }
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
    
    while (true) {}
    return 0;
}
