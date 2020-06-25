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
#include <fep_participant_sdk.h>
#include "connection_interface.h"
#include "system_logger_intf.h"

#include "rpc_components/participant_info_proxy.h"
#include "rpc_components/state_machine_proxy.h"
#include "rpc_components/data_registry_proxy.h"
#include "rpc_components/configuration_proxy.h"
#include <math.h>

namespace fep
{
    struct ParticipantProxy::PrivateParticipantProxy
    {
    public:
        PrivateParticipantProxy(const std::string& participant_name,
            const std::string& branding, 
            ISystemLogger& logger,
            timestamp_t default_timeout) :
            _participant_name(participant_name),
            _system_name(branding),
            _logger(logger),
            _init_priority(0),
            _start_priority(0),
            _default_timeout(default_timeout)
        {
        }
        virtual ~PrivateParticipantProxy()
        {
        }

        void copyValuesTo(PrivateParticipantProxy& other) const
        {
            other._participant_name = _participant_name;
            other._system_name = _system_name;
            other._init_priority = _init_priority;
            other._start_priority = _start_priority;
            other._default_timeout = _default_timeout;
            other._additional_info = _additional_info;
        }

        bool connect()
        {
            try
            {
                _info = getConnection();
                return static_cast<bool>(_info);
            }
            catch (...)
            {
                return false;
            }
        }

        rpc_component<fep::rpc::IRPCParticipantInfo> getConnection() const
        {
            rpc_component<fep::rpc::IRPCParticipantInfo> info;
            getRPCComponentProxyByIID(rpc::IRPCParticipantInfo::getRPCIID(), info, false);
            return info;
        }
        
        std::string getParticipantName()
        {
            return _participant_name;
        }

        void setStartPriority(int32_t prio)
        {
            _start_priority = prio;
        }

        int32_t getStartPriority() const
        {
            return _start_priority;
        }

        void setInitPriority(int32_t prio)
        {
            _init_priority = prio;
        }

        int32_t getInitPriority() const
        {
            return _init_priority;
        }

        bool getRPCComponentProxy(const std::string& component_name,
                                  const std::string& component_iid,
                                  IRPCComponentPtr& proxy_ptr) const
        {
            //we only look for interfaces ... not for specific component names!!
            //this is only for compatibility testing
            bool force_old_ai = (component_name == "force_old_ai");
            
            return getRPCComponentProxyByIID(component_iid, proxy_ptr, force_old_ai);
        }

