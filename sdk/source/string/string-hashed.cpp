#include "string/string-hashed.hpp"

#include "hash/rapidhash.h"

namespace adam 
{
    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template() : m_hash(0) {}

    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template(view vv) : string_type(vv), m_hash(0) 
    {
        calculate_hash();
    }

    template<typename char_type>
    string_hashed_template<char_type>::string_hashed_template(const string_hashed_template& other) : string_type(other), m_hash(other.m_hash) {}

    template<typename char_type>
    string_hashed_template<char_type>::~string_hashed_template() {}

    template<typename char_type>
    void string_hashed_template<char_type>::calculate_hash() 
    {
        m_hash = rapidhash(string_type::data(), string_type::length());
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

    // Explicit instantiation
    template class string_hashed_template<char>;
    template class string_hashed_template<wchar_t>;
    #if defined (ADAM_PLATFORM_LINUX) && (_GLIBCXX_USE_CHAR8_T) || (ADAM_PLATFORM_WINDOWS)
    template class string_hashed_template<char8_t>;
    #endif
    template class string_hashed_template<char16_t>;
    template class string_hashed_template<char32_t>;
}