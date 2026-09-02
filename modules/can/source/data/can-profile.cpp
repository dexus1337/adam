#include "data/can-profile.hpp"
#include "data/profiles/mercedes/w209/w209-can-b.hpp"
#include "data/profiles/mercedes/w209/w209-can-c.hpp"
#include <cstring>

/**
 * @file    can-profile.cpp
 * @author  dexus1337
 * @brief   Implements the CAN Profile and CAN Profile Pool registry methods.
 * @version 1.0
 * @date    24.08.2026
 */

namespace adam::modules::can
{
    can_profile::can_profile
    (
        const string_hashed_ct&     name,
        const string_hashed_ct&     description,
        const can_message_spec*     messages,
        size_t                      message_count,
        endianness                  endian
    )
        : m_name(name),
          m_description(description),
          m_endianness(endian)
    {
        std::memset(m_std_id_table, 0, sizeof(m_std_id_table));

        for (size_t i = 0; i < message_count; ++i)
        {
            const auto& msg = messages[i];
            m_messages.push_back(&msg);

            if (!msg.is_extended && msg.can_id < 2048)
            {
                m_std_id_table[msg.can_id] = &msg;
            }
            else
            {
                m_extended_id_table[msg.can_id] = &msg;
            }
        }
    }

    can_profile_pool& can_profile_pool::get()
    {
        static can_profile_pool instance;
        return instance;
    }

    can_profile_pool::can_profile_pool()
    {
        // Self-initialize with default profiles
        register_profile(&profiles::mercedes::w209::can_b::get_profile());
        register_profile(&profiles::mercedes::w209::can_c::get_profile());
    }

    void can_profile_pool::register_profile(can_profile* profile)
    {
        if (!profile) return;

        m_profiles.push_back(profile);
        m_profiles_by_hash.emplace(profile->get_name().get_hash(), profile);
    }

    const can_profile* can_profile_pool::get_profile(string_hash name_hash) const
    {
        auto it = m_profiles_by_hash.find(name_hash);
        if (it != m_profiles_by_hash.end()) return it->second;

        return nullptr;
    }

    const can_profile* can_profile_pool::get_profile(const string_hashed& name) const
    {
        return get_profile(name.get_hash());
    }

    const can_profile* can_profile_pool::get_default_profile() const
    {
        if (m_profiles.empty()) return nullptr;

        return m_profiles.front();
    }
}
