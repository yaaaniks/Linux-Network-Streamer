/**
 * @file bytestorm_base.hpp
 * @author Semikozov Ian (semikozov.yal@yandex.ru)
 * @brief
 * @version 0.1
 * @date 29.03.2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef BYTESTORM_BASE_HPP_INCLUDED
#define BYTESTORM_BASE_HPP_INCLUDED

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "processor_base.hpp"

namespace ByteStorm
{

enum class Status
{
    OK = 0,
    ErrorOnRx,
    ErrorOnTx,
    ErrorOnBind
};

template <typename T>
class ByteStormBase
{
protected:
    static constexpr int kDefaultPort{ 8090 };

public:
    ByteStormBase(ProcessorBase<T> *processor = nullptr) : processor(processor) {}
    ~ByteStormBase() {};
    virtual void flush() = 0;

    virtual Status send(std::unique_ptr<std::uint8_t[]> &data, const size_t size) = 0;

    [[maybe_unused]] inline bool isConnected() __attribute__((warn_unused_result))
    {
        return connected;
    }

protected:
    ProcessorBase<T> *processor;
    std::atomic_bool connected{ false };
};
}; // namespace ByteStorm

#endif // BYTESTORM_BASE_HPP_INCLUDED
