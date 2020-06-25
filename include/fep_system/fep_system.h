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
#include "fep_system_types.h"
#include "participant_proxy.h"
#include "base/states/fep2_state.h"
#include "base/logging/logging_levels.h"

///The fep::System default timeout for every fep::System call that need to connect to a far participant
#define FEP_SYSTEM_DEFAULT_TIMEOUT_MS 500
///The fep::System state transition timeout for every fep::System call that need to change the states of the participants
#define FEP_SYSTEM_TRANSITION_TIME 10000
///The fep::discoverSystem default timeout
#define FEP_SYSTEM_DISCOVER_TIME_MS 5000
///The fep::ParticipantProxy default timeout for every fep::ParticipantProxy call that need to connect the participant
#define PARTICIPANT_DEFAULT_TIMEOUT 5000

namespace fep
{
    class IEventMonitor;

    /**
     * @brief FEP System class is a collection of fep::ParticipantProxy.
     * 
     * You may create a fep::System by using the CTOR fep::System()
     * @code
     * 
     * //this functionality may be used if you know your system
     * //construct a system
     * fep::System my_system("my_system");
     * my_system.add("my_participant_1");
     * //now your system contains one participant     
     * my_system.add("my_participant_2");
     * //now your system contains two participants
     * 
     * @endcode
     * 
     * @see @ref fep::discoverSystem
     */
    class FEP_SYSTEM_EXPORT System
    {
    public:
        ///redefinition for the current FEP States
        ///@see @ref fep_state_machine
        using State = fep::tState;

    public:
        /**
         * @brief Construct a new System object
         *
         * @remark at the moment of FEP 2 there is no system affiliation implemented within the participants.
         *         for fep3 the systemname will be considered. If empty the system name is really not set!
         */
        System();
        /**
         * @brief Construct a new System object
         * 
         * @param  system_name the name of the system to construct
         * @remark at the moment of FEP 2 there is no system affiliation implemented within the participants.
         */
        System(const std::string& system_name);
        /**
         * @brief Copy Construct a new System object
         * 
         * @param other the other system object to copy from
         * @remark we can not copy the registered event monitor!
         *         This is also a very expensive operation, use move instead if possible!
         */
        System(const System& other);
        /**
         * @brief Move construct a new System object
         *
         * @param other the other system object to move from
         */
        System(System&& other);
        /**
         * @brief Copy assignment
         *
         * @param other the other system object to copy from
         * @remark we can not copy the registered event monitor !
         *         This is also a very expensive operation, use move instead if possible!
         */
        System& operator=(const System& other);
        /**
         * @brief Move assignment 
         *
         * @param other the other system object to move from
         */
        System& operator=(System&& other);
        /**
         * @brief Destroy the System object
         * 
         */
        virtual ~System();

        /* controlling commands */
        /**
         * @c start triggers all participants of the system into the state FS_RUNNING
         * 
         * This call will take the the init fep::ParticipantProxy::setStartPriority into account.
         * 
         * @param timeout_ms the timeout in microsec
         * @see fep::ParticipantProxy::setStartPriority
         * 
         * @throw runtime_error throws if the timeout is reached (@ref FEP_SYSTEM_TRANSITION_TIME)
         *                      throws if the system contains standalone participants
         * for any addtional information you need to check via IEventMonitor::onLog
         * 
         */
        void start(timestamp_t timeout_ms = FEP_SYSTEM_TRANSITION_TIME) const;
        /**
         * @c stop triggers all participants of the system into the state FS_IDLE
         * 
         * This call will take the the init fep::ParticipantProxy::setStartPriority into account.
         * 
         * @param timeout_ms the timeout in microsec
         * @see fep::ParticipantProxy::setStartPriority
         * 
         * @throw runtime_error throws if the timeout is reached (@ref FEP_SYSTEM_TRANSITION_TIME)        
         * for any addtional information you need to check via IEventMonitor::onLog
         * 
         */
        void stop(timestamp_t timeout_ms = FEP_SYSTEM_TRANSITION_TIME) const;
        /**
         * @c shutdown triggers all participants of the system into the state FS_SHUTDOWN
         * 
         * 
         * @param timeout_ms the timeout in microsec
         * @see fep::ParticipantProxy::setStartPriority
         * 
         * @throw runtime_error throws if the timeout is reached (@ref FEP_SYSTEM_TRANSITION_TIME)
         * for any addtional information you need to check via IEventMonitor::onLog
         * 
         */
        void shutdown(timestamp_t timeout_ms = FEP_SYSTEM_TRANSITION_TIME) const;

