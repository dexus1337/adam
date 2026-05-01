#include "string/string-hashed.hpp"

#include "hash/rapidhash.h"

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
        m_ui64_hash = rapidhash(data(), length());
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