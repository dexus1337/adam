#pragma once

/**
 * @file        api.hpp
 * @author      dexus1337
 * @brief       Defines macros for controlling symbol visibility in the ASTERIX module.
 * @version     1.0
 * @date        25.04.2026
 */

// For windows, we have to explicitly use __declspec
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef ADAM_ASTERIX_EXPORTS // This should be defined when building the DLL, not when using it
    #define ADAM_ASTERIX_API __declspec(dllexport)
  #else
    #define ADAM_ASTERIX_API __declspec(dllimport)
  #endif
#else
  // Under Linux/macOS, all symbols are visible by default,
  // unless you use special compiler flags (-fvisibility=hidden)
  #if __GNUC__ >= 4
    #define ADAM_ASTERIX_API __attribute__ ((visibility ("default")))
  #else
    #define ADAM_ASTERIX_API
  #endif
#endif