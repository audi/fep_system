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
#include "connection_interface.h"
#include "fep_system/fep_system.h"
#include "fep_system/system_logger_intf.h"

namespace fep
{
    class SystemLogger : public ISystemLogger
    {
    public:
        SystemLogger() = default;
        void registerMonitor(IEventMonitor* monitor, const std::string& system_name)
        {
            std::lock_guard<std::recursive_mutex> lock(_logging_sync);
            unregisterMonitor(nullptr);
            _emm = std::make_shared<EventMonitorMirror>(monitor, _level);
            if (isFailed(_coin.getAI().RegisterMonitoring("*", _emm.get())))
            {
                _emm->onLog("The EventMonitor is already registered",
                           logging::SEVERITY_ERROR,
                           system_name);
                throw std::runtime_error{ "Duplicated listener registration detected!" };
            }
            if (isFailed(_coin.getAI().AssociateCatchAllStrategy(_emm.get())))
            {
                _emm->onLog("The Incident logging is already registered",
                    logging::SEVERITY_ERROR,
                    system_name);
                throw std::runtime_error{ "Duplicated listener registration detected!" };
            }
        }

        void unregisterMonitor(IEventMonitor*)
        {
            std::lock_guard<std::recursive_mutex> lock(_logging_sync);
            if (_emm)
            {
                _coin.getAI().UnregisterMonitoring(_emm.get());
                _coin.getAI().DisassociateCatchAllStrategy(_emm.get());
            }
            _emm = nullptr;
        }

        void log(logging::Category cat,
            logging::Severity level,
            const std::string& participant_name,
            const std::string& logger_name, //depends on the Category ... 
            const std::string& message) const
        {
            std::lock_guard<std::recursive_mutex> lock(_logging_sync);
            if (_emm)
            {
                _emm->onLog(cat, level, 
                    participant_name, logger_name, message);
            }
        }

        void setSeverityLevel(logging::Severity level)
        {
            std::lock_guard<std::recursive_mutex> lock(_logging_sync);
            _level = level;
            if (_emm)
            {
                _emm->setSeverityLevel(level);
            }
        }

    private:

        class EventMonitorMirror : public IAutomationParticipantMonitor,
                                   public IAutomationIncidentStrategy
        {
        public:
            explicit EventMonitorMirror(IEventMonitor* monitor, logging::Severity level) 
                : _monitor(monitor), _level(level)
            {
            }

            void OnNameChanged(const std::string& sender, const std::string& old_name) override
            {
                std::lock_guard<std::recursive_mutex> lock(_monitor_access_sync);
                _monitor->onNameChanged(sender, old_name);
            }

            void OnStateChanged(const std::string& sender, tState state) override
            {
                std::lock_guard<std::recursive_mutex> lock(_monitor_access_sync);
                _monitor->onStateChanged(sender, state);
            }

            static logging::Severity getSeverityFromLevel(fep::tSeverityLevel level)
            {
                if (level == fep::SL_Warning)
                {
                    return logging::SEVERITY_WARNING;
                }
                else if (level == fep::SL_Info)
                {
                    return logging::SEVERITY_INFO;
                }
                else
                {
                    return logging::SEVERITY_ERROR;
                }
            }

            fep::Result HandleGlobalIncident(const char* strSource,
                const int16_t nIncident,
                const fep::tSeverityLevel eSeverity,
                const timestamp_t tmSimTime,
                const char* strDescription) override
            {
                std::lock_guard<std::recursive_mutex> lock(_monitor_access_sync);
                std::string message;
                if (strDescription && strlen(strDescription) > 0)
                {
                    message = strDescription;
                }
                else
                {
                    message = "Incident number: " + std::to_string(nIncident);
                }

                std::string source_to_set;
                if (strSource && strlen(strSource) > 0)
                {
                    source_to_set = strDescription;
                }
                onLog(logging::Category::CATEGORY_PARTICIPANT,
                    getSeverityFromLevel(eSeverity),
                    strSource,
                    strSource,
                    message);
                return {};
            }

            void onLog(logging::Category category,
                logging::Severity received_severity,
                const std::string& participant_name,
                const std::string& logger_name, //depends on the Category ... 
                const std::string& message) const
            {
                std::lock_guard<std::recursive_mutex> lock(_monitor_access_sync);
                const auto timestamp_julian = a_util::datetime::getCurrentLocalDateTime().toTimestamp();
                _monitor->onLog(timestamp_julian, category, received_severity,
                    participant_name, logger_name, message);

            }
            void onLog(const std::string& message,
                       logging::Severity received_severity,
                       const std::string& system_name) const
            {
                std::lock_guard<std::recursive_mutex> lock(_monitor_access_sync);
                const auto timestamp_julian = a_util::datetime::getCurrentLocalDateTime().toTimestamp();
                _monitor->onLog(timestamp_julian, logging::CATEGORY_SYSTEM, received_severity,
                    "", system_name, message);
            }
            void setSeverityLevel(logging::Severity level)
            {
                std::lock_guard<std::recursive_mutex> lock(_monitor_access_sync);
                _level = level;
            }
            IEventMonitor* _monitor;
            mutable std::recursive_mutex _monitor_access_sync;
            logging::Severity _level = logging::SEVERITY_INFO;
        };
        std::shared_ptr<EventMonitorMirror> _emm;
        ConnectionInterface _coin;
        logging::Severity _level = logging::SEVERITY_INFO;
        mutable std::recursive_mutex _logging_sync;
    };
}
