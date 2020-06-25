/**
 * Implementation of common auxiliary classes used by most of the FEP functional
 * test cases!
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
 *
 * @remarks
 *
 */

#ifndef _FEP_TEST_COMMON_H_INC_
#define _FEP_TEST_COMMON_H_INC_

#include <cmath>
#include <assert.h>

#include <fep_participant_sdk.h>
#include "a_util/system.h"
#include "a_util/logging.h"
#include "a_util/process.h"
#include "a_util/strings.h"

using namespace fep;

#define FEP_PREP_CMD_VERSION 1.0
#define FEP_PREP_REMOTE_PROP_TIMEOUT static_cast<timestamp_t>(5e6)

#ifdef WIN32
    #pragma warning( push )
    #pragma warning( disable : 4250 )
#endif

// Begin of tests
static const size_t s_szAmountOfControlEvents = 6;

// This array contains all control events, i.e. all events that can be triggered
// remotely by ControlCommand.
static const tControlEvent g_aeControlEvents[s_szAmountOfControlEvents] = {
    CE_Initialize,
    CE_Start,
    CE_Stop,
    CE_Shutdown,
    CE_ErrorFixed,
    CE_Restart,
};

// typedef for list of events
typedef fep::Result(IStateMachine::*tStateEvent)();

// This array contains all states available. Keep this array in sync with the columns of
// aaExpectedStates and any other array you use it for.
static const size_t s_szAmountOfStates = FS_SHUTDOWN + 1;
static const fep::tState g_aFepStates[s_szAmountOfStates] = {
    fep::FS_STARTUP,
    fep::FS_IDLE,
    fep::FS_INITIALIZING,
    fep::FS_READY,
    fep::FS_RUNNING,
    fep::FS_ERROR,
    fep::FS_SHUTDOWN };

// This array contains the state events. Keep this array in sync with the rows of
// s_aExpectedStates and any other array you use it for.
static const size_t s_szAmountOfStateEvents = 9;
static const tStateEvent g_apStateChanges[s_szAmountOfStateEvents] = {
    &IStateMachine::ShutdownEvent,
    &IStateMachine::StartupDoneEvent,
    &IStateMachine::InitializeEvent,
    &IStateMachine::InitDoneEvent,
    &IStateMachine::StartEvent,
    &IStateMachine::StopEvent,
    &IStateMachine::ErrorEvent,
    &IStateMachine::ErrorFixedEvent,
    &IStateMachine::RestartEvent,
};

// This array contains the state table. Add a new column, if a new state should be
// tested. Add a new row, if an additional event should be tested.
// Keep in sync with g_apStateChanges and g_aFepStates!
static tState const s_aExpectedStates[s_szAmountOfStateEvents][s_szAmountOfStates] = {
    /*current-state   FS_STARTUP,  FS_IDLE,         FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN*/
    /*event*/                                                                                  
    /*Shutdown*/     {FS_SHUTDOWN,  FS_SHUTDOWN,     FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_SHUTDOWN, FS_SHUTDOWN},
    /*StartupDone*/  {FS_IDLE,      FS_IDLE,         FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN},
    /*Initialize*/   {FS_STARTUP,   FS_INITIALIZING, FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN},
    /*InitDone*/     {FS_STARTUP,   FS_IDLE,         FS_READY,         FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN},
    /*Start*/        {FS_STARTUP,   FS_IDLE,         FS_INITIALIZING,  FS_RUNNING, FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN},
    /*Stop*/         {FS_STARTUP,   FS_IDLE,         FS_IDLE,          FS_IDLE,    FS_IDLE,     FS_ERROR,    FS_SHUTDOWN},
    /*Error*/        {FS_STARTUP,   FS_ERROR,        FS_ERROR,         FS_ERROR,   FS_ERROR,    FS_ERROR,    FS_SHUTDOWN},
    /*ErrorFixed*/   {FS_STARTUP,   FS_IDLE,         FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_IDLE,     FS_SHUTDOWN},
    /*Restart*/      {FS_STARTUP,   FS_STARTUP,      FS_INITIALIZING,  FS_READY,   FS_RUNNING,  FS_ERROR,    FS_SHUTDOWN}};


/**
 * check if vector1 contains vector2
 * @param [in] vector1  vector1 
 * @param [in] vector2  vector2
 * @return The modified name.
 */
static const std::vector<std::string> extractedVectorContains(
    const std::vector<std::string>& vector1,
    const std::vector<std::string>& vector2)
{
    std::vector<std::string> result_vector;
    for (const auto& current2 : vector2)
    {
        for (const auto& current1 : vector1)
        {
            if (current1 == current2)
            {
                result_vector.push_back(current1);
                break;
            }
        }
    }
    return result_vector;
}

/**
 * Create a platform (tester)-dependant name for stand-alone use.
 * @param [in] strOrigName  The original Module name
 * @return The modified name.
 */
