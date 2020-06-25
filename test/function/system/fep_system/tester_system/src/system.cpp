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
#include "a_util/logging.h"
#include "a_util/process.h"

void addingTestParticipants(fep::System& sys)
{
    sys.add("part1");
    sys.add("part2");
    auto part1 = sys.getParticipant("part1");
    part1.setInitPriority(1);
    part1.setStartPriority(11);
    part1.setAdditionalInfo("part1", "part1_value");
    part1.setAdditionalInfo("part1_2", "part1_value_2");

    auto part2 = sys.getParticipant("part2");
    part2.setInitPriority(2);
    part2.setStartPriority(22);
    part2.setAdditionalInfo("part2", "part2_value");
}

bool hasTestParticipants(fep::System& sys)
{
    try
    {
        auto part1 = sys.getParticipant("part1");
        if (!(part1.getInitPriority() == 1))
        {
            return false;
        }
        if (!(part1.getStartPriority() == 11))
        {
            return false;
        }
        if (!(part1.getAdditionalInfo("part1", "") == "part1_value"))
        {
            return false;
        }
        if (!(part1.getAdditionalInfo("part1_2", "") == "part1_value_2"))
        {
            return false;
        }

        auto part2 = sys.getParticipant("part2");
        if (!(part2.getInitPriority() == 2))
        {
            return false;
        }
        if (!(part2.getStartPriority() == 22))
        {
            return false;
        }
        if (!(part2.getAdditionalInfo("part2", "") == "part2_value"))
        {
            return false;
        }
        return true;
    }
    catch (const std::runtime_error&)
    {
        return false;
    }
}

/**
 * @brief Test teh CTORs of the fep::system
 * @req_id <todo>
 */
TEST(SystemLibrary, SystemCtors)
{
    {
        //default CTOR will have a system_name which is empty
        fep::System test_system_default;
        addingTestParticipants(test_system_default);
        ASSERT_EQ(test_system_default.getSystemName(), "");
        ASSERT_TRUE(hasTestParticipants(test_system_default));
    }

    {
        //spec CTOR will have a system_name which is empty
        fep::System test_system("system_name");
        ASSERT_EQ(test_system.getSystemName(), "system_name");
        ASSERT_FALSE(hasTestParticipants(test_system));
    }

    {
        //move CTOR will have a system_name which is empty
        fep::System test_system("system_name");
        addingTestParticipants(test_system);
        fep::System moved_sys(std::move(test_system));
        ASSERT_EQ(moved_sys.getSystemName(), "system_name");
        ASSERT_TRUE(hasTestParticipants(moved_sys));
    }

    {
        fep::System test_system("system_name");
        addingTestParticipants(test_system);
        //copy CTOR will have a system_name which is empty
        fep::System copied_sys(test_system);
        ASSERT_EQ(copied_sys.getSystemName(), "system_name");
        ASSERT_EQ(test_system.getSystemName(), "system_name");
        ASSERT_TRUE(hasTestParticipants(test_system));
        ASSERT_TRUE(hasTestParticipants(copied_sys));
    }

    {
        fep::System test_system("orig");
        addingTestParticipants(test_system);
        //copy CTOR will have a system_name which is empty
        fep::System copied_assigned_sys = test_system;
        ASSERT_EQ(test_system.getSystemName(), "orig");
        ASSERT_EQ(copied_assigned_sys.getSystemName(), "orig");
        ASSERT_TRUE(hasTestParticipants(test_system));
        ASSERT_TRUE(hasTestParticipants(copied_assigned_sys));
    }

    {
        //moved CTOR will have a system_name which is empty
        fep::System test_system("orig");
        addingTestParticipants(test_system);
        fep::System moved_assigned_sys = std::move(test_system);
        ASSERT_EQ(moved_assigned_sys.getSystemName(), "orig");
        ASSERT_TRUE(hasTestParticipants(moved_assigned_sys));
    }
}

class TestEventMonitor : public IEventMonitor
{
public:
    void onStateChanged(const std::string& participant, fep::rpc::IRPCStateMachine::State state) override
    {
        std::unique_lock<std::mutex> lk(_cv_m);
        _participant_names.push_back(participant);
        _states.push_back(state);
        _done = true;
    }
    void onNameChanged(const std::string& new_name, const std::string& old_name) override
    {
        std::unique_lock<std::mutex> lk(_cv_m);
        _new_name = new_name;
        _participant_names.push_back(old_name);
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
        _participant_names.push_back(participant_name);
        _logger_name = logger_name;
        _message = message;
        _done = true;
    }

