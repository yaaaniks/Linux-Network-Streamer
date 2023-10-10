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
    void rxHandle(const unsigned char *data __attribute__((unused)), \
                  unsigned int size __attribute__((unused))) override {}
    void txHandle(const unsigned char *data __attribute__((unused)), \
                  unsigned int size __attribute__((unused))) override {}
    void setServer(ByteStorm::client_ptr server __attribute__((unused))) override {}
};

TestServer::TestServer(int a)
{
}

TestServer::~TestServer()
{
}




int main(int argc, char* argv[]) {
    TestServer testServer(5);
    ByteStorm::client_ptr new_ptr = ByteStorm::ByteStormServer::new_(&testServer);
    return 0;
}