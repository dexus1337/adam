#include "data/categories/048/cat048-uap-ref.hpp"

namespace adam::modules::asterix::cat048::ref
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for REF sub-UAPs
    extern uap cat048_ref_md5_uap;
    extern uap cat048_ref_m5n_uap;
    extern uap cat048_ref_rpc_uap;
    extern uap cat048_ref_rtc_uap;
    extern uap cat048_ref_cpc_uap;
    extern uap cat048_ref_g48_uap;

    // Sub-UAP for Reserved Expansion Field (REF) - Appendix A
    const auto cat048_ref_items = std::to_array<const field_spec>
    ({
        {  1, item_type_compound,    0,      0, "MD5 - Mode 5 Reports",                             &cat048_ref_md5_uap},
        {  2, item_type_compound,    0,      0, "M5N - Mode 5 Reports, New Format",                 &cat048_ref_m5n_uap},
        {  3, item_type_variable,    1,      1, "M4E - Extended Mode 4 Report"                                         },
        {  4, item_type_compound,    0,      0, "RPC - Radar Plot Characteristics",                 &cat048_ref_rpc_uap},
        {  5, item_type_fixed,       0,      3, "ERR - Extended Range Report"                                          },
        {  6, item_type_compound,    0,      0, "RTC - Radar Track Characteristics",                &cat048_ref_rtc_uap},
        {  7, item_type_compound,    0,      0, "CPC - Common and Plot Characteristics",            &cat048_ref_cpc_uap},
        {  8, item_type_compound,    0,      0, "GEN48 - Generic Category 048 Data",                &cat048_ref_g48_uap}
    });

    uap cat048_ref_uap(48, "CAT048 REF 1.13"_ct, cat048_ref_items.data(), cat048_ref_items.size());

    // Sub-UAP for MD5 (Mode 5 Reports)
    const auto cat048_ref_md5_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "Mode 5 Summary"                                                       },
        {  2, item_type_fixed,       0,      4, "Mode 5 PIN / National Origin / Mission Code"                          },
        {  3, item_type_fixed,       0,      6, "Mode 5 Reported Position"                                             },
        {  4, item_type_fixed,       0,      2, "Mode 5 GNSS-derived Altitude"                                         },
        {  5, item_type_fixed,       0,      2, "Extended Mode 1 Code in Octal Representation"                         },
        {  6, item_type_fixed,       0,      1, "Time Offset for POS and GA"                                           },
        {  7, item_type_fixed,       0,      1, "X Pulse Presence"                                                     }
    });

    uap cat048_ref_md5_uap(48, "CAT048 REF MD5 1.13"_ct, cat048_ref_md5_items.data(), cat048_ref_md5_items.size());

    // Sub-UAP for M5N (Mode 5 Reports, New Format)
    const auto cat048_ref_m5n_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "Mode 5 Summary"                                                       },
        {  2, item_type_fixed,       0,      4, "Mode 5 PIN / National Origin"                                         },
        {  3, item_type_fixed,       0,      6, "Mode 5 Reported Position"                                             },
        {  4, item_type_fixed,       0,      2, "Mode 5 GNSS-derived Altitude"                                         },
        {  5, item_type_fixed,       0,      2, "Extended Mode 1 Code in Octal Representation"                         },
        {  6, item_type_fixed,       0,      1, "Time Offset for POS and GA"                                           },
        {  7, item_type_fixed,       0,      1, "X Pulse Presence"                                                     },
        {  8, item_type_fixed,       0,      1, "Figure of Merit"                                                      }
    });

    uap cat048_ref_m5n_uap(48, "CAT048 REF M5N 1.13"_ct, cat048_ref_m5n_items.data(), cat048_ref_m5n_items.size());

    // Sub-UAP for RPC (Radar Plot Characteristics)
    const auto cat048_ref_rpc_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "Score"                                                                },
        {  2, item_type_fixed,       0,      2, "Signal/Clutter Ratio"                                                 },
        {  3, item_type_fixed,       0,      2, "Range Width"                                                          },
        {  4, item_type_fixed,       0,      2, "Ambiguous Range"                                                      }
    });

    uap cat048_ref_rpc_uap(48, "CAT048 REF RPC 1.13"_ct, cat048_ref_rpc_items.data(), cat048_ref_rpc_items.size());

    // Sub-UAP for RTC (Radar Track Characteristics)
    const auto cat048_ref_rtc_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      3, "Plot/Track Link"                                                      },
        {  2, item_type_repetetive,  1,      2, "ADS-B/Track Link"                                                     },
        {  3, item_type_fixed,       0,      1, "Turn State"                                                           },
        {  4, item_type_fixed,       0,     22, "Next Predicted Position"                                              },
        {  5, item_type_repetetive,  1,      1, "Data Link Characteristics"                                            },
        {  6, item_type_fixed,       0,      2, "Lockout Characteristics"                                              },
        {  7, item_type_fixed,       0,      6, "Transition Codes"                                                     },
        {  8, item_type_fixed,       0,      4, "Track Lifecycle"                                                      },
        {  9, item_type_repetetive,  1,      8, "Adjacent Sensor Information"                                          },
        { 10, item_type_fixed,       0,      1, "Track Extrapolation Source"                                           },
        { 11, item_type_fixed,       0,      1, "Identity Requested"                                                   }
    });

    uap cat048_ref_rtc_uap(48, "CAT048 REF RTC 1.13"_ct, cat048_ref_rtc_items.data(), cat048_ref_rtc_items.size());

    // Sub-UAP for CPC (Common and Plot Characteristics)
    const auto cat048_ref_cpc_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      2, "Plot Number"                                                          },
        {  2, item_type_repetetive,  1,      3, "Replies/Plot Link"                                                    },
        {  3, item_type_fixed,       0,      1, "Scan Number"                                                          },
        {  4, item_type_fixed,       0,      4, "Date"                                                                 }
    });

    uap cat048_ref_cpc_uap(48, "CAT048 REF CPC 1.13"_ct, cat048_ref_cpc_items.data(), cat048_ref_cpc_items.size());

    // Sub-UAP for GEN48 (Generic Category 048 Data)
    const auto cat048_ref_gen48_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      2, "Alternative Mode 2 Code"                                              },
        {  2, item_type_fixed,       0,      2, "Alternative Mode 3/A"                                                 },
        {  3, item_type_fixed,       0,      2, "Alternative Flight Level"                                             },
        {  4, item_type_fixed,       0,      2, "Radar Cross Section in dBm2"                                          },
        {  5, item_type_fixed,       0,      4, "Radar Cross Section in m2"                                            }
    });

    uap cat048_ref_g48_uap(48, "CAT048 REF GEN48 1.13"_ct, cat048_ref_gen48_items.data(), cat048_ref_gen48_items.size());

    uap& get_uap()
    {
        return cat048_ref_uap;
    }
}
