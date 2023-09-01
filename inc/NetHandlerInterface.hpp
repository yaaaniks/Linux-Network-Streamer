#ifndef NET_HANDLER_INTERFACE_HPP
#define NET_HANDLER_INTERFACE_HPP

struct NetHandlerInterface
{
    virtual void rxHandle(const unsigned char *data, unsigned int size) = 0;
    virtual void txHandle(const unsigned char *data) = 0;
};


#endif // NET_HANDLER_INTERFACE_HPP
