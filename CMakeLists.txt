# temporary cmake file

cmake_minimum_required(VERSION 3.4.1)

project(siglent-bin2sr)

set (CMAKE_CXX_STANDARD 20)

# Fetch latest argparse libray
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# Fetch latest spdlog library
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.x
)

FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif ()

# find_package(spdlog REQUIRED)

add_executable(siglent-bin2sr
    main.cpp
    siglent_bin.cpp
    srzip.cpp
)

target_link_libraries(siglent-bin2sr zip argparse spdlog::spdlog)

add_executable(siglent-bin2sr-test
    test/test_runner.cpp
    siglent_bin.cpp
    test/test_header.cpp
    srzip.cpp
    test/test_digital.cpp
)

add_test(NAME siglent-bin2sr-test
         COMMAND siglent-bin2sr-test)

add_custom_command(
    TARGET siglent-bin2sr-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/test/SDS00001.bin
            ${CMAKE_CURRENT_BINARY_DIR}/SDS00001.bin)

add_custom_command(
    TARGET siglent-bin2sr-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/test/test-digital-5ch.bin
            ${CMAKE_CURRENT_BINARY_DIR}/test-digital-5ch.bin)
