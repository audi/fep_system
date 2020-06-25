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
#include "fep_system/base/properties/property_type.h"
#include "fep_system/base/properties/property_type_conversion.h"
#include <fep_participant_sdk.h>


template<typename T>
void testGetter(fep::IPropertyTree& pt, fep::rpc::IRPCConfiguration& rpc_config, T value, std::string propertyname, std::string deeper_path)
{
    std::string path_with_dots = deeper_path + "." + propertyname;
    ASSERT_TRUE(fep::isOk(pt.SetPropertyValue(path_with_dots.c_str(), value)));

    auto properties_to_test = rpc_config.getProperties("/" + deeper_path);
    auto property_names = properties_to_test->getPropertyNames();
    ASSERT_TRUE(std::find(property_names.begin(), property_names.end(), propertyname) != property_names.end());

    ASSERT_EQ(properties_to_test->getPropertyType(propertyname), fep::PropertyType<T>::getTypeName());

    ASSERT_EQ(fep::DefaultPropertyTypeConversion<T>::fromString(properties_to_test->getProperty(propertyname)),
        value);
}

inline void testGetter(fep::IPropertyTree& pt, fep::rpc::IRPCConfiguration& rpc_config, std::string value, std::string propertyname, std::string deeper_path)
{
    std::string path_with_dots = deeper_path + "." + propertyname;
    ASSERT_TRUE(fep::isOk(pt.SetPropertyValue(path_with_dots.c_str(), value.c_str())));

    auto properties_to_test = rpc_config.getProperties("/" + deeper_path);
    auto property_names = properties_to_test->getPropertyNames();
    ASSERT_TRUE(std::find(property_names.begin(), property_names.end(), propertyname) != property_names.end());

    ASSERT_EQ(properties_to_test->getPropertyType(propertyname), fep::PropertyType<std::string>::getTypeName());

    ASSERT_EQ(fep::DefaultPropertyTypeConversion<std::string>::fromString(properties_to_test->getProperty(propertyname)),
        value);
}

template<typename T>
void testSetter(fep::IPropertyTree& pt, fep::rpc::IRPCConfiguration& rpc_config, T value, T init_value, std::string propertyname)
{
    std::string path = "deeper_path";
    std::string propertypath_withdots = path + "." + propertyname;

    //only properties that exist will be set
    ASSERT_TRUE(fep::isOk(pt.SetPropertyValue(propertypath_withdots.c_str(), init_value)));

    auto properties_to_test = rpc_config.getProperties("/" + path);
    auto property_names = properties_to_test->getPropertyNames();
    ASSERT_TRUE(std::find(property_names.begin(), property_names.end(), propertyname) != property_names.end());

    ASSERT_EQ(properties_to_test->getPropertyType(propertyname), fep::PropertyType<T>::getTypeName());

    ASSERT_TRUE(properties_to_test->setProperty(propertyname,
        fep::DefaultPropertyTypeConversion<T>::toString(value),
        fep::PropertyType<T>::getTypeName()));

    T ret_value = init_value;
    ASSERT_TRUE(fep::isOk(pt.GetPropertyValue(propertypath_withdots.c_str(), ret_value)));
    ASSERT_EQ(ret_value, value);
}

inline void testSetter(fep::IPropertyTree& pt, fep::rpc::IRPCConfiguration& rpc_config, std::string value, std::string init_value, std::string propertyname)
{
    std::string path = "deeper_path";
    std::string propertypath_withdots = path + "." + propertyname;

    //only properties that exist will be set
    ASSERT_TRUE(fep::isOk(pt.SetPropertyValue(propertypath_withdots.c_str(), init_value.c_str())));

    auto properties_to_test = rpc_config.getProperties("/" + path);
    auto property_names = properties_to_test->getPropertyNames();
    ASSERT_TRUE(std::find(property_names.begin(), property_names.end(), propertyname) != property_names.end());

    ASSERT_EQ(properties_to_test->getPropertyType(propertyname), fep::PropertyType<std::string>::getTypeName());

    ASSERT_TRUE(properties_to_test->setProperty(propertyname,
        fep::DefaultPropertyTypeConversion<std::string>::toString(value),
        fep::PropertyType<std::string>::getTypeName()));

    std::string ret_value = init_value;
    const char* ret_val_as_char;
    ASSERT_TRUE(fep::isOk(pt.GetPropertyValue(propertypath_withdots.c_str(), ret_val_as_char)));
    ret_value = ret_val_as_char;
    ASSERT_EQ(ret_value, value);
}

