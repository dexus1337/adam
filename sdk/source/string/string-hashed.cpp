#include "string/string-hashed.hpp"

namespace adam 
{
    string_hashed::string_hashed() : m_ui64_hash(0) {}

    string_hashed::string_hashed(std::string_view name) : std::string(name), m_ui64_hash(0) 
    {
        calculate_hash();
    }

    string_hashed::string_hashed(const string_hashed& other) : std::string(other), m_ui64_hash(other.m_ui64_hash) {}

    string_hashed::~string_hashed() {}

    void string_hashed::calculate_hash() 
    {
        // Simple hash function (FNV-1a)
        static constexpr hash_datatype fnv_offset_basis = 14695981039346656037ULL;
        static constexpr hash_datatype fnv_prime        = 1099511628211ULL;

        m_ui64_hash = fnv_offset_basis;

        for (auto c : *this) 
        {
            m_ui64_hash ^= static_cast<hash_datatype>(c);
            m_ui64_hash *= fnv_prime;
        }
    }

    string_hashed& string_hashed::operator=(const string_hashed& other) 
    {
        if (this != &other) 
        {
            std::string::operator=(other);
            m_ui64_hash = other.m_ui64_hash;
        }
        return *this;
    }
}