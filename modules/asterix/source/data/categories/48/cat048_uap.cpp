#include "data/categories/48/cat048_uap.hpp"

namespace adam::modules::asterix
{
    namespace 
    {
        // Forward declarations for sub-UAPs so the main UAPs can sit at the top
        extern uap cat048_130_uap;
        extern uap cat048_120_uap;
        extern uap cat048_ref_uap;
        
        // Main UAP for CAT048 Edition 1.32 (July 2024)
        const field_spec cat048_items[] = 
        {
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
            { 28, item_type_explicit,    0,      0, "RE Reserved Expansion Field",                          &cat048_ref_uap}
        };

        uap cat048_uap(48, cat048_items, sizeof(cat048_items) / sizeof(field_spec));

        // Sub-UAP for I048/130 Radar Plot Characteristics (Compound)
        const field_spec cat048_130_items[] =
        {
            {  1, item_type_fixed,       0,      1, "SRL - SSR Plot Runlength"                                             },
            {  2, item_type_fixed,       0,      1, "SSR - Number of Correlated SSR Replies"                               },
            {  3, item_type_fixed,       0,      1, "SAM - Amplitude of Primary Plot"                                      },
            {  4, item_type_fixed,       0,      1, "PRL - Primary Plot Runlength"                                         },
            {  5, item_type_fixed,       0,      1, "PAM - Amplitude of Primary Plot"                                      },
            {  6, item_type_fixed,       0,      1, "RDP - Difference in Range"                                            },
            {  7, item_type_fixed,       0,      1, "SPI - Special Position Identification"                                }
        };

        uap cat048_130_uap(0, cat048_130_items, sizeof(cat048_130_items) / sizeof(field_spec));

        // Sub-UAP for I048/120 Radial Doppler Speed (Compound)
        const field_spec cat048_120_items[] =
        {
            {  1, item_type_fixed,       0,      2, "CAL - Calculated Doppler Speed"                                       },
            {  2, item_type_repetetive,  1,      6, "RDS - Raw Doppler Speed"                                              }
        };

        uap cat048_120_uap(0, cat048_120_items, sizeof(cat048_120_items) / sizeof(field_spec));

        // Forward declarations for REF sub-UAPs
        extern uap cat048_ref_md5_uap;
        extern uap cat048_ref_m5n_uap;
        extern uap cat048_ref_rpc_uap;
        extern uap cat048_ref_rtc_uap;
        extern uap cat048_ref_cpc_uap;
        extern uap cat048_ref_g48_uap;

        // Sub-UAP for Reserved Expansion Field (REF) - Appendix A
        const field_spec cat048_ref_items[] =
        {
            {  1, item_type_compound,    0,      0, "MD5 - Mode 5 Reports",                             &cat048_ref_md5_uap},
            {  2, item_type_compound,    0,      0, "M5N - Mode 5 Reports, New Format",                 &cat048_ref_m5n_uap},
            {  3, item_type_variable,    1,      1, "M4E - Extended Mode 4 Report"                                         },
            {  4, item_type_compound,    0,      0, "RPC - Radar Plot Characteristics",                 &cat048_ref_rpc_uap},
            {  5, item_type_fixed,       0,      3, "ERR - Extended Range Report"                                          },
            {  6, item_type_compound,    0,      0, "RTC - Radar Track Characteristics",                &cat048_ref_rtc_uap},
            {  7, item_type_compound,    0,      0, "CPC - Common and Plot Characteristics",            &cat048_ref_cpc_uap},
            {  8, item_type_compound,    0,      0, "GEN48 - Generic Category 048 Data",                &cat048_ref_g48_uap}
        };

        uap cat048_ref_uap(0, cat048_ref_items, sizeof(cat048_ref_items) / sizeof(field_spec));

