cmake_minimum_required(VERSION 3.20)
project(data-converter LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    inc
)
