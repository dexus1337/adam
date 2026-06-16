#include "data/asterix-uap.hpp"
#include "data/categories/48/cat048_uap.hpp"
#include "data/categories/62/cat062_uap.hpp"
#include <cstring>

namespace adam::modules::asterix
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
        register_uap(&get_cat048_uap());
        register_uap(&get_cat062_uap());
    }

    void uap_pool::register_uap(uap* uap)
    {
        if (uap)
        {
            uap->item_count = uap->compute_total_items(); // Pre-compute total item count for buffer sizing
            registered_uaps[uap->cat_id] = uap;
        }
    }

    const uap* uap_pool::get_uap(uint8_t cat_id) const
    {
        return registered_uaps[cat_id];
    }
}
