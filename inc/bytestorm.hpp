/**
 * @file bytestorm.hpp
 * @author Semikozov Ian (semikozov.yal@yandex.ru)
 * @brief
 * @version 0.1
 * @date 30.03.2024
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef BYTESTORM_HPP_INCLUDED
#define BYTESTORM_HPP_INCLUDED

#include "bytestorm_boost.hpp"
#include "bytestorm_unix.hpp"
#include "processor_base.hpp"

namespace ByteStorm
{
    using Processor = ProcessorBase<TCPConnection>;
}; // namespace ByteStorm

#endif // BYTESTORM_HPP_INCLUDED
