# Copyright 2022 Ingemar Hedvall
# SPDX-License-Identifier: MIT

project(TestAsap
        VERSION 1.0
        DESCRIPTION "Google unit tests for the asap library"
        LANGUAGES CXX)

add_executable(test_asap
        test_client.cpp
       )

target_include_directories(test_asap PRIVATE ../include)
target_include_directories(test_asap PRIVATE ../src)
target_include_directories(test_asap PRIVATE ${GTEST_INCLUDE_DIRS})
target_include_directories(test_asap PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(test_asap PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

cmake_print_properties(TARGETS test_asap PROPERTIES INCLUDE_DIRECTORIES)
target_link_libraries(test_asap PRIVATE util)
target_link_libraries(test_asap PRIVATE asap)
target_link_libraries(test_asap PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_asap PRIVATE expat)
target_link_libraries(test_asap PRIVATE ${GTEST_BOTH_LIBRARIES})

if (WIN32)
target_link_libraries(test_asap PRIVATE ws2_32)
target_link_libraries(test_asap PRIVATE mswsock)
target_link_libraries(test_asap PRIVATE bcrypt)
endif()

if (MINGW)
    target_link_options(test_asap PRIVATE -static -fstack-protector )
elseif (MSVC)
    target_compile_options(test_asap PRIVATE -D_WIN32_WINNT=0x0A00)
endif()


include(GoogleTest)
gtest_discover_tests(test_asap)

