#
# Copyright @ 2019 Audi AG. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
#
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(SET CMP0011 NEW)

if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14)
    message(STATUS "Setting CMAKE_CXX_STANDARD to 14")
    set(CMAKE_CXX_STANDARD 14)
endif()

# Disable extensions here and require the chosen CMAKE_CXX_STANDARD (coming from Conan)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(fep_system_cmake_enable_tests "Enable tests - requires googletest (default: ON)" ON)

# Use shared libraries from current directory on Linux (same behavior as on Windows)
SET(CMAKE_INSTALL_RPATH "$ORIGIN")

project(fep-system-library VERSION 2.6.1)
set(VERSION_SYSTEM_LIBRARY "2020")
set(FEP_SYSTEM_LIBRARY fep_system)

set(FEP_SYSTEM_LIB_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(FEP_SYSTEM_LIB_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(FEP_SYSTEM_LIB_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(DOXYGEN_WARN_IF_UNDOCUMENTED "No")

set(FEP_SYSTEM_LIB_VERSION "${FEP_SYSTEM_LIB_VERSION_MAJOR}.${FEP_SYSTEM_LIB_VERSION_MINOR}.${FEP_SYSTEM_LIB_VERSION_PATCH}")

# Enable strict compiler warnings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4100 /wd4251 /wd4505 /wd4290")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas -Wno-reorder")
endif()

# check for conan
#f(CONAN_COMPILER)
    message(STATUS "Including Conan build info")
    if ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
    elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
    elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
    else()
        message(FATAL_ERROR "Conan build info can't be found.")
    endif()

    if(CORTEX_WORKSPACE)
        conan_basic_setup(TARGETS)
    else()
        conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    endif()
    set(gtest_search_mode CONFIG)
endif()

set(PARTICIPANT_LIB_DIR ${CONAN_FEP_SDK_PARTICIPANT_ROOT})
set(USE_SHARED_RTIDDS false CACHE BOOL "Use shared RTI DDS Libraries.")

include(scripts/cmake/enable_multicore_compilation.cmake)
include(scripts/cmake/use_integrated_debug_symbols.cmake)
include(src/fep_system/fep_system-macros.cmake)

### Set basic and obligatory CMake parameters and find Project Templates package.
set(AEV_PRODUCT_FULL_NAME "FEP System Library - Functional Engineering Platform System Library")
set(AEV_PRODUCT_SHORT_NAME "fep-system-library")

# Enable project folder structure for Visual Studio IDE
set_property(GLOBAL PROPERTY USE_FOLDERS true)

### Product specific
set(FEP_SYSTEM_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(FEP_BUILD_SYSTEM_LIB true)
set(BETA_BUILD false CACHE BOOL "Mark as beta")
set(BUILD_NR 0xffff CACHE STRING "The current build nr")
set(FEP_SYSTEM_LIB_BUILD_NR ${BUILD_NR})
set(REFERENCE_VERSION ${CONAN_PACKAGE_VERSION})

# some settings need to be set explicitly for QNX
if (UNIX)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
    set(CMAKE_SKIP_BUILD_RPATH  OFF)
endif(UNIX)

find_package(fep_participant REQUIRED)
find_package(a_util REQUIRED)
find_package(pkg_rpc REQUIRED)

################################################################################
### Setting up packages
################################################################################

# compensate for the missing platform if building locally
if(NOT DEFINED PLATFORM)
    set(PLATFORM "developer")
endif(NOT DEFINED PLATFORM)

# set shared flags
set(FEP_SYSTEM_SHARED_LIB ${BUILD_SHARED_LIBS})

configure_file(include/fep_system/fep_system_version.h.in include/fep_system/fep_system_version.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/include/fep_system/fep_system_version.h
    DESTINATION include/fep_system
)
install(FILES README.md DESTINATION .)

# add subdirectories core
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(doc/extern/license)

# always build examples
add_subdirectory(examples)

if (fep_system_cmake_enable_tests)
    find_package(GTest REQUIRED)
    add_subdirectory(test)
    enable_testing()
endif()

# install content from include directory
install(
    DIRECTORY
        include
    DESTINATION
        ./
    FILES_MATCHING PATTERN
        "*.h"
    PATTERN
        ".svn" EXCLUDE
)
