cmake_minimum_required(VERSION 3.15)

project(bytestorm 
        VERSION 0.1 
        DESCRIPTION "Socket programming for UNIX-based systems"
        LANGUAGES CXX)

include(CTest)
include(FindPkgConfig)
enable_testing()

# Dependency flags
set(Boost_USE_STATIC_LIBS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Constants
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CMAKE_CXX_STANDARD 17)

# Exporting commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find all .cpp files
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB HEADERS ${PROJECT_INCLUDE_DIR}/*.hpp ${PROJECT_INCLUDE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/common/processor_base.hpp)

find_package(Boost 1.83 QUIET REQUIRED COMPONENTS iostreams date_time)

if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost Not found")
        return()
endif()

message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
message(STATUS "Boost libraries are found in: ${Boost_INCLUDE_DIR}")

include_directories(${PROJECT_NAME} ${PROJECT_INCLUDE_DIR} ${BOOST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/common)

if(UNIX AND NOT APPLE)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        find_package(Threads REQUIRED)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inc/bytestorm.hpp ${CMAKE_BINARY_DIR}/include/bytestorm.hpp COPYONLY)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX
                        VERSION ${PROJECT_VERSION} SOVERSION 1
                        PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/inc/bytestorm.hpp)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/common ${Boost_INCLUDE_DIR})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)


add_subdirectory(example)