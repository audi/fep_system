/**
* Declaration of the Class IRPCStateMachine. (can be reached from over rpc)
*
* @file

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
*
*/

#ifndef __FEP_RPC_PARTICIPANT_STATE_MACHINE_INTF_H
#define __FEP_RPC_PARTICIPANT_STATE_MACHINE_INTF_H

#include <vector>
#include <string>
#include "state_machine_rpc_intf_def.h"
#include "../../rpc/fep_rpc_definition.h"
#include "../../../base/states/fep2_state.h"

namespace fep
{
namespace rpc
{
    /**
     * @brief definition of the external service interface state machine content
     * @see delivered state_machine.json file
     */
    class IRPCStateMachine : public IRPCStateMachineDef
    {
        protected:
            /**
             * @brief Destroy the IRPCStateMachine 
             * 
             */
            virtual ~IRPCStateMachine() = default;

        public:
            /**
             * @brief Gets the state oth the participant
             * 
             * @return State 
             * @retval FS_SHUTDOWN Means the participant is not reechable
             * @remark this function will NOT throw
             * @see @ref fep_state_machine
             */
            virtual State getState() const = 0;
            /**
             * @brief sends an initialize event to reach FS_READY
             * @throw logic_error will throw the logic_error exception if the event is denied by the participant
             * @throw runtime_error will throw the runtime_error exception if timeout occured
             * for all other exceptions see @ref fep::IEventMonitor::onLog
             * @see @ref fep_state_machine
             */
            virtual void initialize() = 0;
            /**
             * @brief sends an initialize event to reach FS_RUNNING
             * @throw logic_error will throw the logic_error exception if the event is denied by the participant
             * @throw runtime_error will throw the runtime_error exception if timeout occured
             * for all other exceptions see @ref fep::IEventMonitor::onLog
             * @see @ref fep_state_machine
             */
            virtual void start() = 0;
            /**
             * @brief sends an initialize event to reach FS_IDLE
             * @throw logic_error will throw the logic_error exception if the event is denied by the participant
             * @throw runtime_error will throw the runtime_error exception if timeout occured
             * for all other exceptions see @ref fep::IEventMonitor::onLog
             * @see @ref fep_state_machine
             */
            virtual void stop() = 0;
            /**
             * @brief sends an initialize event to reach FS_SHUTDOWN (and usually exits the participants application)
             * @throw logic_error will throw the logic_error exception if the event is denied by the participant
             * @throw runtime_error will throw the runtime_error exception if timeout occured
             * for all other exceptions see @ref fep::IEventMonitor::onLog
             * @see @ref fep_state_machine
             */
            virtual void shutdown() = 0;
            /**
             * @brief sends an initialize event to reach FS_IDLE thru FS_STARTUP 
             * @throw logic_error will throw the logic_error exception if the event is denied by the participant
             * @throw runtime_error will throw the runtime_error exception if timeout occured
             * for all other exceptions see @ref fep::IEventMonitor::onLog
             * @see @ref fep_state_machine
             */
            virtual void restart() = 0;
    };
}
}

#endif // __FEP_RPC_PARTICIPANT_STATE_MACHINE_INTF_H