#include "data/categories/048/cat048-uap.hpp"

#include <array>

#include "data/categories/048/cat048-structs.hpp"
#include "data/categories/048/cat048-uap-ref.hpp"

namespace adam::modules::asterix::cat048
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for sub-UAPs so the main UAPs can sit at the top
    extern uap cat048_130_uap;
    extern uap cat048_120_uap;

    extern uap::type_getter_function cat048_type_getter_fn;

    // Fill the type database or this Category
    const uap::type_names_database& get_cat048_types() 
    {
        static const uap::type_names_database table = 
        {
            {cat048::message_type_no_detection,             "No detection"              },
            {cat048::message_type_single_psr,               "Single PSR"                },
            {cat048::message_type_single_ssr,               "Single SSR"                },
            {cat048::message_type_ssr_and_psr,              "SSR + PSR"                 },
            {cat048::message_type_single_mode_s_all_call,   "Single ModeS All-Call"     },
            {cat048::message_type_single_mode_s_roll_call,  "Single ModeS Roll-Call"    },
            {cat048::message_type_mode_s_all_call,          "ModeS All-Call + PSR"      },
            {cat048::message_type_mode_s_roll_call,         "ModeS Roll-Call +PSR"      },
        };
        return table;
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // Main UAP for CAT048
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat048_items = std::to_array<const field_spec> 
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I048/010 Data Source Identifier"                                      },
        {  2, item_type_fixed,       0,      3, "I048/140 Time of Day"                                                 },
        {  3, item_type_variable,    1,      1, "I048/020 Target Report Descriptor"                                    },
        {  4, item_type_fixed,       0,      4, "I048/040 Measured Position in Slant Polar Coordinates"                },
        {  5, item_type_fixed,       0,      2, "I048/070 Mode-3/A Code in Octal Representation"                       },
        {  6, item_type_fixed,       0,      2, "I048/090 Flight Level in Binary Representation"                       },
        {  7, item_type_compound,    0,      0, "I048/130 Radar Plot Characteristics",                  &cat048_130_uap},
        
        // FSPEC Byte 2
        {  8, item_type_fixed,       0,      3, "I048/220 Aircraft Address"                                            },
        {  9, item_type_fixed,       0,      6, "I048/240 Aircraft Identification"                                     },
        { 10, item_type_repetetive,  1,      8, "I048/250 BDS Register Data"                                           },
        { 11, item_type_fixed,       0,      2, "I048/161 Track Number"                                                },
        { 12, item_type_fixed,       0,      4, "I048/042 Calculated Position in Cartesian Coordinates"                },
        { 13, item_type_fixed,       0,      4, "I048/200 Calculated Track Velocity in Polar Coordinates"              },
        { 14, item_type_variable,    1,      1, "I048/170 Track Status"                                                },
        
        // FSPEC Byte 3
        { 15, item_type_fixed,       0,      4, "I048/210 Track Quality"                                               },
        { 16, item_type_variable,    1,      1, "I048/030 Warning/Error Conditions"                                    },
        { 17, item_type_fixed,       0,      2, "I048/080 Mode-3/A Code Confidence Indicator"                          },
        { 18, item_type_fixed,       0,      4, "I048/100 Mode-C Code and Confidence Indicator"                        },
        { 19, item_type_fixed,       0,      2, "I048/110 Height Measured by a 3D Radar"                               },
        { 20, item_type_compound,    0,      0, "I048/120 Radial Doppler Speed",                        &cat048_120_uap},
        { 21, item_type_fixed,       0,      2, "I048/230 Communications/ACAS Capability and Flight Status"            },
        
        // FSPEC Byte 4
        { 22, item_type_fixed,       0,      7, "I048/260 ACAS Resolution Advisory Report"                             },
        { 23, item_type_fixed,       0,      1, "I048/055 Mode-1 Code in Octal Representation"                         },
        { 24, item_type_fixed,       0,      2, "I048/050 Mode-2 Code in Octal Representation"                         },
        { 25, item_type_fixed,       0,      1, "I048/065 Mode-1 Code Confidence Indicator"                            },
        { 26, item_type_fixed,       0,      2, "I048/060 Mode-2 Code Confidence Indicator"                            },
        { 27, item_type_explicit,    0,      0, "SP Special Purpose Field"                                             },
        { 28, item_type_explicit,    0,      0, "RE Reserved Expansion Field",                          &ref::get_uap()},
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // Sub-UAP for I048/130 Radar Plot Characteristics (Compound)
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat048_130_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "SRL - SSR Plot Runlength"                                             },
        {  2, item_type_fixed,       0,      1, "SSR - Number of Correlated SSR Replies"                               },
        {  3, item_type_fixed,       0,      1, "SAM - Amplitude of Primary Plot"                                      },
        {  4, item_type_fixed,       0,      1, "PRL - Primary Plot Runlength"                                         },
        {  5, item_type_fixed,       0,      1, "PAM - Amplitude of Primary Plot"                                      },
        {  6, item_type_fixed,       0,      1, "RDP - Difference in Range"                                            },
        {  7, item_type_fixed,       0,      1, "SPI - Special Position Identification"                                },
    });

    uap cat048_130_uap(48, "CAT048 I048/130 " CAT048_VERSION ""_ct,  cat048_130_items.data(), cat048_130_items.size());

    // -------------------------------------------------------------------------------------------------------------- //
    // Sub-UAP for I048/120 Radial Doppler Speed (Compound)
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat048_120_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      2, "CAL - Calculated Doppler Speed"                                       },
        {  2, item_type_repetetive,  1,      6, "RDS - Raw Doppler Speed"                                              },
    });

    uap cat048_120_uap(48, "CAT048 I048/120 " CAT048_VERSION ""_ct,  cat048_120_items.data(), cat048_120_items.size());

    // -------------------------------------------------------------------------------------------------------------- //
    // CAT048 Type Getter Function
    //
    // Reads (parsed) I048/020 (Target Report Descriptor) to extract the type and return the corresponding enum entry.
    // -------------------------------------------------------------------------------------------------------------- //
    uap::type_getter_function cat048_type_getter_fn = [](const record* rec, const adam::buffer* buf) -> uint8_t
    {
        auto* trd_itm = rec->get_item(3);

        if (!trd_itm->is_populated())
            return 0xff;

        auto* data = trd_itm->get_data_as<const cat048::target_report_descriptor>(buf);

        if (!data)
            return 0xff;

        return data->msg_type;
    };

    uap& get_uap()
    {
        static uap cat048_uap
        (
            48,                             /**< CAT Number.          */
            "CAT048 " CAT048_VERSION ""_ct, /**< CAT Names.           */
            cat048_items.data(),            /**< Items Array start.   */
            cat048_items.size(),            /**< Items Array length.  */
            1,                              /**< SAC/SIC FRN.         */
            get_cat048_types(),             /**< Type Database.       */
            cat048_type_getter_fn           /**< Type Getter Function */
        );

        return cat048_uap;
    }
}
