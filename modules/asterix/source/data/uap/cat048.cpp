#include "data/uap/cat048.hpp"

namespace adam::modules::asterix::data::uap
{
    namespace 
    {
        // Forward declarations for sub-UAPs so the main UAP can sit at the top
        extern asterix_uap cat048_130_uap;
        extern asterix_uap cat048_120_uap;
        extern asterix_uap cat048_ref_uap;

        // Full CAT048 definition based on Eurocontrol ASTERIX standard:
        // Specification: Edition 1.32 (July 2, 2024)
        // Appendix A (Reserved Expansion Field): Edition 1.13 (December 1, 2024)
        //
        // Note on REF (FRN 25): As per Appendix A, the REF begins with a 1-octet Length Indicator 
        // specifying the total length, followed by an Items Indicator (FSPEC) and sub-items 
        // (Mode 5 Data, Extended Mode 1 Code, ERR, M4E). Our parser handles it natively as an 
        // EXPLICIT item, which perfectly bounds and extracts it using the leading length indicator octet.
        // 
        // The "frn" field is the Field Reference Number, which corresponds to the 1-based bit index in the FSPEC.
        const asterix_field_spec cat048_items[] = 
        {
            // FSPEC Byte 1
            {  1, asterix_item_type::FIXED,       2, "I048/010 Data Source Identifier"                                      },
            {  2, asterix_item_type::FIXED,       3, "I048/140 Time of Day"                                                 },
            {  3, asterix_item_type::VARIABLE,    0, "I048/020 Target Report Descriptor"                                    },
            {  4, asterix_item_type::FIXED,       4, "I048/040 Measured Position in Slant Polar Coordinates"                },
            {  5, asterix_item_type::FIXED,       2, "I048/070 Mode-3/A Code in Octal Representation"                       },
            {  6, asterix_item_type::FIXED,       2, "I048/090 Flight Level in Binary Representation"                       },
            {  7, asterix_item_type::COMPOUND,    0, "I048/130 Radar Plot Characteristics",                  &cat048_130_uap},
            
            // FSPEC Byte 2
            {  8, asterix_item_type::FIXED,       3, "I048/220 Aircraft Address"                                            },
            {  9, asterix_item_type::FIXED,       6, "I048/240 Aircraft Identification"                                     },
            { 10, asterix_item_type::REPETITIVE,  8, "I048/250 BDS Register Data"                                           },
            { 11, asterix_item_type::FIXED,       2, "I048/161 Track Number"                                                },
            { 12, asterix_item_type::FIXED,       4, "I048/042 Calculated Position in Cartesian Coordinates"                },
            { 13, asterix_item_type::FIXED,       4, "I048/200 Calculated Track Velocity in Polar Coordinates"              },
            { 14, asterix_item_type::VARIABLE,    0, "I048/170 Track Status"                                                },
            
            // FSPEC Byte 3
            { 15, asterix_item_type::FIXED,       4, "I048/210 Track Quality"                                               },
            { 16, asterix_item_type::VARIABLE,    0, "I048/030 Warning/Error Conditions"                                    },
            { 17, asterix_item_type::FIXED,       2, "I048/080 Mode-3/A Code Confidence Indicator"                          },
            { 18, asterix_item_type::FIXED,       4, "I048/100 Mode-C Code and Confidence Indicator"                        },
            { 19, asterix_item_type::FIXED,       2, "I048/110 Height Measured by a 3D Radar"                               },
            { 20, asterix_item_type::COMPOUND,    0, "I048/120 Radial Doppler Speed",                        &cat048_120_uap},
            { 21, asterix_item_type::FIXED,       2, "I048/230 Communications/ACAS Capability and Flight Status"            },
            
            // FSPEC Byte 4
            { 22, asterix_item_type::FIXED,       7, "I048/260 ACAS Resolution Advisory Report"                             },
            { 23, asterix_item_type::FIXED,       1, "I048/055 Mode-1 Code in Octal Representation"                         },
            { 24, asterix_item_type::FIXED,       2, "I048/050 Mode-2 Code in Octal Representation"                         },
            { 25, asterix_item_type::EXPLICIT,    0, "RE Reserved Expansion Field",                          &cat048_ref_uap},
            { 26, asterix_item_type::EXPLICIT,    0, "SP Special Purpose Field"                                             }
        };

        asterix_uap cat048_uap(48, cat048_items, sizeof(cat048_items) / sizeof(cat048_items[0]));

