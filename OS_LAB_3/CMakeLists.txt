cmake_minimum_required(VERSION 3.15)
project(OS_LAB_3 LANGUAGES CXX)

# ------------------------- Настройки C++ -------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------- Основная программа -------------------------
add_executable(OS_LAB_3
    main.cpp
    threadfunctions.cpp
    threadfunctions.h
    mathfunctions.cpp
    mathfunctions.h
)


if (MSVC)
    target_compile_options(OS_LAB_3 PRIVATE /W4)
else()
    target_compile_options(OS_LAB_3 PRIVATE -Wall -Wextra -pedantic)
endif()

# ------------------------- GoogleTest -------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(OS_LAB_3_tests
    tests.cpp
    threadfunctions.cpp
    threadfunctions.h
    mathfunctions.cpp
    mathfunctions.h
)

target_link_libraries(OS_LAB_3_tests gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(OS_LAB_3_tests)
