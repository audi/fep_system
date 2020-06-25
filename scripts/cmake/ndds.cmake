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
set(NDDS_DIR "" CACHE PATH "Path to NDDS root containing lib and include folder.")
if(NDDS_DIR)
    set(NDDS_INCLUDE_DIRS ${NDDS_DIR}/include ${NDDS_DIR}/include/ndds)
    set(NDDS_LINK_DIR ${NDDS_DIR}/lib)
    
    if(USE_SHARED_RTIDDS)
        set(NDDS_STATIC_LINK_FLAG "")
    else()
        set(NDDS_STATIC_LINK_FLAG "z")
    endif()
    
    set(NDDS_LIBS
        debug nddscpp${NDDS_STATIC_LINK_FLAG}d
        debug nddsc${NDDS_STATIC_LINK_FLAG}d
        debug nddscore${NDDS_STATIC_LINK_FLAG}d
        optimized nddscpp${NDDS_STATIC_LINK_FLAG}
        optimized nddsc${NDDS_STATIC_LINK_FLAG}
        optimized nddscore${NDDS_STATIC_LINK_FLAG}
    )
    if (WIN32)
        if (NOT USE_SHARED_RTIDDS)
            install(FILES
                ${NDDS_LINK_DIR}/advlogzd.pdb
                ${NDDS_LINK_DIR}/advlog_log_normalzd.pdb
                ${NDDS_LINK_DIR}/cdrzd.pdb
                ${NDDS_LINK_DIR}/cdr_log_normalzd.pdb
                ${NDDS_LINK_DIR}/clockzd.pdb
                ${NDDS_LINK_DIR}/clock_log_normalzd.pdb
                ${NDDS_LINK_DIR}/commendzd.pdb
                ${NDDS_LINK_DIR}/commend_log_normalzd.pdb
                ${NDDS_LINK_DIR}/core_versionzd.pdb
                ${NDDS_LINK_DIR}/core_version_log_normalzd.pdb
                ${NDDS_LINK_DIR}/dds_cppzd.pdb
                ${NDDS_LINK_DIR}/dds_czd.pdb
                ${NDDS_LINK_DIR}/dds_c_log_normalzd.pdb
                ${NDDS_LINK_DIR}/disczd.pdb
                ${NDDS_LINK_DIR}/disc_log_normalzd.pdb
                ${NDDS_LINK_DIR}/dl_driverzd.pdb
                ${NDDS_LINK_DIR}/dl_driver_log_normalzd.pdb
                ${NDDS_LINK_DIR}/eventzd.pdb
                ${NDDS_LINK_DIR}/event_log_normalzd.pdb
                ${NDDS_LINK_DIR}/log_normalzd.pdb
                ${NDDS_LINK_DIR}/migzd.pdb
                ${NDDS_LINK_DIR}/mig_log_normalzd.pdb
                ${NDDS_LINK_DIR}/netiozd.pdb
                ${NDDS_LINK_DIR}/netio_log_normalzd.pdb
                ${NDDS_LINK_DIR}/osapizd.pdb
                ${NDDS_LINK_DIR}/osapi_log_normalzd.pdb
                ${NDDS_LINK_DIR}/preszd.pdb
                ${NDDS_LINK_DIR}/pres_log_normalzd.pdb
                ${NDDS_LINK_DIR}/redazd.pdb
                ${NDDS_LINK_DIR}/reda_log_normalzd.pdb
                ${NDDS_LINK_DIR}/rtixmlzd.pdb
                ${NDDS_LINK_DIR}/rtixml_log_normalzd.pdb
                ${NDDS_LINK_DIR}/transportzd.pdb
                ${NDDS_LINK_DIR}/transport_log_normalzd.pdb
                ${NDDS_LINK_DIR}/writer_historyzd.pdb
                ${NDDS_LINK_DIR}/writer_history_log_normalzd.pdb
                DESTINATION lib CONFIGURATIONS Debug
            )
            
            install(FILES
                ${NDDS_LINK_DIR}/advlogz.pdb
                ${NDDS_LINK_DIR}/advlog_log_normalz.pdb
                ${NDDS_LINK_DIR}/cdrz.pdb
                ${NDDS_LINK_DIR}/cdr_log_normalz.pdb
                ${NDDS_LINK_DIR}/clockz.pdb
                ${NDDS_LINK_DIR}/clock_log_normalz.pdb
                ${NDDS_LINK_DIR}/commendz.pdb
                ${NDDS_LINK_DIR}/commend_log_normalz.pdb
                ${NDDS_LINK_DIR}/core_versionz.pdb
                ${NDDS_LINK_DIR}/core_version_log_normalz.pdb
                ${NDDS_LINK_DIR}/dds_cppz.pdb
                ${NDDS_LINK_DIR}/dds_cz.pdb
                ${NDDS_LINK_DIR}/dds_c_log_normalz.pdb
                ${NDDS_LINK_DIR}/discz.pdb
                ${NDDS_LINK_DIR}/disc_log_normalz.pdb
                ${NDDS_LINK_DIR}/dl_driverz.pdb
                ${NDDS_LINK_DIR}/dl_driver_log_normalz.pdb
                ${NDDS_LINK_DIR}/eventz.pdb
                ${NDDS_LINK_DIR}/event_log_normalz.pdb
                ${NDDS_LINK_DIR}/log_normalz.pdb
                ${NDDS_LINK_DIR}/migz.pdb
                ${NDDS_LINK_DIR}/mig_log_normalz.pdb
                ${NDDS_LINK_DIR}/netioz.pdb
                ${NDDS_LINK_DIR}/netio_log_normalz.pdb
                ${NDDS_LINK_DIR}/osapiz.pdb
                ${NDDS_LINK_DIR}/osapi_log_normalz.pdb
                ${NDDS_LINK_DIR}/presz.pdb
                ${NDDS_LINK_DIR}/pres_log_normalz.pdb
                ${NDDS_LINK_DIR}/redaz.pdb
                ${NDDS_LINK_DIR}/reda_log_normalz.pdb
                ${NDDS_LINK_DIR}/rtixmlz.pdb
                ${NDDS_LINK_DIR}/rtixml_log_normalz.pdb
                ${NDDS_LINK_DIR}/transportz.pdb
                ${NDDS_LINK_DIR}/transport_log_normalz.pdb
                ${NDDS_LINK_DIR}/writer_historyz.pdb
                ${NDDS_LINK_DIR}/writer_history_log_normalz.pdb
                DESTINATION lib CONFIGURATIONS RelWithDebInfo
            )
        endif()

        set(NDDS_COMPILE_DEFINITIONS RTI_WIN32)
        set(NDDS_QUALIFIED_LIBS
            $<$<CONFIG:Debug>:${NDDS_LINK_DIR}/nddsc${NDDS_STATIC_LINK_FLAG}d.lib>
            $<$<CONFIG:Debug>:${NDDS_LINK_DIR}/nddscore${NDDS_STATIC_LINK_FLAG}d.lib>
            $<$<CONFIG:Debug>:${NDDS_LINK_DIR}/nddscpp${NDDS_STATIC_LINK_FLAG}d.lib>
            $<$<CONFIG:RelWithDebInfo>:${NDDS_LINK_DIR}/nddsc${NDDS_STATIC_LINK_FLAG}.lib>
            $<$<CONFIG:RelWithDebInfo>:${NDDS_LINK_DIR}/nddscore${NDDS_STATIC_LINK_FLAG}.lib>
            $<$<CONFIG:RelWithDebInfo>:${NDDS_LINK_DIR}/nddscpp${NDDS_STATIC_LINK_FLAG}.lib>
            $<$<CONFIG:Release>:${NDDS_LINK_DIR}/nddsc${NDDS_STATIC_LINK_FLAG}.lib>
            $<$<CONFIG:Release>:${NDDS_LINK_DIR}/nddscore${NDDS_STATIC_LINK_FLAG}.lib>
            $<$<CONFIG:Release>:${NDDS_LINK_DIR}/nddscpp${NDDS_STATIC_LINK_FLAG}.lib>
        )
    else()
        set(NDDS_COMPILE_DEFINITIONS RTI_UNIX)
        set(NDDS_QUALIFIED_LIBS 
            $<$<CONFIG:Debug>:${NDDS_LINK_DIR}/libnddsc${NDDS_STATIC_LINK_FLAG}d.a>
            $<$<CONFIG:Debug>:${NDDS_LINK_DIR}/libnddscore${NDDS_STATIC_LINK_FLAG}d.a>
            $<$<CONFIG:Debug>:${NDDS_LINK_DIR}/libnddscpp${NDDS_STATIC_LINK_FLAG}d.a>
            $<$<CONFIG:RelWithDebInfo>:${NDDS_LINK_DIR}/libnddsc${NDDS_STATIC_LINK_FLAG}.a>
            $<$<CONFIG:RelWithDebInfo>:${NDDS_LINK_DIR}/libnddscore${NDDS_STATIC_LINK_FLAG}.a>
            $<$<CONFIG:RelWithDebInfo>:${NDDS_LINK_DIR}/libnddscpp${NDDS_STATIC_LINK_FLAG}.a>
            $<$<CONFIG:Release>:${NDDS_LINK_DIR}/libnddsc${NDDS_STATIC_LINK_FLAG}.a>
            $<$<CONFIG:Release>:${NDDS_LINK_DIR}/libnddscore${NDDS_STATIC_LINK_FLAG}.a>
            $<$<CONFIG:Release>:${NDDS_LINK_DIR}/libnddscpp${NDDS_STATIC_LINK_FLAG}.a>
        )
    endif()
    link_directories(${NDDS_LINK_DIR})
else(NDDS_DIR)
    MESSAGE(FATAL_ERROR "You have to set NDDS_DIR.")
endif(NDDS_DIR)
