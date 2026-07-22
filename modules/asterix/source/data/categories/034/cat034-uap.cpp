#include "data/categories/034/cat034-uap.hpp"

#include <array>

#include "data/categories/034/cat034-structs.hpp"

namespace adam::modules::asterix::cat034
{
    using namespace adam::string_hashed_ct_literals;

    extern uap::type_getter_function cat034_type_getter_fn;

    // Fill the type database or this Category
    const uap::type_names_database& get_cat034_types() 
    {
        static const uap::type_names_database table = 
        {
            {cat034::message_type_north_marker,     "North Marker"              },
            {cat034::message_type_sector_crossing,  "Sector crossing"           },
            {cat034::message_type_geo_filter,       "Geographical filtering"    },
            {cat034::message_type_strobe,           "Jamming Strobe"            },
            {cat034::message_type_solar_storm,      "Solar Storm "              },
            {cat034::message_type_ssr_strobe,       "SSR Jamming Strobe"        },
            {cat034::message_type_modes_strobe,     "Mode S Jamming Strobe"     },
        };
        return table;
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // Standard UAP for Radar Service Messages – Ed. 1.29
    // Implements the layout defined in Table 3 of the Part 2b specification.
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat034_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I034/340 Data Source Identifier"                                      },
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

    // -------------------------------------------------------------------------------------------------------------- //
    // CAT034 Type Getter Function
    //
    // Reads (parsed) I034/000 (Message Type) to extract the type and return the corresponding enum entry.
    // -------------------------------------------------------------------------------------------------------------- //
    uap::type_getter_function cat034_type_getter_fn = [](const record* rec, const adam::buffer* buf) -> uint8_t
    {
        auto* msg_typ_itm = rec->get_item(2);

        if (!msg_typ_itm->is_populated())
            return 0xff;

        auto* data = msg_typ_itm->get_data_as<const cat034::message_type>(buf);

        if (!data)
            return 0xff;

        return *data;
    };

    uap cat034_uap
    (
        34,                             /**< CAT Number.          */
        "CAT034 " CAT034_VERSION ""_ct, /**< CAT Names.           */
        cat034_items.data(),            /**< Items Array start.   */
        cat034_items.size(),            /**< Items Array length.  */
        1,                              /**< SAC/SIC FRN.         */
        get_cat034_types(),             /**< Type Database.       */
        cat034_type_getter_fn           /**< Type Getter Function */
    );

    uap& get_uap()
    {
        return cat034_uap;
    }
}