inline void testSetterExistingProperty(fep::rpc::IRPCConfiguration& rpc_config,
    const std::string& value,
    const std::string& init_value,
    const std::string& property_path,
    const std::string& property_name)
{
    auto properties_to_test = rpc_config.getProperties(property_path);

    ASSERT_EQ(fep::DefaultPropertyTypeConversion<std::string>::fromString(properties_to_test->getProperty(property_name)),
        init_value);

    ASSERT_TRUE(properties_to_test->setProperty(property_name,
        fep::DefaultPropertyTypeConversion<std::string>::toString(value),
        fep::PropertyType<std::string>::getTypeName()));

    ASSERT_EQ(fep::DefaultPropertyTypeConversion<std::string>::fromString(properties_to_test->getProperty(property_name)),
        value);
}

template<typename T>
inline void testSetGetInvalidFormat(fep::rpc::IRPCConfiguration& rpc_config,
    T value,
    std::string property_path)
{
    auto properties_to_test = rpc_config.getProperties("/");
    auto property_names = properties_to_test->getPropertyNames();

    ASSERT_FALSE(properties_to_test->setProperty(property_path,
        fep::DefaultPropertyTypeConversion<T>::toString(value),
        fep::PropertyType<std::string>::getTypeName()));

    ASSERT_EQ("", properties_to_test->getProperty(property_path));

    ASSERT_EQ("", properties_to_test->getPropertyType(property_path));
}

inline void testSetGetInvalidFormat(fep::rpc::IRPCConfiguration& rpc_config,
    std::string value,
    std::string property_path)
{
    auto properties_to_test = rpc_config.getProperties("/");
    auto property_names = properties_to_test->getPropertyNames();

    ASSERT_FALSE(properties_to_test->setProperty(property_path,
        fep::DefaultPropertyTypeConversion<std::string>::toString(value),
        fep::PropertyType<std::string>::getTypeName()));

    ASSERT_EQ("", properties_to_test->getProperty(property_path));

    ASSERT_EQ("", properties_to_test->getPropertyType(property_path));
}

template<typename T>
inline void testSetGetRoot(fep::rpc::IRPCConfiguration& rpc_config,
    T initial_value,
    T value,
    std::string property_path,
    std::string property_name)
{
    std::string property_name_with_preceding_slash = "/" + property_name;
    auto properties_to_test = rpc_config.getProperties(property_path);

    ASSERT_EQ(properties_to_test->getProperty(property_name_with_preceding_slash),
        fep::DefaultPropertyTypeConversion<T>::toString(initial_value));
    ASSERT_EQ(properties_to_test->getPropertyType(property_name_with_preceding_slash),
        fep::PropertyType<T>::getTypeName());

    ASSERT_TRUE(properties_to_test->setProperty(property_name_with_preceding_slash,
        fep::DefaultPropertyTypeConversion<T>::toString(value),
        fep::PropertyType<T>::getTypeName()));

    ASSERT_EQ(properties_to_test->getProperty(property_name_with_preceding_slash),
        fep::DefaultPropertyTypeConversion<T>::toString(value));
}