        // Sub-UAP for I048/130 Radar Plot Characteristics (Compound)
        const asterix_field_spec cat048_130_items[] =
        {
            {  1, asterix_item_type::FIXED,       1, "SRL - SSR Plot Runlength"                                             },
            {  2, asterix_item_type::FIXED,       1, "SSR - Number of Correlated SSR Replies"                               },
            {  3, asterix_item_type::FIXED,       1, "SAM - Amplitude of Primary Plot"                                      },
            {  4, asterix_item_type::FIXED,       1, "PRL - Primary Plot Runlength"                                         },
            {  5, asterix_item_type::FIXED,       1, "PAM - Amplitude of Primary Plot"                                      },
            {  6, asterix_item_type::FIXED,       1, "RDP - Difference in Range"                                            },
            {  7, asterix_item_type::FIXED,       1, "SPI - Special Position Identification"                                }
        };

        asterix_uap cat048_130_uap(130, cat048_130_items, sizeof(cat048_130_items) / sizeof(cat048_130_items[0]));

        // Sub-UAP for I048/120 Radial Doppler Speed (Compound)
        const asterix_field_spec cat048_120_items[] =
        {
            {  1, asterix_item_type::FIXED,       2, "CAL - Calculated Doppler Speed"                                       },
            {  2, asterix_item_type::REPETITIVE,  6, "RDS - Raw Doppler Speed"                                              }
        };

        asterix_uap cat048_120_uap(120, cat048_120_items, sizeof(cat048_120_items) / sizeof(cat048_120_items[0]));

        // Sub-UAP for MD5 (Mode 5 Reports)
        const asterix_field_spec cat048_ref_md5_items[] =
        {
            {  1, asterix_item_type::FIXED,       1, "Mode 5 Summary"                                                       },
            {  2, asterix_item_type::FIXED,       4, "Mode 5 PIN / National Origin / Mission Code"                          },
            {  3, asterix_item_type::FIXED,       6, "Mode 5 Reported Position"                                             },
            {  4, asterix_item_type::FIXED,       2, "Mode 5 GNSS-derived Altitude"                                         },
            {  5, asterix_item_type::FIXED,       2, "Extended Mode 1 Code in Octal Representation"                         },
            {  6, asterix_item_type::FIXED,       1, "Time Offset for POS and GA"                                           },
            {  7, asterix_item_type::FIXED,       1, "X Pulse Presence"                                                     }
        };

        asterix_uap cat048_ref_md5_uap(1, cat048_ref_md5_items, sizeof(cat048_ref_md5_items) / sizeof(cat048_ref_md5_items[0]));

        // Sub-UAP for M5N (Mode 5 Reports, New Format)
        const asterix_field_spec cat048_ref_m5n_items[] =
        {
            {  1, asterix_item_type::FIXED,       1, "Mode 5 Summary"                                                       },
            {  2, asterix_item_type::FIXED,       4, "Mode 5 PIN / National Origin"                                         },
            {  3, asterix_item_type::FIXED,       6, "Mode 5 Reported Position"                                             },
            {  4, asterix_item_type::FIXED,       2, "Mode 5 GNSS-derived Altitude"                                         },
            {  5, asterix_item_type::FIXED,       2, "Extended Mode 1 Code in Octal Representation"                         },
            {  6, asterix_item_type::FIXED,       1, "Time Offset for POS and GA"                                           },
            {  7, asterix_item_type::FIXED,       1, "X Pulse Presence"                                                     },
            {  8, asterix_item_type::FIXED,       1, "Figure of Merit"                                                      }
        };

        asterix_uap cat048_ref_m5n_uap(2, cat048_ref_m5n_items, sizeof(cat048_ref_m5n_items) / sizeof(cat048_ref_m5n_items[0]));

        // Sub-UAP for RPC (Radar Plot Characteristics)
        const asterix_field_spec cat048_ref_rpc_items[] =
        {
            {  1, asterix_item_type::FIXED,       1, "Score"                                                                },
            {  2, asterix_item_type::FIXED,       2, "Signal/Clutter Ratio"                                                 },
            {  3, asterix_item_type::FIXED,       2, "Range Width"                                                          },
            {  4, asterix_item_type::FIXED,       2, "Ambiguous Range"                                                      }
        };

        asterix_uap cat048_ref_rpc_uap(4, cat048_ref_rpc_items, sizeof(cat048_ref_rpc_items) / sizeof(cat048_ref_rpc_items[0]));

