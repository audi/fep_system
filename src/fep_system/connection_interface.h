/**
* @file
*
* @copyright
* @verbatim
Copyright @ 2018 AUDI AG. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.
@endverbatim
*/

#pragma once
#include <string> 
#include "fep_participant_sdk.h"
#include "a_util/datetime.h"

#include "fep_system/fep_system.h"

//if using this connection interface the we want to use automation interface by purpose
//thats why we disable the deprecated warning here! 
#pragma warning(disable : 4996)

namespace fep
{
    class ConnectionInterface
    {
    public:        
        ConnectionInterface() = default;
        ~ConnectionInterface() = default;
        ConnectionInterface(const ConnectionInterface&) = delete;
        ConnectionInterface& operator=(const ConnectionInterface&) = delete;

        // core guidelines are happy with this implementation
        AutomationInterface& getAI(int domain_id=-1) const
        {
            static cModuleOptions option;
            if (domain_id >= 0)
            {
                option.SetDomainId(static_cast<uint16_t>(domain_id));
            }
            static AutomationInterface _ai(option);
            return _ai;
        }
    };
}
