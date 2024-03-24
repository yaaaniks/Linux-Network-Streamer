#include "bytestorm.hpp"
#include "NetHandlerInterface.hpp"

class TestServer : public NetHandlerInterface<ByteStorm::client_ptr>
{
private:
    /* data */
public:
    TestServer(int a);
    ~TestServer();

protected:
    [[maybe_unused]] void handle(const unsigned char *data, unsigned int size) override {}
};

TestServer::TestServer(int a) {}

TestServer::~TestServer() {}

int main(int argc, char *argv[])
{
    TestServer testServer(5);
    ByteStorm::client_ptr new_ptr = ByteStorm::ByteStorm::new_(&testServer);
    while (true) {}
    return 0;
}
