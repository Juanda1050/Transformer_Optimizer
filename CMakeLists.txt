cmake_minimum_required(VERSION 3.16)

project(transformer_optimizer
    VERSION 1.0
    LANGUAGES CXX
)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(transformer_optimizer
    src/main.cpp
    src/data.cpp
    src/evaluator.cpp
    src/optimizer.cpp
    src/reporting.cpp
)

set_target_properties(transformer_optimizer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(transformer_optimizer
    PRIVATE
        include
)