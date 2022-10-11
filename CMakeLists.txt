# Copyright 2022 Ingemar Hedvall
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)
project(asaplib
        VERSION 1.0
        DESCRIPTION "C++ library including ASAP3 and ASAP2 interfaces."
        LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Static libraries are preferred" OFF)
option(ASAP_DOC "If doxygen is installed, then build documentation in Release mode" OFF)
option(ASAP_TEST "If Google Test is installed, then build the unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX d)

include(CMakePrintHelpers)
include(script/utillib.cmake)
include(script/boost.cmake)
include(script/googletest.cmake)
include(script/doxygen.cmake)
include(script/wxwidgets.cmake)

add_library(asap STATIC
            src/asap3def.cpp include/asap/asap3def.h
            src/irequest.cpp include/asap/irequest.h
            src/iresponse.cpp include/asap/iresponse.h
            src/asap3helper.cpp src/asap3helper.h
            src/itelegram.cpp include/asap/itelegram.h
            src/iclient.cpp include/asap/iclient.h
            src/asap3factory.cpp include/asap/asap3factory.h
            src/queryparameters.cpp src/queryparameters.h src/asap3client.cpp src/asap3client.h)

target_include_directories(asap PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )
		
target_include_directories(asap PRIVATE ${utillib_SOURCE_DIR}/include)
target_include_directories(asap PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(asap PRIVATE expat)

cmake_print_properties(TARGETS asap PROPERTIES INCLUDE_DIRECTORIES)

target_compile_definitions(asap PRIVATE XML_STATIC)

if (MSVC)
    target_compile_definitions(asap PRIVATE _WIN32_WINNT=0x0A00)
endif()

if (GTEST_FOUND AND ASAP_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

#if (DOXYGEN_FOUND AND (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease") )
#
#   set(DOXYGEN_REPEAT_BRIEF NO)
#    set(DOXYGEN_PROJECT_NAME "PubSubLib")
#    set(DOXYGEN_HTML_EXTRA_STYLESHEET doxygen/utillib.css)
#    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/manual)
#    doxygen_add_docs( doc_pubsub ALL
#            include/pubsub doxygen
#            COMMENT "Publish/Subscribe Library generating API documentation with Doxygen")
#endif()
