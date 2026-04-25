#pragma once

/**
 * @file        string-hashed.hpp
 * @author      dexus1337
 * @brief       Defines a class for handling hashed strings in the ADAM system.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include <string>
#include <cstdint>


namespace adam 
{
    /**
     * @class string_hashed
     * @brief A class for handling hashed strings in the ADAM system.
     */
    class ADAM_SDK_API string_hashed : public std::string   
    {
    public:

        using hash_datatype = uint64_t;   /**< The data type used for storing the hash value of the string. */

        /** @brief Constructs a new string_hashed object. */
        string_hashed();

        /** @brief Constructs a new string_hashed object. */
        string_hashed(std::string_view name);

        /** @brief Destroys the string_hashed object and cleans up resources. */
        ~string_hashed();

        hash_datatype get_hash() const { return m_ui64_hash; }

        /** @brief Calculates the hash value of the string and stores it in the m_ui64_hash member variable. */
        void calculate_hash();

        /** @brief Compares two string_hashed objects for equality based on their hash values. */
        bool operator==( const string_hashed& other ) const { return m_ui64_hash == other.m_ui64_hash; }

        /** @brief Compares two string_hashed objects for inequality based on their hash values. */
        bool operator!=( const string_hashed& other ) const { return m_ui64_hash != other.m_ui64_hash; }

        /** @brief Compares two string_hashed objects for ordering based on their hash values. Used for std::set and std::map. */
        bool operator<( const string_hashed& other ) const { return m_ui64_hash < other.m_ui64_hash; }

    private:
        hash_datatype m_ui64_hash;    /**< The hash value of the string, used for efficient comparisons. */

    };
}

namespace std 
{
    template<>
    struct hash<adam::string_hashed> 
    {
        /**
         * @brief Returns the pre-calculated hash stored in the adam::string_hashed.
         * This is O(1) and involves zero string processing.
         */
        size_t operator() (const adam::string_hashed& hs) const noexcept 
        {
            // We cast to size_t to match the std::hash interface
            return static_cast<size_t>(hs.get_hash());
        }
    };
}