        // Sub-UAP for RTC (Radar Track Characteristics)
        const asterix_field_spec cat048_ref_rtc_items[] =
        {
            {  1, asterix_item_type::FIXED,       3, "Plot/Track Link"                                                      },
            {  2, asterix_item_type::REPETITIVE,  2, "ADS-B/Track Link"                                                     },
            {  3, asterix_item_type::FIXED,       1, "Turn State"                                                           },
            {  4, asterix_item_type::FIXED,      22, "Next Predicted Position"                                              },
            {  5, asterix_item_type::REPETITIVE,  1, "Data Link Characteristics"                                            },
            {  6, asterix_item_type::FIXED,       2, "Lockout Characteristics"                                              },
            {  7, asterix_item_type::FIXED,       6, "Transition Codes"                                                     },
            {  8, asterix_item_type::FIXED,       4, "Track Lifecycle"                                                      },
            {  9, asterix_item_type::REPETITIVE,  8, "Adjacent Sensor Information"                                          },
            { 10, asterix_item_type::FIXED,       1, "Track Extrapolation Source"                                           },
            { 11, asterix_item_type::FIXED,       1, "Identity Requested"                                                   }
        };

        asterix_uap cat048_ref_rtc_uap(6, cat048_ref_rtc_items, sizeof(cat048_ref_rtc_items) / sizeof(cat048_ref_rtc_items[0]));

        // Sub-UAP for CPC (Common and Plot Characteristics)
        const asterix_field_spec cat048_ref_cpc_items[] =
        {
            {  1, asterix_item_type::FIXED,       2, "Plot Number"                                                          },
            {  2, asterix_item_type::REPETITIVE,  3, "Replies/Plot Link"                                                    },
            {  3, asterix_item_type::FIXED,       1, "Scan Number"                                                          },
            {  4, asterix_item_type::FIXED,       4, "Date"                                                                 }
        };

        asterix_uap cat048_ref_cpc_uap(7, cat048_ref_cpc_items, sizeof(cat048_ref_cpc_items) / sizeof(cat048_ref_cpc_items[0]));

        // Sub-UAP for GEN48 (Generic Category 048 Data)
        const asterix_field_spec cat048_ref_gen48_items[] =
        {
            {  1, asterix_item_type::FIXED,       2, "Alternative Mode 2 Code"                                              },
            {  2, asterix_item_type::FIXED,       2, "Alternative Mode 3/A"                                                 },
            {  3, asterix_item_type::FIXED,       2, "Alternative Flight Level"                                             },
            {  4, asterix_item_type::FIXED,       2, "Radar Cross Section in dBm2"                                          },
            {  5, asterix_item_type::FIXED,       4, "Radar Cross Section in m2"                                            }
        };

        asterix_uap cat048_ref_gen48_uap(8, cat048_ref_gen48_items, sizeof(cat048_ref_gen48_items) / sizeof(cat048_ref_gen48_items[0]));

        // Sub-UAP for Reserved Expansion Field (REF) - Appendix A
        const asterix_field_spec cat048_ref_items[] =
        {
            {  1, asterix_item_type::COMPOUND,    0, "MD5 - Mode 5 Reports",                               &cat048_ref_md5_uap},
            {  2, asterix_item_type::COMPOUND,    0, "M5N - Mode 5 Reports, New Format",                   &cat048_ref_m5n_uap},
            {  3, asterix_item_type::VARIABLE,    0, "M4E - Extended Mode 4 Report"                                           },
            {  4, asterix_item_type::COMPOUND,    0, "RPC - Radar Plot Characteristics",                   &cat048_ref_rpc_uap},
            {  5, asterix_item_type::FIXED,       3, "ERR - Extended Range Report"                                            },
            {  6, asterix_item_type::COMPOUND,    0, "RTC - Radar Track Characteristics",                  &cat048_ref_rtc_uap},
            {  7, asterix_item_type::COMPOUND,    0, "CPC - Common and Plot Characteristics",              &cat048_ref_cpc_uap},
            {  8, asterix_item_type::COMPOUND,    0, "GEN48 - Generic Category 048 Data",                  &cat048_ref_gen48_uap}
        };

        asterix_uap cat048_ref_uap(25, cat048_ref_items, sizeof(cat048_ref_items) / sizeof(cat048_ref_items[0]));
    }

    asterix_uap& get_cat048_uap()
    {
        return cat048_uap;
    }
}
