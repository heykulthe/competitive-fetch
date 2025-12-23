cmake_minimum_required(VERSION 4.0)
project(competitive_fetch)

set(CMAKE_CXX_STANDARD 17)

add_executable(competitive_fetch main.cpp
        headers/args.hpp
        headers/fzf.hpp
        headers/fetch.hpp
        args.cpp
        fzf.cpp
        fetch.cpp
        generator.cpp
        headers/generator.hpp)
