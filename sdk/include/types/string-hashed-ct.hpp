#pragma once

/**
 * @file    string-hashed-ct.hpp
 * @author  dexus1337
 * @brief   Defines a class for handling hashed strings at COMPILE TIME.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"

#include <string>
#include <cstdint>
#include <functional>

#include "hash/rapidhash-ct.h"


namespace adam 
{
    /**
     * @class string_hashed_ct_template
     * @brief A class for handling hashed strings in the ADAM system.
     */
    template<typename char_type>
    class ADAM_SDK_API string_hashed_ct_template
    {
    public:

        using hash_datatype = uint64_t;   /**< The data type used for storing the hash value of the string. */
        
        /** @brief Constructs a new string_hashed_ct_template object. */
        template<size_t string_len>
        consteval string_hashed_ct_template(const char_type (&arr)[string_len]) : m_text(arr), m_length(string_len - 1), m_hash(rapidhash_ct(&arr[0], (string_len - 1) * sizeof(char_type))) { }

        constexpr hash_datatype get_hash()  const { return m_hash; }
        
        /** @brief Implicit conversion operator to the underlying hash type, allowing direct usage in switch statements. */
        constexpr operator hash_datatype()  const { return m_hash; }
        constexpr size_t get_length()       const { return m_length; }
        constexpr const char_type* c_str()  const { return m_text; }

        /** @brief Compares two string_hashed_ct_template objects for equality based on their hash values. */
        constexpr bool operator==(const string_hashed_ct_template& other) const { return m_hash == other.m_hash; }

        /** @brief Compares two string_hashed_ct_template objects for inequality based on their hash values. */
        constexpr bool operator!=(const string_hashed_ct_template& other) const { return m_hash != other.m_hash; }

        /** @brief Compares two string_hashed_ct_template objects for ordering based on their hash values. Used for std::set and std::map. */
        constexpr bool operator<(const string_hashed_ct_template& other) const { return m_hash < other.m_hash; }

        /** @brief Assigns the value of another string_hashed_ct_template object to this one. */
        constexpr string_hashed_ct_template& operator=(const string_hashed_ct_template& other);

    private:
        const char_type*    m_text;
        size_t              m_length;
        hash_datatype       m_hash;    /**< The hash value of the string, used for efficient comparisons. */

    };

    using string_hashed_ct          = string_hashed_ct_template<char>;
    using wstring_hashed_ct         = string_hashed_ct_template<wchar_t>;
    #if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
    using string_hashed_ct_utf8     = string_hashed_ct_template<char8_t>;
    #endif
    using string_hashed_ct_utf16    = string_hashed_ct_template<char16_t>;
    using string_hashed_ct_utf32    = string_hashed_ct_template<char32_t>;
}

namespace std 
{
    template<>
    struct hash<adam::string_hashed_ct> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed_ct.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed_ct& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        size_t operator() (uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed_ct> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed_ct& lhs, const adam::string_hashed_ct& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed_ct& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed_ct& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };

    template<>
    struct hash<adam::wstring_hashed_ct> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed_ct.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::wstring_hashed_ct& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        size_t operator() (uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::wstring_hashed_ct> 
    {
        using is_transparent = void;

        bool operator()(const adam::wstring_hashed_ct& lhs, const adam::wstring_hashed_ct& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::wstring_hashed_ct& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::wstring_hashed_ct& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };
    
    #if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
    template<>
    struct hash<adam::string_hashed_ct_utf8> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed_ct_utf8.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed_ct_utf8& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        size_t operator() (uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed_ct_utf8> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed_ct_utf8& lhs, const adam::string_hashed_ct_utf8& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed_ct_utf8& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed_ct_utf8& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };
    #endif

    template<>
    struct hash<adam::string_hashed_ct_utf16> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed_ct_utf16& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        size_t operator() (uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed_ct_utf16> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed_ct_utf16& lhs, const adam::string_hashed_ct_utf16& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed_ct_utf16& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed_ct_utf16& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };

    template<>
    struct hash<adam::string_hashed_ct_utf32> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed_ct_utf32& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        size_t operator() (uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed_ct_utf32> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed_ct_utf32& lhs, const adam::string_hashed_ct_utf32& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed_ct_utf32& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed_ct_utf32& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };
}