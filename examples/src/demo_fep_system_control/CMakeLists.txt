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
if(EXAMPLES_INTEGRATED)
else()
    find_package(fep_system REQUIRED)
endif()

set(DEMO_FEP_SYSTEM_CONTROL demo_fep_system_control)

add_executable(${DEMO_FEP_SYSTEM_CONTROL}
    main.cpp
)

target_link_libraries(${DEMO_FEP_SYSTEM_CONTROL} fep_system)

fep_install(${DEMO_FEP_SYSTEM_CONTROL} examples/bin)
fep_deploy_libraries(${DEMO_FEP_SYSTEM_CONTROL})
install(
        FILES
            $<TARGET_FILE:fep_system>
        CONFIGURATIONS Release Debug RelWithDebInfo
        DESTINATION examples/bin
    )
add_custom_command(TARGET ${DEMO_FEP_SYSTEM_CONTROL}  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_system> $<TARGET_FILE_DIR:${DEMO_FEP_SYSTEM_CONTROL}>
)
set_target_properties(${DEMO_FEP_SYSTEM_CONTROL} PROPERTIES INSTALL_RPATH "$ORIGIN")


fep_set_folder(${DEMO_FEP_SYSTEM_CONTROL} examples)
