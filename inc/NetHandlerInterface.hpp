#ifndef NET_HANDLER_INTERFACE_HPP
#define NET_HANDLER_INTERFACE_HPP

template <typename ServerType>
struct NetHandlerInterface
{
    virtual void rxHandle(const unsigned char *data, unsigned int size) = 0;
    virtual void txHandle(const unsigned char *data, unsigned int size) = 0;
    virtual void setServer(ServerType server) = 0;
};


#endif // NET_HANDLER_INTERFACE_HPP