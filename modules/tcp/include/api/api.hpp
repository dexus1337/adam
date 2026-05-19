#pragma once

/**
 * @file    api.hpp
 * @author  dexus1337
 * @brief   Defines macros for controlling symbol visibility in the Replay/Recording module.
 * @version 1.0
 * @date    28.04.2026
 */

// For windows, we have to explicitly use __declspec
#if defined(ADAM_PLATFORM_WINDOWS)
    #define NOMINMAX // so we can use std::min etc
    #ifdef ADAM_TCP_EXPORTS // This should be defined when building the DLL, not when using it
        #define ADAM_TCP_API __declspec(dllexport)
    #elif defined(ADAM_USE_SHARED_TCP)
        #define ADAM_TCP_API __declspec(dllimport)
    #else 
        #define ADAM_TCP_API
    #endif
#else
    // Under Linux/macOS, all symbols are visible by default,
    // unless you use special compiler flags (-fvisibility=hidden)
    #if __GNUC__ >= 4
        #define ADAM_TCP_API __attribute__ ((visibility ("default")))
    #else
        #define ADAM_TCP_API
    #endif
#endif