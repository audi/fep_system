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

#include <fep3/components/rpc/fep_rpc.h>
#include <fep_system_stubs/state_machine_proxy_stub.h>

#include "rpc_components/legacy/state_machine/state_machine_rpc_intf.h"

namespace fep
{
    template<typename T>
    class StateMachineProxy : public rpc_object_proxy< rpc_proxy_stub::RPCStateMachineProxy, T>
    {
    private:
        typedef rpc_object_proxy< rpc_proxy_stub::RPCStateMachineProxy, T> base_type;

    public:
        using base_type::GetStub;
        StateMachineProxy(std::string participant_name,
                          std::string rpc_component_name,
                          IRPC& rpc) :
                          base_type(participant_name.c_str(), rpc_component_name.c_str(), rpc)
        {
        }

        rpc::IRPCStateMachine::State getState() const override 
        {
            try
            {
                int val = GetStub().getState();
                rpc::IRPCStateMachine::State state = static_cast<rpc::IRPCStateMachine::State>(val);
                return state;
            }
            catch (...)
            {
                return fep::FS_SHUTDOWN;
            }
        }
        void initialize() override
        {
            if (!GetStub().initialize())
            {
                throw std::logic_error("state machine intialize denied");
            }
        }
        void start() override
        {
            if (!GetStub().start())
            {
                throw std::logic_error("state machine start denied");
            }
        }
        void stop() override
        {
            if (!GetStub().initialize())
            {
                throw std::logic_error("state machine initialize denied");
            }
        }
        void shutdown() override
        {
            if (!GetStub().shutdown())
            {
                throw std::logic_error("state machine shutdown denied");
            }
        }
        void restart() override
        {
            if (!GetStub().restart())
            {
                throw std::logic_error("state machine restart denied");
            }
        }
    };

    class StateMachineProxyOldSql : public IRPCObjectClient, public rpc::IRPCStateMachine
    {
    private:
        std::string _participant_name;
        std::string _component_name;
        ConnectionInterface _coin;
        ISystemLogger& _logger;

    public:
        StateMachineProxyOldSql(std::string participant_name, ISystemLogger& logger,
            std::string rpc_component_name) :
            _participant_name(participant_name),
            _logger(logger),
            _component_name(rpc_component_name)
        {
        }
        std::string getRPCObjectIID() const override
        {
            return rpc::IRPCStateMachine::getRPCIID();
        }
        std::string getRPCObjectDefaultName() const override
        {
            return rpc::IRPCStateMachine::getRPCDefaultName();
        }

        State getState() const override
        {
            State state;
            auto res = _coin.getAI().GetParticipantState(state, _participant_name);
            if (res == ERR_TIMEOUT)
            {
                return FS_SHUTDOWN;
            }
            else if (isFailed(res))
            {
                _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_FATAL, _participant_name, "IRPCStateMachine",
                    "Couldn't determine the state of the following participant: " + _participant_name);
                throw std::runtime_error{ "Couldn't determine the state of the following participant: " + _participant_name };
            }
            return state;
        }
        void initialize() override
        {
            if (isFailed(_coin.getAI().TriggerEvent(CE_Initialize, _participant_name)))
            {
                _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_FATAL, _participant_name, "IRPCStateMachine",
                    "Couldn't trigger the following participant: " + _participant_name);
                throw std::runtime_error{ "Couldn't trigger the following participant: " + _participant_name };
            }
        }
        void start() override
        {
            if (isFailed(_coin.getAI().TriggerEvent(CE_Start, _participant_name)))
            {
                _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_FATAL, _participant_name, "IRPCStateMachine",
                    "Couldn't trigger the following participant: " + _participant_name);
                throw std::runtime_error{ "Couldn't trigger the following participant: " + _participant_name };
            }
        }
        void stop() override
        {
            if (isFailed(_coin.getAI().TriggerEvent(CE_Stop, _participant_name)))
            {
                _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_FATAL, _participant_name, "IRPCStateMachine",
                    "Couldn't trigger the following participant: " + _participant_name);
                throw std::runtime_error{ "Couldn't trigger the following participant: " + _participant_name };
            }
        }
        void shutdown() override
        {
            if (isFailed(_coin.getAI().TriggerEvent(CE_Shutdown, _participant_name)))
            {
                _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_FATAL, _participant_name, "IRPCStateMachine",
                    "Couldn't trigger the following participant: " + _participant_name);
                throw std::runtime_error{ "Couldn't trigger the following participant: " + _participant_name };
            }
        }
        void restart() override
        {
            if (isFailed(_coin.getAI().TriggerEvent(CE_Restart, _participant_name)))
            {
                _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_FATAL, _participant_name, "IRPCStateMachine",
                    "Couldn't trigger the following participant: " + _participant_name);
                throw std::runtime_error{ "Couldn't trigger the following participant: " + _participant_name };
            }
        }
    };
}