    bool waitForDone(timestamp_t timeout_ms = 5000)
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

    void reset()
    {
        std::unique_lock<std::mutex> lk(_cv_m);
        _participant_names.clear();
        _done = false;
    }

    bool contains(const std::string& part_name)
    {
        std::unique_lock<std::mutex> lk(_cv_m);
        return (std::find(_participant_names.begin(),
            _participant_names.end(),
            part_name) != _participant_names.end());
    }

    logging::Category _category;
    logging::Severity _severity_level;
    std::vector<std::string> _participant_names;
    std::string _message;
    std::string _logger_name;
    std::vector<fep::rpc::IRPCStateMachine::State> _states;
    std::string _new_name;
    
private:
    std::mutex _cv_m;
    std::atomic_bool _done{ false };
};

using Modules = std::map<std::string, std::unique_ptr<cTestBaseModule>>;

/**
* @brief It's tested that a system containing standalone participants can not be started
* @req_id FEPSDK-2129
*/
TEST(SystemLibrary, SystemWithStandaloneParticipantCanNotBeStarted)
{
    const auto participant_names = std::vector<std::string>{ "participant1", "participant2" };
    const Modules modules = createTestModules(participant_names);
        
    ASSERT_EQ(
        modules.begin()->second->GetPropertyTree()->SetPropertyValue(FEP_STM_STANDALONE_PATH, true),
        a_util::result::Result());

    fep::System my_system = fep::System("my_system");
    EXPECT_NO_THROW(my_system.add(participant_names));

    /// start fails because standalone module is part of fep system
    EXPECT_THROW(my_system.start(), std::runtime_error);
}

/**
 * @brief It's tested that setPropertyValueToAll will raise an exception if setting a property fails
 * @req_id <todo>
 */
TEST(SystemLibrary, TestExceptionSetPropertyToAll)
{
    //@todo quite a lot of effort to test that functionality -> so refactor it to be a private test instead

    const auto participant_names = std::vector<std::string>{ "participant1" , "participant2"};
    const Modules modules = createTestModules(participant_names);     

    ASSERT_EQ(
        modules.at("participant1")->GetPropertyTree()->DeleteProperty(FEP_TIMING_MASTER_PARTICIPANT),
        a_util::result::Result());

    ASSERT_EQ(
        modules.at("participant2")->GetPropertyTree()->DeleteProperty(FEP_TIMING_MASTER_PARTICIPANT),
        a_util::result::Result());
    
    auto my_system = fep::System("my_system");
    EXPECT_NO_THROW(my_system.add(participant_names));    

    try
    {
        my_system.configureTiming3NoMaster();
        FAIL() << "Call is supposed to fail, but did not";
    }
    catch(std::runtime_error& exception)
    {
        ASSERT_STREQ(exception.what(), "property ComponentConfig/Timing/TimingMaster/strMasterElement could not be set for the following participants: participant1, participant2");
      
    }
    catch (std::exception&)
    {
        FAIL() << "Wrong exception was risen";
    }  
}

/**
 * @brief It's tested that setPropertyValue will raise an exception if setting a property fails
 * @req_id <todo>
 */
TEST(SystemLibrary, TestExceptionSetProperty)
{
    //@todo quite a lot of effort to test that functionality -> so refactor it to be a private test instead

    const auto participant_names = std::vector<std::string>{ "master" , "client" };
    const Modules modules = createTestModules(participant_names);

    ASSERT_EQ(
        modules.at("master")->GetPropertyTree()->DeleteProperty(FEP_TIMING_MASTER_TRIGGER_MODE),
        a_util::result::Result());
      
    auto my_system = fep::System("my_system");
    EXPECT_NO_THROW(my_system.add(participant_names));

    try
    {
        my_system.configureTiming2AFAP("master");
        FAIL() << "Call is supposed to fail, but did not";
    }
    catch (std::runtime_error& exception)
    {
        ASSERT_STREQ(exception.what(), "property ComponentConfig/Timing/TimingMaster/strTriggerMode could not be set for the following participant: master");

    }
    catch (std::exception&)
    {
        FAIL() << "Wrong exception was risen";
    }
}


