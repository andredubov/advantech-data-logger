cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

file(GLOB DAQNavi_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

if(DAQNavi_SOURCES)
    add_library(library_DAQNavi STATIC 
        ${DAQNavi_SOURCES} 
    )
    target_include_directories(library_DAQNavi PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )
else()
    add_library(library_DAQNavi INTERFACE)
    target_include_directories(library_DAQNavi INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )
endif()

add_library(library::DAQNavi ALIAS library_DAQNavi)
target_compile_features(library_DAQNavi INTERFACE cxx_std_17)