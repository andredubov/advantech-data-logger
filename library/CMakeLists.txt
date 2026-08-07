cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(library VERSION 1.0.0 LANGUAGES CXX)

# Включение модулей
add_subdirectory(DAQNavi)

# Объединяющий интерфейс для всей библиотеки
add_library(library INTERFACE)

target_link_libraries(library INTERFACE
    library::DAQNavi
)

set_target_properties(library PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)