/**
 * @file    VulkanObject.h
 * @brief   This file contains base class implementation for all Vulkan wrapper classes.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <iostream>

namespace common::vulkan_wrapper
{

template<typename ParentType, typename HandleType>
class VulkanObject {
public:
    VulkanObject() = default;
    VulkanObject(std::shared_ptr<ParentType> parent, HandleType handle)
        : parentObject_(std::move(parent)), handle_(handle) {}

    virtual ~VulkanObject() = default;

    VulkanObject(const VulkanObject&) = delete;
    VulkanObject& operator=(const VulkanObject&) = delete;

    VulkanObject(VulkanObject&& o) noexcept
        : parentObject_(o.parentObject_), handle_(o.handle_) {
        o.handle_ = nullptr;
    }
    VulkanObject& operator=(VulkanObject&& o) noexcept {
        if (this != &o) {
            parentObject_ = o.parentObject_;
            handle_ = o.handle_;
            o.handle_ = nullptr;
        }
        return *this;
    }


    HandleType GetHandle() const { return handle_; }
    std::shared_ptr<ParentType> GetParent() const
    {
        auto parentObject = parentObject_.lock();
        if (!parentObject) {
            std::cerr << "Parent object not found!" << std::endl;
            return nullptr;
        }
        return parentObject;
    }

    explicit operator bool() const { return handle_ != nullptr; }

protected:
    std::weak_ptr<ParentType> parentObject_{nullptr};
    HandleType handle_{nullptr};
};
} // common::vulkan_wrapper