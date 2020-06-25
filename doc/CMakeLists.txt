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

option(fep_system_cmake_enable_documentation
       "If enabled, generate the source code documentation - requires doxygen (default: ON)" ON)
option(fep_system_cmake_enable_internal_documentation
       "If enabled, generate internal the source code documentation -\
 requires doxygen (default: OFF)" OFF)
option(fep_system_cmake_enable_post_install_doc_generation "If enabled, doxygen will generate the\
 documentation as post-install event. Disable only for development purposes. (default: ON)" ON)
option(DOXYGEN_SKIP_DOT "If true this module will skip trying to find Dot (default: OFF)" OFF)

if(fep_system_cmake_enable_documentation)
    ## Doxygen is not required. But only built html Doku if doxygen is found
    ## Otherwise just the Markdown docu will be copied.
    find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(STATUS "No doxygen executable found.")
        return()
    endif(NOT DOXYGEN_FOUND)
    if(NOT DOXYGEN_SKIP_DOT AND NOT DOXYGEN_DOT_FOUND)
        message(FATAL_ERROR "No dot executable found. Either set the "
                            "correct DOXYGEN_DOT_EXECUTABLE or enable DOXYGEN_SKIP_DOT.")
    elseif(NOT DOXYGEN_SKIP_DOT AND DOXYGEN_DOT_FOUND)
        EXECUTE_PROCESS(COMMAND ${DOXYGEN_DOT_EXECUTABLE} -V OUTPUT_VARIABLE dot_version_info
                                                             ERROR_VARIABLE dot_version_info
                                                             OUTPUT_STRIP_TRAILING_WHITESPACE
                                                             ERROR_STRIP_TRAILING_WHITESPACE)
        set(dot_version_info "(found version: \"${dot_version_info}\")")
        message(STATUS "Found dot: ${DOXYGEN_DOT_EXECUTABLE} ${dot_version_info}")
        unset(dot_version_info)
    endif(NOT DOXYGEN_SKIP_DOT AND NOT DOXYGEN_DOT_FOUND)
else()
    return()
endif()

##Set necessary variables to configure into the run_doxygen.cmake.in file
if(BETA_BUILD)
    set(BETA_FLAG "BETA")
else(BETA_BUILD)
    set(BETA_FLAG "")
endif(BETA_BUILD)

if(fep_system_cmake_enable_internal_documentation)
    set(EXTRACT_PRIVATE YES)
    set(INTERNAL_DOCS YES)
    set(INPUT_INTERNAL ${PROJECT_ROOT}/src)
    message(STATUS "Internal documentation is enabled\
 -> Doxygen generated documentation will include all internal and private structures!")
else()
    set(EXTRACT_PRIVATE NO)
    set(INTERNAL_DOCS NO)
    set(INPUT_INTERNAL "")
endif()

configure_file(run_doxygen.cmake.in run_doxygen.cmake @ONLY)
##create seperate target excluded from all
add_custom_target(fep_system_DOC ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/run_doxygen.cmake)
set_target_properties(fep_system_DOC PROPERTIES FOLDER process)

install(FILES extern/fep-system.html extern/changelog.md DESTINATION doc)
install(DIRECTORY input DESTINATION doc/doxygen PATTERN "fep_main.dox" EXCLUDE)

##post install script
if(fep_system_cmake_enable_post_install_doc_generation)
    install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/run_doxygen.cmake)
endif()
