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
#include <regex>

#include <fep3/components/rpc/fep_rpc.h>
//this will be installed !!
#include "rpc_components/configuration/configuration_rpc_intf.h"
#include <fep3/rpc_components/configuration/configuration_service_client.h>
#include "connection_interface.h"
#include "base/properties/property_type.h"
#include "base/properties/property_type_conversion.h"

#include <a_util/strings.h>

#define FEP_CONFIG_LOG_RESULT(_res_, _participant_name_, _component_name_, _method_, _path_) { \
_logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_ERROR, _participant_name_, \
_component_name_, \
a_util::strings::format("Can not %s '%s' due to following error : (%d - %s)  ", \
    _method_.c_str(), \
    _path_.c_str(), \
    _res_.getErrorCode(), \
    _res_.getErrorLabel(), \
    _res_.getDescription())); }

#define FEP_CONFIG_LOG_AND_THROW_RESULT(_res_, _participant_name_, _component_name_, _method_, _path_) \
FEP_CONFIG_LOG_RESULT(_res_, _participant_name_, _component_name_, _method_, _path_) \
throw std::runtime_error{ "Could not execute " + _participant_name_ + "->" + _component_name_ + "->" + _method_ + " - Error: " + _res_.getErrorLabel() };

namespace fep
{
    class ConfigurationProxy : public rpc_object_proxy< rpc_stubs::RPCConfigurationClient, rpc::IRPCConfiguration>,
                               public std::enable_shared_from_this<const ConfigurationProxy>
    {
        private:
            typedef rpc_object_proxy< rpc_stubs::RPCConfigurationClient, rpc::IRPCConfiguration> base_type;
            friend class ConnectionInterfaceProperty;

        class ConfigurationProperty : public IProperties
        {
            std::shared_ptr<const ConfigurationProxy> _clientsafe_ptr;
            rpc_stubs::RPCConfigurationClient& _stub;
            std::string                       _property_path;
            std::string                       _participant_name;
            std::string                       _component_name;
            ISystemLogger&                    _logger;
            const std::regex                  _property_path_regex = std::regex("([/]?([a-zA-Z0-9_]+[/]?)*)");
            AutomationInterface*              _ai_hacky_for_timing_config_check;

        public:
            ConfigurationProperty() = delete;
            ~ConfigurationProperty() = default;
            ConfigurationProperty(ConfigurationProperty&&) = delete;
            ConfigurationProperty(const ConfigurationProperty&) = delete;
            ConfigurationProperty& operator=(ConfigurationProperty&&) = delete;
            ConfigurationProperty& operator=(const ConfigurationProperty&) = delete;
            ConfigurationProperty(std::shared_ptr<const ConfigurationProxy> client,
                                  rpc_stubs::RPCConfigurationClient& stub,
                                  std::string property_path,
                                  std::string participant_name,
                                  std::string component_name,
                                  ISystemLogger& logger,
                                  AutomationInterface* ai_hacky_for_timing_config_check) :
                _clientsafe_ptr(client),
                _stub(stub),
                _property_path(std::move(property_path)),
                _participant_name(std::move(participant_name)),
                _component_name(std::move(component_name)),
                _logger(logger),
                _ai_hacky_for_timing_config_check(ai_hacky_for_timing_config_check)
            {
            }

            bool setProperty(const std::string& name,
                             const std::string& value,
                             const std::string& type)
            {
                std::string path = _property_path + normalizeName(name);
                if (!isPropertyPathValid(path))
                {
                    fep::Result result(ERR_INVALID_ARG);
                    FEP_CONFIG_LOG_RESULT(result, _participant_name, _component_name, std::string("setProperty"), path);
                    return false;
                }
                else
                {
                    int32_t retval = _stub.setProperty(path, type, value);
                    if (retval == 0)
                    {
                        return true;
                    }
                    else
                    {
                        if (_ai_hacky_for_timing_config_check
                            && retval == -7
                            && (path.find("ComponentConfig/Timing/TimingClient/strTimingConfig") != std::string::npos))
                        {
                            auto ai_res = _ai_hacky_for_timing_config_check->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE,
                                value,
                                _participant_name);
                            if (fep::isOk(ai_res))
                            {
                                return true;
                            }
                            else
                            {
                                FEP_CONFIG_LOG_RESULT(ai_res,
                                                      _participant_name,
                                                      _component_name,
                                                      std::string("setProperty"), path);
                                return false;
                            }
                        }
                        fep::Result ret_code(retval);
                        FEP_CONFIG_LOG_RESULT(ret_code,
                                              _participant_name,
                                              _component_name,
                                              std::string("setProperty"), path);
                        return false;
                    }
                }
            }
            
            std::string getProperty(const std::string& name) const
            {
                std::string path = _property_path + normalizeName(name);
                if (!isPropertyPathValid(path))
                {
                    fep::Result result(ERR_INVALID_ARG);
                    FEP_CONFIG_LOG_RESULT(result, _participant_name, _component_name, std::string("getProperty"), path);
                    return "";
                }
                else
                {
                    std::string type = _stub.getProperty(path)["type"].asString();
                    if (type.empty())
                    {
                        FEP_CONFIG_LOG_RESULT(fep::Result(ERR_PATH_NOT_FOUND), _participant_name, _component_name, std::string("getProperty"), path);
                        return "";
                    }
                    else
                    {
                        std::string value = _stub.getProperty(path)["value"].asString();
                        return value;
                    }
                }
            }

            std::string getPropertyType(const std::string& name) const
            {
                std::string path = _property_path + normalizeName(name);
                if (!isPropertyPathValid(path))
                {
                    fep::Result result(ERR_INVALID_ARG);
                    FEP_CONFIG_LOG_RESULT(result, _participant_name, _component_name, std::string("getPropertyType"), path);
                    return "";
                }
                else
                {

                    std::string type = _stub.getProperty(path)["type"].asString();
                    if (type.empty())
                    {
                        FEP_CONFIG_LOG_RESULT(fep::Result(ERR_PATH_NOT_FOUND), _participant_name, _component_name, std::string("getPropertyType"), path);
                        return "";
                    }
                    else
                    {
                        return type;
                    }
                }
            }
            
            bool isEqual(const IProperties& properties) const
            {
                Properties<IProperties> mirrored_properties;
                auto prop_names = getPropertyNames();
                for (const auto& prop_name : prop_names)
                {
                    std::string path = _property_path + prop_name;
                    auto val = _stub.getProperty(path);
                    mirrored_properties.setProperty(prop_name, val["value"].asString(), val["type"].asString());
                }
                return mirrored_properties.isEqual(properties);
            }
            
            void copy_to(IProperties& properties) const
            {
                Properties<IProperties> mirrored_properties;
                auto prop_names = getPropertyNames();
                for (const auto& prop_name : prop_names)
                {
                    std::string path = _property_path + prop_name;
                    auto val = _stub.getProperty(path);
                    properties.setProperty(prop_name, val["value"].asString(), val["type"].asString());
                }
            }

            std::vector<std::string> getPropertyNames() const
            {
                auto props = _stub.getProperties(_property_path);
                return a_util::strings::split(props, ",");
            }

        private:
            /**
            * @brief Check a property path which includes the property name for validity.
            * Currently only the '/' syntax is considered valid.
            * '.' syntax is not supported.
            *
            * @param property_path the propery path to be checked
            *
            * @return bool success boolean
            */
            bool isPropertyPathValid(const std::string& property_path) const
            {
                return std::regex_match(property_path, _property_path_regex);
            }

            /**
            * @brief Remove leading and concluding '/' characters a property name.
            *
            * @param property_name the propery name to be normalized
            *
            * @return string the normalized property name
            */
            std::string normalizeName(const std::string& property_name) const
            {
                std::string normalized_name = property_name;

                if (normalized_name.at(0) == '/')
                {
                    normalized_name = normalized_name.substr(1, normalized_name.size());
                }
                if (normalized_name.at(normalized_name.size() - 1) == '/')
                {
                    normalized_name = normalized_name.substr(0, normalized_name.size() - 1);
                }

                return normalized_name;
            }
        };

