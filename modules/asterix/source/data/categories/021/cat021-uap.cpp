#include "data/categories/021/cat021-uap.hpp"

#include <array>

namespace adam::modules::asterix::cat021
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for CAT021 sub-UAPs
    extern uap cat021_110_uap;
    extern uap cat021_220_uap;
    extern uap cat021_295_uap;

    // -------------------------------------------------------------------------------------------------------------- //
    // Standard UAP for ADS-B Target Reports
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat021_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I021/010 Data Source Identification"                                  },
        {  2, item_type_variable,    1,      1, "I021/040 Target Report Descriptor"                                    },
        {  3, item_type_fixed,       0,      2, "I021/161 Track Number"                                                },
        {  4, item_type_fixed,       0,      1, "I021/015 Service Identification"                                      },
        {  5, item_type_fixed,       0,      3, "I021/071 Time of Applicability for Position"                          },
        {  6, item_type_fixed,       0,      6, "I021/130 Position in WGS-84 co-ordinates"                             },
        {  7, item_type_fixed,       0,      8, "I021/131 Position in WGS-84 co-ordinates, high res."                  },

        // FSPEC Byte 2
        {  8, item_type_fixed,       0,      3, "I021/072 Time of Applicability for Velocity"                          },
        {  9, item_type_fixed,       0,      2, "I021/150 Air Speed"                                                   },
        { 10, item_type_fixed,       0,      2, "I021/151 True Air Speed"                                              },
        { 11, item_type_fixed,       0,      3, "I021/080 Target Address"                                              },
        { 12, item_type_fixed,       0,      3, "I021/073 Time of Message Reception of Position"                       },
        { 13, item_type_fixed,       0,      4, "I021/074 Time of Message Reception of Position-High Precision"        },
        { 14, item_type_fixed,       0,      3, "I021/075 Time of Message Reception of Velocity"                       },

        // FSPEC Byte 3
        { 15, item_type_fixed,       0,      4, "I021/076 Time of Message Reception of Velocity-High Precision"        },
        { 16, item_type_fixed,       0,      2, "I021/140 Geometric Height"                                            },
        { 17, item_type_variable,    1,      1, "I021/090 Quality Indicators"                                          },
        { 18, item_type_fixed,       0,      1, "I021/210 MOPS Version"                                                },
        { 19, item_type_fixed,       0,      2, "I021/070 Mode 3/A Code"                                               },
        { 20, item_type_fixed,       0,      2, "I021/230 Roll Angle"                                                  },
        { 21, item_type_fixed,       0,      2, "I021/145 Flight Level"                                                },

        // FSPEC Byte 4
        { 22, item_type_fixed,       0,      2, "I021/152 Magnetic Heading"                                            },
        { 23, item_type_fixed,       0,      1, "I021/200 Target Status"                                               },
        { 24, item_type_fixed,       0,      2, "I021/155 Barometric Vertical Rate"                                    },
        { 25, item_type_fixed,       0,      2, "I021/157 Geometric Vertical Rate"                                     },
        { 26, item_type_fixed,       0,      4, "I021/160 Airborne Ground Vector"                                      },
        { 27, item_type_fixed,       0,      2, "I021/165 Track Angle Rate"                                            },
        { 28, item_type_fixed,       0,      3, "I021/077 Time of Report Transmission"                                 },

        // FSPEC Byte 5
        { 29, item_type_fixed,       0,      6, "I021/170 Target Identification"                                       },
        { 30, item_type_fixed,       0,      1, "I021/020 Emitter Category"                                            },
        { 31, item_type_compound,    0,      0, "I021/220 Met Information",                             &cat021_220_uap},
        { 32, item_type_fixed,       0,      2, "I021/146 Selected Altitude"                                           },
        { 33, item_type_fixed,       0,      2, "I021/148 Final State Selected Altitude"                               },
        { 34, item_type_compound,    0,      0, "I021/110 Trajectory Intent",                           &cat021_110_uap},
        { 35, item_type_fixed,       0,      1, "I021/016 Service Management"                                          },

        // FSPEC Byte 6
        { 36, item_type_fixed,       0,      1, "I021/008 Aircraft Operational Status"                                 },
        { 37, item_type_variable,    1,      1, "I021/271 Surface Capabilities and Characteristics"                    },
        { 38, item_type_fixed,       0,      1, "I021/132 Message Amplitude"                                           },
        { 39, item_type_repetetive,  1,      8, "I021/250 BDS Register Data"                                           },
        { 40, item_type_fixed,       0,      7, "I021/260 ACAS Resolution Advisory Report"                             },
        { 41, item_type_fixed,       0,      1, "I021/400 Receiver ID"                                                 },
        { 42, item_type_compound,    0,      0, "I021/295 Data Ages",                                   &cat021_295_uap},

        // FSPEC Byte 7
        // FRNs 43 to 47 are Not Used
        { 48, item_type_explicit,    0,      0, "RE Reserved Expansion Field"                                          },
        { 49, item_type_explicit,    0,      0, "SP Special Purpose Field"                                             },
    });

    uap cat021_uap(21, "CAT021 " CAT021_VERSION ""_ct, cat021_items.data(), cat021_items.size());

    // -------------------------------------------------------------------------------------------------------------- //
    // Sub-UAP for I021/110 Trajectory Intent (Compound)
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat021_110_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "TIS - Trajectory Intent Status"                                       },
        {  2, item_type_repetetive,  1,     15, "TID - Trajectory Intent Data"                                         },
    });

    uap cat021_110_uap(21, "CAT021 I021/110 " CAT021_VERSION ""_ct, cat021_110_items.data(), cat021_110_items.size());

    // -------------------------------------------------------------------------------------------------------------- //
    // Sub-UAP for I021/220 Met Information (Compound)
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat021_220_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      2, "WS - Wind Speed"                                                      },
        {  2, item_type_fixed,       0,      2, "WD - Wind Direction"                                                  },
        {  3, item_type_fixed,       0,      2, "TMP - Temperature"                                                    },
        {  4, item_type_fixed,       0,      1, "TRB - Turbulence"                                                     },
    });

    uap cat021_220_uap(21, "CAT021 I021/220 " CAT021_VERSION ""_ct, cat021_220_items.data(), cat021_220_items.size());

    // -------------------------------------------------------------------------------------------------------------- //
    // Sub-UAP for I021/295 Data Ages (Compound)
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat021_295_items = std::to_array<const field_spec>
    ({
        {  1, item_type_fixed,       0,      1, "AOS - Aircraft Operational Status Age"                                },
        {  2, item_type_fixed,       0,      1, "TRD - Target Report Descriptor Age"                                   },
        {  3, item_type_fixed,       0,      1, "M3A - Mode 3/A Age"                                                   },
        {  4, item_type_fixed,       0,      1, "QI - Quality Indicators Age"                                          },
        {  5, item_type_fixed,       0,      1, "TI - Trajectory Intent Age"                                           },
        {  6, item_type_fixed,       0,      1, "MAM - Message Amplitude Age"                                          },
        {  7, item_type_fixed,       0,      1, "GH - Geometric Height Age"                                            },
        {  8, item_type_fixed,       0,      1, "FL - Flight Level age"                                                },
        {  9, item_type_fixed,       0,      1, "SAL - Selected Altitude Age"                                          },
        { 10, item_type_fixed,       0,      1, "FSA - Final State Selected Altitude Age"                              },
        { 11, item_type_fixed,       0,      1, "AS - Air Speed Age"                                                   },
        { 12, item_type_fixed,       0,      1, "TAS - True Air Speed Age"                                             },
        { 13, item_type_fixed,       0,      1, "MH - Magnetic Heading Age"                                            },
        { 14, item_type_fixed,       0,      1, "BVR - Barometric Vertical Rate Age"                                   },
        { 15, item_type_fixed,       0,      1, "GVR - Geometric Vertical Rate Age"                                    },
        { 16, item_type_fixed,       0,      1, "GV - Ground Vector Age"                                               },
        { 17, item_type_fixed,       0,      1, "TAR - Track Angle Rate Age"                                           },
        { 18, item_type_fixed,       0,      1, "TID - Target Identification Age"                                      },
        { 19, item_type_fixed,       0,      1, "TS - Target Status age"                                               },
        { 20, item_type_fixed,       0,      1, "MET - Met Information age"                                            },
        { 21, item_type_fixed,       0,      1, "ROA - Roll Angle age"                                                 },
        { 22, item_type_fixed,       0,      1, "ARA - ACAS Resolution Advisory age"                                   },
        { 23, item_type_fixed,       0,      1, "SCC - Surface Capabilities and Characteristics Age"                   },
    });

    uap cat021_295_uap(21, "CAT021 I021/295 " CAT021_VERSION ""_ct, cat021_295_items.data(), cat021_295_items.size());

    uap& get_uap()
    {
        return cat021_uap;
    }
}
