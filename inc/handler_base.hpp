/**
 * @file handler_base.hpp
 * @author Semikozov Ian (y.semikozov@geoscan.ru)
 * @brief
 * @version 0.1
 * @date 29.03.2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef HANDLER_BASE_HPP_INCLUDED
#define HANDLER_BASE_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <memory>

template <typename T>
class HandlerBase
{
public:
    HandlerBase(std::uint8_t message) : id(message) {}
    ~HandlerBase() {};
    inline bool at(std::uint8_t message)
    {
        return id == message;
    }

    virtual void handle(std::unique_ptr<std::uint8_t[]> p, const size_t size) = 0;

    void set(T ptr)
    {
        this->ptr = ptr;
    }

private:
    std::uint8_t id;
    T ptr;
};

#endif // HANDLER_BASE_HPP_INCLUDED