        bool getRPCComponentProxyByIID(const std::string& component_iid,
                                       IRPCComponentPtr& proxy_ptr,
                                       bool force_old_ai) const
        {
            //if (version == below_2.4) we can add 2 different factories and check here which version is the fep participant
            //in 2.3 we have a rpc_info (see element_object) and the AI Interface
            //in 2.4 we have can use a new participant info 
            //this must be reworked to be more generic and a real factory !
            double version;            
            auto res = _coin.getAI().GetParticipantFEPVersion(version, _participant_name);
            if (fep::ERR_TIMEOUT == res)
            {
                _logger.log(logging::CATEGORY_PARTICIPANT, logging::SEVERITY_FATAL, _participant_name,
                    _system_name, "Participant was not reachable: " + _participant_name);
                throw std::runtime_error{ "Participant was not reachable: " + _participant_name };
            }
            else if (isFailed(res))
            {
                _logger.log(logging::CATEGORY_PARTICIPANT, logging::SEVERITY_FATAL, _participant_name,
                    _system_name, "Can't determine the version of the participant: " + _participant_name );
                throw std::runtime_error{ "Can't determine the version of the participant: " + _participant_name };
            }
            
            /// the participant info is always wrapped within FEP 2 (i think)
            if (component_iid == getRPCIID<fep::rpc::IRPCParticipantInfo>())
            {
                if (std::isless(version, 3.0))
                {
                    std::shared_ptr<IRPCObjectClient> part_object;
                    part_object.reset(new ParticipantInfoProxyOldSql(_participant_name, _system_name,
                        fep::rpc::IRPCParticipantInfo::getRPCIID()));
                    return proxy_ptr.reset(part_object);
                }
                else
                {
                    std::shared_ptr<IRPCObjectClient> part_object;
                    part_object.reset(new ParticipantInfoProxy<fep::rpc::IRPCParticipantInfo>(_participant_name.c_str(),
                        fep::rpc::IRPCParticipantInfo::getRPCIID(),
                        _coin.getAI().getInternalRPC()));
                    return proxy_ptr.reset(part_object);
                }
            }
            else
            {
                std::string found_component_name = getComponentNameWhichSupports(component_iid);
                if (component_iid == getRPCIID<fep::rpc::IRPCStateMachine>())
                {
                    if (force_old_ai || found_component_name.empty())
                    {
                        if (std::isless(version, 3.0))
                        {
                            // since 2.0 we can use the AI Interface
                            std::shared_ptr<IRPCObjectClient> part_object;
                            part_object.reset(new StateMachineProxyOldSql(_participant_name.c_str(),
                                _logger,
                                fep::rpc::IRPCStateMachine::getRPCDefaultName()));
                            return proxy_ptr.reset(part_object);
                        }
                    }
                    else
                    {
                        //we use the new interface
                        std::shared_ptr<IRPCObjectClient> part_object;
                        part_object.reset(new StateMachineProxy<fep::rpc::IRPCStateMachine>(_participant_name.c_str(),
                            found_component_name,
                            _coin.getAI().getInternalRPC()));
                        return proxy_ptr.reset(part_object);
                    }
                }
                else if (component_iid == getRPCIID<fep::rpc::IRPCDataRegistry>())
                {
                    if (force_old_ai || found_component_name.empty())
                    {
                        if (std::isless(version, 3.0))
                        {
                            std::shared_ptr<IRPCObjectClient> part_object;
                            part_object.reset(new DataRegistryProxyOldSql(_participant_name.c_str(),
                                _logger,
                                fep::rpc::IRPCDataRegistry::getRPCDefaultName()));
                            return proxy_ptr.reset(part_object);
                        }
                    }
                    else
                    {
                        std::shared_ptr<IRPCObjectClient> part_object;
                        part_object.reset(new DataRegistryProxy<fep::rpc::IRPCDataRegistry>(_participant_name.c_str(),
                            found_component_name,
                            _coin.getAI().getInternalRPC()));
                        return proxy_ptr.reset(part_object);
                    }
                }
                else if (component_iid == getRPCIID<fep::rpc::IRPCConfiguration>())
                {
                    if (force_old_ai || found_component_name.empty())
                    {
                        if (std::isless(version, 3.0))
                        {
                            std::shared_ptr<IRPCObjectClient> part_object;
                            part_object.reset(new ConfigurationProxyOldSql(_participant_name.c_str(),
                                _logger,
                                fep::rpc::IRPCConfiguration::getRPCDefaultName(),
                                _default_timeout));
                            return proxy_ptr.reset(part_object);
                        }
                    }
                    else
                    {
                        //FEPSDK-2058 hacky check for timing config property 
                        //in FEP 2.6 we added it by default
                        AutomationInterface* _ai_if_less = nullptr;
                        if (std::isless(version, 2.6))
                        {
                            _ai_if_less = &_coin.getAI();
                        }
                        std::shared_ptr<ConfigurationProxy> part_object =
                        std::make_shared<ConfigurationProxy>(_participant_name.c_str(),
                            found_component_name,
                            _coin.getAI().getInternalRPC(),
                            _logger,
                            _ai_if_less);
                        return proxy_ptr.reset(part_object);
                    }
                }
            }
            return false;
        }

        std::string getComponentNameWhichSupports(std::string iid) const
        {
            std::vector<std::string> found_objects;
            rpc_component<fep::rpc::IRPCParticipantInfo> use_info = _info;
            if (!static_cast<bool>(use_info))
            {
                use_info = getConnection();
            }
            found_objects = use_info->getRPCComponents();
            for (const auto& current_object : found_objects)
            {
                std::vector<std::string> found_interfaces;
                found_interfaces = _info->getRPCComponentIIDs(current_object);
                for (const auto& current_iid : found_interfaces)
                {
                    if (iid == current_iid)
                    {
                        return current_object;
                    }
                }
            }
            return std::string();
        }

        void setAdditionalInfo(const std::string& key, const std::string& value)
        {
            _additional_info[key] = value;
        }

        std::string getAdditionalInfo(const std::string& key, const std::string& value_default) const
        {
            auto value = _additional_info.find(key);
            if (value != _additional_info.cend())
            {
                return value->second;
            }
            else
            {
                return value_default;
            }
        }

    private:
        ConnectionInterface _coin;
        ISystemLogger& _logger;
        std::string _participant_name;
        std::string _system_name;
        rpc_component<fep::rpc::IRPCParticipantInfo> _info;
        int32_t _init_priority;
        int32_t _start_priority;
        timestamp_t _default_timeout;
        std::map<std::string, std::string> _additional_info;
    };
}
