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
##################################################################
# tester_system_library
##################################################################
fep_add_gtest(
    tester_system_library
    280
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    system.cpp
    fep_test_common.h)
set_target_properties(tester_system_library PROPERTIES FOLDER test/fep_system)
# fep_core link is needed because of helper library
target_link_libraries(tester_system_library PRIVATE fep_system fep_participant)

fep_deploy_libraries(tester_system_library)
# copy library for integrated tests
if(WIN32)
    add_custom_command(TARGET tester_system_library POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_system> $<TARGET_FILE_DIR:tester_system_library>
    )
endif()

##################################################################
# tester_system_discovery
##################################################################

fep_add_gtest(
    tester_system_discovery
    280
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    system_discovery.cpp
    fep_test_common.h
)
set_target_properties(tester_system_discovery PROPERTIES FOLDER test/fep_system)
# fep_core link is needed because of helper library
target_link_libraries(tester_system_discovery PRIVATE fep_system fep_participant)

fep_deploy_libraries(tester_system_discovery)
# copy library for integrated tests
if(WIN32)
    add_custom_command(TARGET tester_system_discovery POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_system> $<TARGET_FILE_DIR:tester_system_discovery>
    )
endif()

##################################################################
# tester_system_library_participant
##################################################################

fep_add_gtest(
    tester_system_library_participant
    280
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    participant.cpp
    participant_configuration.cpp
    participant_configuration_old.cpp
    fep_test_common.h
)
set_target_properties(tester_system_library_participant PROPERTIES FOLDER test/fep_system)
# fep_core link is needed because of helper library
target_link_libraries(tester_system_library_participant PRIVATE fep_system fep_participant)

fep_deploy_libraries(tester_system_library_participant)
# copy library for integrated tests
if(WIN32)
    add_custom_command(TARGET tester_system_library_participant POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_system> $<TARGET_FILE_DIR:tester_system_library>
    )
endif()
