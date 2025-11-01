/**
 * @file    CoreDefines.h
 * @brief   Contains core definitions of the project.
 * @author  Mustafa Yemural (myemural)
 * @date    1.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(COMMON_EXPORTS)
    #define COMMON_API __declspec(dllexport)
  #else
    #define COMMON_API __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #define COMMON_API __attribute__((visibility("default")))
  #else
    #define COMMON_API
  #endif
#endif