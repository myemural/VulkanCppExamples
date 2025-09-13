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
#include <iostream>
#include <mutex>

namespace common::utility
{
class ParameterServer
{
public:
    ParameterServer() = default;

    ParameterServer(const ParameterServer &) = delete;

    ParameterServer &operator=(const ParameterServer &) = delete;

    ParameterServer(ParameterServer &&other) noexcept
    {
        std::lock_guard lock(other.mutex_);
        params_ = std::move(other.params_);
    }

    ParameterServer &operator=(ParameterServer &&other) noexcept
    {
        if (this != &other) {
            std::lock_guard lock1(mutex_);
            std::lock_guard lock2(other.mutex_);
            params_ = std::move(other.params_);
        }
        return *this;
    }

    /**
     * @brief Sets a parameter with a specified name with a value of a specified type.
     * @tparam T Type of the parameter.
     * @param key Key name of the parameter.
     * @param val Value of the parameter.
     * @param writeOnce Specifies whether the parameter will be set only once.
     */
    template<typename T>
    void Set(const std::string &key, T val, const bool writeOnce = false)
    {
        std::lock_guard lock(mutex_);
        Parameter &param = params_[key];
        if (param.writeOnce && param.isSet) {
            std::cout << "Warning: Attempt to modify write-once parameter ignored.\n";
            return;
        }

        param.value = std::make_any<T>(val);
        param.writeOnce = writeOnce;
        param.isSet = true;
    }

    /**
     * @brief Gets a parameter with a specific name.
     * @tparam T Type of the parameter.
     * @param key Key name of the parameter.
     * @return Value of the parameter in specified type.
     */
    template<typename T>
    T Get(const std::string &key) const
    {
        std::lock_guard lock(mutex_);
        const auto it = params_.find(key);
        if (it == params_.end() || !it->second.value.has_value()) {
            throw std::runtime_error("Parameter not found or not set: " + key);
        }
        return std::any_cast<T>(it->second.value);
    }

private:
    struct Parameter
    {
        std::any value;
        bool writeOnce = false;
        bool isSet = false;
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Parameter> params_;
};
} // common::utility