/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestConfigureSystemOK)
{
    cTestBaseModule mod1;
    ASSERT_EQ(a_util::result::SUCCESS, mod1.Create("Participant1"));
    {
        fep::System my_sys("MeinLieblingssystem");
        my_sys.add(mod1.GetName());
        my_sys.add("Participant2");
        my_sys.remove("Participant2");
        my_sys.getParticipant(mod1.GetName()).setInitPriority(1);
        auto p1 = my_sys.getParticipant(mod1.GetName());
        ASSERT_TRUE(mod1.GetName() == p1.getName());
        my_sys.clear();
        auto ps = my_sys.getParticipants();
        ASSERT_TRUE(ps.empty());
        ASSERT_TRUE(my_sys.getSystemName() == "MeinLieblingssystem");
    }

    mod1.Destroy();
}


/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestConfigureSystemNOK)
{
    {
        fep::System my_sys("MeinLieblingssystem");

        bool caught = false;
        try
        {
            my_sys.getParticipant("does_not_exist");
        }
        catch (std::runtime_error e)
        {
            auto msg = e.what();
            ASSERT_TRUE(a_util::strings::isEqual(msg, "The requested participant does not exist"));
            caught = true;
        }
        ASSERT_TRUE(caught);

        caught = false;
        try
        {
            my_sys.getSystemState();
        }
        catch (std::runtime_error e)
        {
            auto msg = e.what();
            ASSERT_TRUE(a_util::strings::isEqual(msg, "System state couldn't be determined"));
            caught = true;
        }
        ASSERT_TRUE(caught);
    }
}


/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestControlSystemOK)
{
    {
        cTestBaseModule mod1;
        ASSERT_EQ(a_util::result::SUCCESS, mod1.Create("Participant1"));
        cTestBaseModule mod2;
        ASSERT_EQ(a_util::result::SUCCESS, mod2.Create("Participant2"));
        fep::System my_sys("MeinLieblingssystem");
        my_sys.add(mod1.GetName());
        my_sys.add(mod2.GetName());
        // start system
        my_sys.start();
        auto p1 = my_sys.getParticipant(mod1.GetName());

        //check if inti priority are taking in concern
        p1.setInitPriority(23);
        ASSERT_EQ(p1.getInitPriority(), 23);
        auto check_p1 = my_sys.getParticipant(mod1.GetName());
        ASSERT_EQ(check_p1.getInitPriority(), 23);

        //check additional info
        check_p1.setAdditionalInfo("my_value", "this is the information i want");
        ASSERT_EQ(check_p1.getAdditionalInfo("my_value", ""), "this is the information i want");
        ASSERT_EQ(check_p1.getAdditionalInfo("my_value_does_not_exist_using_default", "does not exist"), "does not exist");
        

        auto state1 = p1.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        auto p2 = my_sys.getParticipant(mod2.GetName());
        auto state2 = p2.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        ASSERT_EQ(state1, FS_RUNNING);
        ASSERT_EQ(state2, FS_RUNNING);

        // stop system
        my_sys.stop();
        state1 = p1.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        state2 = p2.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        ASSERT_EQ(state1, FS_IDLE);
        ASSERT_EQ(state2, FS_IDLE);

        // error event should be fixed by stop
        mod1.GetStateMachine()->ErrorEvent();
        my_sys.stop();
        state1 = p1.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        state2 = p2.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        ASSERT_EQ(state1, FS_IDLE);
        ASSERT_EQ(state2, FS_IDLE);

        // shutdown system
        my_sys.shutdown();
        state1 = p1.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        state2 = p2.getRPCComponentProxy<fep::rpc::IRPCStateMachine>()->getState();
        ASSERT_EQ(state1, FS_SHUTDOWN);
        ASSERT_EQ(state2, FS_SHUTDOWN);
    }
}


/**
 * @req_id <todo>
 */
