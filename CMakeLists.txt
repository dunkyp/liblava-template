# file      : CMakeLists.txt
# copyright : Copyright (c) 2018-present, Lava Block OÜ
# license   : MIT; see accompanying LICENSE file

cmake_minimum_required(VERSION 3.12)

project(template VERSION 0.4.3 LANGUAGES C CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/liblava ${CMAKE_CURRENT_BINARY_DIR}/liblava)

message(">> template")

add_executable(template
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        )

target_link_libraries(template lava::tool)
