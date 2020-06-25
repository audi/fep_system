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
# fep system library 
set(SYSTEM_SOURCES_PUBLIC
    ${PROJECT_SOURCE_DIR}/include/fep_system/fep_system_export.h
    ${PROJECT_SOURCE_DIR}/include/fep_system/fep_system_types.h
    ${PROJECT_SOURCE_DIR}/include/fep_system/fep_system.h
    ${PROJECT_SOURCE_DIR}/include/fep_system/system_logger_intf.h
    ${PROJECT_SOURCE_DIR}/include/fep_system/participant_proxy.h
    ${PROJECT_SOURCE_DIR}/include/fep_system/rpc_component_proxy.h)

# install destination should not be forgotten: include/fep_system/rpc_components/rpc
set(SYSTEM_EXT_PUBLIC_SOURCES_RPC
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/rpc/fep_rpc_definition.h
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/rpc/fep_rpc_object_client_intf.h
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/rpc/fep_rpc_client.h
    )
    
# install destination should not be forgotten: include/fep_system/base/logging
set(SYSTEM_EXT_PUBLIC_SOURCES_LOGGING_BASE
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/logging/logging_levels.h
    )

# subtle difference: on unix the command silently fails if the output directory does not exist...
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs) 

# subtle difference: on unix the command silently fails if the output directory does not exist...
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include/fep_system/rpc_components) 

###################################################
# Participant Info
###################################################

# install destination should not be forgotten: include/fep_system/rpc_components/participant_info
set(SYSTEM_EXT_PUBLIC_SOURCES_PARTICIPANT_INFO
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/participant_info/participant_info_rpc_intf_def.h
    )
     
jsonrpc_generate_client_stub(${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/participant_info/participant_info.json
                             fep::rpc_proxy_stub::RPCParticipantInfoProxy
                             ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/participant_info_proxy_stub.h)
     

###################################################
# Legacy State Machine
###################################################

# install destination should not be forgotten: include/fep_system/rpc_components/legacy/state_machine
set(SYSTEM_EXT_PUBLIC_SOURCES_STATE_MACHINE_FEP2
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/legacy/state_machine/state_machine_rpc_intf_def.h
    )
set(SYSTEM_EXT_PUBLIC_SOURCES_STATES_BASE
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/states/fep2_state.h
    )
    
jsonrpc_generate_client_stub(${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/legacy/state_machine/state_machine.json
                             fep::rpc_proxy_stub::RPCStateMachineProxy
                             ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/state_machine_proxy_stub.h)
                   
jsonrpc_generate_client_stub(${PARTICIPANT_LIB_DIR}/include/fep3/components/rpc/rpc_element_object_server.json
                             fep::rpc_stubs::cRPCElementObjectClient
                             ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/rpc_stubs_element_object_client.h)
 

###################################################
# Data Registry
###################################################

# install destination should not be forgotten: include/fep_system/rpc_components/legacy/state_machine
set(SYSTEM_EXT_PUBLIC_SOURCES_DATA_REGISTRY
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/data_registry/data_registry_rpc_intf_def.h
    )

# install destination should not be forgotten: ${PROJECT_BINARY_DIR}/include/fep_system/rpc_components/data_registry
set(SYSTEM_EXT_PUBLIC_SOURCES_STREAMTYPE_BASE
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/streamtype/default_streamtype.h
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/streamtype/streamtype.h
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/streamtype/streamtype_intf.h
    ) 
    
# install destination should not be forgotten: ${PROJECT_BINARY_DIR}/include/fep_system/rpc_components/properties
set(SYSTEM_EXT_PUBLIC_SOURCES_PROPERTY_BASE
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/properties/properties.h
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/properties/properties_intf.h
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/properties/property_type.h
    ${PARTICIPANT_LIB_DIR}/include/fep3/base/properties/property_type_conversion.h
    )

jsonrpc_generate_client_stub(
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/data_registry/data_registry.json
    fep::rpc_proxy_stub::RPCDataRegistryProxy
    ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/data_registry_proxy_stub.h)

# properties for the moment
file(COPY ${SYSTEM_EXT_PUBLIC_SOURCES_PROPERTY_BASE}
     DESTINATION ${PROJECT_BINARY_DIR}/include/fep_system/base/properties)

###################################################
# Configuration
################################################### 

# install destination should not be forgotten: include/fep_system/rpc_components/configuration
set(SYSTEM_EXT_PUBLIC_SOURCES_CONFIGURATION
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/configuration/configuration_rpc_intf_def.h
    )






set(SYSTEM_PROXY_SOURCES_PRIVATE
    rpc_components/participant_info_proxy.h
    ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/participant_info_proxy_stub.h

    rpc_components/state_machine_proxy.h
    ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/state_machine_proxy_stub.h

    rpc_components/data_registry_proxy.h
    ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/data_registry_proxy_stub.h

    ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs/rpc_stubs_element_object_client.h
    
    rpc_components/configuration_proxy.h)
    
set(SYSTEM_SOURCES_PRIVATE
    fep_system.cpp
    participant_proxy.cpp
    connection_interface.h
	system_logger.h
    private_participant_proxy.h)

add_library(${FEP_SYSTEM_LIBRARY} SHARED
    ${SYSTEM_SOURCES_PUBLIC}
    ${SYSTEM_SOURCES_PRIVATE}
    ${SYSTEM_PROXY_SOURCES_PRIVATE}
    ${SYSTEM_EXT_PUBLIC_SOURCES_RPC}
    ${SYSTEM_EXT_PUBLIC_SOURCES_LOGGING_BASE}

    ${SYSTEM_EXT_PUBLIC_SOURCES_PARTICIPANT_INFO}

    ${SYSTEM_EXT_PUBLIC_SOURCES_STATE_BASE}
    ${SYSTEM_EXT_PUBLIC_SOURCES_STATE_MACHINE_FEP2}
    
    ${SYSTEM_EXT_PUBLIC_SOURCES_CONFIGURATION}
    ${SYSTEM_EXT_PUBLIC_SOURCES_PROPERTY_BASE}

    ${SYSTEM_EXT_PUBLIC_SOURCES_DATA_REGISTRY}
    ${SYSTEM_EXT_PUBLIC_SOURCES_STREAMTYPE_BASE}
)

set_target_properties(${FEP_SYSTEM_LIBRARY} PROPERTIES
    OUTPUT_NAME ${FEP_SYSTEM_LIBRARY}
    DEBUG_POSTFIX "d${FEP_SYSTEM_LIB_VERSION_MAJOR}.${FEP_SYSTEM_LIB_VERSION_MINOR}"
    RELEASE_POSTFIX "${FEP_SYSTEM_LIB_VERSION_MAJOR}.${FEP_SYSTEM_LIB_VERSION_MINOR}"
    RELWITHDEBINFO_POSTFIX "${FEP_SYSTEM_LIB_VERSION_MAJOR}.${FEP_SYSTEM_LIB_VERSION_MINOR}"
)

set_target_properties(${FEP_SYSTEM_LIBRARY} PROPERTIES CXX_VISIBILITY_PRESET hidden)

set(BUILD_INTERFACE_INCLUDES 
    ${CMAKE_CURRENT_LIST_DIR} 
    ${PROJECT_SOURCE_DIR}/include 
    ${PROJECT_BINARY_DIR}/include 
    ${PROJECT_SOURCE_DIR}/include/fep_system
    ${PARTICIPANT_LIB_DIR}/include/fep3
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/clock
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/participant_info
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/data_registry
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/configuration
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/legacy/state_machine
    ${PARTICIPANT_LIB_DIR}/include/fep3/rpc_components/rpc
    ${PROJECT_BINARY_DIR}/src/fep_system
    ${PROJECT_BINARY_DIR}/src/fep_system/fep_system_stubs
)

set(INSTALL_INTERFACE_INCLUDES 
    include)

if(WIN32)
    set(BUILD_LIBRARY_FLAGS "")
else()
    set(BUILD_LIBRARY_FLAGS "-Wl,--start-group;-Wl,--no-as-needed")
endif()

target_link_libraries(${FEP_SYSTEM_LIBRARY}
    PRIVATE
        a_util_datetime
        a_util_system
        a_util_process
        a_util_strings
        fep_participant
        pkg_rpc
    PUBLIC
        ${BUILD_LIBRARY_FLAGS}
    
)

target_include_directories(${FEP_SYSTEM_LIBRARY} PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
)
#install base types needed within the interfaces
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_RPC} DESTINATION include/fep_system/rpc_components/rpc)
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_LOGGING_BASE} DESTINATION include/fep_system/base/logging)
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_PROPERTY_BASE} DESTINATION include/fep_system/base/properties)
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_STREAMTYPE_BASE} DESTINATION include/fep_system/base/streamtype)
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_STATES_BASE} DESTINATION include/fep_system/base/states)