static const std::string MakePlatformDepName(const char* strOrigName)
{
    std::string strModuleNameDep(strOrigName);

    //strModuleNameDep.append(cTime::GetCurrentSystemTime().format("_%H%M%S"));

    #if (_MSC_VER == 1600)
        strModuleNameDep.append("_win64_vc100");
    #elif (_MSC_VER == 1900)
        strModuleNameDep.append("_win64_vc140");
    #elif (_MSC_VER == 1910 || _MSC_VER == 1911)
        strModuleNameDep.append("_win64_vc141");
    #elif (defined (__linux))
        strModuleNameDep.append("_linux");
    #else
        // this goes for vc90, vc120 or apple or arm or whatever.
        #error "Platform currently not supported";
    #endif // Version check

    return strModuleNameDep + "_" + a_util::strings::toString(a_util::process::getCurrentProcessId());
}

/**
 * Wait until a specific state is reached. A timeout value of -1 is interpreted as 
 * an infinite waiting time.
 * @param [in] poSTM The state machine instance
 * @param [in] eState The desired state to wait for
 * @param [in] tmTimeout Timeout after which the function will return
 * @return Standard result
 */
static fep::Result WaitForState(IStateMachine * poSTM,
                                tState eState,
                                timestamp_t tmTimeout_ms = -1, //never ever use it without a timeout ... ! otherwise its almost impossible to find the error
                                bool error_is_failure = false)
{
    return poSTM->WaitForState(eState, tmTimeout_ms, !error_is_failure);
}

class LocalTestTime : public fep::DiscreteClock
{
public:
    LocalTestTime() : fep::DiscreteClock("test_time")
    {
    }
};

/// set participinat header information, used in all main methods here
inline void setModuleHeaderInformation(IModule& mod,
    const char* header_description,
    const char* header_type_id)
{
    setProperty(mod, FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    setProperty(mod, FEP_PARTICIPANT_HEADER_DESCRIPTION, header_description);
    auto fFepVersion =
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    setProperty(mod, FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    setProperty(mod, FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    setProperty(mod, FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    setProperty(mod, FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    setProperty(mod, FEP_PARTICIPANT_HEADER_TYPE_ID, header_type_id);
}



//############################### Test Base Module #######################################

/**
 * @brief The cTestBaseModule class
 * This is a base class form which <b>EVERY</b> FEP Module used within a local
 * functional test should be derived from!
 * - It assures a "properly" filled in Module header (avoids arbtitrary notifications)
 * - It ensures a platform-dependant "randomized" naming scheme to allow jenkins to run
 * tests in parallel without NDDS interfering.
 * - It "publishes" vital access to the Property Tree, the State Machine, the Incident Handler
 * and the Name to be used by the functional test implementation
 * - Provides means of directly starting the Module to reach running state without a hassle.
 *
 * \warning When using this base-implementation in a test which has to either query the
 * property tree or to check the addressee / origin of messages and commands, <b>ALWAYS</b>
 * use GetName() instead of an own name constant!
 */
class cTestBaseModule : public virtual fep::cModule
{

public:
    /// Container type to register incidents.
    typedef struct sLastEvent
    {
        int16_t nIncident;
        fep::tSeverityLevel eSeverity;
        std::string strLastDescription;
        std::string strOrigin;
        std::string strFile;
        int nLine;
        timestamp_t tmSimTime;
    } tIncidentContainer;

public:
    /// container type holding the very last encountered incident.
    tIncidentContainer m_sLastEvent;
    /// event tracking incoming asynchronous events.
    a_util::concurrency::semaphore m_oIncidentEvent;
    /// Guarding asynchronous incident handling
    a_util::concurrency::recursive_mutex m_oIncidentGuard;
    /// Flag whether this module shall put itself in the running state
    bool m_bGoToRunning;
    bool m_bJumpOverInit;
    bool m_bErrorHappenedInInit;
    bool m_with_local_time;
    LocalTestTime _local_test_time;

public:
    cTestBaseModule(bool with_local_time = false)
    {
        m_sLastEvent.eSeverity = fep::SL_Info;
        m_sLastEvent.nIncident = 0;
        m_sLastEvent.strOrigin = std::string("Not initialized");
        m_sLastEvent.strFile = std::string("Not initialized");
        m_sLastEvent.nLine = 99;
        m_sLastEvent.tmSimTime =-1;
        m_bGoToRunning = false; 
        m_bJumpOverInit = false;
        m_bErrorHappenedInInit = false;
        m_with_local_time = with_local_time;
    }

    ~cTestBaseModule()
    {

    }
      
    virtual fep::Result Create
        (const cModuleOptions& oModuleOptions
        , ITransmissionDriver* pDriver=NULL
        , const std::function<std::string(std::string)>& participant_name_transform = [](const std::string& name)
          {
                return MakePlatformDepName(name.c_str());
          })
    {        
        cModuleOptions oMyModuleOptions(oModuleOptions);
        std::string strModuleName = std::string(oMyModuleOptions.GetParticipantName());
        if (strModuleName.empty())
        {
            return ERR_EMPTY;
        }

        oMyModuleOptions.SetParticipantName(participant_name_transform(strModuleName).c_str());
                
        setModuleHeaderInformation(*this, "bla bla", "bla bla ID");

        // overload to prevent stray notifications by filling in the participant header!
        RETURN_IF_FAILED(cModule::Create(oMyModuleOptions, pDriver));
        
        if (m_with_local_time)
        {
            auto clockserv = getComponent<fep::IClockService>(*this);
            _local_test_time.setNewTime(0, false);
            clockserv->registerClock(_local_test_time);
            clockserv->setMainClock(_local_test_time.getName());
        }

        FillModuleHeader();

        return ERR_NOERROR;
    }

    void SetTime(timestamp_t test_time)
    {
        _local_test_time.setNewTime(test_time, true);
    }

    void WaitUntilTimeIsGreaterZero() const
    {
        auto clock = getComponent<fep::IClockService>(*this);
        timestamp_t current_time = 0;
        while (current_time == 0)
        {
            //this will wait forever if fails
            std::this_thread::yield();
            current_time = clock->getTime();
        }
    }

    fep::Result StartUpModule(bool bWaitForRunningState = false)
    {
        if (GetStateMachine()->GetState() == FS_RUNNING)
        {
            return ERR_NOERROR;
        };

        m_bGoToRunning = true;
        GetStateMachine()->StartupDoneEvent();
        if (bWaitForRunningState)
        {
            RETURN_IF_FAILED(WaitForState(FS_RUNNING, -1));
        }
        m_bGoToRunning = false;
        return ERR_NOERROR;
    }

public:
    fep::Result FillModuleHeader()
    {
        // make sure to set something OTHER than default values! Otherweise
        // a warning will be issued (why would it be worth a warning to
        // target a default FEP version?!!)
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, 5.0);
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementName, GetName());
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription,"Blablubb");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion, FEP_SDK_PARTICIPANT_VERSION_MAJOR + 0.1 * FEP_SDK_PARTICIPANT_VERSION_MINOR);
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementPlatform, "IDontCare");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext,"Whatever");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion, 44.0);
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor,"AEV");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDisplayName, "WhoCares");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementCompilationDate,"today");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID,"TheRightType");

        return ERR_NOERROR;
    }

