/**
 * Implementation of an exemplary FEP Element Application
 *

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

#include <stdlib.h>
#include <iostream>

#include <fep_system/fep_system.h>

void print_vector(const std::vector<std::string>& vec)
{
    bool first = true;
    for (auto& value : vec)
    {
        if (first)
        {
            std::cout << value;
            first = false;
        }
        else
        {
            std::cout << ", " << value;
        }
    }
}

class Monitor : public fep::IEventMonitor
{
public:
    void onStateChanged(const std::string& participant, fep::rpc::IRPCStateMachine::State state) override
    {
    }
    void onNameChanged(const std::string& new_name, const std::string& old_name) override
    {
    }

    void onLog(timestamp_t log_time,
        fep::logging::Category category,
        fep::logging::Severity severity_level,
        const std::string& participant_name,
        const std::string& logger_name, //depends on the Category ... 
        const std::string& message) override
    {
        std::cout << std::endl;
        std::cout << "####### Received logging information! #######" << std::endl;
        std::cout << "        Category: " << category << std::endl;
        std::cout << "        Severity: " << severity_level << std::endl;
        std::cout << "        Participant: " << participant_name << std::endl;
        std::cout << "        Logger_name: " << logger_name << std::endl;
        std::cout << "        Message: " << message << std::endl;
    }
};
/*
 * Main function of this example
 *
 * This merely serves as a "script" to drive several use-cases involving
 * incidents.
 */
int main()
{
    //Use Case 1: How to get all available participant of a certain system? 

    //this will discover all participants of a system with following constraints
    fep::System my_system_to_discover = fep::discoverSystem("temp", 1000);

    //print the system name
    std::cout << "System name : " << my_system_to_discover.getSystemName() << std::endl;
    try 
    {
        //print the system state
        std::cout << "System state : " << my_system_to_discover.getSystemState() << std::endl;
    }
    catch (std::runtime_error e)
    {
    	std::cout << "No Participant started. Please start at least one participant to make sure"
            " this example works as expected!" << std::endl;
        std::cout << e.what() << std::endl;
        exit(1);
    }
    // register monitoring
    Monitor moni;  
    my_system_to_discover.registerMonitoring(moni); 
    std::vector<fep::ParticipantProxy> participants = my_system_to_discover.getParticipants();
    for (auto& participant : participants) 
    {
        std::cout << "found participant: " << participant.getName() << std::endl;
        auto participant_info = participant.getRPCComponentProxy<fep::rpc::IRPCParticipantInfo>();
        
        // print the components with rpc interfaces
        std::cout << "    supported components: ";
        std::vector<std::string> component_names = participant_info->getRPCComponents();
        print_vector(component_names);
        std::cout << std::endl; 

        // print the state of the participant
        auto participant_state_machine = participant.getRPCComponentProxy<fep::rpc::IRPCStateMachine>();
        std::cout << "    current state: ";
        fep::rpc::IRPCStateMachine::State state = participant_state_machine->getState();
        std::cout << state;
        std::cout << std::endl;

        // print the signals of the participant
        auto participant_data_registry = participant.getRPCComponentProxy<fep::rpc::IRPCDataRegistry>();
        //print the out signals of the participant
        std::cout << "    out signals: ";
        std::vector<std::string> signal_out_list = participant_data_registry->getSignalsOut();
        print_vector(signal_out_list);
        std::cout << std::endl;

        // print the in signals of the participant
        std::cout << "    in signals: ";
        std::vector<std::string> signal_in_list = participant_data_registry->getSignalsIn();
        print_vector(signal_in_list); 
        std::cout << std::endl;

        // print main properties of the Participant
        auto participant_configuration = participant.getRPCComponentProxy<fep::rpc::IRPCConfiguration>();
        //get properties from root
        auto root_properties = participant_configuration->getProperties("/");
        auto property_names = root_properties->getPropertyNames();
        std::cout << "    properties: " << std::endl;
        for (const auto& property_name : property_names)
        {
            // print the property name of the participant and there types and values
            std::cout << "        " << property_name << " - " 
                      << root_properties->getProperty(property_name) 
                      <<  "(" << root_properties->getPropertyType(property_name) + ")";
            std::cout << std::endl;
        }
    }

    // NEVER forget to unregister monitoring when we are done! 
    my_system_to_discover.unregisterMonitoring(moni);
    return 0;
}
