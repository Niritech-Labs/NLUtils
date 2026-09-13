cmake_minimum_required(VERSION 3.30)
project(NLUtils VERSION 1.3.1 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

include(CheckIPOSupported)
check_ipo_supported(RESULT ltoSupported OUTPUT ltoError)

if(ltoSupported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    message(STATUS "LTO (IPO) ENABLED")
else()
    message(WARNING "LTO DISABLED: ${ltoError}")
endif()

option(BUILD_NLUTILS_EXTRA "Build NLUtils Extra lib" ON)
option(BUILD_NLUTILS_NLGTOOLKIT "Build NLUtils NL Graphics Toolkit lib (requires Extra)" ON)

option(BUILD_NLUTILS_VENDORED_MODE "Build NLUtils in vendored mode" OFF)

if(BUILD_NLUTILS_VENDORED_MODE)
    include(BuildVendored.cmake)
else()
    include(BuildSystem.cmake)
endif()

