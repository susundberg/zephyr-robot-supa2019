# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/devicetree.overlay")
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

project(SUPA_ROB)

file(GLOB_RECURSE app_sources "src/*.c" )
file(GLOB_RECURSE app_sources_cpp "src/*.cpp" )

target_sources(app PRIVATE ${app_sources} ${app_sources_cpp})

