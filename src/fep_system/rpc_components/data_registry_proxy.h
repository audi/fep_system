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
//this will be installed !!
#include "rpc_components/data_registry/data_registry_rpc_intf.h"
#include <fep_system_stubs/data_registry_proxy_stub.h>
#include "system_logger_intf.h"
#ifdef SEVERITY_ERROR
#undef SEVERITY_ERROR
#endif

namespace fep
{
    template<typename T>
    class DataRegistryProxy : public rpc_object_proxy< rpc_proxy_stub::RPCDataRegistryProxy, T>
    {
    private:
        typedef rpc_object_proxy< rpc_proxy_stub::RPCDataRegistryProxy, T> base_type;

    public:
        using base_type::GetStub;
        DataRegistryProxy(std::string participant_name,
                          std::string rpc_component_name,
                          IRPC& rpc) :
                          base_type(participant_name.c_str(), rpc_component_name.c_str(), rpc)
        {
        }
        std::vector<std::string> getSignalsIn() const override
        {
            try
            {
                std::string signal_list = GetStub().getSignalsIn();
                return detail::string_to_stringlist(signal_list);
            }
            catch (...)
            {
                return std::vector<std::string>();
            }
        }
        std::vector<std::string> getSignalsOut() const override
        {
            try
            {
                std::string signal_list = GetStub().getSignalsOut();
                return detail::string_to_stringlist(signal_list);
            }
            catch (...)
            {
                return std::vector<std::string>();
            }
        }
        StreamType getStreamType(const std::string& signal_name) const override
        {
            return StreamType(StreamMetaType("hook"));
        }

    };

    class DataRegistryProxyOldSql : public IRPCObjectClient, public rpc::IRPCDataRegistry
    {
    private:
        std::string _participant_name;
        std::string _component_name;
        ConnectionInterface _coin;
        ISystemLogger& _logger;

        std::vector<fep::cUserSignalOptions> getSignals() const
        {
            std::vector<fep::cUserSignalOptions> signals;
            if (isFailed(_coin.getAI().GetParticipantSignals(signals, _participant_name)))
            {
                _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_ERROR, _participant_name, 
                    "DataRegistry", "Participant does not respond during the given timeout");
                throw std::runtime_error{ "Couldn't determine signals of the following participant: " + _participant_name };
            }
            return std::move(signals);
        }

        std::vector<std::string> getSignals(tSignalDirection direction) const
        {
            std::vector<fep::cUserSignalOptions> signals = getSignals();
            std::vector<std::string> output;
            for (const auto& sig : signals)
            {
                if (sig.GetSignalDirection() == direction)
                {
                    output.push_back(sig.GetSignalName());
                }
            }
            if (output.empty())
            {
                if (direction == SD_Input)
                {
                    _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_INFO, _participant_name,
                        "DataRegistry", "No input signals registered");
                }                
                else if (direction == SD_Output)
                {
                    _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_INFO, _participant_name,
                        "DataRegistry", "No output signals registered");
                }
                else
                {
                    _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_WARNING, _participant_name,
                        "DataRegistry", "Unknown signal direction detected");
                }
            }
            return std::move(output);
        }

    public:
        DataRegistryProxyOldSql(std::string participant_name, ISystemLogger& logger,
            std::string rpc_component_name) :
            _participant_name(participant_name),
            _logger(logger),
            _component_name(rpc_component_name)
        {
        }
        std::string getRPCObjectIID() const override
        {
            return rpc::IRPCDataRegistry::getRPCIID();
        }
        std::string getRPCObjectDefaultName() const override
        {
            return rpc::IRPCDataRegistry::getRPCDefaultName();
        }
        std::vector<std::string> getSignalsIn() const override
        {
            return std::move(getSignals(SD_Input));
        }
        std::vector<std::string> getSignalsOut() const override
        {
            return std::move(getSignals(SD_Output));
        }

        StreamType getStreamType(const std::string& signal_name) const override
        {
            std::vector<fep::cUserSignalOptions> signals = getSignals();
            for (const auto& sig : signals)
            {
                if (sig.GetSignalName() == signal_name)
                {
                    if (sig.IsSignalRaw())
                    {
                        return StreamTypeRaw();
                    }
                    else
                    {
                        return StreamTypeDDL(sig.GetSignalType());
                    }
                }
            }
            _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_ERROR, _participant_name,
                "DataRegistry", "StreamType is not DDL or Raw. Therefore it is currently not supported");
            throw std::runtime_error{ "Signal not found or stream type is not available" };
        }
    };
}
