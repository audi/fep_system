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
#include <fep_system_stubs/participant_info_proxy_stub.h>
#include <rpc_stubs_element_object_client.h>

#include "rpc_components/participant_info/participant_info_rpc_intf.h"
#include "connection_interface.h"

namespace fep
{
    template<typename T>
    class ParticipantInfoProxy : public rpc_object_proxy< rpc_proxy_stub::RPCParticipantInfoProxy, T>
    {
    private:
        typedef rpc_object_proxy< rpc_proxy_stub::RPCParticipantInfoProxy, T> base_type;
        
    public:
        using base_type::GetStub;
        ParticipantInfoProxy(std::string participant_name,
                             std::string rpc_component_name,
                             IRPC& rpc) :
                             base_type(participant_name.c_str(), rpc_component_name.c_str(), rpc)
        {
        }

        std::string getName() const override 
        {
            try
            {
                return GetStub().getName();
            }
            catch (...)
            {
                return std::string();
            }
        }
        std::string getSystemName() const override
        {
            try
            {
                return GetStub().getSystemName();
            }
            catch (...)
            {
                return std::string();
            }
        }

        std::vector<std::string> getRPCComponents() const override
        {
            try
            {
                std::string list = GetStub().getRPCComponents();
                return detail::string_to_stringlist(list);
            }
            catch (...)
            {
                return  std::vector<std::string>();
            }
        }
        std::vector<std::string> getRPCComponentIIDs(const std::string& rpc_component_name) const override
        {
            try
            {
                std::string list = GetStub().getRPCComponentIIDs(rpc_component_name);
                return detail::string_to_stringlist(list);
            }
            catch (...)
            {
                return  std::vector<std::string>();
            }
        }
        std::string getRPCComponentInterfaceDefinition(std::string& rpc_component_name,
                                                       std::string& rpc_component_iid) const override
        {
            try
            {
                return GetStub().getRPCComponenttInterfaceDefinition(rpc_component_name, rpc_component_iid);
            }
            catch (...)
            {
                return std::string();
            }
        }

    };

    class ParticipantInfoProxyOldSql : public IRPCObjectClient, public rpc::IRPCParticipantInfo
    {
    private:
        std::string _participant_name;
        std::string _sys_name;
        std::string _component_name;
        ConnectionInterface _coin;
        fep::rpc_client<fep::rpc::IRPCElementInfo> _obj_stub;

    public:
        ParticipantInfoProxyOldSql(const std::string& participant_name, const std::string& sys_name,
            const std::string& rpc_component_name) : 
            _sys_name(sys_name),
            _participant_name(participant_name),
            _component_name(rpc_component_name)
        {
            create_rpc_object_client(_participant_name, rpc::IRPCElementInfo::getRPCDefaultName(), 
                _coin.getAI().getInternalRPC(), _obj_stub);
        }

        std::string getName() const override
        {
            return _participant_name;
        }
        std::string getSystemName() const override
        {
            return _sys_name;
        }

        std::vector<std::string> getRPCComponents() const override
        {
            std::vector<std::string> vec = _obj_stub->GetObjects();
            return vec;
        }
        std::vector<std::string> getRPCComponentIIDs(const std::string& rpc_component_name) const override
        {
            return _obj_stub->GetRPCIIDsForObject(rpc_component_name);
        }
        std::string getRPCComponentInterfaceDefinition(std::string& rpc_component_name,
            std::string& rpc_component_iid) const override
        {
            return _obj_stub->GetRPCInterfaceDefinition(rpc_component_name, rpc_component_iid);
        }
        std::string getRPCObjectIID() const override
        {
            return IRPCParticipantInfo::getRPCIID();
        }
        std::string getRPCObjectDefaultName() const override
        {
            return IRPCParticipantInfo::getRPCDefaultName();
        }
    };
}