inline void testSetGetRoot(fep::rpc::IRPCConfiguration& rpc_config,
    std::string initial_value,
    std::string value,
    std::string property_path,
    std::string property_name)
{
    std::string property_name_with_preceding_slash = "/" + property_name;
    auto properties_to_test = rpc_config.getProperties(property_path);
    ASSERT_EQ(properties_to_test->getProperty(property_name_with_preceding_slash),
        fep::DefaultPropertyTypeConversion<std::string>::toString(initial_value));
    ASSERT_EQ(properties_to_test->getPropertyType(property_name_with_preceding_slash),
        fep::PropertyType<std::string>::getTypeName());

    ASSERT_TRUE(properties_to_test->setProperty(property_name_with_preceding_slash,
        fep::DefaultPropertyTypeConversion<std::string>::toString(value),
        fep::PropertyType<std::string>::getTypeName()));

    ASSERT_EQ(properties_to_test->getProperty(property_name_with_preceding_slash),
        fep::DefaultPropertyTypeConversion<std::string>::toString(value));
}

template<typename T>
void testArraySetter(fep::IPropertyTree& pt,
                     fep::rpc::IRPCConfiguration& rpc_config,
                     T value,
                     T init_value,
                     std::string propertyname)
{
    std::string path = "deeper_path";
    std::string propertypath_withdots = path + "." + propertyname;

    //only properties that exists will be set
    pt.SetPropertyValue(propertypath_withdots.c_str(), init_value[0]);
    auto prop = pt.GetProperty(propertypath_withdots.c_str());
    bool first = true;
    for (const auto& current_val : init_value)
    {
        if (first)
        {
            first = false;
            prop->SetValue(current_val);
        }
        else
        {
            prop->AppendValue(current_val);
        }
    }

    auto properties_to_test = rpc_config.getProperties("/" + path);
    auto property_names = properties_to_test->getPropertyNames();
    ASSERT_TRUE(std::find(property_names.begin(), property_names.end(), propertyname) != property_names.end());

    ASSERT_EQ(properties_to_test->getPropertyType(propertyname), fep::PropertyType<T>::getTypeName());

    ASSERT_TRUE(properties_to_test->setProperty(propertyname,
        fep::DefaultPropertyTypeConversion<T>::toString(value),
        fep::PropertyType<T>::getTypeName()));

    T ret_value;
    
    //only properties that exists will be set
    prop = pt.GetProperty(propertypath_withdots.c_str());
    for (size_t idx = 0; idx != prop->GetArraySize(); idx++)
    {
        typename T::value_type val_type;
        prop->GetValue(val_type, idx);
        ret_value.push_back(val_type);
    }
    ASSERT_EQ(ret_value, value);
}

inline void testArraySetter(fep::IPropertyTree& pt,
                fep::rpc::IRPCConfiguration& rpc_config,
                std::vector<std::string> value,
                std::vector<std::string> init_value,
                std::string propertyname)
{
    std::string path = "deeper_path";
    std::string propertypath_withdots = path + "." + propertyname;

    //only properties that exists will be set
    pt.SetPropertyValue(propertypath_withdots.c_str(), init_value[0].c_str());
    auto prop = pt.GetProperty(propertypath_withdots.c_str());
    bool first = true;
    for (auto& current_val : init_value)
    {
        if (first)
        {
            first = false;
            prop->SetValue(current_val.c_str());
        }
        else
        {
            prop->AppendValue(current_val.c_str());
        }
    }

    auto properties_to_test = rpc_config.getProperties("/" + path);
    auto property_names = properties_to_test->getPropertyNames();
    ASSERT_TRUE(std::find(property_names.begin(), property_names.end(), propertyname) != property_names.end());

    ASSERT_EQ(properties_to_test->getPropertyType(propertyname), fep::PropertyType<std::vector<std::string>>::getTypeName());

    ASSERT_TRUE(properties_to_test->setProperty(propertyname,
        fep::DefaultPropertyTypeConversion<std::vector<std::string>>::toString(value),
        fep::PropertyType<std::vector<std::string>>::getTypeName()));

    std::vector<std::string> ret_value;

    //only properties that exists will be set
    prop = pt.GetProperty(propertypath_withdots.c_str());
    for (size_t idx = 0; idx != prop->GetArraySize(); idx++)
    {
        const char* val_type;
        prop->GetValue(val_type, idx);
        ret_value.push_back(std::string(val_type));
    }

    ASSERT_EQ(ret_value, value);
}

