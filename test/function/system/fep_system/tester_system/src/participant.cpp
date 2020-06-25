/**
 * Implementation of the tester for the FEP Data Sample (locking)
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

 /**
 * Test Case:   TestSystemLibrary
 * Test ID:     1.0
 * Test Title:  FEP System Library base test
 * Description: Test if controlling a test system is possible
 * Strategy:    Invoke Controller and issue commands
 * Passed If:   no errors occur
 * Ticket:      -
 * Requirement: -
 */

#include <gtest/gtest.h>
#include <fep_system/fep_system.h>
#include <string.h>
#include "fep_test_common.h"


class TestEventMonitor : public IEventMonitor
{
public:
    void onStateChanged(const std::string& participant, fep::rpc::IRPCStateMachine::State state) override
    {
        std::unique_lock<std::mutex> lk(_cv_m);
        _participant_name = participant;
        _states.push_back(state);
        _done = true;
    }
    void onNameChanged(const std::string& new_name, const std::string& old_name) override
    {
        std::unique_lock<std::mutex> lk(_cv_m);
        _new_name = new_name;
        _participant_name = old_name;
        _done = true;
    }

    void onLog(timestamp_t log_time,
        logging::Category category,
        logging::Severity severity_level,
        const std::string& participant_name,
        const std::string& logger_name, //depends on the Category ... 
        const std::string& message) override
    {
        std::unique_lock<std::mutex> lk(_cv_m);
        _category = category;
        _severity_level = severity_level;
        _participant_name = participant_name;
        _logger_name = logger_name;
        _message = message;
        _done = true;
    }

    bool waitForDone(timestamp_t timeout_ms = 2000)
    {
        if (_done)
        {
            _done = false;
            return true;
        }
        int i = 0;
        auto single_wait = timeout_ms / 10;
        while (!_done && i < 10)
        {
            a_util::system::sleepMilliseconds(single_wait);
            ++i;
        }
        if (_done)
        {
            _done = false;
            return true;
        }
        return false;
    }

    logging::Category _category;
    logging::Severity _severity_level;
    std::string _participant_name;
    std::string _message;
    std::string _logger_name;
    std::vector<fep::rpc::IRPCStateMachine::State> _states;
    std::string _new_name;
private:
    std::mutex _cv_m;
    std::atomic_bool _done{ false };
};




/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestStateProxy)
{
    System systm("Blackbox");
    cTestBaseModule mod;
    ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant1"));
    systm.add(mod.GetName());
    auto p1 = systm.getParticipant(mod.GetName());
    auto sm = p1.getRPCComponentProxy<fep::rpc::IRPCStateMachine>();
   
    ASSERT_STREQ(sm->getRPCDefaultName(), "state_machine_fep2");
    ASSERT_STREQ(sm->getRPCIID(), "state_machine_fep2.iid");
    sm->initialize();
    // beauty sleep to secure state is reached
    a_util::system::sleepMilliseconds(200);
    ASSERT_EQ(sm->getState(), FS_READY);
    sm->start();
    // beauty sleep to secure state is reached
    a_util::system::sleepMilliseconds(200);
    ASSERT_EQ(sm->getState(), FS_RUNNING);
    sm->stop();
    // beauty sleep to secure state is reached
    a_util::system::sleepMilliseconds(500);
    ASSERT_EQ(sm->getState(), FS_IDLE);
    sm->restart();
    // beauty sleep to secure state is reached
    a_util::system::sleepMilliseconds(200);
    ASSERT_EQ(sm->getState(), FS_IDLE);
    sm->shutdown();
    // beauty sleep to secure state is reached
    a_util::system::sleepMilliseconds(200);
    ASSERT_EQ(sm->getState(), FS_SHUTDOWN);
}

/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestParticpantInfo)
{
    System systm("Blackbox");
    cTestBaseModule mod;
    ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant1"));
    systm.add(mod.GetName());
    auto p1 = systm.getParticipant(mod.GetName());
    auto pi = p1.getRPCComponentProxy<fep::rpc::IRPCParticipantInfo>();
 
    ASSERT_STREQ(pi->getRPCDefaultName(), "participant_info");
    ASSERT_STREQ(pi->getRPCIID(), "participant_info.iid");
    ASSERT_STREQ(pi->getSystemName().c_str(), systm.getSystemName().c_str());
    ASSERT_STREQ(pi->getName().c_str(), mod.GetName());
    std::string rpc_info = "rpc_info";
    std::string rpc_infoiid = "rpc_info.iid";
    ASSERT_TRUE(!pi->getRPCComponentInterfaceDefinition(rpc_info, rpc_infoiid).empty());
    std::vector<std::string>reference_vec = {
        "clock",
        "clock_sync_master",
        "configuration",
        "rpc_info",
        "scheduler"
    };
    ASSERT_EQ(pi->getRPCComponents(), reference_vec);
}

/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestDataRegistry)
{
    System systm("Blackbox");
    cTestBaseModule mod;
    ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant1"));
    // register some random signals
    mod.GetSignalRegistry()->RegisterSignalDescription("files/timing_example.description");
    handle_t some_handle;
    mod.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("tFEP_Examples_ObjectState", SD_Output), some_handle);
    // trigger participant into state FS_READY
    mod.GetStateMachine()->InitializeEvent();
    // verify signals are given out by DRProxy
    systm.add(mod.GetName());
    auto p1 = systm.getParticipant(mod.GetName());
    auto dr = p1.getRPCComponentProxy<fep::rpc::IRPCDataRegistry>();
  
    ASSERT_STREQ(dr->getRPCDefaultName(), "data_registry");
    ASSERT_STREQ(dr->getRPCIID(), "data_registry.iid");
    std::vector<std::string> ref_vec{ "tFEP_Examples_ObjectState" };
    ASSERT_TRUE(dr->getSignalsOut() == ref_vec);
    ASSERT_TRUE(dr->getSignalsIn().empty());
    ASSERT_STREQ(dr->getStreamType("tFEP_Examples_ObjectState").getMetaTypeName(), "ddl");
}

/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestProxiesNOK)
{
    System systm("Blackbox");
    TestEventMonitor tem;
    systm.registerMonitoring(tem);
    systm.add("does_not_exist");
    auto p1 = systm.getParticipant("does_not_exist");
    bool caught = false;
    try
    {
        p1.getRPCComponentProxy<fep::rpc::IRPCStateMachine>();
    }
    catch (std::runtime_error e)
    {
        std::string msg = e.what();
        caught = true;
        ASSERT_STREQ("Participant was not reachable: does_not_exist", e.what());
    }



    ASSERT_TRUE(caught);
    tem.waitForDone(3000);
    ASSERT_EQ(tem._category, logging::CATEGORY_PARTICIPANT);
    ASSERT_EQ(tem._severity_level, logging::SEVERITY_FATAL);
    ASSERT_STREQ(tem._participant_name.c_str(), "does_not_exist");
    ASSERT_STREQ(tem._logger_name.c_str(), "Blackbox");
    ASSERT_TRUE(tem._message.find("Participant was not reachable") != std::string::npos);
    {
        cTestBaseModule mod;
        ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant1"));
        systm.clear();
        systm.add(mod.GetName());
        p1 = systm.getParticipant(mod.GetName());
        auto sm = p1.getRPCComponentProxy<fep::rpc::IRPCStateMachine>();

        mod.Destroy();
    }

    systm.unregisterMonitoring(tem);
}
