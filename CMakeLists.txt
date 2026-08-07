cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(data-logger VERSION 1.0.0 LANGUAGES CXX)

# Настройки компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:RelWithDebInfo>:/MT>
        $<$<CONFIG:MinSizeRel>:/MT>
        $<$<NOT:$<OR:$<CONFIG:Release>,$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>>:/MT>)
else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    if(MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ --enable-stdcall-fixup")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "-static")
    endif()
endif()

# Исходные файлы
file(GLOB APPLICATION_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/application/src/*.cpp
)

# Добавление исполняемого файла
add_executable(${PROJECT_NAME} ${APPLICATION_SOURCES})

add_subdirectory(library)

# Связывание с библиотеками
target_link_libraries(${PROJECT_NAME} PUBLIC
    library::DAQNavi
)

# target_link_libraries(library INTERFACE
#     library::DAQNavi
# )




# Связывание с библиотеками

# Подключение заголовочных файлов
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/application/inc
)