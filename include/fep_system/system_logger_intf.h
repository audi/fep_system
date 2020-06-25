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
#include "base/logging/logging_levels.h"

namespace fep
{
    class ISystemLogger
    {
        protected:
            virtual ~ISystemLogger() = default;
        public:
            virtual void log(logging::Category cat,
                             logging::Severity level,
                             const std::string& participant_name,
                             const std::string& logger_name, //depends on the Category ... 
                             const std::string& message) const = 0;
    };
}
