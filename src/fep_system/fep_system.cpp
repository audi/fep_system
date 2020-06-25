/**

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
 */

#include <fep_system/fep_system.h>
#include <a_util/system/system.h>
#include "private_participant_proxy.h"
#include "a_util/process.h"
#include "connection_interface.h"
#include "system_logger.h"
#include <map>
#include <mutex>
#include <thread>
#include <algorithm>
#include <iterator>

using namespace a_util::strings;
namespace
{
    std::string replaceDotsWithSlashes(const std::string& initial_string)
    {
        std::string temp_string = initial_string;
        std::replace(temp_string.begin(), temp_string.end(), '.', '/');
        return temp_string;
    }
}

namespace fep
{
    bool isParticipantStandalone(const ParticipantProxy& participant)
    {      
        auto configuration = participant.getRPCComponentProxy<fep::rpc::IRPCConfiguration>();
        auto properties = configuration->getProperties(replaceDotsWithSlashes(FEP_COMPONENT_CONFIG_STATEMACHINE));
        
        bool standalone = DefaultPropertyTypeConversion<bool>::fromString(properties->getProperty(FEP_STM_STANDALONE_FIELD));
        return standalone;
    }

    std::vector<std::string> getStandaloneParticipants(const std::vector<ParticipantProxy>& participants)
    {
        std::vector<ParticipantProxy> standalone_participants;
        std::copy_if
            (participants.begin()
            , participants.end()
            , std::back_inserter(standalone_participants)
            , [&](ParticipantProxy const& participant)
                {
                    return isParticipantStandalone(participant);
                }
            );

        std::vector<std::string> standalone_participant_names;
        std::transform
            (standalone_participants.begin()
            , standalone_participants.end()
            , std::back_inserter(standalone_participant_names)
            , [&](ParticipantProxy const& entry)
                {
                    return entry.getName();
                }
            );

        return standalone_participant_names;      
    }

   
    static constexpr int min_timeout = 500;
    static constexpr int timeout_divident = 10;

    struct System::Implementation
    {
    public:
        explicit Implementation(std::string system_name) : _system_name(std::move(system_name))
        {
        }

        Implementation(const Implementation& other) = delete;
        Implementation& operator=(const Implementation& other) = delete;

        // why move constructor is not called?
        Implementation& operator=(Implementation&& other)
        {
            _system_name = std::move(other._system_name);
            _participants = std::move(other._participants);
            _logger = std::move(other._logger);
        }

        ~Implementation()
        {
            unregisterMonitoring(nullptr);
            clear();
        }

        std::vector<std::string> mapToStringVec() const
        { 
            std::vector<std::string> participants;
            for (const auto& p : _participants)
            {
                participants.push_back(p.first);
            }
            return participants;
        }

        std::vector<ParticipantProxy> mapToProxyVec() const
        {
            std::vector<ParticipantProxy> participants;
            for (const auto& p : _participants)
            {
                participants.push_back(p.second);
            }
            return participants;
        }

        void trigger_participants(fep::tControlEvent ev, std::vector<std::string>& failed_ones) const
        {
            std::vector<ParticipantProxy> participants_in_order;
            for (const auto& part : _participants)
            {
                participants_in_order.push_back(part.second);
            }
            if (ev == fep::tControlEvent::CE_Initialize)
            {
                std::sort(participants_in_order.begin(), participants_in_order.end(), 
                    [](const ParticipantProxy lhs, const ParticipantProxy rhs)
                    {
                        return (lhs.getInitPriority() > rhs.getInitPriority());
                    }
                );
            } 
            else if (ev == fep::tControlEvent::CE_Start)
            {
                std::sort(participants_in_order.begin(), participants_in_order.end(), 
                    [](const ParticipantProxy lhs, const ParticipantProxy rhs)
                    {
                        return (lhs.getStartPriority() > rhs.getStartPriority());
                    }
                );
            }
           
            for (const auto& part : participants_in_order)
            {
                if (fep::isFailed(_coin.getAI().TriggerEvent(ev, part.getName().c_str())))
                {
                    failed_ones.push_back(part.getName());
                }
            }
        }