        public:
            using base_type::GetStub;
            ConfigurationProxy(std::string participant_name,
                              std::string rpc_component_name,
                              IRPC& rpc,
                              ISystemLogger& logger,
                              AutomationInterface* ai_hacky_for_timing_config_check=nullptr) :
                              _logger(logger),
                              _ai_hacky_for_timing_config_check(ai_hacky_for_timing_config_check),
                              base_type(participant_name.c_str(), rpc_component_name.c_str(), rpc),
                              _participant_name(participant_name),
                              _component_name(rpc_component_name)
            {
            }

            std::string normalizePath(const std::string& property_path) const
            {
                if (property_path == "/" || property_path.empty())
                {
                    return "/";
                }
                else
                {
                    if (property_path.at(property_path.size() - 1) == '/')
                    {
                        return property_path;
                    }
                    else
                    {
                        std::string retval = property_path + "/";
                        return retval;
                    }
                }
            }
    
            /**
             * @brief Get a list of the signal names going in
             *
             * @return std::vector<std::string> signal list names
             */
            std::shared_ptr<IProperties> getProperties(const std::string& property_path) override
            {
                std::shared_ptr<const ConfigurationProxy> ptr = shared_from_this();
                std::string normalized_path = normalizePath(property_path);

                if (base_type::GetStub().exists(normalized_path))
                {
                    return std::make_shared<ConfigurationProperty>(ptr,
                        GetStub(),
                        normalized_path,
                        _participant_name,
                        _component_name,
                        _logger,
                        _ai_hacky_for_timing_config_check);
                }
                else
                {
                    FEP_CONFIG_LOG_AND_THROW_RESULT(fep::Result(ERR_PATH_NOT_FOUND), _participant_name, _component_name, std::string("getProperties"), property_path);
                }
            }
            /**
             * @brief Get a list of the signal names going out
             *
             * @return std::vector<std::string> signal list names
             */
            std::shared_ptr<const IProperties> getProperties(const std::string& property_path) const override
            {
                std::shared_ptr<const ConfigurationProxy> ptr = shared_from_this();
                std::string normalized_path = normalizePath(property_path);

                if (base_type::GetStub().exists(normalized_path))
                {
                    return std::make_shared<ConfigurationProperty>(shared_from_this(),
                        GetStub(),
                        normalized_path,
                        _participant_name,
                        _component_name,
                        _logger,
                        _ai_hacky_for_timing_config_check);
                }
                else
                {
                    FEP_CONFIG_LOG_AND_THROW_RESULT(fep::Result(ERR_PATH_NOT_FOUND), _participant_name, _component_name, std::string("getProperties"), property_path);
                }
            }

