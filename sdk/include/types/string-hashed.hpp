#pragma once

/**
 * @file    string-hashed.hpp
 * @author  dexus1337
 * @brief   Defines a class for handling hashed strings in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"

#include <string>
#include <functional>
#include <cstdint>

#include "string-hashed-ct.hpp"
#include "hash/rapidhash.h"


namespace adam 
{
    /**
     * @class string_hashed_template
     * @brief A class for handling hashed strings in the ADAM system.
     */
    template<typename char_type>
    class string_hashed_template : public std::basic_string<char_type>
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

        /** @brief Disabled string literal constructor to enforce compile-time hashing. Use string_hashed_ct instead! */
        template<size_t string_len>
        string_hashed_template(const char_type (&arr)[string_len]) = delete;

        /** @brief Destroys the string_hashed_template object and cleans up resources. */
        ~string_hashed_template();

        inline hash_datatype get_hash() const { return m_hash; }

        /** @brief Implicit conversion operator to the underlying hash type, allowing direct usage in switch statements. */
        inline operator hash_datatype() const { return m_hash; }

        /** @brief Calculates the hash value of the string and stores it in the m_hash member variable. */
        inline void calculate_hash();

        /** @brief Compares two string_hashed_template objects for equality based on their hash values. */
        inline bool operator==(const string_hashed_template& other) const { return m_hash == other.m_hash; }

        /** @brief Compares two string_hashed_template objects for equality based on their hash values. */
        inline bool operator==(const string_hashed_ct_template<char_type>& other) const { return m_hash == other.get_hash(); }

        /** @brief Compares a string_hashed_template object with a raw hash value for equality. */
        inline bool operator==(hash_datatype other) const { return m_hash == other; }

        /** @brief Compares two string_hashed_template objects for inequality based on their hash values. */
        inline bool operator!=(const string_hashed_template& other) const { return m_hash != other.m_hash; }

        /** @brief Compares two string_hashed_template objects for inequality based on their hash values. */
        inline bool operator!=(const string_hashed_ct_template<char_type>& other) const { return m_hash != other.get_hash(); }

        /** @brief Compares a string_hashed_template object with a raw hash value for inequality. */
        inline bool operator!=(hash_datatype other) const { return m_hash != other; }

        /** @brief Compares two string_hashed_template objects for ordering based on their hash values. Used for std::set and std::map. */
        inline bool operator<(const string_hashed_template& other) const { return m_hash < other.m_hash; }

        /** @brief Compares two string_hashed_template objects for ordering based on their hash values. Used for std::set and std::map. */
        inline bool operator<(const string_hashed_ct_template<char_type>& other) const { return m_hash < other.get_hash(); }

        /** @brief Compares a string_hashed_template object with a raw hash value for ordering. */
        inline bool operator<(hash_datatype other) const { return m_hash < other; }

        /** @brief Assigns the value of another string_hashed_template object to this one. */
        string_hashed_template& operator=(const string_hashed_template& other);

        /** @brief Assigns the value of another string_hashed_template object to this one. */
        string_hashed_template& operator=(const string_hashed_ct_template<char_type>& other);

    private:
        hash_datatype m_hash;    /**< The hash value of the string, used for efficient comparisons. */

    };

    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template() : string_type(), m_hash(0) {}

    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template(view vv) : string_type(vv), m_hash(0)
    {
        calculate_hash();
    }

    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template(const char_type* str) : string_type(str), m_hash(0)
    {
        calculate_hash();
    }

    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template(const string_hashed_template& other) : string_type(other), m_hash(other.m_hash) {}

    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template(const string_hashed_ct_template<char_type>& ct_str)
        : string_type(ct_str.c_str()), m_hash(ct_str.get_hash())
    {
    }

    template<typename char_type>
    string_hashed_template<char_type>::~string_hashed_template() {}

    template<typename char_type>
    inline void string_hashed_template<char_type>::calculate_hash()
    {
        m_hash = string_type::empty() ? 0 : rapidhash(string_type::data(), string_type::length() * sizeof(char_type));
    }

    template<typename char_type>
    string_hashed_template<char_type>& string_hashed_template<char_type>::operator=(const string_hashed_template& other)
    {
        if (this != &other) 
        {
            string_type::operator=(other);
            m_hash = other.m_hash;
        }
        return *this;
    }

    template<typename char_type>
    string_hashed_template<char_type>& string_hashed_template<char_type>::operator=(const string_hashed_ct_template<char_type>& other)
    {
        string_type::operator=(other.c_str());
        m_hash = other.get_hash();
        return *this;
    }

    using string_hashed          = string_hashed_template<char>;
    using wstring_hashed         = string_hashed_template<wchar_t>;
    #if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
    using string_hashed_utf8     = string_hashed_template<char8_t>;
    #endif
    using string_hashed_utf16    = string_hashed_template<char16_t>;
    using string_hashed_utf32    = string_hashed_template<char32_t>;

    using string_hash            = string_hashed::hash_datatype;
    using wstring_hash           = wstring_hashed::hash_datatype;
    #if (defined(ADAM_PLATFORM_LINUX) && defined(_GLIBCXX_USE_CHAR8_T)) || defined(ADAM_PLATFORM_WINDOWS)
    using string_hash_utf8       = string_hashed::hash_datatype;
    #endif
    using string_hash_utf16      = string_hashed_utf16::hash_datatype;
    using string_hash_utf32      = string_hashed_utf32::hash_datatype;
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
        inline size_t operator()(const adam::string_hashed& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        inline size_t operator()(uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed> 
    {
        using is_transparent = void;

        inline bool operator()(const adam::string_hashed& lhs, const adam::string_hashed& rhs) const noexcept { return lhs == rhs; }
        inline bool operator()(const adam::string_hashed& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        inline bool operator()(uint64_t lhs, const adam::string_hashed& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };

    template<>
    struct hash<adam::wstring_hashed> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::wstring_hashed.
         * This is O(1) and involves zero string processing.
         */
        inline size_t operator()(const adam::wstring_hashed& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        inline size_t operator()(uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::wstring_hashed> 
    {
        using is_transparent = void;

        inline bool operator()(const adam::wstring_hashed& lhs, const adam::wstring_hashed& rhs) const noexcept { return lhs == rhs; }
        inline bool operator()(const adam::wstring_hashed& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        inline bool operator()(uint64_t lhs, const adam::wstring_hashed& rhs) const noexcept { return lhs == rhs.get_hash(); }
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
        inline size_t operator()(const adam::string_hashed_utf8& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        inline size_t operator()(uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed_utf8> 
    {
        using is_transparent = void;

        inline bool operator()(const adam::string_hashed_utf8& lhs, const adam::string_hashed_utf8& rhs) const noexcept { return lhs == rhs; }
        inline bool operator()(const adam::string_hashed_utf8& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        inline bool operator()(uint64_t lhs, const adam::string_hashed_utf8& rhs) const noexcept { return lhs == rhs.get_hash(); }
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
        inline size_t operator()(const adam::string_hashed_utf16& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        inline size_t operator()(uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed_utf16> 
    {
        using is_transparent = void;

        inline bool operator()(const adam::string_hashed_utf16& lhs, const adam::string_hashed_utf16& rhs) const noexcept { return lhs == rhs; }
        inline bool operator()(const adam::string_hashed_utf16& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        inline bool operator()(uint64_t lhs, const adam::string_hashed_utf16& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };

    template<>
    struct hash<adam::string_hashed_utf32> 
    {
        using is_transparent = void;

        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed.
         * This is O(1) and involves zero string processing.
         */
        inline size_t operator()(const adam::string_hashed_utf32& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }

        inline size_t operator()(uint64_t hash_val) const noexcept 
        {
            return static_cast<size_t>(hash_val);
        }
    };

    template<>
    struct equal_to<adam::string_hashed_utf32> 
    {
        using is_transparent = void;

        inline bool operator()(const adam::string_hashed_utf32& lhs, const adam::string_hashed_utf32& rhs) const noexcept { return lhs == rhs; }
        inline bool operator()(const adam::string_hashed_utf32& lhs, uint64_t rhs) const noexcept { return lhs.get_hash() == rhs; }
        inline bool operator()(uint64_t lhs, const adam::string_hashed_utf32& rhs) const noexcept { return lhs == rhs.get_hash(); }
    };
}