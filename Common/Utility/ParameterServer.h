/**
 * @file    ParameterServer.h
 * @brief   Parameter server implementation to keep general or project-specific configs.
 * @author  Mustafa Yemural (myemural)
 * @date    7.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <unordered_map>
#include <string>
#include <any>
#include <mutex>
#include <typeindex>
#include <utility>

namespace common::utility
{
struct ParameterInfo
{
    std::type_index Type;
    std::any DefaultValue;
    bool HasDefaultValue;
    bool IsImmutable;
};

class ParameterSchema
{
public:
    template<typename ParamType>
    void RegisterParam(const std::string &name, ParamType defaultValue)
    {
        if (params_.contains(name)) {
            throw std::runtime_error("Parameter already registered: " + name);
        }

        params_.emplace(name, ParameterInfo{typeid(ParamType), defaultValue, true, false});
    }

    template<typename ParamType>
    void RegisterParam(const std::string &name)
    {
        if (params_.contains(name)) {
            throw std::runtime_error("Parameter already registered: " + name);
        }

        params_.emplace(name, ParameterInfo{typeid(ParamType), std::any{}, false, false});
    }

    template<typename ParamType>
    void RegisterImmutableParam(const std::string &name, ParamType defaultValue)
    {
        if (params_.contains(name)) {
            throw std::runtime_error("Parameter already registered: " + name);
        }

        params_.emplace(name, ParameterInfo{typeid(ParamType), defaultValue, true, true});
    }

    [[nodiscard]] const ParameterInfo &GetInfo(const std::string &name) const
    {
        const auto it = params_.find(name);
        if (it == params_.end()) {
            throw std::runtime_error("Parameter not registered: " + name);
        }

        return it->second;
    }

    [[nodiscard]] bool HasParam(const std::string &name) const
    {
        return params_.contains(name);
    }

private:
    std::unordered_map<std::string, ParameterInfo> params_;
};

class ParameterServer
{
public:
    explicit ParameterServer(ParameterSchema schema)
        : schema_{std::move(schema)}
    {
    }

    /**
     * @brief Sets a parameter with a value of a specified type.
     * @tparam ParamType Type of the parameter.
     * @param key Key name of the parameter.
     * @param value Value of the parameter.
     */
    template<typename ParamType>
    void Set(const std::string &key, const ParamType &value)
    {
        if (!schema_.HasParam(key)) {
            throw std::runtime_error("Parameter not registered: " + key);
        }

        const auto &info = schema_.GetInfo(key);

        if (info.Type != std::type_index(typeid(ParamType))) {
            throw std::runtime_error("Type mismatch for parameter: " + key);
        }

        if (info.IsImmutable && params_.contains(key)) {
            throw std::runtime_error("Parameter is immutable after first set: " + key);
        }

        params_[key] = value;
    }

    /**
     * @brief Gets a parameter with a specific name.
     * @tparam ParamType Type of the parameter.
     * @param key Key name of the parameter.
     * @return Value of the parameter in specified type.
     */
    template<typename ParamType>
    ParamType Get(const std::string &key) const
    {
        if (!schema_.HasParam(key)) {
            throw std::runtime_error("Parameter not registered: " + key);
        }

        const auto &info = schema_.GetInfo(key);

        if (info.Type != std::type_index(typeid(ParamType))) {
            throw std::runtime_error("Type mismatch for parameter: " + key);
        }

        if (const auto it = params_.find(key); it != params_.end()) {
            return std::any_cast<ParamType>(it->second);
        }

        if (info.HasDefaultValue) {
            return std::any_cast<ParamType>(info.DefaultValue);
        }

        throw std::runtime_error("Parameter not set and no default value: " + key);
    }

private:
    ParameterSchema schema_;
    std::unordered_map<std::string, std::any> params_;
};
} // common::utility