protected:
    virtual fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cModule::ProcessIdleEntry(eOldState));
        if (m_bGoToRunning)
        {
            if (FS_STARTUP != eOldState)
            {
                GetStateMachine()->ShutdownEvent();
            }
            else
            {
                GetStateMachine()->InitializeEvent();
            }
        }
        return ERR_NOERROR;
    }

    virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cModule::ProcessInitializingEntry(eOldState));
        GetStateMachine()->InitDoneEvent();
        
        return ERR_NOERROR;
    }

    virtual fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cModule::ProcessStartupEntry(eOldState));
        GetStateMachine()->StartupDoneEvent();
        return ERR_NOERROR;
    }

    virtual fep::Result ProcessReadyEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cModule::ProcessReadyEntry(eOldState));
        if (m_bGoToRunning)
        {
            GetStateMachine()->StartEvent();
        }
        return ERR_NOERROR;
    }

    virtual fep::Result ProcessRunningEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cModule::ProcessRunningEntry(eOldState));
        if (m_bGoToRunning)
        {
            m_bGoToRunning = false;
        }
        return ERR_NOERROR;
    }

public:
    fep::Result InvokeError(int16_t nCode, const char* strDescription,const char *strOrigin = NULL,
                        int nLine = 0, const char* strFile = NULL)
    {
        return cModule::GetIncidentHandler()->InvokeIncident(
                    nCode, fep::SL_Critical_Local, strDescription,strOrigin,nLine,strFile);
    }

    fep::Result InvokeGlobalError(int16_t nCode, const char* strDescription,const char *strOrigin = NULL,
                              int nLine = 0, const char* strFile = NULL)
    {
        return cModule::GetIncidentHandler()->InvokeIncident(
                    nCode, fep::SL_Critical_Global, strDescription,strOrigin,nLine,strFile);
    }

    fep::Result InvokeWarning(int16_t nCode, const char* strDescription,const char *strOrigin = NULL,
                          int nLine = 0, const char* strFile = NULL)
    {
        return cModule::GetIncidentHandler()->InvokeIncident(
                    nCode, fep::SL_Warning, strDescription,strOrigin,nLine,strFile);
    }

    fep::Result InvokeInfo(int16_t nCode, const char* strDescription,const char *strOrigin = NULL,
                       int nLine = 0, const char* strFile = NULL)
    {
        return cModule::GetIncidentHandler()->InvokeIncident(
                    nCode, fep::SL_Info, strDescription,strOrigin,nLine,strFile);
    }

    fep::Result HandleLocalIncident(const int16_t nIncident, const fep::tSeverityLevel eSeverity,
                                const char *strOrigin, int nLine, const char *strFile,
                                const timestamp_t tmSimTime,
                                const  char* strDescription = NULL)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oIncidentGuard);

        m_sLastEvent.nIncident = nIncident;
        m_sLastEvent.eSeverity = eSeverity;
        m_sLastEvent.strLastDescription = std::string(GetName()) + ": " + std::string(strDescription ? strDescription : "");
        m_sLastEvent.strOrigin = std::string(strOrigin ? strOrigin : "");
        m_sLastEvent.strFile = std::string(strFile ? strOrigin : "");
        m_sLastEvent.nLine = nLine;

        m_oIncidentEvent.notify();
        return cModule::HandleLocalIncident(nIncident, eSeverity, strOrigin, nLine, strFile, tmSimTime, strDescription);
    }

    fep::Result WaitForLocalEvent(tIncidentContainer& sIncidentEvent, timestamp_t nTimeout)
    {
        if (!m_oIncidentEvent.wait_for(a_util::chrono::milliseconds(nTimeout)))
        {
            return ERR_TIMEOUT;
        }

        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oIncidentGuard);
        sIncidentEvent = m_sLastEvent;

        return ERR_NOERROR;
    }
};

