#pragma once

/**
 * @file    api.hpp
 * @author  dexus1337
 * @brief   Defines macros for controlling symbol visibility in the CAN module.
 * @version 1.0
 * @date    25.04.2026
 */

// For windows, we have to explicitly use __declspec
#if defined(ADAM_PLATFORM_WINDOWS)
    #define NOMINMAX // so we can use std::min etc
    #ifdef ADAM_CAN_EXPORTS // This should be defined when building the DLL, not when using it
        #define ADAM_CAN_API __declspec(dllexport)
    #elif defined(ADAM_USE_SHARED_CAN)
        #define ADAM_CAN_API __declspec(dllimport)
    #else 
        #define ADAM_CAN_API
    #endif
#else
    // Under Linux/macOS, all symbols are visible by default,
    // unless you use special compiler flags (-fvisibility=hidden)
    #if __GNUC__ >= 4
        #define ADAM_CAN_API __attribute__ ((visibility ("default")))
    #else
        #define ADAM_CAN_API
    #endif
#endif