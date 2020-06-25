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

#include "participant_configuration_helper.hpp"

/**
 * @req_id <todo>
 */
TEST(ParticipantConfigurationOld, TestProxyConfigGetter)
{
    System systm("Blackbox");
    cTestBaseModule mod;
    ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant1_configuration_test"));
    systm.add(mod.GetName());
    auto p1 = systm.getParticipant(mod.GetName());

    rpc_component<fep::rpc::IRPCConfiguration> config;
    p1.getRPCComponentProxy("force_old_ai", fep::rpc::IRPCConfiguration::getRPCIID(), config);
    ASSERT_TRUE(static_cast<bool>(config));

    auto pt = getComponent<IPropertyTree>(mod);
    ASSERT_TRUE(pt != nullptr);

    bool bool_value = true;
    testGetter(*pt, config.getInterface(), bool_value, "test_bool", "deeper");
    bool_value = false;
    testGetter(*pt, config.getInterface(), bool_value, "test_bool", "deeper");

    int32_t int_value = 3456;
    testGetter(*pt, config.getInterface(), int_value, "test_int", "deeper");

    double double_value = 1.0;
    testGetter(*pt, config.getInterface(), double_value, "test_double", "deeper");

    std::string string_val = "this_is_may_string";
    testGetter(*pt, config.getInterface(), string_val, "test_string", "deeper");
}


/**
 * @req_id <todo>
 */
TEST(ParticipantConfigurationOld, TestProxyConfigSetter)
{
    System systm("Blackbox");
    cTestBaseModule mod;
    ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant1_configuration_test"));
    systm.add(mod.GetName());
    auto p1 = systm.getParticipant(mod.GetName());

    rpc_component<fep::rpc::IRPCConfiguration> config;
    p1.getRPCComponentProxy("force_old_ai", fep::rpc::IRPCConfiguration::getRPCIID(), config);
    ASSERT_TRUE(static_cast<bool>(config));

    auto pt = getComponent<IPropertyTree>(mod);
    ASSERT_TRUE(pt != nullptr);

    bool bool_value = true;
    testSetter(*pt, config.getInterface(), bool_value, false, "test_bool");
    bool_value = false;
    testSetter(*pt, config.getInterface(), bool_value, true, "test_bool");

    int32_t int_value = 3456;
    testSetter(*pt, config.getInterface(), int_value, 1, "test_int");

    double double_value = 1.0;
    testSetter(*pt, config.getInterface(), double_value, 0.0, "test_double");

    std::string string_val = "this_is_may_string";
    testSetter(*pt, config.getInterface(), string_val, "init_val", "test_string");
}

/**
 * @req_id <todo>
 */
TEST(ParticipantConfigurationOld, TestProxyConfigArraySetter)
{
    System systm("Blackbox");
    cTestBaseModule mod;
    ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant1_Array_configuration_test"));
    systm.add(mod.GetName());
    auto p1 = systm.getParticipant(mod.GetName());

    rpc_component<fep::rpc::IRPCConfiguration> config;
    p1.getRPCComponentProxy("force_old_ai", fep::rpc::IRPCConfiguration::getRPCIID(), config);
    ASSERT_TRUE(static_cast<bool>(config));

    auto pt = getComponent<IPropertyTree>(mod);
    ASSERT_TRUE(pt != nullptr);

    std::vector<bool> bool_value_array = { true, false, true };
    testArraySetter<std::vector<bool>>(*pt, config.getInterface(), bool_value_array, { false, true, false }, "test_bool_array");

    std::vector<int32_t> int_value_array = { 3456, 2, 3, 4};
    testArraySetter(*pt, config.getInterface(), int_value_array, { 1, 2 }, "test_int_array");

    std::vector<double> double_value_array = { 1.0, 2.0 };
    testArraySetter(*pt, config.getInterface(), double_value_array, { 0.0, 0.0 }, "test_double_array");

    std::vector<std::string> string_val_array = { "this_is_may_string", "test2", "test3" };
    testArraySetter(*pt, config.getInterface(), string_val_array, { "init_val", "another_val" }, "test_string");
}

/**
 * @req_id FEPSDK-2164
 */
TEST(ParticipantConfigurationOld, TestProxyConfigPropNotExistsNoCreate)
{
    System systm("blackbox_no_create");
    cTestBaseModule mod;
    ASSERT_EQ(a_util::result::SUCCESS, mod.Create("Participant_no_prop_exists_old"));
    systm.add(mod.GetName());
    auto p1 = systm.getParticipant(mod.GetName());

    rpc_component<fep::rpc::IRPCConfiguration> config;
    p1.getRPCComponentProxy("force_old_ai", fep::rpc::IRPCConfiguration::getRPCIID(), config);
    ASSERT_TRUE(static_cast<bool>(config));

    auto pt = getComponent<IPropertyTree>(mod);
    ASSERT_TRUE(pt != nullptr);

    auto prop_pointer = pt->GetProperty("test_does_not_exists");
    //check if it does not exists
    ASSERT_EQ(prop_pointer, nullptr);

    auto props = config->getProperties("/");
    auto retval = props->getProperty("test_does_not_exists");
    auto retval_type = props->getPropertyType("test_does_not_exists");
    //check if empty
    ASSERT_EQ(retval, std::string());
    ASSERT_EQ(retval, std::string());
    props->setProperty("test_does_not_exists", "value", "string");
    //check if still empty
    retval = props->getProperty("test_does_not_exists");
    retval_type = props->getPropertyType("test_does_not_exists");

    //the old AI will create that!
    //i can only prevent this with an workaround, but i do not want to implement that in system library!
    ASSERT_EQ(retval, "value");
    ASSERT_EQ(retval_type, "string");

    prop_pointer = pt->GetProperty("test_does_not_exists");
    //check if it is not created in the tree
    ASSERT_NE(prop_pointer, nullptr);
}
