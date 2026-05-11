#pragma once

/**
 * @file    string-hashed.hpp
 * @author  dexus1337
 * @brief   Defines a class for handling hashed strings in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"

#include <string>
#include <functional>
#include <cstdint>

#include "string-hashed-ct.hpp"


namespace adam 
{
    /**
     * @class string_hashed_template
     * @brief A class for handling hashed strings in the ADAM system.
     */
    template<typename char_type>
    class ADAM_SDK_API string_hashed_template : public std::basic_string<char_type>
    {
    public:

        using hash_datatype = uint64_t;                             /**< The data type used for storing the hash value of the string. */
        using view          = std::basic_string_view<char_type>;    /**< The string_view */
        using string_type   = std::basic_string<char_type>;         /**< The string type */

        /** @brief Constructs a new string_hashed_template object. */
        string_hashed_template();

        /** @brief Constructs a new string_hashed_template object. */
        string_hashed_template(view vv);

        /** @brief Constructs a new string_hashed_template object from a C-style string. */
        string_hashed_template(const char_type* str);

        /** @brief Constructs a new string_hashed_template object. */
        string_hashed_template(const string_hashed_template& other);

        /** @brief Constructs a new string_hashed_template object from a string_hashed_ct_template. */
        string_hashed_template(const string_hashed_ct_template<char_type>& ct_str);

        /** @brief Constructs a new string_hashed_template object from a compile-time string literal, evaluating the hash instantly. */
        template<size_t string_len>
        string_hashed_template(const char_type (&arr)[string_len]) : string_type(arr, string_len - 1), m_hash(rapidhash_ct(&arr[0], (string_len - 1) * sizeof(char_type))) { }

        /** @brief Destroys the string_hashed_template object and cleans up resources. */
        ~string_hashed_template();

        hash_datatype get_hash() const { return m_hash; }

        /** @brief Implicit conversion operator to the underlying hash type, allowing direct usage in switch statements. */
        operator hash_datatype() const { return m_hash; }

        /** @brief Calculates the hash value of the string and stores it in the m_hash member variable. */
        void calculate_hash();

        /** @brief Compares two string_hashed_template objects for equality based on their hash values. */
        bool operator==(const string_hashed_template& other) const { return m_hash == other.m_hash; }

        /** @brief Compares two string_hashed_template objects for equality based on their hash values. */
        bool operator==(const string_hashed_ct_template<char_type>& other) const { return m_hash == other.get_hash(); }

        /** @brief Compares two string_hashed_template objects for inequality based on their hash values. */
        bool operator!=(const string_hashed_template& other) const { return m_hash != other.m_hash; }

        /** @brief Compares two string_hashed_template objects for inequality based on their hash values. */
        bool operator!=(const string_hashed_ct_template<char_type>& other) const { return m_hash != other.get_hash(); }

        /** @brief Compares two string_hashed_template objects for ordering based on their hash values. Used for std::set and std::map. */
        bool operator<(const string_hashed_template& other) const { return m_hash < other.m_hash; }

        /** @brief Compares two string_hashed_template objects for ordering based on their hash values. Used for std::set and std::map. */
        bool operator<(const string_hashed_ct_template<char_type>& other) const { return m_hash < other.get_hash(); }

        /** @brief Assigns the value of another string_hashed_template object to this one. */
        string_hashed_template& operator=(const string_hashed_template& other);

        /** @brief Assigns the value of another string_hashed_template object to this one. */
        string_hashed_template& operator=(const string_hashed_ct_template<char_type>& other);

    private:
        hash_datatype m_hash;    /**< The hash value of the string, used for efficient comparisons. */

    };

    using string_hashed          = string_hashed_template<char>;
    using wstring_hashed         = string_hashed_template<wchar_t>;
    #if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
    using string_hashed_utf8     = string_hashed_template<char8_t>;
    #endif
    using string_hashed_utf16    = string_hashed_template<char16_t>;
    using string_hashed_utf32    = string_hashed_template<char32_t>;
}

namespace std 
{
    template<>
    struct hash<adam::string_hashed> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed& hs) const noexcept 
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
    struct equal_to<adam::string_hashed> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed& lhs, const adam::string_hashed& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };

    template<>
    struct hash<adam::wstring_hashed> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::wstring_hashed.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::wstring_hashed& hs) const noexcept 
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
    struct equal_to<adam::wstring_hashed> 
    {
        using is_transparent = void;

        bool operator()(const adam::wstring_hashed& lhs, const adam::wstring_hashed& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::wstring_hashed& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::wstring_hashed& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };

    #if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
    template<>
    struct hash<adam::string_hashed_utf8> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed_utf8.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed_utf8& hs) const noexcept 
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
    struct equal_to<adam::string_hashed_utf8> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed_utf8& lhs, const adam::string_hashed_utf8& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed_utf8& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed_utf8& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };
    #endif

    template<>
    struct hash<adam::string_hashed_utf16> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed_utf16& hs) const noexcept 
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
    struct equal_to<adam::string_hashed_utf16> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed_utf16& lhs, const adam::string_hashed_utf16& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed_utf16& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed_utf16& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };

    template<>
    struct hash<adam::string_hashed_utf32> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed_utf32& hs) const noexcept 
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
    struct equal_to<adam::string_hashed_utf32> 
    {
        using is_transparent = void;

        bool operator()(const adam::string_hashed_utf32& lhs, const adam::string_hashed_utf32& rhs) const noexcept { return lhs == rhs; }
        bool operator()(const adam::string_hashed_utf32& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        bool operator()(uint64_t lhs, const adam::string_hashed_utf32& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };
}