TEST(SystemLibrary, TestControlSystemNOK)
{
    {
        fep::System my_sys("MeinLieblingssystem");
        // test with element that doesn't exist
        TestEventMonitor tem;
        my_sys.registerMonitoring(tem);
        my_sys.add("does_not_exist");
        bool caught = false;
        try
        {
            my_sys.start(500);
        }
        catch (std::runtime_error e)
        {
            std::string msg = e.what();
            ASSERT_STREQ("Participant was not reachable: does_not_exist", msg.c_str());
            caught = true;
        }
        ASSERT_TRUE(caught);
        caught = false;
        ASSERT_TRUE(tem.waitForDone());        

        // test with empty system
        my_sys.clear();
        try
        {
            my_sys.start(500);
        }
        catch (std::runtime_error e)
        {
            caught = true;
        }
        // no exception thrown 
        ASSERT_FALSE(caught);
        // but logging message
        ASSERT_TRUE(tem.waitForDone());
        ASSERT_TRUE(tem._message.find("No participants within") != std::string::npos);
        ASSERT_TRUE(tem._severity_level == logging::SEVERITY_WARNING);
        my_sys.unregisterMonitoring(tem);
    }
}


/**
 * @req_id <todo>
 */

TEST(SystemLibrary, TestMonitorSystemOK)
{
    // testing state monitor and log function
    {
        TestEventMonitor tem;
        fep::System my_sys("MeinLieblingssystem");
        my_sys.registerMonitoring(tem);
        // sleep is needed to secure our callbacks are ready
        a_util::system::sleepMilliseconds(2000);

        cTestBaseModule mod1;
        ASSERT_EQ(a_util::result::SUCCESS, mod1.Create("Participant1"));

        ASSERT_TRUE(tem.waitForDone());
        ASSERT_TRUE(tem.contains(mod1.GetName()));
        my_sys.add(mod1.GetName());
        my_sys.start();
        ASSERT_TRUE(tem.waitForDone());
        std::vector<fep::rpc::IRPCStateMachine::State> reference = {FS_STARTUP, FS_IDLE, FS_INITIALIZING, FS_READY, FS_RUNNING};
        ASSERT_TRUE(tem._states == reference);

        ASSERT_TRUE(tem._message.find("started") != std::string::npos);
        // unregister monitoring is important! 
        my_sys.unregisterMonitoring(tem);
        mod1.Destroy();
    }

    // testing rename monitor and log function
    {
        TestEventMonitor tem;
        fep::System my_sys("MeinLieblingssystem");
        my_sys.registerMonitoring(tem);
        // sleep is needed to secure our callbacks are ready
        a_util::system::sleepMilliseconds(2000);

        cTestBaseModule mod1;
        ASSERT_EQ(a_util::result::SUCCESS, mod1.Create("Participant1"));
        my_sys.add(mod1.GetName());

        my_sys.rename(mod1.GetName(), "New_Name");

        ASSERT_TRUE(tem.waitForDone());
        ASSERT_EQ(tem._new_name, "New_Name");
        ASSERT_TRUE(tem._message.find("rename") != std::string::npos);
        mod1.Destroy();
    }
}


void print_vector(const std::vector<std::string>& strings, int number)
{
    std::cout << number << ">";
    for (const auto& current : strings)
    {
        std::cout << current + " ";
    }
    std::cout << std::endl;
}

void print_message(const std::string& message, int number, bool excp=false)
{
    std::cout << number << ">";
    std::cout << message;
    std::cout << std::endl;
}


