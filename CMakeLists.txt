cmake_minimum_required(VERSION 3.15)

project(netlinux VERSION 0.1 
                 DESCRIPTION "Socket programming for RPi microcomputer")

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/NetHandlerInterface.hpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/inc/netlinux.h)

set(LIBS)

if(UNIX AND NOT APPLE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
endif()

add_library(${PROJECT_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/inc/netlinux.h)


set_target_properties(${PROJECT_NAME} PROPERTIES
                        LINKER_LANGUAGE CXX
                        VERSION ${PROJECT_VERSION}
                        SOVERSION 1
                        PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/inc/netlinux.h)
target_include_directories(${PROJECT_NAME} PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})


target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS} "-static")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc/
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(UNIX AND NOT APPLE)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

