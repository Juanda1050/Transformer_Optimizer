cmake_minimum_required(VERSION 3.16)

project(transformer_optimizer
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(transformer_optimizer
    src/main.cpp
    src/data.cpp
    src/evaluator.cpp
    src/optimizer.cpp
)

target_include_directories(transformer_optimizer
    PRIVATE
        include
)