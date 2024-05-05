cmake_minimum_required(VERSION 3.1)

project(LocalChat)

add_compile_options(-Werror -Wall)

file(GLOB CONNECTIONS connections/conn_*.cpp)

foreach(connection_path ${CONNECTIONS})
    get_filename_component(FILE_NAME ${connection_path} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})

    add_executable(
        host_${CONN_NAME}
        host.cpp client.cpp
        connections/conn.hpp connections/conn_${CONN_NAME}.cpp
    )
endforeach()