        /**
        * @c getParticipant returns the participant object
        *
        */
        ParticipantProxy getParticipant(const std::string& participant_name) const;
        /**
        * @c getParticipants delivers a list of all current participants that are part of the system
        *
        */ 
        std::vector<ParticipantProxy> getParticipants() const;



        /* configuring */
       /**
        * @c adds the participant to the system
        * @param[in]  participant         participant name
        *
        */
        void add(const std::string& participant);
        /**
        * @c adds the list of participants to the system
        * @param[in]  participants         list of participant names
        *
        */
        void add(const std::vector<std::string>& participants);
        /**
        * @c removes the participant from the system
        * @param[in]  participant         participant name
        *
        */
        void remove(const std::string& participant);

        /**
        * @c removes the list of participants from the system
        * @param[in]  participants         list of participant names
        *
        */
        void remove(const std::vector<std::string>& participants);

        /**
        * @c clearSystem removes all current participants from the system
        *
        */
        void clear();
        /**
        * Renames the Participant. This will cause a IEventMonitor::onNameChanged
        * to be broadcast by the participant.
        *
        * @note As a remote request needs multiple transmissions, the method might take more than
        *       @e twice as long as @c timeout_ms
        * @note A Valid Participant name follows the following RegEx: "^([a-zA-Z0-9_\.\-]+)$"
        *
        * @param[in]  old_participant_name  The participant that should be renamed
        * @param[in]  new_participant_name  The new participant name
        * @param[in]  timeout_ms           (ms) timeout for remote request; has to be positive
        *
        * @throw runtime_error On Failure a Exception will be thrown with a detailed description
        */
        void rename(const std::string& old_participant_name, const std::string& new_participant_name,
                    timestamp_t timeout_ms = PARTICIPANT_DEFAULT_TIMEOUT) const;

        /**
        * Configures the timing. The general function is for custom timing types only.
        *
        * @param[in] master_clock_name      The name of the clock of the timing master.
        * @param[in] slave_clock_name       The name of the sync clock of the timing clients.
        * @param[in] scheduler              The name of the scheduler.
        * @param[in] master_element_id      The element id (participant name) of the timing master.
        * @param[in] master_time_stepsize   The time in ms between discrete time steps. Won't be set if an empty string gets passed.
        * @param[in] master_time_factor     Multiplication factor of the simulation speed. Won't be set if an empty string gets passed.
        * @param[in] slave_sync_cycle_time  The time in ms between synchronizations. Won't be set if an empty string gets passed.
        *
        * @throws std::runtime_error if one of the timing properties can not be set
        *
        * @note master_clock_name, master_time_stepsize, master_time_factor and slave_sync_cycle_time will only
        *       be set if the master_element_id is a non empty string.
        *   
        */
        void configureTiming(const std::string& master_clock_name, const std::string& slave_clock_name, 
            const std::string& scheduler, const std::string& master_element_id, const std::string& master_time_stepsize,
            const std::string& master_time_factor, const std::string& slave_sync_cycle_time) const;

        // Native FEP 2 configurations
        void configureTiming2SystemTime(const std::string& master_element_id, const std::string& master_time_factor) const;
        void configureTiming2NoMaster() const;
        void configureTiming2AFAP(const std::string& master_element_id) const;
        // Native FEP 3 configurations
        void configureTiming3NoMaster() const;
        void configureTiming3ClockSyncOnlyInterpolation(const std::string& master_element_id, const std::string& slave_sync_cycle_time) const;
        void configureTiming3ClockSyncOnlyDiscrete(const std::string& master_element_id, const std::string& slave_sync_cycle_time) const;
        void configureTiming3DiscreteSteps(const std::string& master_element_id, const std::string& master_time_stepsize, const std::string& master_time_factor) const;
        void configureTiming3AFAP(const std::string& master_element_id, const std::string& master_time_stepsize) const;



