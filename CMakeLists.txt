cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(Chess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

ADD_SUBDIRECTORY(src/proto)

file(GLOB_RECURSE SRC_LIST src/*.cpp)
file(GLOB_RECURSE HDR_LIST src/*.h)

include_directories(src)
add_executable(chess ${SRC_LIST} ${HDR_LIST})

target_link_libraries(chess Qt5::Core Qt5::Widgets Qt5::Network Threads::Threads ${Protobuf_LIBRARIES} proto)