//############################### Test Master ############################################

/**
 * @brief The cTestMaster class
 * Standard Implementation of a Test-Master Module which is capable of monitoring
 * slave modules by their status messages and otherwise general notifications.
 * - Provides for convenient start, stop, init, etc of the entire FEP system
 * - Monitor slaves synchronously for state and notification messages
 * - Allows to "scroll" through a bunch of previously received notifications from the
 * FEP network
 *
 * \note Extends the cTestBaseModule; Warnings apply.
 */
class cTestMaster : public cTestBaseModule,
                       public virtual cModule,
                       public fep::cNotificationListener
{
public:
    typedef struct sNotification
    {
        std::string strDescription;
        std::string strSender;
        int16_t nIncident;
        fep::tSeverityLevel nSeverity;
    } tNotificationContainer;

public:
    cTestMaster() : m_nExpState(fep::FS_STARTUP),
                       m_nLastState(fep::FS_STARTUP),
                       m_mapStates(), m_bBeSilent(false)
    {
        // nothing to do here..
    }

    ~cTestMaster()
    {
        if (GetStateMachine() != NULL)
        {
            // explicitly call Destroy here to ensure calls to our virtual callbacks
            Destroy();
        }
    }

public:
    fep::Result TransmitInit(const std::string &strAddressee)
    {
        cModule::GetCommandAccess();
        return TransmitControlCmd(strAddressee.c_str(), CE_Initialize);
    }

    fep::Result TransmitStart(const std::string &strAddressee)
    {
        return TransmitControlCmd(strAddressee.c_str(), CE_Start);
    }

    fep::Result TransmitStop(const std::string &strAddressee)
    {
        return TransmitControlCmd(strAddressee.c_str(), CE_Stop);
    }

    fep::Result TransmitShutdown(const std::string &strAddressee)
    {
        return TransmitControlCmd(strAddressee.c_str(), CE_Shutdown);
    }

    fep::Result TransmitErrorFixed(const std::string &strAddressee)
    {
        return TransmitControlCmd(strAddressee.c_str(), CE_ErrorFixed);
    }

    fep::Result TransmitControlCmd(const std::string &strReceiver,
                               const fep::tControlEvent nEvent)
    {
        return GetStateMachine()->TriggerRemoteEvent(nEvent, strReceiver.c_str());
    }

    fep::Result SetSilentFlag(bool bBeSilent)
    {
        m_bBeSilent = bBeSilent;
        return ERR_NOERROR;
    }

public:
    fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cTestBaseModule::ProcessIdleEntry(eOldState));
        if (FS_STARTUP != eOldState)
        {
            RETURN_IF_FAILED(cModule::GetNotificationAccess()->UnregisterNotificationListener(this));
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cModule::GetNotificationAccess()->RegisterNotificationListener(this));
        RETURN_IF_FAILED(cTestBaseModule::ProcessInitializingEntry(eOldState));
        return ERR_NOERROR;
    }

public:
    fep::Result Update(fep::IStateNotification const * pStateNotification)
    {
        if (NULL == pStateNotification)
        {
            return ERR_POINTER;
        }
        else
        {
            if (!m_bBeSilent)
            {
                LOG_INFO(a_util::strings::format("Got state %s: %d!", pStateNotification->GetSender(),
                    pStateNotification->GetState()).c_str());
            }
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oNotificationGuard);
            if (m_strWaitableName == pStateNotification->GetSender() &&
                    pStateNotification->GetState() >= m_nExpState)
            {
                m_oStateWaitable.notify();
            }

            m_nLastState = pStateNotification->GetState();
            m_strLastModuleName = pStateNotification->GetSender();

            m_mapStates[pStateNotification->GetSender()] = pStateNotification->GetState();
        }

        return ERR_NOERROR;
    }

    fep::Result Update(fep::IIncidentNotification const * pLogNotification)
    {
        if (NULL == pLogNotification)
        {
            return ERR_POINTER;
        }

        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oNotificationGuard);

        tNotificationContainer sLog;
        sLog.nIncident = pLogNotification->GetIncidentCode();
        sLog.nSeverity = pLogNotification->GetSeverity();
        sLog.strDescription = std::string(pLogNotification->GetDescription());
        sLog.strSender = std::string(pLogNotification->GetSender());

        if (!m_bBeSilent)
        {
            LOG_INFO(a_util::strings::format("Received [from %s]: %s", pLogNotification->GetSender(),
                pLogNotification->GetDescription()).c_str());
        }

        if (m_dequeNotifications.size() > 15)
        {
            m_dequeNotifications.erase(m_dequeNotifications.begin());
        }

        m_dequeNotifications.push_back(sLog);
        m_oLogWaitable.notify();

        return ERR_NOERROR;
    }

