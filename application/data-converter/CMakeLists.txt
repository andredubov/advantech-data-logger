cmake_minimum_required(VERSION 3.20)
project(data-converter LANGUAGES CXX)

set(Boost_NO_WARN_NEW_VERSIONS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_DEBUG_LIBS ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

add_executable(${PROJECT_NAME}
    main/src/main.cpp
    main/src/CommandLineOptions.cpp
    core/src/BinaryReader.cpp
    core/src/CsvWriter.cpp
    core/src/DataConverter.cpp
    core/src/TimeFormatter.cpp
    validation/src/FileExtensionValidator.cpp
    validation/src/FileValidator.cpp
    utils/src/Logger.cpp
    utils/src/ProgressReporter.cpp
    factory/src/ConverterFactory.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    main/inc
    core/inc
    validation/inc
    utils/inc
    factory/inc
)

find_package(Boost REQUIRED COMPONENTS system program_options filesystem)
if(Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${Boost_INCLUDE_DIRS}
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::system
        Boost::program_options
        Boost::filesystem
    )
else()
    message(FATAL_ERROR "Boost not found!")
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