        private:
            ISystemLogger&                    _logger;
            std::string                       _participant_name;
            std::string                       _component_name;
            AutomationInterface*              _ai_hacky_for_timing_config_check;
    };

    class ConfigurationProxyOldSql : public IRPCObjectClient, public rpc::IRPCConfiguration
    {
        class ConnectionInterfaceProperty : public IProperties
        {
            public:
                ConnectionInterfaceProperty(std::string participant_name,
                    std::string component_name,
                    std::string currentpath,
                    timestamp_t timeout,
                    ISystemLogger& logger) :
                    _participant_name(std::move(participant_name)),
                    _component_name(std::move(component_name)),
                    _current_path(std::move(currentpath)),
                    _timeout(timeout),
                    _logger(logger)
                {
                }
                virtual ~ConnectionInterfaceProperty() = default;
                ConnectionInterfaceProperty() = delete;
                ConnectionInterfaceProperty(const ConnectionInterfaceProperty&) = delete;
                ConnectionInterfaceProperty(ConnectionInterfaceProperty&&) = delete;
                ConnectionInterfaceProperty& operator=(const ConnectionInterfaceProperty&) = delete;
                ConnectionInterfaceProperty& operator=(ConnectionInterfaceProperty&&) = delete;

            private:
                bool checkResult(const std::string& method, std::string& path, fep::Result res) const
                {
                    if (fep::isOk(res))
                    {
                        return true;
                    }
                    else
                    {
                        //also TIMEOUT is a valid error code (for not existing property)
                        if (res.getErrorCode() == ERR_FAILED)
                        {
                            FEP_CONFIG_LOG_AND_THROW_RESULT(res, _participant_name, _component_name, method, path);
                        }
                        else
                        {
                            _logger.log(logging::CATEGORY_COMPONENT, logging::SEVERITY_WARNING, _participant_name,
                                _component_name,
                                a_util::strings::format("Can not %s '%s' due following error : (%d - %s)  ",
                                    method.c_str(),
                                    path.c_str(),
                                    res.getErrorCode(),
                                    res.getErrorLabel(),
                                    res.getDescription()));
                        }
                        return false;
                    }
                }
                std::string addPath(const std::string& path) const
                {
                    if (_current_path.empty())
                    {
                        return path;
                    }
                    else
                    {
                        return _current_path + "." + path;
                    }
                }
            public:
                bool setProperty(const std::string& name,
                                 const std::string& value,
                                 const std::string& type) override
                {
                    std::string path = addPath(name);
                    if (type == PropertyType<bool>::getTypeName())
                    {
                        auto res = _coin.getAI().SetPropertyValue(path,
                            DefaultPropertyTypeConversion<bool>::fromString(value),
                            _participant_name,
                            _timeout);
                        return checkResult("setProperty", path, res);
                    }
                    else if (type == PropertyType<int32_t>::getTypeName())
                    {
                        auto res = _coin.getAI().SetPropertyValue(path,
                            DefaultPropertyTypeConversion<int32_t>::fromString(value),
                            _participant_name,
                            _timeout);
                        return checkResult("setProperty", path, res);
                    }
                    else if (type == PropertyType<double>::getTypeName())
                    {
                        auto res = _coin.getAI().SetPropertyValue(path,
                            DefaultPropertyTypeConversion<double>::fromString(value),
                            _participant_name,
                            _timeout);
                        return checkResult("setProperty", path, res);
                    }
                    else if (type == PropertyType<std::string>::getTypeName())
                    {
                        auto res = _coin.getAI().SetPropertyValue(path,
                            DefaultPropertyTypeConversion<std::string>::fromString(value),
                            _participant_name,
                            _timeout);
                        return checkResult("setProperty", path, res);
                    }
                    else if (type == PropertyType<std::vector<bool>>::getTypeName())
                    {
                        std::vector<bool> vector_value = DefaultPropertyTypeConversion<std::vector<bool>>::fromString(value);
                        auto res = _coin.getAI().SetPropertyValues(path,
                            vector_value,
                            _participant_name,
                            _timeout);
                         return checkResult("setProperty", path, res);
                    }
                    else if (type == PropertyType<std::vector<int32_t>>::getTypeName())
                    {
                        std::vector<int32_t> vector_value = DefaultPropertyTypeConversion<std::vector<int32_t>>::fromString(value);
                        auto res = _coin.getAI().SetPropertyValues(path,
                            vector_value,
                            _participant_name,
                            _timeout);
                        return checkResult("setProperty", path, res);
                    }
                    else if (type == PropertyType<std::vector<double>>::getTypeName())
                    {
                        std::vector<double> vector_value = DefaultPropertyTypeConversion<std::vector<double>>::fromString(value);
                        auto res = _coin.getAI().SetPropertyValues(path,
                            vector_value,
                            _participant_name,
                            _timeout);
                        return checkResult("setProperty", path, res);
                    }
                    else if (type == PropertyType<std::vector<std::string>>::getTypeName())
                    {
                        std::vector<std::string> vector_value = DefaultPropertyTypeConversion<std::vector<std::string>>::fromString(value);
                        auto res = _coin.getAI().SetPropertyValues(path,
                            vector_value,
                            _participant_name,
                            _timeout);
                        return checkResult("setProperty", path, res);
                    }
                    else
                    {
                        return checkResult("setProperty", path, ERR_INVALID_TYPE);
                    }
                }

                template<typename T> std::string getPropertyValueAsString(const std::unique_ptr<IProperty>& property_val) const
                {
                    T value;
                    property_val->GetValue(value);
                    return DefaultPropertyTypeConversion<T>::toString(value);
                }

                template<typename T>
                std::string getArrayPropertyValueAsString(const std::unique_ptr<IProperty>& property_val) const
                {
                    std::vector<T> value_array;
                    size_t arr_size = property_val->GetArraySize();
                    for (size_t idx = 0;
                        idx < arr_size;
                        idx++)
                    {
                        T current_val;
                        property_val->GetValue(current_val, idx);
                        value_array.push_back(current_val);
                    }
                    return DefaultPropertyTypeConversion<std::vector<T>>::toString(value_array);
                }
            

                std::string getProperty(const std::string& name) const override
                {
                    std::string path = addPath(name);
                    std::unique_ptr<IProperty> retrieved_property;
                    auto res = _coin.getAI().GetProperty(path, retrieved_property, _participant_name, _timeout);
                    if (fep::isFailed(res))
                    {
                        checkResult("getProperty", path, res);
                        return std::string();
                    }
                    else
                    {
                        if (retrieved_property->IsArray())
                        {
                            if (retrieved_property->IsBoolean())
                            {
                                return getArrayPropertyValueAsString<bool>(retrieved_property);
                            }
                            else if (retrieved_property->IsInteger())
                            {
                                return getArrayPropertyValueAsString<int32_t>(retrieved_property);
                            }
                            else if (retrieved_property->IsFloat())
                            {
                                return getArrayPropertyValueAsString<double>(retrieved_property);
                            }
                            else if (retrieved_property->IsString())
                            {
                                std::vector<std::string> value;
                                size_t arr_size = retrieved_property->GetArraySize();
                                for (size_t idx = 0;
                                    idx < arr_size;
                                    idx++)
                                {
                                    const char* current_val;
                                    retrieved_property->GetValue(current_val, idx);
                                    value.push_back(std::string(current_val));
                                }
                                return DefaultPropertyTypeConversion<std::vector<std::string>>::toString(value);
                            }
                            else
                            {
                                checkResult("getProperty", path, ERR_INVALID_TYPE);
                                return std::string();
                            }
                        }
                        else
                        {
                            if (retrieved_property->IsBoolean())
                            {
                                return getPropertyValueAsString<bool>(retrieved_property);
                            }
                            else if (retrieved_property->IsInteger())
                            {
                                return getPropertyValueAsString<int32_t>(retrieved_property);
                            }
                            else if (retrieved_property->IsFloat())
                            {
                                return getPropertyValueAsString<double>(retrieved_property);
                            }
                            else if (retrieved_property->IsString())
                            {
                                const char* val;
                                retrieved_property->GetValue(val);
                                return std::string(val);
                            }
                            else
                            {
                                checkResult("getProperty", path, ERR_INVALID_TYPE);
                                return std::string();
                            }
                        }
                    }
                }

                /**
                 * @brief gets the property value
                 *
                 * @param name name of the
                 * @return std::string the value
                 */
                std::string getPropertyType(const std::string& name) const override
                {
                    std::string path = addPath(name);
                    std::unique_ptr<IProperty> retrieved_property;
                    auto res = _coin.getAI().GetProperty(path, retrieved_property, _participant_name, _timeout);
                    if (fep::isFailed(res))
                    {
                        checkResult("getPropertyType", path, res);
                        return std::string();
                    }
                    else
                    {
                        if (retrieved_property->IsArray())
                        {
                            if (retrieved_property->IsBoolean())
                            {
                                return PropertyType<std::vector<bool>>::getTypeName();
                            }
                            else if (retrieved_property->IsInteger())
                            {
                                return PropertyType<std::vector<int32_t>>::getTypeName();
                            }
                            else if (retrieved_property->IsFloat())
                            {
                                return PropertyType<std::vector<double>>::getTypeName();
                            }
                            else if (retrieved_property->IsString())
                            {
                                return PropertyType<std::vector<std::string>>::getTypeName();
                            }
                            else
                            {
                                checkResult("getPropertyType", path, ERR_INVALID_TYPE);
                                return std::string();
                            }
                        }
                        else
                        {
                            if (retrieved_property->IsBoolean())
                            {
                                return PropertyType<bool>::getTypeName();
                            }
                            else if (retrieved_property->IsInteger())
                            {
                                return PropertyType<int32_t>::getTypeName();
                            }
                            else if (retrieved_property->IsFloat())
                            {
                                return PropertyType<double>::getTypeName();
                            }
                            else if (retrieved_property->IsString())
                            {
                                return PropertyType<std::string>::getTypeName();
                            }
                            else
                            {
                                checkResult("getPropertyType", path, ERR_INVALID_TYPE);
                                return std::string();
                            }
                        }
                    }
                }
                /**
                 * @brief compares this key value list with the given properties instance
                 * the properties are equal if each property of this will have the same value within \p properties
                 *
                 * @param properties the properties instance to compare to
                 * @return true each properties of this have the same value within \p properties
                 * @return false \p properties have different values
                 */
                bool isEqual(const IProperties& properties) const override
                {
                    Properties<IProperties> this_properties;
                    std::vector<std::string> property_names = getPropertyNames();
                    for (const auto& prop_name : property_names)
                    {
                        auto value = getProperty(prop_name);
                        auto type_name = getPropertyType(prop_name);
                        auto succeeded = this_properties.setProperty(prop_name, value, type_name);
                        if (!succeeded)
                        {
                            //the log should be done already by getProperty or getPropertyType
                            return false;
                        }
                    }
                    return this_properties.isEqual(properties);
                }
                /**
                 * @brief assignment helper class
                 *
                 * @param properties properties to copy my values to
                 */
                void copy_to(IProperties& properties) const
                {
                    std::vector<std::string> property_names = getPropertyNames();
                    for (const auto& prop_name : property_names)
                    {
                        auto value = getProperty(prop_name);
                        auto type_name = getPropertyType(prop_name);
                        properties.setProperty(prop_name, value, type_name);
                    }
                }

                /**
                 * @brief returns a list of all property names of this node
                 *
                 * @param list of all properties of this node
                 */
                std::vector<std::string> getPropertyNames() const
                {
                    std::string path = _current_path;
                    
                    std::vector<std::string> ret_val;
                    std::unique_ptr<IProperty> retrieved_property;
                    auto res = _coin.getAI().GetProperty(path, retrieved_property, _participant_name, _timeout);
                    if (fep::isFailed(res))
                    {
                        checkResult("getPropertyNames", path, res);
                    }
                    else
                    {
                        auto sub_props = retrieved_property->GetSubProperties();
                        ret_val.reserve(sub_props.size());
                        for (const auto& sub_prop_current : sub_props)
                        {
                            ret_val.push_back(sub_prop_current->GetName());
                        }
                    }
                    return ret_val;
                }
            private:
                std::string _participant_name;
                std::string _component_name;
                ConnectionInterface _coin;
                std::string _current_path;
                timestamp_t _timeout;
                ISystemLogger& _logger;
        };

        public:
            ConfigurationProxyOldSql(std::string participant_name,
                ISystemLogger& logger,
                std::string rpc_component_name,
                timestamp_t timeout) :
                    _participant_name(std::move(participant_name)),
                    _logger(logger),
                    _component_name(std::move(rpc_component_name)),
                    _timeout(timeout)
            {
            }
            std::string getRPCObjectIID() const override
            {
                return rpc::IRPCConfiguration::getRPCIID();
            }
            std::string getRPCObjectDefaultName() const override
            {
                return rpc::IRPCConfiguration::getRPCDefaultName();
            }

            std::string normalizePath(const std::string& path) const
            {
                if (path.empty() || path == "/")
                {
                    return "";
                }
                else
                {
                    std::string norm_path;
                    if (path.at(0) == '/')
                    {
                        norm_path = path.substr(1, path.size() - 1);
                    }
                    else
                    {
                        norm_path = path;
                    }
                    a_util::strings::replace(norm_path, "/", ".");
                    return norm_path;
                }
            }

            /**
             * @brief Get a list of the signal names going in
             *
             * @return std::vector<std::string> signal list names
             */
            std::shared_ptr<IProperties> getProperties(const std::string& property_path) override
            {
                std::unique_ptr<fep::IProperty> retrieved_property;
                std::string normalized_path = normalizePath(property_path);
                
                auto res = _coin.getAI().GetProperty(normalized_path, retrieved_property, _participant_name, _timeout);
                if (fep::isOk(res) && retrieved_property.get() != nullptr)
                {
                    return std::make_shared<ConnectionInterfaceProperty>(_participant_name,
                                                                         _component_name,
                                                                         normalized_path,
                                                                         _timeout,
                                                                         _logger);
                }
                else
                {
                    FEP_CONFIG_LOG_AND_THROW_RESULT(res, _participant_name, _component_name, std::string("getProperties"), property_path);
                }
            }
            /**
             * @brief Get a list of the signal names going out
             *
             * @return std::vector<std::string> signal list names
             */
            std::shared_ptr<const IProperties> getProperties(const std::string& property_path) const override
            {
                std::unique_ptr<fep::IProperty> retrieved_property;
                std::string normalized_path = normalizePath(property_path);

                auto res = _coin.getAI().GetProperty(normalized_path, retrieved_property, _participant_name, _timeout);
                if (fep::isOk(res) && retrieved_property.get() != nullptr)
                {
                    return std::make_shared<ConnectionInterfaceProperty>(_participant_name,
                        _component_name,
                        normalized_path,
                        _timeout,
                        _logger);
                }
                else
                {
                    FEP_CONFIG_LOG_AND_THROW_RESULT(res, _participant_name, _component_name, std::string("getProperties"), property_path);
                }
            }
        private:
            std::string _participant_name;
            std::string _component_name;
            std::string _current_name;
            ConnectionInterface _coin;
            ISystemLogger& _logger;
            timestamp_t  _timeout;
    };
}