install(DIRECTORY ${PROJECT_BINARY_DIR}/include/fep_system/rpc_components DESTINATION include/fep_system)

install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_PARTICIPANT_INFO} DESTINATION include/fep_system/rpc_components/participant_info)
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_STATE_MACHINE_FEP2} DESTINATION include/fep_system/rpc_components/legacy/state_machine)
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_DATA_REGISTRY} DESTINATION include/fep_system/rpc_components/data_registry)
install(FILES ${SYSTEM_EXT_PUBLIC_SOURCES_CONFIGURATION} DESTINATION include/fep_system/rpc_components/configuration)

target_compile_definitions(${FEP_SYSTEM_LIBRARY} PRIVATE FEP_SYSTEM_LIB_DO_EXPORT)
set_target_properties(${FEP_SYSTEM_LIBRARY} PROPERTIES CXX_VISIBILITY_PRESET hidden)

fep_set_folder(${FEP_SYSTEM_LIBRARY} system_library)
source_group(system_library\\public FILES ${SYSTEM_SOURCES_PUBLIC})
source_group(system_library FILES ${SYSTEM_SOURCES_PRIVATE})
source_group(system_library\\proxy FILES ${SYSTEM_PROXY_SOURCES_PRIVATE})

configure_file(fep_system-config.cmake.in ${CMAKE_BINARY_DIR}/fep_system-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/fep_system-config.cmake DESTINATION .)

# Create version config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/fep_system-config-version.cmake
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fep_system-config-version.cmake DESTINATION .)
install(FILES fep_system-macros.cmake DESTINATION .)

fep_install(${FEP_SYSTEM_LIBRARY} lib)
fep_deploy_libraries(${FEP_SYSTEM_LIBRARY})

install(TARGETS ${FEP_SYSTEM_LIBRARY} EXPORT fep_system_export
    DESTINATION lib
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(EXPORT fep_system_export DESTINATION lib/cmake
    FILE fep_system_targets.cmake   
)