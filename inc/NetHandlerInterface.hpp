#ifndef NET_HANDLER_INTERFACE_HPP
#define NET_HANDLER_INTERFACE_HPP

template <typename T>
struct NetHandlerInterface
{
    virtual void handle(const unsigned char *data, unsigned int size) = 0;
    void set(T ptr)
    {
        this->ptr = ptr;
    }

private:
    T ptr;
};

#endif // NET_HANDLER_INTERFACE_HPP
