cmake_minimum_required(VERSION 3.16)
project(smart_pong_eip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(smart_pong_demo
    src/smart_pong_ai.cpp
    src/smart_pong_model_backend.cpp
    src/smart_pong_persistence.cpp
    examples/full_smart_pong_example.cpp
)

target_include_directories(smart_pong_demo PRIVATE include)
