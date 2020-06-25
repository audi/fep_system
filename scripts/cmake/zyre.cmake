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
if(ZYRE_FOUND)
    # libzmq
    get_target_property(LIBZMQ_LIBRARY_DEBUG           ${LIBZMQ_LIBRARIES} IMPORTED_LOCATION_DEBUG)
    get_target_property(LIBZMQ_LIBRARY_RELEASE         ${LIBZMQ_LIBRARIES} IMPORTED_LOCATION_RELEASE)
    get_target_property(LIBZMQ_LIBRARY_RELWITHDEBINFO  ${LIBZMQ_LIBRARIES} IMPORTED_LOCATION_RELWITHDEBINFO)
    get_target_property(LIBZMQ_LIBRARY_MINSIZEREL      ${LIBZMQ_LIBRARIES} IMPORTED_LOCATION_MINSIZEREL )
    
    get_target_property(CZMQ_LIBRARY_DEBUG           ${CZMQ_LIBRARY} IMPORTED_LOCATION_DEBUG)
    get_target_property(CZMQ_LIBRARY_RELEASE         ${CZMQ_LIBRARY} IMPORTED_LOCATION_RELEASE)
    get_target_property(CZMQ_LIBRARY_RELWITHDEBINFO  ${CZMQ_LIBRARY} IMPORTED_LOCATION_RELWITHDEBINFO)
    get_target_property(CZMQ_LIBRARY_MINSIZEREL      ${CZMQ_LIBRARY} IMPORTED_LOCATION_MINSIZEREL )

    get_target_property(ZYRE_LIBRARY_DEBUG           ${ZYRE_LIBRARY} IMPORTED_LOCATION_DEBUG)
    get_target_property(ZYRE_LIBRARY_RELEASE         ${ZYRE_LIBRARY} IMPORTED_LOCATION_RELEASE)
    get_target_property(ZYRE_LIBRARY_RELWITHDEBINFO  ${ZYRE_LIBRARY} IMPORTED_LOCATION_RELWITHDEBINFO)
    get_target_property(ZYRE_LIBRARY_MINSIZEREL      ${ZYRE_LIBRARY} IMPORTED_LOCATION_MINSIZEREL )
    
    set(ZYRE_QUALIFIED_LIBS
        $<$<CONFIG:Debug>:${ZYRE_LIBRARY_DEBUG}>
        $<$<CONFIG:Release>:${ZYRE_LIBRARY_RELEASE}>
        $<$<CONFIG:RelWithDebInfo>:${ZYRE_LIBRARY_RELWITHDEBINFO}>
        $<$<CONFIG:MinSizeRel>:${ZYRE_LIBRARY_MINSIZEREL}>
        $<$<CONFIG:Debug>:${CZMQ_LIBRARY_DEBUG}>
        $<$<CONFIG:Release>:${CZMQ_LIBRARY_RELEASE}>
        $<$<CONFIG:RelWithDebInfo>:${CZMQ_LIBRARY_RELWITHDEBINFO}>
        $<$<CONFIG:MinSizeRel>:${CZMQ_LIBRARY_MINSIZEREL}>
        $<$<CONFIG:Debug>:${LIBZMQ_LIBRARY_DEBUG}>
        $<$<CONFIG:Release>:${LIBZMQ_LIBRARY_RELEASE}>
        $<$<CONFIG:RelWithDebInfo>:${LIBZMQ_LIBRARY_RELWITHDEBINFO}>
        $<$<CONFIG:MinSizeRel>:${LIBZMQ_LIBRARY_MINSIZEREL}>
   )
else(ZYRE_FOUND)
    MESSAGE(FATAL_ERROR "You have to found zyre (ZYRE_FOUND).")
endif(ZYRE_FOUND)