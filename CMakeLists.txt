cmake_minimum_required(VERSION 3.15)

project(netlinux VERSION 0.1 
                 DESCRIPTION "Socket programming for RPi microcomputer"
                 LANGUAGES CXX)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/NetHandlerInterface.hpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/inc/netlinux.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/inc/bytestorm.hpp)

include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

find_package(Boost 1.83 QUIET REQUIRED COMPONENTS date_time)

if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost Not found")
endif()

message("Boost libraries: ${Boost_LIBRARIES}")
message("Boost libraries are found in: ${Boost_INCLUDE_DIR}")

include_directories(${BOOST_INCLUDE_DIRS})

if(UNIX AND NOT APPLE)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        find_package(Threads REQUIRED)
endif()

add_library(${PROJECT_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/inc/netlinux.h
                                   ${CMAKE_CURRENT_SOURCE_DIR}/inc/bytestorm.hpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inc/netlinux.h ${CMAKE_BINARY_DIR}/include/netlinux.h COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inc/bytestorm.hpp ${CMAKE_BINARY_DIR}/include/bytestorm.hpp COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inc/NetHandlerInterface.hpp ${CMAKE_BINARY_DIR}/include/NetHandlerInterface.hpp COPYONLY)

set_target_properties(${PROJECT_NAME} PROPERTIES
                        LINKER_LANGUAGE CXX
                        VERSION ${PROJECT_VERSION}
                        SOVERSION 1
                        PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/inc/netlinux.h)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})


target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc
        DESTINATION ${CMAKE_BINARY_SOURCE_DIR}/inc)

target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

add_subdirectory(example)

