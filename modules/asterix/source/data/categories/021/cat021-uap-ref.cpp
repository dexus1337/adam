#include "data/categories/021/cat021-uap-ref.hpp"

namespace adam::modules::asterix::cat021::ref
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for REF sub-UAPs
    extern uap cat021_ref_mes_uap;

    // -------------------------------------------------------------------------------------------------------------- //
    // Sub-UAP for Reserved Expansion Field (REF) - Appendix A
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat021_ref_items = std::to_array<const field_spec>
    (
        {
            {  1, item_type_fixed,       0,      2, "BPS - Barometric Pressure Setting"                                },
            {  2, item_type_fixed,       0,      2, "SelH - Selected Heading"                                          },
            {  3, item_type_fixed,       0,      1, "NAV - Navigation Mode Settings"                                   },
            {  4, item_type_fixed,       0,      1, "GAO - GPS Antenna Offset"                                         },
            {  5, item_type_variable,    2,      1, "SGV - Surface Ground Vector"                                      },
            {  6, item_type_variable,    1,      1, "STA - Aircraft Status"                                            },
            {  7, item_type_fixed,       0,      2, "TNH - True North Heading"                                         },
            {  8, item_type_compound,    0,      0, "MES - Military Extended Squitters",            &cat021_ref_mes_uap}
        }
    );

    uap cat021_ref_uap(21, "CAT021 REF " CAT021_REF_VERSION ""_ct, cat021_ref_items.data(), cat021_ref_items.size());

    // -------------------------------------------------------------------------------------------------------------- //
    // Sub-UAP for MES (Military Extended Squitters)
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat021_ref_mes_items = std::to_array<const field_spec>
    (
        {
            {  1, item_type_fixed,       0,      1, "Mode 5 Summary"                                                   },
            {  2, item_type_fixed,       0,      4, "Mode 5 PIN / National Origin"                                     },
            {  3, item_type_fixed,       0,      2, "Extended Mode 1 Code in Octal Representation"                     },
            {  4, item_type_fixed,       0,      1, "X Pulse Presence"                                                 },
            {  5, item_type_fixed,       0,      1, "Figure of Merit"                                                  },
            {  6, item_type_fixed,       0,      2, "Mode 2 Code in Octal Representation"                              }
        }
    );

    uap cat021_ref_mes_uap(21, "CAT021 REF MES " CAT021_REF_VERSION ""_ct, cat021_ref_mes_items.data(), cat021_ref_mes_items.size());

    uap& get_uap()
    {
        return cat021_ref_uap;
    }
}
