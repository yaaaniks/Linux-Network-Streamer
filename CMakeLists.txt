cmake_minimum_required(VERSION 3.15)

project(networkstreamer VERSION 0.1 
                DESCRIPTION "Socket programming for RPi microcomputer")

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/client.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)

set(LIBS)

add_library(${PROJECT_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)


set_target_properties(${PROJECT_NAME} PROPERTIES
                        VERSION ${PROJECT_VERSION}
                        SOVERSION 1
                        PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/inc/client.h)

target_include_directories(${PROJECT_NAME} PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/inc
                           ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})


target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS} "-static")


install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc/
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(UNIX AND NOT APPLE)
endif()