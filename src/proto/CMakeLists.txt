cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
cmake_policy(SET CMP0071 NEW)

find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})
file(GLOB_RECURSE PROTO_LIST ./*.proto)
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${PROTO_LIST})
add_library(proto ${PROTO_HEADER} ${PROTO_SRC})