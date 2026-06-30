#include "data/categories/048/cat048-uap.hpp"

#include "data/categories/048/cat048-uap-ref.hpp"

namespace adam::modules::asterix::cat048
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for sub-UAPs so the main UAPs can sit at the top
    extern uap cat048_130_uap;
    extern uap cat048_120_uap;

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

    uap cat048_uap(48, "CAT048 " CAT048_VERSION ""_ct, cat048_items.data(), cat048_items.size());

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

    uap& get_uap()
    {
        return cat048_uap;
    }
}