        bool states_are_ok(const Result& res, const std::map<std::string, fep::tState>& state_map,
            const tState& expected_state) const
        {
            // timeout error means that participants are not found but we can evaluate the result
            if (fep::isOk(res) || res == fep::ERR_TIMEOUT)
            {
                for (const auto& p : state_map)
                {
                    if (p.second != expected_state)
                    {
                        return false;
                    }
                }
                if (!state_map.empty())
                {
                    return true;
                }
            }
            return false;
        }

        void await_state(tState expected_state, std::map<std::string, fep::tState>& failed_map,
            timestamp_t timeout_ms) const
        {
            const timestamp_t until = a_util::system::getCurrentMilliseconds() +
                (timeout_ms);

            // we prefer several small lookups instead of one long
            uint64_t timeout_limiter = timeout_ms / timeout_divident;
            if (timeout_limiter < min_timeout)
            {
                timeout_limiter = min_timeout;
            }

            bool done = false;
            std::map<std::string, fep::tState> state_map;
            while (!done)
            {
                if (_participants.size() == 0)
                {
                    done = true;
                    break;
                }
                state_map.clear();
                auto res = _coin.getAI().GetParticipantsState(state_map, mapToStringVec(), timeout_limiter);

                if (states_are_ok(res, state_map, expected_state))
                {
                    break;
                }

                timestamp_t remaining = until - a_util::system::getCurrentMilliseconds();
                if (remaining < 0)
                {
                    break;
                }

                a_util::system::sleepMilliseconds(100);
            }

            // validate the finished state list
            for (const auto& p : state_map)
            {
                if (p.second != expected_state)
                {
                    failed_map[p.first] = p.second;
                }
            }
        }

        void throw_error
            (const std::vector<std::string>& failed_participants
            , const std::string& message = "Couldn't trigger all participants, the following participants failed: "
            ) const
        {          
            auto error_msg = message;
            bool first = true;
            for (auto& value : failed_participants)
            {
                if (first)
                {
                    error_msg.append(value);
                    first = false;
                }
                else
                {
                    error_msg.append(", " + value);
                }
            }
            _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_ERROR, "",
                _system_name, error_msg);
            throw std::runtime_error{ error_msg.c_str() };
        }