void testParallelInvocation(fep::System* system, int number, int option)
{
    //option 1 we use the system given in parameter
    //option 2 we do a deep copy and work with different system instances
    fep::System copied_system;
    if (option == 2)
    {
        a_util::system::sleepMilliseconds(10);
        copied_system = *system;
        a_util::system::sleepMilliseconds(10);
        system = &copied_system;
    }

    TestEventMonitor tem;
    system->registerMonitoring(tem);

    try
    {

        auto sys_state = system->getSystemState();
        print_message("System state : " + std::to_string(sys_state), number);

        ///
        std::vector<fep::ParticipantProxy> participants = system->getParticipants();

        for (auto& participant : participants)
        {
            a_util::system::sleepMilliseconds(10);
            print_message("found participant: " + participant.getName(), number);
            auto participant_info = participant.getRPCComponentProxy<fep::rpc::IRPCParticipantInfo>();

            // print the components with rpc interfaces
            print_message("    supported components: ", number);
            std::vector<std::string> component_names = participant_info->getRPCComponents();
            print_vector(component_names, number);

            // print the state of the participant
            auto participant_state_machine = participant.getRPCComponentProxy<fep::rpc::IRPCStateMachine>();

            fep::rpc::IRPCStateMachine::State state = participant_state_machine->getState();
            print_message("    current state: " + std::to_string(state), number);

            // print the signals of the participant
            auto participant_data_registry = participant.getRPCComponentProxy<fep::rpc::IRPCDataRegistry>();
            //print the out signals of the participant
            std::vector<std::string> signal_out_list = participant_data_registry->getSignalsOut();
            print_message("    out signals: ", number);
            print_vector(signal_out_list, number);

            // print the in signals of the participant
            std::vector<std::string> signal_in_list = participant_data_registry->getSignalsIn();
            print_message("    in signals: ", number);
            print_vector(signal_in_list, number);

            // print main properties of the Participant
            auto participant_configuration = participant.getRPCComponentProxy<fep::rpc::IRPCConfiguration>();
            //get properties from root
            auto root_properties = participant_configuration->getProperties("/");
            auto property_names = root_properties->getPropertyNames();
            print_message("    properties: ", number);
            for (const auto& property_name : property_names)
            {
                // print the property name of the participant and there types and values
                print_message("        "
                    + property_name + " - "
                    + root_properties->getProperty(property_name)
                    + "(" + root_properties->getPropertyType(property_name) + ")", number);
            }
        }
    }
    catch (const std::exception& ex)
    {
        print_message(ex.what(), number, true);
    }
    system->unregisterMonitoring(tem);
}


/**
 * @req_id <todo>
 */

TEST(SystemLibrary, ParallelAccess)
{
    TestEventMonitor tem;
    fep::System my_sys("MeinLieblingssystem");
    my_sys.registerMonitoring(tem);
    // sleep is needed to secure our callbacks are ready
    a_util::system::sleepMilliseconds(2000);

    std::map<std::string, cTestBaseModule> participants;

    for (size_t idx = 0; idx < 5; idx++)
    {
        std::string part_name = "Participant" + std::to_string(idx);
        tem.reset();
        ASSERT_EQ(a_util::result::SUCCESS, participants[part_name].Create(part_name.c_str()));
        std::string created_name = participants[part_name].GetName();
        ASSERT_EQ(a_util::result::SUCCESS, participants[part_name].WaitForState(fep::FS_IDLE));
        ASSERT_NO_THROW(
            my_sys.add(created_name);
        );
    }

    my_sys.start();
    ASSERT_TRUE(tem.waitForDone());
    ASSERT_TRUE(tem._message.find("started") != std::string::npos);
    
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < 50; ++i)
        {
            threads.push_back(std::thread(
                testParallelInvocation
                ,&my_sys , i, 1));
        }
        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    std::cout << "Destroy all" << std::endl;
    for (auto& part : participants)
    {
        part.second.Destroy();
    }
    std::cout << "Destroyed" << std::endl;

}


/**
 * @req_id <todo>
 */
TEST(SystemLibrary, ParallelAccessOnDiferentSystemInstances)
{
    TestEventMonitor tem;
    fep::System my_sys("MeinLieblingssystem");
    my_sys.registerMonitoring(tem);
    // sleep is needed to secure our callbacks are ready
    a_util::system::sleepMilliseconds(2000);

    std::map<std::string, cTestBaseModule> participants;

    for (size_t idx = 0; idx < 5; idx++)
    {
        std::string part_name = "Participant" + std::to_string(idx);
        tem.reset();
        ASSERT_EQ(a_util::result::SUCCESS, participants[part_name].Create(part_name.c_str()));
        std::string created_name = participants[part_name].GetName();
        ASSERT_EQ(a_util::result::SUCCESS, participants[part_name].WaitForState(fep::FS_IDLE));
        ASSERT_NO_THROW(
            my_sys.add(created_name);
        );
    }

    my_sys.start();
    ASSERT_TRUE(tem.waitForDone());
    ASSERT_TRUE(tem._message.find("started") != std::string::npos);

    {
        std::vector<std::thread> threads;
        for (int i = 0; i < 5; ++i)
        {
            threads.push_back(std::thread(
                testParallelInvocation
                , &my_sys, i, 2));
        }
        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    my_sys.stop();
    ASSERT_TRUE(tem.waitForDone());
    ASSERT_TRUE(tem._message.find("stopped") != std::string::npos);

    for (auto& part : participants)
    {
        part.second.Destroy();
    }

    // unregister monitoring is important! 
    my_sys.unregisterMonitoring(tem);

}


