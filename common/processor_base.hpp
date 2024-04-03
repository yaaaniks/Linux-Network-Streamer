/**
 * @file processor_base.hpp
 * @author Semikozov Ian (y.semikozov@geoscan.ru)
 * @brief
 * @version 0.1
 * @date 29.03.2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef PROCESSOR_BASE_HPP_INCLUDED
#define PROCESSOR_BASE_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <memory>

template <typename T>
class ProcessorBase
{
public:
    ProcessorBase() = default;

    virtual void process(std::unique_ptr<uint8_t[]> &p, const size_t size) = 0;

    void set(T *ptr)
    {
        this->ptr = ptr;
    }
    
protected:
    T *ptr;
};

#endif // PROCESSOR_BASE_HPP_INCLUDED
