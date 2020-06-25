/**
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
 
#include <gtest/gtest.h>
#include <fep_system/fep_system.h>
#include <string.h>
#include "fep_test_common.h"
#include "a_util/logging.h"
#include "a_util/process.h"

using Modules = std::map<std::string, std::unique_ptr<cTestBaseModule>>;

 /**
 * @brief It's tested that a standalone participant won't be discovered by fep::discoverSystem
 * @req_id FEPSDK-2128
 */
TEST(SystemDiscovery, StandaloneModulesWillNotBeDiscovered)
{    
    const auto participant_names = std::vector<std::string>
        { MakePlatformDepName("standalone_participant")
        , MakePlatformDepName("participant_no_standalone_path")
        , MakePlatformDepName("participant3") };

    const Modules modules = createTestModules(participant_names);

    ASSERT_EQ(
        modules.at(MakePlatformDepName("standalone_participant"))->GetPropertyTree()->SetPropertyValue(FEP_STM_STANDALONE_PATH, true),
        a_util::result::Result());

    ASSERT_EQ(
        modules.at(MakePlatformDepName("participant_no_standalone_path"))->GetPropertyTree()->DeleteProperty(FEP_STM_STANDALONE_PATH),
        a_util::result::Result());

    /// discover
    const auto domain_id = modules.begin()->second->GetDomainId();
    fep::System my_system = fep::discoverSystemOnDDS("my_system", domain_id, 1000);
    const auto discovered_participants = my_system.getParticipants();

    /// compare with expectation
    const std::vector<std::string> discoverd_names = [discovered_participants]() {
        std::vector<std::string> temp;
        std::transform
        (discovered_participants.begin()
            , discovered_participants.end()
            , std::back_inserter(temp)
            , [](decltype(discovered_participants)::value_type participant)
        {
            return participant.getName();
        }
        );
        return temp;
    }();

    auto expected = std::vector<std::string>{ MakePlatformDepName("participant3")
                                          , MakePlatformDepName("participant_no_standalone_path") };
    auto notinlist_expected = std::vector<std::string>{ MakePlatformDepName("standalone_participant") };
    EXPECT_EQ(expected,
        extractedVectorContains(discoverd_names, expected));
    EXPECT_EQ(std::vector<std::string>(),
        extractedVectorContains(discoverd_names, notinlist_expected));
}

/**
 * @brief The discovery of participants is tested
 * @req_id <todo>
 */
TEST(SystemDiscovery, DiscoverSystem)
{       
    const auto participant_names = std::vector<std::string>{
                                    MakePlatformDepName("participant1"),
                                    MakePlatformDepName("participant2") };
    const Modules modules = createTestModules(participant_names);
    
    /// discover system
    const auto domain_id = modules.begin()->second->GetDomainId();
    fep::System my_system = fep::discoverSystemOnDDS("my_system", domain_id, 1000);
    const auto discovered_participants = my_system.getParticipants();

    /// compare with expectation
    const std::vector<std::string> discovered_names = [discovered_participants](){
        std::vector<std::string> temp;
        std::transform
            (discovered_participants.begin()
            , discovered_participants.end()
            , std::back_inserter(temp)
            , [](decltype(discovered_participants)::value_type participant)
                {
                    return participant.getName();
                }    
            );
        return temp;
    }();
    EXPECT_EQ(participant_names,
        extractedVectorContains(discovered_names, participant_names));
}
