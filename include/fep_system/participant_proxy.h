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

#include "fep_system/fep_system_types.h"
#include "fep_system/rpc_component_proxy.h"
#include "rpc_components/participant_info/participant_info_rpc_intf.h"
#include "rpc_components/legacy/state_machine/state_machine_rpc_intf.h"
#include "rpc_components/data_registry/data_registry_rpc_intf.h"
#include "rpc_components/configuration/configuration_rpc_intf.h"
#include "system_logger_intf.h"

#include <string>

namespace fep
{


    /**
     * @brief The ParticipantProxy will provide common system access to the participants system interfaces (RPC Interface (@ref fep_rpc)).
     * use fep::System to connect
     * 
     * With the getRPCComponentProxy following interfaces are supported:
     * \li fep::rpc::IRPCParticipantInfo
     * \li fep::rpc::IRPCDataRegistry
     * \li fep::rpc::IRPCStateMachine
     */
    class FEP_SYSTEM_EXPORT ParticipantProxy 
    {
        public:
            /**
             * @brief Construct a new Participant Proxy object
             * 
             */
            ParticipantProxy() = default; 
            /**
             * @brief Construct a new Participant Proxy object
             * 
             * @param name name of the participant
             */
            ParticipantProxy(const std::string& name,
                             const std::string& system_name, 
                             ISystemLogger& logger,
                             timestamp_t default_timeout);
            /**
             * @brief Construct a new Participant Proxy object
             * 
             * @param other the other to move 
             */
            ParticipantProxy(ParticipantProxy&& other);
            /**
             * @brief move the other to the current instance
             * 
             * @param other the other 
             * @return ParticipantProxy& value of this
             */
            ParticipantProxy& operator=(ParticipantProxy&& other);
            /**
             * @brief Construct a new Participant Proxy object
             * 
             * @param other the other to copy
             */
            ParticipantProxy(const ParticipantProxy& other);
            /**
             * @brief copies another participant prox instance to the current
             * 
             * @param other the other to copy
             * @return ParticipantProxy& 
             */
            ParticipantProxy& operator=(const ParticipantProxy& other);

        /**
         * @brief check if underlying participant instance is invalid
         * 
         * @return true is valid
         * @return false is not valid
         */
        operator bool()
        {
            return static_cast<bool>(_impl);
        }

        /**
         * Helper function to copy content.
         *
         * @param other the participant to copy values to
         */
        void copyValuesTo(ParticipantProxy& other) const;

        /**
        * set priority that the participant should use when the system is triggered 
        * into state FS_READY.
        * /note the initializing priority has nothing in common with the start priority
        * and will be evaluated separately
        * The lower the priority the later the participant will be triggered.
        * Negative values are allowed
        *
        * @param [in] priority    priority, the participant should use for initialization
        * @see @ref fep::System::start
        */
        void setInitPriority(int32_t priority);
        /**
         * @brief Get the Init Priority
         * 
         * @return int32_t the value of priority
         */
        int32_t getInitPriority() const;
        /**
         * set priority that the participant should use when the system is triggered
         * into state FS_RUNNING.
         * /note the starting priority has nothing in common with the init priority
         * and will be evaluated separately
         * The lower the priority the later the participant will be triggered.
         * Negative values are allowed
         *
         * @param [in] priority    priority, the participant should use for initialization
         * @see @ref fep::System::start
         */
        void setStartPriority(int32_t priority);
        /**
         * @brief Get the Start Priority
         * 
         * @return int32_t the value of priority
         */
        int32_t getStartPriority() const;

        /**
         * @brief Get the Name of the participant
         * 
         * @return std::string  the name of the participant
         */
        std::string getName() const;

        /**
         * @brief sets additional information might be needed internally
         *
         * @param key additional information identifier
         * @param value the value
         */
        void setAdditionalInfo(const std::string& key, const std::string& value);

        /**
         * @brief gets additional information corresponding with the participant
         *
         * @param key additional information identifier you want to retrieve
         * @param value_default the value if the key is not found
         * 
         * @return std::string the value found or the value_default
         */
        std::string getAdditionalInfo(const std::string& key, const std::string& value_default) const;

        /**
         * @brief the getRPComponent internal interface will try to connect to the Participant RPC Server (@ref fep_rpc_server)
         * 
         * @param component_name the rpc server name of the rpc component
         * @param component_iid  the rpc server interface identifier of the rpc component to request
         * @param rpc_proxy_ptr will contain the valid interface if available
         * @return true the server is reachable 
         * @return false the server is not reachable, the server object does not exist or the given interface is not supported
         * \throw runtime_error See exception for more information
         */
        bool getRPCComponentProxy(const std::string& component_name,
                                  const std::string& component_iid,
                                  IRPCComponentPtr& rpc_proxy_ptr) const;
        /**
         * @brief the getRPComponent internal interface will connect the participant and retrieve a connection 
         * to the component supports the given \p component_iid.
         *
         * @param component_iid  the rpc component service interface identifier looking for
         * @param rpc_proxy_ptr will contain the valid interface if available
         * @return true the server is reachable
         * @return false the server is not reachable, the server object does not exist or the given interface is not supported
         * \throw runtime_error See exception for more information
         */
        bool getRPCComponentProxyByIID(const std::string& component_iid,
                                       IRPCComponentPtr& proxy_ptr) const;

        /**
         * This will retrieve a rpc server object interface.
         * The fep::ParticipantProxy does support following interfaces at the moment:
         * \li see @ref fep::ParticipantProxy
         * 
         * @tparam T RPC Interfaces which are valid (see fep::rpc)
         * @return rpc_component<T> the fep_rpc_client interface
         */
        template<typename T>
        rpc_component<T> getRPCComponentProxy() const
        {
            rpc_component<T> component_proxy;
            if (getRPCComponentProxy(fep::getRPCDefaultName<T>(), fep::getRPCIID<T>(), component_proxy))
            {
                return component_proxy;
            }
            else
            {
                component_proxy.reset();
                return component_proxy;
            }
        }
        
        /**
         * This will retrieve a rpc server object interface.
         * The fep::ParticipantProxy does support following interfaces at the moment:
         * \li see @ref fep::ParticipantProxy
         *
         * @tparam T RPC Interfaces which are valid (see fep::rpc)
         * @return rpc_component<T> the fep_rpc_client interface
         */
        template<typename T>
        rpc_component<T> getRPCComponentProxyByIID() const
        {
            rpc_component<T> component_proxy;
            if (getRPCComponentProxyByIID(fep::getRPCIID<T>(), component_proxy))
            {
                return component_proxy;
            }
            else
            {
                component_proxy.reset();
                return component_proxy;
            }
        }
    /// @cond no_doc    
    private:
        struct PrivateParticipantProxy;
        std::shared_ptr<PrivateParticipantProxy> _impl;
    /// @endcond no_doc
    };
}
