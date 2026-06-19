#include "data/categories/062/cat062-uap.hpp"

namespace adam::modules::asterix::cat062::ref
{    
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for REF sub-UAPs
    extern uap cat062_ref_av3_uap;
    extern uap cat062_ref_moi_uap;
    extern uap cat062_ref_mti_uap;
    extern uap cat062_ref_g62_uap;

    // Sub-UAP for Reserved Expansion Field (REF) - Appendix A
    const auto cat062_ref_items = std::to_array<const field_spec>
    ({
        {  1, item_type_repetetive,  1,      5, "CST - Contributing Sensors with local tracknumber"                    },
        {  2, item_type_repetetive,  1,      3, "CSN - Contributing Sensors No local tracknumber"                      },
        {  3, item_type_fixed,       0,      4, "TVS - Calculated Track Velocity relative to System Reference Point"   },
        {  4, item_type_variable,    1,      1, "STS - Supplementary Track Status"                                     },
        {  5, item_type_compound,    0,      0, "V3 - ADS-B Version 3 Data",                        &cat062_ref_av3_uap},
        {  6, item_type_explicit,    0,      0, "MOI - Miscellaneous Operational Items",            &cat062_ref_moi_uap},
        {  7, item_type_explicit,    0,      0, "MTI - Miscellaneous Technical Items",              &cat062_ref_mti_uap},
        {  8, item_type_compound,    0,      0, "GEN62 - Generic Category 062 Data",                &cat062_ref_g62_uap}
    });

    uap cat062_ref_uap(62, "CAT062 REF 1.4"_ct, cat062_ref_items.data(), cat062_ref_items.size());

    // Sub-UAP for V3 (ADS-B Version 3 Data)
    const auto cat062_ref_v3_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "PS3 - Priority Status ADS-B Version 3"                                },
        {  2, item_type_fixed,       0,      3, "AS - Aircraft Status"                                                 },
        {  3, item_type_fixed,       0,      1, "UAS - UAS/RPAS Status"                                                },
        {  4, item_type_fixed,       0,      1, "CASS - Collision Avoidance System Status"                             }
    });

    uap cat062_ref_av3_uap(62, "CAT062 REF ADSB3 1.4"_ct, cat062_ref_v3_items.data(), cat062_ref_v3_items.size());

    // Sub-UAP for MOI (Miscellaneous Operational Items)
    const auto cat062_ref_moi_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "ATAD - Age of Target Address"                                         },
        {  2, item_type_fixed,       0,      1, "ATID - Age of Target Identification"                                  },
        {  3, item_type_fixed,       0,      1, "ADTNH - Age of Downlinked True North Heading"                         },
        {  4, item_type_fixed,       0,      1, "ADMNH - Age of Downlinked Magnetic North Heading"                     },
        {  5, item_type_fixed,       0,      1, "ADTNT - Age of Downlinked True North Track Angle"                     },
        {  6, item_type_fixed,       0,      1, "ADMNT - Age of Downlinked Magnetic North Track Angle"                 },
        {  7, item_type_fixed,       0,      1, "ASEPMIN - Age of the System Calculated Separation Minimum"            },
        {  8, item_type_fixed,       0,      1, "AM5I - Age of Mode 5 Interrogation"                                   },
        {  9, item_type_fixed,       0,      1, "AM5L2S - Age of Mode 5 Level 2 Squitter"                              },
        { 10, item_type_repetetive,  1,      2, "ABDS - BDS Register Data Age"                                         },
        { 11, item_type_repetetive,  1,      1, "MPID - Movement Plan ID"                                              },
        { 12, item_type_fixed,       0,      4, "LS - Leading Source"                                                  },
        { 13, item_type_fixed,       0,      4, "LSQI - Leading Source Quality Indication"                             },
        { 14, item_type_fixed,       0,      2, "DTNH - Downlinked True North Heading"                                 },
        { 15, item_type_fixed,       0,      2, "DMNH - Downlinked Magnetic North Heading"                             },
        { 16, item_type_fixed,       0,      2, "DTNT - Downlinked True North Track Angle"                             },
        { 17, item_type_fixed,       0,      2, "DMNT - Downlinked Magnetic North Track Angle"                         },
        { 18, item_type_fixed,       0,      2, "TBP - Tracker Barometric Pressure"                                    },
        { 19, item_type_fixed,       0,      2, "ALTQCMFL - Altitude from QNH Corrected Measured Flight Level"         },
        { 20, item_type_fixed,       0,      2, "CTBA - Calculated Track Barometric Altitude"                          },
        { 21, item_type_repetetive,  1,      8, "INPS - Input to SDPS with Service ID and Local Track Number"          },
        { 22, item_type_fixed,       0,      2, "FPVHR - Fractional Part of Velocity (High Resolution)"                },
        { 23, item_type_fixed,       0,      7, "SCT - SUC Correlation Text"                                           },
        { 24, item_type_fixed,       0,      2, "SCSM - System Calculated Separation Minimum"                          },
        { 25, item_type_variable,    2,      2, "TCAT - Target Category"                                               }
    });

    uap cat062_ref_moi_uap(62, "CAT062 REF MOI 1.4"_ct, cat062_ref_moi_items.data(), cat062_ref_moi_items.size());

    // Sub-UAP for MTI (Miscellaneous Technical Items)
    const auto cat062_ref_mti_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      4, "DATE - Date of Track Information"                                     },
        {  2, item_type_fixed,       0,      2, "TENTU - Time to Expected Next Track Update"                           },
        {  3, item_type_fixed,       0,      4, "IPMC - Inverse Point Scale Factor and Meridian Convergence"           },
        {  4, item_type_repetetive,  1,      2, "IMP - IMM Model Probability"                                          },
        {  5, item_type_repetetive,  1,     15, "IST - IMM State"                                                      },
        {  6, item_type_fixed,       0,      3, "TTT - Time of Track Termination"                                      },
        {  7, item_type_fixed,       0,      2, "EXM3A - Expired Mode 3/A Code"                                        },
        {  8, item_type_fixed,       0,      3, "EXADDR - Expired Target Address"                                      },
        {  9, item_type_fixed,       0,      6, "EXTID - Expired Target Identification"                                }
    });

    uap cat062_ref_mti_uap(62, "CAT062 REF MTI 1.4"_ct, cat062_ref_mti_items.data(), cat062_ref_mti_items.size());

    // Sub-UAP for GEN62 (Generic Category 062 Data) - Placeholder
    const auto cat062_ref_gen62_items = std::to_array<const field_spec>
    ({
        {  1, item_type_variable,    0,      0, "Spare/Placeholder"                                                    }
    });

    uap cat062_ref_g62_uap(62, "CAT062 REF GEN62 1.4"_ct, cat062_ref_gen62_items.data(), cat062_ref_gen62_items.size());

    uap& get_uap()
    {
        return cat062_ref_uap;
    }
}