public:
    fep::Result WaitForState(const char* strModule, tState nState,
                         timestamp_t nTimeout)
    {
        if (!LastStateEquals(strModule, nState))
        {
            {
                a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oNotificationGuard);

                m_strWaitableName = strModule;
                m_nExpState = nState;
            }

            if (!m_oStateWaitable.wait_for(a_util::chrono::microseconds(nTimeout)))
            {
                return ERR_TIMEOUT;
            }

            return WaitForState(strModule, nState, nTimeout);
        }

        return ERR_NOERROR;
    }

    using cTestBaseModule::WaitForState;

    bool LastStateEquals(const char* strModule, tState nState)
    {
        return m_mapStates[strModule] == nState;
    }

    fep::Result ResetLastState()
    {
        m_nLastState = fep::FS_STARTUP;
        m_nExpState = fep::FS_STARTUP;
        return ERR_NOERROR;
    }

    bool WaitForIncident(const char* strText, int16_t nIncidentCode,
                           tSeverityLevel nSeverity, timestamp_t nTimeout_ms)
    {
        {
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oNotificationGuard);

            for (unsigned int nStrCnt = 0; nStrCnt < m_dequeNotifications.size(); nStrCnt++)
            {
                tNotificationContainer* pNotif = &(m_dequeNotifications.at(nStrCnt));

                if (pNotif->strDescription.find(strText, 0) != std::string::npos &&
                    pNotif->nIncident == nIncidentCode &&
                    pNotif->nSeverity == nSeverity)
                {
                    return true;
                }
            }
        }

        // if not already found, wait and try again.
        if (!m_oLogWaitable.wait_for(a_util::chrono::milliseconds(nTimeout_ms)))
        {
            return false;
        }

        return WaitForIncident(strText, nIncidentCode, nSeverity, nTimeout_ms);
    }

    void ClearLog()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oNotificationGuard);

        m_dequeNotifications.clear();
    }

public:
    a_util::concurrency::recursive_mutex m_oNotificationGuard;
    std::string m_strWaitableName;
    std::string m_strLastModuleName;
    tState m_nExpState;
    tState m_nLastState;
    a_util::concurrency::semaphore m_oStateWaitable;
    a_util::concurrency::semaphore m_oLogWaitable;
    std::deque<tNotificationContainer> m_dequeNotifications;
    std::map<std::string, tState> m_mapStates;
    bool m_bBeSilent;
};

//##############
/**
 * @brief The cPropertyTester class
 * Helper class to "batch-test" for properties. Once initialized with the base path
 * (root path of current use-case), this allows to query for individual sub-properties
 * with a one-line test whilst allowing for non-exisiting properties (NULL checks) or
 * range checks in case of floating point values.
 */
class cPropertyTester
{
public:
    cPropertyTester(fep::IPropertyTree* pTree, const char* strBasePath) :
        m_strBasePath(strBasePath), m_pTree(pTree)
    {
        assert(pTree);
    }

    fep::Result Validate(const char* strEntry, double fRefValue)
    {
        if (!strEntry) { return ERR_POINTER; }

        double fFloatValue;
        const fep::IProperty* pEntry =
                m_pTree->GetLocalProperty((m_strBasePath + strEntry).c_str());
        if (!pEntry)
        {
            return ERR_PATH_NOT_FOUND;
        }

        RETURN_IF_FAILED(pEntry->GetValue(fFloatValue));

        double fDiff = fFloatValue - fRefValue;
        if (fabs(fDiff) > 0.00001)
        {
            return ERR_INVALID_ARG;
        }

        return ERR_NOERROR;
    }

    fep::Result Validate(const char* strEntry, int32_t nRefValue)
    {
        if (!strEntry) { return ERR_POINTER; }

        int32_t nIntValue;
        const fep::IProperty* pEntry =
                m_pTree->GetLocalProperty((m_strBasePath + strEntry).c_str());
        if (!pEntry)
        {
            return ERR_PATH_NOT_FOUND;
        }

        RETURN_IF_FAILED(pEntry->GetValue(nIntValue));

        if (nRefValue != nIntValue)
        {
            return ERR_INVALID_ARG;
        }

        return ERR_NOERROR;
    }

    fep::Result Validate(const char* strEntry, bool bRefValue)
    {
        if (!strEntry) { return ERR_POINTER; }

        bool bBoolVal;
        const fep::IProperty* pEntry =
                m_pTree->GetLocalProperty((m_strBasePath + strEntry).c_str());
        if (!pEntry)
        {
            return ERR_PATH_NOT_FOUND;
        }

        RETURN_IF_FAILED(pEntry->GetValue(bBoolVal));

        if (bBoolVal != bRefValue)
        {
            return ERR_INVALID_ARG;
        }

        return ERR_NOERROR;
    }

