cmake_minimum_required(VERSION 3.1.0)
project(coolsocket VERSION 1.0
        DESCRIPTION "CoolSocket TCP communication library"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

find_package(Qt5 COMPONENTS Core Network REQUIRED)

set(HEADER_FILES
        include/coolsocket.h)

set(SOURCE_FILES
        src/coolsocket.cpp)

add_library(coolsocket ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(coolsocket PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(coolsocket PUBLIC Qt5::Core Qt5::Network)

set_target_properties(coolsocket PROPERTIES
        FRAMEWORK ON
        SOVERSION 0
        VERSION 0.1.0)