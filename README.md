<!---
  Copyright @ 2019 Audi AG. All rights reserved.
  
      This Source Code Form is subject to the terms of the Mozilla
      Public License, v. 2.0. If a copy of the MPL was not distributed
      with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
  
  If it is not possible or desirable to put the notice in a particular file, then
  You may include the notice in a location (such as a LICENSE file in a
  relevant directory) where a recipient would be likely to look for such a notice.
  
  You may add additional accurate notices of copyright ownership.
  -->
FEP SDK - System Library
============================

## Description ##

This installed package contains (or pulls in) all components of the FEP SDK - System Library

* FEP System Library
  * connects the FEP Service Bus to discover and control a system
* FEP System Library Example
* FEP System Library Documentation 

## How to use ###

The FEP System Library provides a CMake >= 3.5 configuration. Here's how to use it from your own CMake projects:

    find_package(fep_system REQUIRED)

You can append the FEP System Libray Target to your existing targets to add a dependency:

    target_link_libraries(my_existing_target PRIVATE fep_system)
    fep_deploy_libraries(my_existing_target)

Note that the fep_system target will transitively pull in all required include directories and libraries.

### Build Environment ####

The libraries are build and tested only under following compilers and operating systems: 
* Windows 10 x64 with Visual Studio C++ 2017 and compiler toolset v140
* Linux Ubuntu 16.04 LTS x64 with GCC 5.4 and libstdc++11 (C++11 ABI)

## How to build the examples ###

Simply point CMake to the examples directory (containing the CMakeLists.txt file) and generate a project.
Choose "Visual Studio 15 2017 Win64" and "v140" toolset the or "Unix Makefiles" generator, depending on your platform.

CMake might ask for the CMAKE_BUILD_TYPE variable to be defined. Possible values are Debug, Release or RelWithDebInfo
