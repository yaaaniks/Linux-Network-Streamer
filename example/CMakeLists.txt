cmake_minimum_required(VERSION 3.15)

project(example_lns
        VERSION 0.1
        DESCRIPTION "Example usage of netlinux library"
        LANGUAGES CXX)

add_executable(${PROJECT_NAME} example.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC bytestorm)