    fep::Result Validate(const char* strEntry, const char* strRefString)
    {
        if (!strEntry || !strRefString) { return ERR_POINTER; }

        std::string strFullPath = 
                 a_util::strings::format("%s%s", m_strBasePath.c_str(), strEntry);

        const fep::IProperty* pEntry = 
                    m_pTree->GetLocalProperty(strFullPath.c_str());
        if (!pEntry)
        {
            return ERR_PATH_NOT_FOUND;
        }

        const char * strVal = NULL;
        pEntry->GetValue(strVal);
        std::string strPropVal(strVal);
        if (strPropVal != strRefString)
        {
            return ERR_INVALID_ARG;
        }

        return ERR_NOERROR;
    }

    fep::Result SetBasePath(const char* strBasePath)
    {
        m_strBasePath = std::string(strBasePath);
        return ERR_NOERROR;
    }

private:
    std::string m_strBasePath;
    fep::IPropertyTree* m_pTree;
};

/**
 * @brief The cRemotePropertyTester class
 * Separated class to help batch-processing complex property trees of remote modules.
 * As opposed to the standard cPropertyTester, this class prefixes the base path with
 * the correct platform-dependant module name.
 */
class cRemotePropertyTester : public cPropertyTester
{
public:
    cRemotePropertyTester(fep::IPropertyTree* pTree, const char* strRemoteModule,
                          const char* strBasePath)
        : cPropertyTester(pTree, strBasePath)
    {
        std::string strFullBasePath = std::string(MakePlatformDepName(strRemoteModule));
        strFullBasePath = strFullBasePath + std::string(".") + std::string(strBasePath);

        if ('.' != strFullBasePath[strFullBasePath.size() - 1])
        {
            strFullBasePath.append(".");
        }

        SetBasePath(strFullBasePath.c_str());
    }
};

/**
 * @brief The cTestIncidentHandler class
 *
 * An internal test class implementing the IIncidentHandler interface.
 * This class is useful when testing internal components that actually rely on the incident
 * handler but for which it is quite simply an overkill to instantiate a FEP Module solely
 * for its incident handler.
 *
 */
class cTestIncidentHandler : public fep::IIncidentHandler
{
public:
    /// Default constructor
    cTestIncidentHandler()
    {
        Reset();
    }

    /// Default destructor
    ~cTestIncidentHandler()
    {
    }

    // IIncidentHandler interface
public:
    fep::Result AssociateStrategy( const int16_t nFEPIncident,
                               fep::IIncidentStrategy* pStrategyDelegate,
                               const char* strConfigurationPath = "",
                               const fep::tStrategyAssociation eAssociation = SA_REPLACE)
    {
        return ERR_NOT_SUPPORTED;
    }

    fep::Result AssociateStrategy(const int16_t nFEPIncident,
                              const fep::tIncidentStrategy eStrategyDelegate,
                              const fep::tStrategyAssociation eAssociation)
    {
        return ERR_NOT_SUPPORTED;
    }

    fep::Result DisassociateStrategy(const int16_t nFEPIncident,
                                 fep::IIncidentStrategy* pStrategyDelegate)
    {
        return ERR_NOT_SUPPORTED;
    }

    fep::Result DisassociateStrategy( const int16_t nFEPIncident,
                                  const fep::tIncidentStrategy eStrategyDelegate)
    {
        return ERR_NOT_SUPPORTED;
    }

    fep::Result AssociateCatchAllStrategy(IIncidentStrategy *pStrategyDelegate,
                                      const char *strConfigurationPath,
                                      const tStrategyAssociation eAssociation)
    {
        return ERR_NOT_SUPPORTED;
    }

    fep::Result DisassociateCatchAllStrategy(IIncidentStrategy *pStragetyDelegate)
    {
        return ERR_NOT_SUPPORTED;
    }

    fep::Result InvokeIncident(int16_t nFEPIncident, tSeverityLevel eSeverity,
                           const char *strDescription,
                           const char *strOrigin,
                           int nLine,
                           const char *strFile)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oInvocationGuard);

        // this is the only method we're actually interested in.
        m_sLastIncident.eSeverity = eSeverity;
        m_sLastIncident.nIncident = nFEPIncident;
        a_util::strings::copy(m_sLastIncident.strMessage, ENTRY_MESSAGE_LENGTH, strDescription);
        a_util::strings::copy(m_sLastIncident.strOrigin, ENTRY_ORIGIN_LENGTH, strOrigin);
        m_nIncidentCount++;
        return ERR_NOERROR;
    }

    fep::Result GetLastIncident(const tIncidentEntry **ppIncidentEntry)
    {
        if (!ppIncidentEntry) { return ERR_POINTER; }
        if (0 == m_nIncidentCount)
        {
            return ERR_EMPTY;
        }

        *ppIncidentEntry = &m_sLastIncident;
        return ERR_NOERROR;
    }

    fep::Result RetrieveIncidentHistory(tIncidentListConstIter &io_iterHistBegin,
                                    tIncidentListConstIter &io_iterHistEnd)
    {
        return ERR_NOT_SUPPORTED;
    }

    fep::Result FreeIncidentHistory()
    {
        return ERR_NOT_SUPPORTED;
    }

