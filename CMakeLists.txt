cmake_minimum_required(VERSION 4.0)
project(competitive_fetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(cpf
        main.cpp
        args.cpp
        fzf.cpp
        fetch.cpp
        generator.cpp
)

target_include_directories(cpf PRIVATE headers)
