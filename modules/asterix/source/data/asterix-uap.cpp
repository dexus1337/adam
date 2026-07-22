#include "data/asterix-uap.hpp"
#include "data/categories/001/cat001-uap.hpp"
#include "data/categories/007/cat007-uap.hpp"
#include "data/categories/021/cat021-uap.hpp"
#include "data/categories/034/cat034-uap.hpp"
#include "data/categories/048/cat048-uap.hpp"
#include "data/categories/062/cat062-uap.hpp"
#include <cstring>

namespace adam::modules::asterix
{
    uap::uap
    (
        uint8_t                     cat, 
        const string_hashed_ct&     name, 
        const field_spec*           spec_array, 
        size_t                      spec_count,
        uint8_t                     sacsic_frn,
        const type_names_database&  type_names,
        type_getter_function        type_get_fn
    )
     :  m_cat_id(cat),
        m_sacsic_frn(sacsic_frn),
        m_name(name),
        m_last_frn(0),
        m_subitem_count(0),
        m_alternatives(),
        m_selector_fn(),
        m_types_database(type_names),
        m_type_getter_fn(type_get_fn)
    {
        std::memset(m_items_by_frn, 0, sizeof(m_items_by_frn));
        std::memset(m_custom_expansions, 0, sizeof(m_custom_expansions));

        for (size_t i = 0; i < spec_count; ++i)
        {
            m_items_by_frn[spec_array[i].frn] = &spec_array[i];
            if (spec_array[i].frn > m_last_frn)
                m_last_frn = spec_array[i].frn; // Track highest FRN for fixed-FSPEC sizing
        }
    }

    uap::uap
    (
        uint8_t                     cat, 
        const string_hashed_ct&     name, 
        const field_spec*           spec_array, 
        size_t                      spec_count, 
        const uap*const*            alt_array, 
        size_t                      alt_cout, 
        selector_function           sel, 
        uint8_t                     sacsic_frn,
        const type_names_database&  type_names,
        type_getter_function        type_get_fn
    ) 
     :  uap
        (
            cat, 
            name, 
            spec_array, 
            spec_count, 
            sacsic_frn,
            type_names,
            type_get_fn
        )
    {
        for (size_t i = 0; i < alt_cout; i++)
            m_alternatives.emplace(alt_array[i]->get_name().get_hash(), alt_array[i]);
        
        m_selector_fn = std::move(sel); 
    }

    void uap::setup()
    {
        compute_subitem_count();
    }

    void uap::expand_parameter(uint8_t frn, uap_expansion* data)
    {
        m_custom_expansions[frn] = data;
    }

    void uap::compute_subitem_count()
    {
        m_subitem_count = 0;
        
        for (size_t i = 0; i <= m_last_frn; ++i)
        {
            if (m_items_by_frn[i] && m_items_by_frn[i]->sub_uap)
            {
                auto* const_casted_uap = const_cast<class uap*>(m_items_by_frn[i]->sub_uap);

                const_casted_uap->compute_subitem_count();

                m_subitem_count += const_casted_uap->m_subitem_count;
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
        std::memset(m_registered_uaps, 0, sizeof(m_registered_uaps));

        // Self-initialize with standard supported categories
        register_uap(&cat001::get_uap());
        register_uap(&cat007::get_uap());
        register_uap(&cat021::get_uap());
        register_uap(&cat034::get_uap());
        register_uap(&cat048::get_uap());
        register_uap(&cat062::get_uap());
    }

    void uap_pool::register_uap(uap* uap)
    {
        if (uap)
        {
            uap->setup();
            m_registered_uaps[uap->get_cat_number()] = uap;
        }
    }
}
