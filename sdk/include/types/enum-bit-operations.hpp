/**
 * @file    enum-bit-operations.hpp
 * @author  dexus1337
 * @brief   Defines a macro to generate bitwise operators for enum types.
 * @version 1.0
 * @date    19.06.2026
 */


#include "api/api-sdk.hpp"


#define enable_enum_bit_operations(T) \
    inline ADAM_CONSTEXPR T  operator|(T lhs, T rhs)    { return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs)); } \
    inline ADAM_CONSTEXPR T  operator&(T lhs, T rhs)    { return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs)); } \
    inline ADAM_CONSTEXPR T  operator^(T lhs, T rhs)    { return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) ^ static_cast<std::underlying_type_t<T>>(rhs)); } \
    inline ADAM_CONSTEXPR T  operator~(T rhs)           { return static_cast<T>(~static_cast<std::underlying_type_t<T>>(rhs)); } \
    inline T&                operator|=(T& lhs, T rhs)  { lhs = lhs | rhs; return lhs; } \
    inline T&                operator&=(T& lhs, T rhs)  { lhs = lhs & rhs; return lhs; }
    