        // Sub-UAP for MD5 (Mode 5 Reports)
        const field_spec cat048_ref_md5_items[] =
        {
            {  1, item_type_fixed,       0,      1, "Mode 5 Summary"                                                       },
            {  2, item_type_fixed,       0,      4, "Mode 5 PIN / National Origin / Mission Code"                          },
            {  3, item_type_fixed,       0,      6, "Mode 5 Reported Position"                                             },
            {  4, item_type_fixed,       0,      2, "Mode 5 GNSS-derived Altitude"                                         },
            {  5, item_type_fixed,       0,      2, "Extended Mode 1 Code in Octal Representation"                         },
            {  6, item_type_fixed,       0,      1, "Time Offset for POS and GA"                                           },
            {  7, item_type_fixed,       0,      1, "X Pulse Presence"                                                     }
        };

        uap cat048_ref_md5_uap(0, cat048_ref_md5_items, sizeof(cat048_ref_md5_items) / sizeof(field_spec));

        // Sub-UAP for M5N (Mode 5 Reports, New Format)
        const field_spec cat048_ref_m5n_items[] =
        {
            {  1, item_type_fixed,       0,      1, "Mode 5 Summary"                                                       },
            {  2, item_type_fixed,       0,      4, "Mode 5 PIN / National Origin"                                         },
            {  3, item_type_fixed,       0,      6, "Mode 5 Reported Position"                                             },
            {  4, item_type_fixed,       0,      2, "Mode 5 GNSS-derived Altitude"                                         },
            {  5, item_type_fixed,       0,      2, "Extended Mode 1 Code in Octal Representation"                         },
            {  6, item_type_fixed,       0,      1, "Time Offset for POS and GA"                                           },
            {  7, item_type_fixed,       0,      1, "X Pulse Presence"                                                     },
            {  8, item_type_fixed,       0,      1, "Figure of Merit"                                                      }
        };

        uap cat048_ref_m5n_uap(0, cat048_ref_m5n_items, sizeof(cat048_ref_m5n_items) / sizeof(field_spec));

        // Sub-UAP for RPC (Radar Plot Characteristics)
        const field_spec cat048_ref_rpc_items[] =
        {
            {  1, item_type_fixed,       0,      1, "Score"                                                                },
            {  2, item_type_fixed,       0,      2, "Signal/Clutter Ratio"                                                 },
            {  3, item_type_fixed,       0,      2, "Range Width"                                                          },
            {  4, item_type_fixed,       0,      2, "Ambiguous Range"                                                      }
        };

        uap cat048_ref_rpc_uap(0, cat048_ref_rpc_items, sizeof(cat048_ref_rpc_items) / sizeof(field_spec));

        // Sub-UAP for RTC (Radar Track Characteristics)
        const field_spec cat048_ref_rtc_items[] =
        {
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
        };

        uap cat048_ref_rtc_uap(0, cat048_ref_rtc_items, sizeof(cat048_ref_rtc_items) / sizeof(field_spec));

        // Sub-UAP for CPC (Common and Plot Characteristics)
        const field_spec cat048_ref_cpc_items[] =
        {
            {  1, item_type_fixed,       0,      2, "Plot Number"                                                          },
            {  2, item_type_repetetive,  1,      3, "Replies/Plot Link"                                                    },
            {  3, item_type_fixed,       0,      1, "Scan Number"                                                          },
            {  4, item_type_fixed,       0,      4, "Date"                                                                 }
        };

        uap cat048_ref_cpc_uap(0, cat048_ref_cpc_items, sizeof(cat048_ref_cpc_items) / sizeof(field_spec));

        // Sub-UAP for GEN48 (Generic Category 048 Data)
        const field_spec cat048_ref_gen48_items[] =
        {
            {  1, item_type_fixed,       0,      2, "Alternative Mode 2 Code"                                              },
            {  2, item_type_fixed,       0,      2, "Alternative Mode 3/A"                                                 },
            {  3, item_type_fixed,       0,      2, "Alternative Flight Level"                                             },
            {  4, item_type_fixed,       0,      2, "Radar Cross Section in dBm2"                                          },
            {  5, item_type_fixed,       0,      4, "Radar Cross Section in m2"                                            }
        };

        uap cat048_ref_g48_uap(0, cat048_ref_gen48_items, sizeof(cat048_ref_gen48_items) / sizeof(field_spec));
    }

    uap& get_cat048_uap()
    {
        return cat048_uap;
    }
}
