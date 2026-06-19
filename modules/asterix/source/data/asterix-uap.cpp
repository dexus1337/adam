#include "data/asterix-uap.hpp"
#include "data/categories/001/cat001-uap.hpp"
#include "data/categories/048/cat048-uap.hpp"
#include "data/categories/062/cat062-uap.hpp"
#include <cstring>

namespace adam::modules::asterix
{
    uap::uap(uint8_t cat, const string_hashed_ct& name, const field_spec* spec_array, size_t spec_count)
        : cat_id(cat), name(name), last_frn(0)
    {
        std::memset(items_by_frn, 0, sizeof(items_by_frn));
        std::memset(custom_expansions, 0, sizeof(custom_expansions));

        for (size_t i = 0; i < spec_count; ++i)
        {
            items_by_frn[spec_array[i].frn] = &spec_array[i];
            if (spec_array[i].frn > last_frn)
                last_frn = spec_array[i].frn; // Track highest FRN for fixed-FSPEC sizing
        }
    }

    uap::uap(uint8_t cat, const string_hashed_ct& name, const field_spec* spec_array, size_t spec_count, const uap* alt_array, size_t alt_cout, selector_function sel)
     :  uap(cat, name, spec_array, spec_count)
    {
        for (size_t i = 0; i < alt_cout; i++)
            alternatives.emplace(alt_array[i].get_name().get_hash(), &alt_array[i]);
        
        selector_fn = sel; 
    }

    void uap::setup()
    {
        compute_subitem_count();
    }

    void uap::expand_parameter(uint8_t frn, uap_expansion* data)
    {
        custom_expansions[frn] = data;
    }

    void uap::compute_subitem_count()
    {
        subitem_count = 0;
        
        for (size_t i = 0; i <= last_frn; ++i)
        {
            if (items_by_frn[i] && items_by_frn[i]->sub_uap)
            {
                auto* const_casted_uap = const_cast<class uap*>(items_by_frn[i]->sub_uap);

                const_casted_uap->compute_subitem_count();

                subitem_count += const_casted_uap->subitem_count;
            }
        }
    }

    uap_pool& uap_pool::get()
    {
        static uap_pool instance;
        return instance;
    }

    uap_pool::uap_pool()
    {
        std::memset(registered_uaps, 0, sizeof(registered_uaps));

        // Self-initialize with standard supported categories
        register_uap(&cat001::get_uap());
        register_uap(&cat048::get_uap());
        register_uap(&cat062::get_uap());
    }

    void uap_pool::register_uap(uap* uap)
    {
        if (uap)
        {
            uap->setup();
            registered_uaps[uap->get_cat_number()] = uap;
        }
    }
}
