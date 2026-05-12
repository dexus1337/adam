#pragma once

/**
 * @file    sdk-api.hpp
 * @author  dexus1337
 * @brief   Defines macros for controlling symbol visibility in the SDK and platform/architecture detection.
 * @version 1.0
 * @date    25.04.2026
 */

// Use the compiler's built-in knowledge
#if not defined (ADAM_PLATFORM_LINUX) && not defined (ADAM_PLATFORM_WINDOWS) && not defined (ADAM_PLATFORM_APPLE)
    #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        #define ADAM_PLATFORM_WINDOWS
    #elif defined(__linux__)
        #define ADAM_PLATFORM_LINUX
    #elif defined(__APPLE__)
        #define ADAM_PLATFORM_APPLE
    #endif
#endif

// For windows, we have to explicitly use __declspec
#if defined(ADAM_PLATFORM_WINDOWS)
    #ifndef NOMINMAX
    #define NOMINMAX // so we can use std::min etc
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #ifdef ADAM_SDK_EXPORTS // This should be defined when building the DLL, not when using it
        #define ADAM_SDK_API __declspec(dllexport)
    #elifdef ADAM_USE_SHARED_SDK_
        #define ADAM_SDK_API __declspec(dllimport)
    #else 
        #define ADAM_SDK_API
    #endif
#else
    // Under Linux/macOS, all symbols are visible by default,
    // unless you use special compiler flags (-fvisibility=hidden)
    #if __GNUC__ >= 4
        #define ADAM_SDK_API __attribute__ ((visibility ("default")))
    #else
        #define ADAM_SDK_API
    #endif
#endif

// Architecture detection
#if defined(__aarch64__) || defined(_M_ARM64)
    #define ADAM_CPU_ARM64
#elif defined(__x86_64__) || defined(_M_X64)
    #define ADAM_CPU_X64
#endif

// Debug statement
#if not defined (ADAM_BUILD_DEBUG) && not defined (ADAM_BUILD_RELEASE)
#endif

#if defined ADAM_BUILD_DEBUG
#define debug_statement(x) x
#else
#define debug_statement(x)
#endif