        /**
        * @c getSystemState determines the aggregated state of all participants in a system.
        * To find the aggregated state of the system, the states of all participants are sorted
        * according to their weight (see below). The lowest state in the list is chosen.
        * The relative weights are:
        *         Shutdown < Error < Startup < Idle < Initializing < Ready < Running.
        *
        * @note Even if the timeout parameter is lower than 500 ms, the call will
        *       still take at least 500 ms if no participant list is specified
        * @note This method is _not_ thread safe. Do not call concurrently inside the same participant!
        *
        * @param[in]  timeout_ms      (ms) time how long this method waits maximally for other
        *                           participants to respond
        *
        * @return State see also @ref fep_state_machine
        * On Failure a IEventMonitor::onLog will be send with a detailed description
        */
        State getSystemState(timestamp_t timeout_ms = FEP_SYSTEM_DEFAULT_TIMEOUT_MS) const;

        /**
         * @brief Get the System Name object
         * 
         * @return std::string the name given at construction time
         */
        std::string getSystemName() const;

        /**
        * Register monitoring listener for state and name changed notifications of the whole system
        *
        * @param [in] event_listener The listener
        * @retval true/false        Everything went fine/Something went wrong.
        * On Failure a Incident will be send with a detailed description
        */
        void registerMonitoring(IEventMonitor& event_listener);

        /**
         * Unregister monitoring listener for state and name changed and logging notifications
         *
         * @param [in] event_listener The listener
         */
        void unregisterMonitoring(IEventMonitor& event_listener); 

        /**
         * @brief Set the System Severity Level for all participants 
         * 
         * @param severity_level minimum severity level 
         */
        void setSeverityLevel(logging::Severity severity_level);

        /// @cond no_doc    
        private:
            struct Implementation;
            std::unique_ptr<Implementation> _impl;
        /// @endcond no_doc    
    };

    /// Virtual class to implement FEP System Event Monitor, used with 
    /// @ref fep::System::registerMonitoring 
    class IEventMonitor
    {
    public:
        /// DTOR
        virtual ~IEventMonitor() = default;

        /** 
         * Callback by state change
         *
         * @param [in] participant   The name of the issuing FEP Participant
         * @param [in] state         The new state of the issuing FEP Participant
         */
        virtual void onStateChanged(const std::string& participant, rpc::IRPCStateMachine::State state) = 0;

        /**
         * Callback by name change
         *
         * @param [in] new_name    The name of the issuing FEP Participant
         * @param [in] old_name  The old name of the issuing FEP Participant
         */
        virtual void onNameChanged(const std::string& new_name, const std::string& old_name) = 0;

        /**
         * @brief Callback on log at system, participant, component or element category.
         * 
         * You will only retrieve log messages above the given fep::System::setSystemSeverityLevel
         * 
         * @param log_time time of the log
         * @param category category of the log
         * @param severity_level severity level of the log
         * @param participant_name participant name (is empty on system category )
         * @param logger_name (usually the system, participant, component or element name)
         * @param message detailed message
         */
        virtual void onLog(timestamp_t log_time,
                            logging::Category category,
                            logging::Severity severity_level,
                            const std::string& participant_name,
                            const std::string& logger_name, //depends on the Category ... 
                            const std::string& message) = 0;
    };
    /**
     * discoverSystem discovers all participants which are added to the system named by @p name.
     * Standalone participants won't be part of the created system.
     * 
     * It will use the default service bus discovery provided by the fep sdk library
     *                    
     * @param[in]  name         name of the system which is discovered
     * @param[in]  timeout_ms   (ms) timeout for remote request; has to be positive
     * @return Discovered system
     * @throw runtime_error throws if one of the discovered participants is not available 
     *                      (for filtering of standalone participants)
     */
    System FEP_SYSTEM_EXPORT discoverSystem(std::string name,
                                            timestamp_t timeout_ms = FEP_SYSTEM_DISCOVER_TIME_MS);
    /**
     * discoverSystemOnDDS dicovers all participants which are added to the system named by @p name.
     * Standalone participants won't be part of the created system.
     * 
     * It will use the DDS discovery and the given domain id. 
     * Only participants using DDS and the domain ID @p dds_domain_id will be discovered.
     *                    
     * @param[in]  name           name of the system to discover
     * @param[in]  dds_domain_id  dds domain 
     * @param[in]  timeout_ms     (ms) timeout for remote request; has to be positive
     * @return Discovered system
     * @throw runtime_error throws if one of the discovered participants is not available 
     *                      (for filtering of standalone participants)
     *
     */
    System FEP_SYSTEM_EXPORT discoverSystemOnDDS(std::string name,
                                                 uint16_t dds_domain_id,
                                                 timestamp_t timeout_ms = FEP_SYSTEM_DISCOVER_TIME_MS);

}
