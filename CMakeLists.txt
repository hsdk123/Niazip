cmake_minimum_required(VERSION 3.16)
project(NIAZIP LANGUAGES CXX C)

if (NOT DEFINED NIAZIP_LOCAL_BUILD)
set(NIAZIP_LOCAL_BUILD ON)
endif()

option(WITH_NIAZIP_TESTS "Build niazip tests" ON)
message(STATUS "NIAZIP_LOCAL_BUILD: ${NIAZIP_LOCAL_BUILD}")
message(STATUS "WITH_NIAZIP_TESTS: ${WITH_NIAZIP_TESTS}")

#----------------------------------------------------------------------------------
# Project settings for default dev build
if (NIAZIP_LOCAL_BUILD)

# global settings
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CORE_BIN_DIRECTORY ${PROJECT_SOURCE_DIR}/bin-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}${CMAKE_SIZEOF_VOID_P})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CORE_BIN_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CORE_BIN_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CORE_BIN_DIRECTORY})

# reference: https://cmake.org/cmake/help/latest/prop_tgt/MSVC_RUNTIME_LIBRARY.html
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

endif()
#----------------------------------------------------------------------------------

# Subdirectories
add_subdirectory(extlibs)
add_subdirectory(src)

if (WITH_NIAZIP_TESTS)
add_subdirectory(tests)
endif()