        void throw_error(const std::map<std::string, fep::tState>& failed_map) const 
        {
            std::string error_msg = "Couldn't reach the expected system state, "
                "the following participants failed: \n";
            for (auto& value : failed_map)
            {
                
                error_msg.append(value.first + " with state = ");
                error_msg.append(std::string(cState::ToString(value.second)) + "\n");
            }            
            _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_ERROR, "",
                _system_name, error_msg);
            throw std::runtime_error{ error_msg.c_str() };
        }

        void check_for_standalone_participants() const
        {            
            const auto standalone_participants_names = getStandaloneParticipants(getParticipants());
             
            if (standalone_participants_names.size() > 0)
            {
                throw_error(standalone_participants_names,
                    "A system can not contain standalone participants. "
                         "The following participants are standalone participants: ");
            }
        }

        void start(timestamp_t timeout_ms /*= FEP_SYSTEM_TRANSITION_TIME*/) const
        {
            if (_participants.empty())
            {
                _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_WARNING, "",
                    _system_name, "No participants within the current system");
                return;
            }       
            
            check_for_standalone_participants();          

            std::vector<std::string> failed_participants;
            trigger_participants(CE_Initialize, failed_participants);
            if (!failed_participants.empty())
            {
                throw_error(failed_participants);
            }
            std::map<std::string, fep::tState> failed_map;
            await_state(FS_READY, failed_map, timeout_ms);
            if (!failed_map.empty())
            {
                throw_error(failed_map);
            }

            trigger_participants(CE_Start, failed_participants);
            if (!failed_participants.empty())
            {
                throw_error(failed_participants);
            }
            await_state(FS_RUNNING, failed_map, timeout_ms);
            if (!failed_map.empty())
            {
                throw_error(failed_map);
            }

            _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_INFO, "",
                _system_name, "system started successfully");
        }

        void stop(timestamp_t timeout_ms /*= FEP_SYSTEM_TRANSTI*/) const
        {
            if (_participants.empty())
            {
                _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_WARNING, "",
                    _system_name, "No participants within the current system");
                return;
            }
            std::vector<std::string> failed_participants;
            trigger_participants(CE_ErrorFixed, failed_participants);
            trigger_participants(CE_Stop, failed_participants);
            if (!failed_participants.empty())
            {
                throw_error(failed_participants);
            }
            std::map<std::string, fep::tState> failed_map;
            await_state(FS_IDLE, failed_map, timeout_ms);
            if (!failed_map.empty())
            {
                throw_error(failed_map);
            }            
            _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_INFO, "",
                _system_name, "system stopped successfully");
        }

        void shutdown(timestamp_t timeout_ms /*= FEP_SYSTEM_TRANSTI*/) const
        {
            if (_participants.empty())
            {
                _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_WARNING, "",
                    _system_name, "No participants within the current system");
                return;
            }
            std::vector<std::string> failed_participants;
            trigger_participants(CE_Shutdown, failed_participants);
            if (!failed_participants.empty())
            {
                throw_error(failed_participants);
            }

            std::map<std::string, fep::tState> failed_map;
            await_state(FS_SHUTDOWN, failed_map, timeout_ms);
            if (!failed_map.empty())
            {
                throw_error(failed_map);
            }            
            _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_INFO, "",
                _system_name, "system finished successfully");
        }

        std::string getName()
        {
            return _system_name;
        }

        System::State getSystemState(timestamp_t timeout_ms /*= FEP_SYSTEM_DEFAULT_TIMEOUT_MS*/)
        {
            System::State state;
            auto vec = mapToStringVec();
            auto res = _coin.getAI().GetSystemState(state, vec, timeout_ms);
            if (isFailed(res))
            {
                _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_FATAL, "", _system_name,
                    "System state couldn't be determined. Is the system empty?");
                throw std::runtime_error{ "System state couldn't be determined" };
            }
            return state;
        }

        void registerMonitoring(IEventMonitor* pMonitor)
        {
            _logger->registerMonitor(pMonitor, _system_name);
        }

        void unregisterMonitoring(IEventMonitor* pMonitor)
        {
            _logger->unregisterMonitor(pMonitor);
        }

        void setSeverityLevel(logging::Severity level)
        {
            _logger->setSeverityLevel(level);
        }

        bool getAvailableParticipants(std::map<std::string, tState>& participant_map, const timestamp_t& timeout_ms)
        {
            _coin.getAI().GetAvailableParticipants(participant_map,
                timeout_ms);
            return true;
        }

        void clear()
        {
            _participants.clear();
        }

        void add(const std::string& participant)
        {
            _participants[participant] = ParticipantProxy(participant,
                _system_name,
                *_logger.get(),
                PARTICIPANT_DEFAULT_TIMEOUT);
        }

        void remove(const std::string& participant)
        {
            _participants.erase(participant);
        }

        void rename(const std::string& old_participant_name,
            const std::string& new_participant_name, timestamp_t timeout_ms /*= PARTICIPANT_DEFAULT_TIMEOUT*/) const
        {
            if (isOk(_coin.getAI().RenameParticipant(new_participant_name, old_participant_name, timeout_ms)))
            {
                _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_INFO, "",
                    _system_name, "Participant successfully renamed");
            }
        }

        ParticipantProxy getParticipant(const std::string& participant_name) const
        {
            auto p = _participants.find(participant_name);
            if (p != _participants.end())
            {
                return p->second;
            }
            _logger->log(logging::CATEGORY_SYSTEM, logging::SEVERITY_FATAL, "", _system_name,
                "No Participant with the name " + participant_name + " found");
            throw std::runtime_error{ "The requested participant does not exist" };
        }

        std::vector<ParticipantProxy> getParticipants() const
        {
            return mapToProxyVec();
        }

        void setPropertyValue(const std::string& participant,
            const std::string& node,
            const std::string& property_name,
            const std::string& value,
            const std::string& type) const
        {
            const auto property_normalized = replaceDotsWithSlashes(property_name);

            auto part = getParticipant(participant);
            if (part)
            {
                auto config_rpc_client = part.getRPCComponentProxy<fep::rpc::IRPCConfiguration>();
                auto props = config_rpc_client->getProperties(node);
                if (!props)
                {
                    throw std::runtime_error(format("access to properties node %s not possible", node));
                }

                if (!props->setProperty(property_normalized, value, type))
                {                           
                    const auto message = format("property %s could not be set for the following participant: %s"
                        , property_normalized.c_str()
                        , part.getName().c_str());

                    throw std::runtime_error(message);
                }                              
            }
            else
            {
                throw std::runtime_error(format("participant %s within system %s not found to configure %s",
                    participant.c_str(),
                    _system_name.c_str(),
                    property_normalized.c_str()));
            }
        }

        void setPropertyValueToAll(const std::string& node,
            const std::string& property_name,
            const std::string& value,
            const std::string& type,
            const std::string& except_participant = std::string()) const
        {
            const auto property_normalized = replaceDotsWithSlashes(property_name);
            auto failing_participants = std::vector<std::string>();

            for (fep::ParticipantProxy participant : getParticipants())
            {
                if (!except_participant.empty())
                {
                    if (participant.getName() == except_participant)
                    {
                        continue;
                    }
                }
                auto config_rpc_client = participant.getRPCComponentProxy<fep::rpc::IRPCConfiguration>();
                auto props = config_rpc_client->getProperties(node);
                if (props)
                {
                    auto success = props->setProperty(property_normalized, value, type);
                    if (!success)
                    {
                        failing_participants.push_back(participant.getName());
                    }
                }
            }                       

            if (failing_participants.size() > 0)
            {
                auto paritipants = join(failing_participants, ", ");
                auto message = format("property %s could not be set for the following participants: %s"
                    , property_normalized.c_str()
                    , paritipants.c_str());

                throw std::runtime_error(message);
            }           
        }

        void configureTiming(const std::string& master_clock_name, const std::string& slave_clock_name,
            const std::string& scheduler, const std::string& master_element_id, const std::string& master_time_stepsize,
            const std::string& master_time_factor, const std::string& slave_sync_cycle_time) const
        {
            setPropertyValueToAll("/", FEP_TIMING_MASTER_PARTICIPANT, master_element_id, fep::PropertyType<std::string>::getTypeName());
            setPropertyValueToAll("/", FEP_SCHEDULERSERVICE_SCHEDULER, scheduler, fep::PropertyType<std::string>::getTypeName());

            if (!master_element_id.empty())
            {
                setPropertyValueToAll("/", FEP_CLOCKSERVICE_MAIN_CLOCK, slave_clock_name, fep::PropertyType<std::string>::getTypeName(), master_element_id);
                setPropertyValue(master_element_id, "/", FEP_CLOCKSERVICE_MAIN_CLOCK, master_clock_name, fep::PropertyType<std::string>::getTypeName());
                if (!master_time_factor.empty())
                {
                    setPropertyValue(master_element_id, "/", FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR, master_time_factor, fep::PropertyType<double>::getTypeName());
                }
                if (!master_time_stepsize.empty())
                {
                    setPropertyValue(master_element_id, "/", FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME, master_time_stepsize, fep::PropertyType<int32_t>::getTypeName());
                }
                if (!slave_sync_cycle_time.empty())
                {
                    setPropertyValueToAll("/", FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, slave_sync_cycle_time, fep::PropertyType<int32_t>::getTypeName(), master_element_id);
                }
            }
            else
            {
                setPropertyValueToAll("/", FEP_CLOCKSERVICE_MAIN_CLOCK, slave_clock_name, fep::PropertyType<std::string>::getTypeName());
            }
        }

        std::map<std::string, ParticipantProxy> _participants;
        ConnectionInterface _coin;
        std::shared_ptr<SystemLogger> _logger = std::make_shared<SystemLogger>();
        std::string _system_name;
    };

    System::System() : _impl(new Implementation(""))
    {
    }

    System::System(const std::string& system_name) : _impl(new Implementation(system_name))
    {
        
    }

    System::System(const System& other) : _impl(new Implementation(other.getSystemName()))
    {
        auto proxies = other.getParticipants();
        for (const auto& proxy : proxies)
        {
            _impl->add(proxy.getName());
            auto new_proxy = _impl->getParticipant(proxy.getName());
            proxy.copyValuesTo(new_proxy);
        }
    }

    System& System::operator=(const System& other)
    {
        _impl->_system_name = getSystemName();
        auto proxies = other.getParticipants();
        for (const auto& proxy : proxies)
        {
            _impl->add(proxy.getName());
            auto new_proxy = _impl->getParticipant(proxy.getName());
            proxy.copyValuesTo(new_proxy);
        }
        return *this;
    }

    System::System(System&& other)
    {
        std::swap(_impl, other._impl);
    }

    System& System::operator=(System&& other)
    {
        std::swap(_impl, other._impl);
        return *this;
    }

    System::~System()
    {
    }

    void System::start(timestamp_t timeout_ms /*= FEP_SYSTEM_TRANSITION_TIME*/) const
    {
        _impl->start(timeout_ms);
    }

    void System::stop(timestamp_t timeout_ms /*= FEP_SYSTEM_TRANSITION_TIME*/) const
    {
        _impl->stop(timeout_ms);
    }
    void System::shutdown(timestamp_t timeout_ms /*= FEP_SYSTEM_TRANSITION_TIME*/) const
    {
        _impl->shutdown(timeout_ms);
    }

    void System::add(const std::string& participant)
    {
        _impl->add(participant);
    }

    void System::add(const std::vector<std::string>& participants)
    {
        for (const auto& participant : participants)
        {
            _impl->add(participant);
        }
    }

    void System::remove(const std::string& participant)
    {
        _impl->remove(participant);
    }

    void System::remove(const std::vector<std::string>& participants)
    {
        for (const auto& participant : participants)
        {
            _impl->remove(participant);
        }
    }

    void System::clear()
    {
        _impl->clear();
    }

    void System::rename(const std::string& old_participant_name, 
        const std::string& new_participant_name, timestamp_t timeout_ms /*= PARTICIPANT_DEFAULT_TIMEOUT*/) const
    {
        _impl->rename(old_participant_name, new_participant_name, timeout_ms);
    }

    System::State System::getSystemState(timestamp_t timeout_ms /*= FEP_SYSTEM_DEFAULT_TIMEOUT_MS*/) const
    {
        return _impl->getSystemState(timeout_ms);
    }

    std::string System::getSystemName() const
    {
        return _impl->getName();
    }

    ParticipantProxy System::getParticipant(const std::string& participant_name) const
    {
        return _impl->getParticipant(participant_name);
    }

    std::vector<ParticipantProxy> System::getParticipants() const
    {
        return _impl->getParticipants();
    }

    void System::registerMonitoring(IEventMonitor& pEventListener)
    {
        _impl->registerMonitoring(&pEventListener);
    }

    void System::unregisterMonitoring(IEventMonitor& pEventListener)
    {
        _impl->unregisterMonitoring(&pEventListener);
    }

    void System::setSeverityLevel(logging::Severity severity_level)
    {
        _impl->setSeverityLevel(severity_level);
    }

    void System::configureTiming(const std::string& master_clock_name, const std::string& slave_clock_name,
        const std::string& scheduler, const std::string& master_element_id, const std::string& master_time_stepsize,
        const std::string& master_time_factor, const std::string& slave_sync_cycle_time) const
    {
        _impl->configureTiming(master_clock_name, slave_clock_name, scheduler, master_element_id, 
            master_time_stepsize, master_time_factor, slave_sync_cycle_time);
    }

    void System::configureTiming2SystemTime(const std::string& master_element_id, const std::string& master_time_factor) const
    {
        _impl->setPropertyValue(master_element_id, "/", FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME", fep::PropertyType<std::string>::getTypeName());
        _impl->setPropertyValue(master_element_id, "/", FEP_TIMING_MASTER_TIME_FACTOR, master_time_factor, fep::PropertyType<double>::getTypeName());
        _impl->configureTiming(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME, master_element_id, "", "", "");
    }

    void System::configureTiming2NoMaster() const
    {
        _impl->configureTiming("", FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME, "", "", "", "");
    }

    void System::configureTiming2AFAP(const std::string& master_element_id) const
    {
        _impl->setPropertyValue(master_element_id, "/", FEP_TIMING_MASTER_TRIGGER_MODE, "AFAP", fep::PropertyType<std::string>::getTypeName());
        _impl->configureTiming(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME, master_element_id, "", "", "");
    }

    void System::configureTiming3NoMaster() const
    {
        _impl->configureTiming("", FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED, "", "", "", "");
    }

    void System::configureTiming3ClockSyncOnlyInterpolation(const std::string& master_element_id, const std::string& slave_sync_cycle_time) const
    {
        _impl->configureTiming(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED, master_element_id, "", "", slave_sync_cycle_time);
    }

    void System::configureTiming3ClockSyncOnlyDiscrete(const std::string& master_element_id, const std::string& slave_sync_cycle_time) const
    {
        _impl->configureTiming(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED, master_element_id, "", "", slave_sync_cycle_time);
    }

    void System::configureTiming3DiscreteSteps(const std::string& master_element_id, const std::string& master_time_stepsize, const std::string& master_time_factor) const
    {
        _impl->configureTiming(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED, master_element_id, master_time_stepsize, master_time_factor, "");
    }

    void System::configureTiming3AFAP(const std::string& master_element_id, const std::string& master_time_stepsize) const
    {
        _impl->configureTiming(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE, 
            FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED, master_element_id, master_time_stepsize, "0.0", "");
    }

/**************************************************************
* discoveries 
***************************************************************/

    System discoverSystemOnDDS(std::string name,
        AutomationInterface& ai,
        timestamp_t timeout_ms /*= FEP_SYSTEM_DISCOVER_TIME_MS*/)
    {
         
        std::vector<std::string> participants;
        ai.GetAvailableParticipants(participants, timeout_ms);

        System discovered_sys(name);
        discovered_sys.add(participants);

        const auto standalone_participant_names = getStandaloneParticipants(discovered_sys.getParticipants());
        if (standalone_participant_names.size() > 0)
        {
            std::for_each
                (standalone_participant_names.begin()
                , standalone_participant_names.end()
                , [&](const std::string& name)
                    {
                        discovered_sys.remove(name);
                    }
                );
        }
   
        return std::move(discovered_sys);
    }

    System discoverSystemOnDDS(std::string name,
        uint16_t dds_domain_id,
        timestamp_t timeout_ms /*= FEP_SYSTEM_DISCOVER_TIME_MS*/)
    {
        ConnectionInterface conn;
        return discoverSystemOnDDS(name, conn.getAI(dds_domain_id), timeout_ms);
    }

    fep::System discoverSystem(std::string name, timestamp_t timeout_ms /*= FEP_SYSTEM_DISCOVER_TIME_MS*/)
    {
        // default discovery is actually the default of participant lib (usually domain 0)
        ConnectionInterface conn;
        return std::move(discoverSystemOnDDS(name, conn.getAI(), timeout_ms));
    }
}