public:
    /**
     * Returns the total incident count up until calling this.
     * @return Number of encounterd incidents.
     */
    uint32_t GetIncidentCount()
    {
        return m_nIncidentCount;
    }

    /**
     * Resetting all statistics variables.
     * @return ERR_NOERROR
     */
    fep::Result Reset()
    {
        m_nIncidentCount = 0 ;
        a_util::memory::set(&m_sLastIncident, sizeof(m_sLastIncident), 0, sizeof(m_sLastIncident));
        return ERR_NOERROR;
    }

private:
    /// Last recorded incident
    fep::tIncidentEntry m_sLastIncident;
    /// Mutex guarding the incident invocation
    a_util::concurrency::recursive_mutex m_oInvocationGuard;
    /// Total count of encountered incidents
    uint32_t m_nIncidentCount;
};

/**
 * @brief The cLimiterTestModule class
 *
 * Used in tests concerning the Transmission Limiter.
 *
 */
class cLimiterTestModule : public cTestBaseModule, public fep::IUserDataListener
{
public:
    /** CTOR */
    cLimiterTestModule()
        :   cTestBaseModule(),
            m_nLastFrameId(0),
            m_nLastSampleNumber(0),
            m_nSamplesReceived(0),
            m_nLastValueReceived(0),
            m_nSumOfReceivedValues(0),
            m_strIncident(""),
            m_nIncident(0),
            m_nSetDataValueOffset(0),
            m_nSetMaxSentSample(65000),
            m_poUserSample(NULL)
    {
    }

    /**
     * Method to make the cModule::GetUserDataAccess() public...
     * @copydoc fep::cModule::GetUserDataAccess
     */
    fep::IUserDataAccess * const GetLimiter()  
    {
        return this->GetUserDataAccess();
    }

    /**
     * Implementation of \ref fep::IUserDataListener::Update for test module, performing the 
     * following steps:
     * \li saving the received value to m_nLastValueReceived
     * \li sums the received value to m_nSumOfReceivedValues
     * \li increases m_nSamplesReceived
     * (All members named above are public and can be (re-)set every time.)
     * @copydoc fep::IUserDataListener::Update
     * @retval ERR_NOERROR Method will always indicate success
     */
    virtual fep::Result Update(const fep::IUserDataSample * poSample)  
    {
        m_nLastValueReceived = *((uint16_t *) poSample->GetPtr());
        m_nSumOfReceivedValues += m_nLastValueReceived;
        m_nSamplesReceived++;
        const fep::IPreparationDataSample* poPrepSample = dynamic_cast<const fep::IPreparationDataSample*>(poSample);
        m_nLastFrameId = poPrepSample->GetFrameId();
        m_nLastSampleNumber = poPrepSample->GetSampleNumberInFrame();
        return ERR_NOERROR;
    }

    /**
     *  Implementation of \ref fep::IIncidentStrategy::HandleLocalIncident for test module, saving
     *  the received incident.
     */
    virtual fep::Result HandleLocalIncident(const int16_t nIncident,
                                        const fep::tSeverityLevel eSeverity,
                                        const char *strOrigin,
                                        int nLine,
                                        const char *strFile,
                                        const timestamp_t tmSimTime,
                                        const char* strDescription = NULL)
    {
        m_nIncident = nIncident;
        m_strIncident = strDescription;
        return ERR_NOERROR;
    }

    /**
     * Tries to spot the (remaining) buffer size by sending a burst of samples till an error
     * code unequal to ERR_NOERROR is returned (if the return code is ERR_MEMORY we succeeded to 
     * fill the buffer). To ensure the data is not transmitted faster than we fill the buffer the 
     * TX frequency is set to 0.1 Hz before bursting and re-set to the previous value after. 
     * Before returning this function tries to clear the buffer by calling ClearBuffer() method.
     *
     * @param[out] szSentTillError  Amount of samples sent, i.e. value is [0; m_nSetMaxSentSample]
     * @return fep::Result              The last error code returned by TrasnmitData(), i.e. either
     *                              ERR_NOERROR or the first error code unequal to ERR_NOERROR
     */
    fep::Result CheckBufferSize(size_t &szSentTillError)
    {
    fep::Result nResult = ERR_NOERROR;
    size_t szSentTillFull = 0;
    double fFormerFrequency = 0.0;
    double fHaltFrequency = 0.1;

    /* save previous frequency */
    m_poPropFreq->GetValue(fFormerFrequency);

    /* slow down limiter to make it easier to measure everything*/
    m_poPropFreq->SetValue(fHaltFrequency); /* fFrequency := 0.1 Hz */
    a_util::system::sleepMilliseconds(2000);

    /* fill the buffer */
    nResult = FillBuffer(szSentTillFull);

    /* clear the buffer */
    ClearBuffer(fHaltFrequency, szSentTillFull);
    m_poPropFreq->SetValue(fFormerFrequency);

    szSentTillError = szSentTillFull;
    return nResult;
    }

