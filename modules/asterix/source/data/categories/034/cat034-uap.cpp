#include "data/categories/034/cat034-uap.hpp"

#include <array>

namespace adam::modules::asterix::cat034
{
    using namespace adam::string_hashed_ct_literals;

    // -------------------------------------------------------------------------------------------------------------- //
    // Standard UAP for Radar Service Messages – Ed. 1.29
    // Implements the layout defined in Table 3 of the Part 2b specification.
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat034_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I034/010 Data Source Identifier"                                      },
        {  2, item_type_fixed,       0,      1, "I034/000 Message Type"                                                },
        {  3, item_type_fixed,       0,      3, "I034/030 Time-of-Day"                                                 },
        {  4, item_type_fixed,       0,      1, "I034/020 Sector Number"                                               },
        {  5, item_type_fixed,       0,      2, "I034/041 Antenna Rotation Period"                                     },
        {  6, item_type_variable,    1,      1, "I034/050 System Configuration and Status"                             },
        {  7, item_type_variable,    1,      1, "I034/060 System Processing Mode"                                      },

        // FSPEC Byte 2
        {  8, item_type_repetetive,  1,      2, "I034/070 Message Count Values"                                        },
        {  9, item_type_fixed,       0,      8, "I034/100 Generic Polar Window"                                        },
        { 10, item_type_fixed,       0,      1, "I034/110 Data Filter"                                                 },
        { 11, item_type_fixed,       0,      8, "I034/120 3D-Position of Data Source"                                  },
        { 12, item_type_fixed,       0,      2, "I034/090 Collimation Error"                                           },
        { 13, item_type_explicit,    0,      0, "RE Reserved Expansion Field"                                          },
        { 14, item_type_explicit,    0,      0, "SP Special Purpose Field"                                             },
    });

    uap cat034_uap(34, "CAT034 " CAT034_VERSION ""_ct, cat034_items.data(), cat034_items.size(), 1);

    uap& get_uap()
    {
        return cat034_uap;
    }
}
