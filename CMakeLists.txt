cmake_minimum_required(VERSION 3.20)

project(der
        VERSION
        0.1.0
        DESCRIPTION
        "A low level statically typed programming language inspired from moroccan darija that compiles down to C."
        LANGUAGES
        CXX
)
set(CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")

set(DER_ALLOW_DEBUG OFF)
if(DER_ALLOW_DEBUG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDER_ALLOW_DEBUG")
endif()
set(default_build_type "Release")
add_executable(derijac ./main.cpp)