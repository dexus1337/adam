#include "data/asterix-uap.hpp"
#include "data/uap/cat048.hpp"
#include <cstring>

namespace adam::modules::asterix::data
{
    uap_pool& uap_pool::get_instance()
    {
        static uap_pool instance;
        return instance;
    }

    uap_pool::uap_pool()
    {
        std::memset(registered_uaps, 0, sizeof(registered_uaps));

        // Self-initialize with standard supported categories
        register_uap(&uap::get_cat048_uap());
    }

    void uap_pool::register_uap(const asterix_uap* uap)
    {
        if (uap)
        {
            registered_uaps[uap->cat_id] = uap;
        }
    }

    const asterix_uap* uap_pool::get_uap(uint8_t cat_id) const
    {
        return registered_uaps[cat_id];
    }
}
