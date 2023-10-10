cmake_minimum_required(VERSION 3.15)

project(netlinux_example VERSION 0.1
                        DESCRIPTION "Example usage of netlinux library"
                        LANGUAGES CXX)

find_package(Boost 1.83 REQUIRED COMPONENTS)
include_directories(${BOOST_INCLUDE_DIRS})

add_executable(example example.cpp)

target_link_libraries(example PRIVATE netlinux ${Boost_LIBRARIES})