    /**
     * Sends samples until an error code (not equal to ERR_NOERROR) is returned by
     * \ref IUserDataAccess::TrasnmitData(). This might indicate a full buffer (error code 
     * is ERR_MEMORY). The error code is returned by this method.
     * The "content" of the sent sample is an tUint16, starting with m_nSetDataValueOffset and 
     * increasing by one every sample.
     * The maximum amount of samples sent if no error occurs at all is m_nSetMaxSentSample.
     * @param[out] szSentTillError  Amount of samples sent, i.e. value is [0; m_nSetMaxSentSample]
     * @return fep::Result              The last error code returned by TrasnmitData(), i.e. either
     *                              ERR_NOERROR or the first error code unequal to ERR_NOERROR
     */
    fep::Result FillBuffer(size_t &szSentTillError)
    {
    fep::Result nResult = ERR_NOERROR;
    size_t szSentTillFull = 0;
    fep::IUserDataAccess * poLimiter = this->GetLimiter();
    uint16_t * pUi16Data = (uint16_t *) m_poUserSample->GetPtr();
    for (uint16_t idx=m_nSetDataValueOffset; idx < m_nSetMaxSentSample; idx++)
    {
        *pUi16Data = idx; 
        nResult = poLimiter->TransmitData(m_poUserSample, false);
        if (fep::isFailed(nResult))
        {
            break;
        }
        else
        {
            ++szSentTillFull;
        }
    }
    szSentTillError = szSentTillFull;
    return nResult;
    }

    /**
     * Clears the buffer of an TX signal by setting the TX frequency to maximum for half a second.
     * @param[in] fResetFrequency [optional] value the frequency is re-set to (Default: 1.0 Hz)
     * @param[in] nSlotCnt Number of slots that need clearing (allows to estimate how long to wait)
     */
    void ClearBuffer(double fResetFrequency, size_t nSlotCnt)
    {
    // note: SetValue() is blocking until the limiter cycle has actually applied
    // the new frequency!
    m_poPropFreq->SetValue(-1.0);          /* fFrequency := <max> Hz */
    /* Ensure the buffer is empty; this depends on the systems scheduler but should be quite fast*/
    a_util::system::sleepMilliseconds(5000);
    m_poPropFreq->SetValue(fResetFrequency);/* re-set frequency */
    /* slowing down might need a moment for scheduler to adjust thread */
    a_util::system::sleepMilliseconds(2000); /* 2.0s */
    }


public: /* attributes - all attributes are public to allow easy (re-)set if needed */
    /** The last frame ID received by DataListenerCallback Update()*/
    uint64_t m_nLastFrameId;
    /** The last sample number received by DataListenerCallback Update()*/
    uint64_t m_nLastSampleNumber;
    /** amount of samples received by DataListenerCallback Update()*/
    size_t m_nSamplesReceived;
    /** the last value received by DataListenerCallback Update() */
    uint16_t m_nLastValueReceived;
    /** the sum of all values received by DataListenerCallback */
    uint64_t m_nSumOfReceivedValues;
    /** The last received incident information */
    std::string m_strIncident;
    /** The last received incident code */
    int16_t m_nIncident;
    /** 1st value used when sending a bust of samples by FillBuffer(); FillBuffer() will increase 
     * the sent value every sample but will NOT change this member - so every new call of Fill
     * Buffer() will again start with this value */
    uint16_t m_nSetDataValueOffset; 
    /** max amount of samples used when trying to fill the TX buffer of a signal */
    uint16_t m_nSetMaxSentSample;
    /** sample used for sending (whenever a sample is needed, should be configured)*/
    fep::IUserDataSample * m_poUserSample; 
    /** frequency property for test signal */
    fep::IProperty * m_poPropFreq;
};

/**
* @brief The cMappingTimingModule class
*
* Used in tests concerning the cMappingTimingModule.
*
*/
class cMappingTimingModule : public cTestBaseModule
{

public:
    void timingCallback(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
    }

    static void timingCallback_caller(void* _this, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cMappingTimingModule*>(_this)->timingCallback(tmSimulation, pStepDataAccess);
    }

};

/**
* @brief Creates modules from the incoming list of names
*
*/
inline std::map<std::string, std::unique_ptr<cTestBaseModule>> createTestModules(const std::vector<std::string>& participant_names)
{
    std::map<std::string, std::unique_ptr<cTestBaseModule>> modules;
    std::for_each
        (participant_names.begin()
        , participant_names.end()
        , [&modules](const std::string& name)
            {
                auto module = std::unique_ptr<cTestBaseModule>(new cTestBaseModule());
                module->Create(name.c_str(), nullptr, [](const std::string name)
                    {
                        return name;
                    });
                modules.emplace(name, std::move(module));
            }
        );

    return modules;
}



#ifdef WIN32
    #pragma warning( pop )
#endif

#endif // _FEP_TEST_COMMON_H_INC_
