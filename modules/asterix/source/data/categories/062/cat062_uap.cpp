#include "data/categories/062/cat062_uap.hpp"

namespace adam::modules::asterix::cat062
{
    // Forward declarations for main UAP compound fields
    extern uap cat062_110_uap;
    extern uap cat062_290_uap;
    extern uap cat062_295_uap;
    extern uap cat062_340_uap;
    extern uap cat062_380_uap;
    extern uap cat062_390_uap;
    extern uap cat062_500_uap;
    extern uap cat062_ref_uap;

    // Main UAP for CAT062
    const field_spec cat062_items[] = 
    {
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I062/010 Data Source Identifier"                                      },
        // FRN 2 is Spare
        {  3, item_type_fixed,       0,      1, "I062/015 Service Identification"                                      },
        {  4, item_type_fixed,       0,      3, "I062/070 Time Of Track Information"                                   },
        {  5, item_type_fixed,       0,      8, "I062/105 Calculated Track Position (WGS-84)"                          },
        {  6, item_type_fixed,       0,      6, "I062/100 Calculated Track Position (Cartesian)"                       },
        {  7, item_type_fixed,       0,      4, "I062/185 Calculated Track Velocity (Cartesian)"                       },

        // FSPEC Byte 2
        {  8, item_type_fixed,       0,      2, "I062/210 Calculated Acceleration (Cartesian)"                         },
        {  9, item_type_fixed,       0,      2, "I062/060 Track Mode 3/A Code"                                         },
        { 10, item_type_fixed,       0,      7, "I062/245 Target Identification"                                       },
        { 11, item_type_compound,    0,      0, "I062/380 Aircraft Derived Data",                       &cat062_380_uap},
        { 12, item_type_fixed,       0,      2, "I062/040 Track Number"                                                },
        { 13, item_type_variable,    1,      1, "I062/080 Track Status"                                                },
        { 14, item_type_compound,    0,      0, "I062/290 System Track Update Ages",                    &cat062_290_uap},

        // FSPEC Byte 3
        { 15, item_type_fixed,       0,      1, "I062/200 Mode of Movement"                                            },
        { 16, item_type_compound,    0,      0, "I062/295 Track Data Ages",                             &cat062_295_uap},
        { 17, item_type_fixed,       0,      2, "I062/136 Measured Flight Level"                                       },
        { 18, item_type_fixed,       0,      2, "I062/130 Calculated Track Geometric Altitude"                         },
        { 19, item_type_fixed,       0,      2, "I062/135 Calculated Track Barometric Altitude"                        },
        { 20, item_type_fixed,       0,      2, "I062/220 Calculated Rate Of Climb/Descent"                            },
        { 21, item_type_compound,    0,      0, "I062/390 Flight Plan Related Data",                    &cat062_390_uap},

        // FSPEC Byte 4
        { 22, item_type_variable,    1,      1, "I062/270 Target Size & Orientation"                                   },
        { 23, item_type_fixed,       0,      1, "I062/300 Vehicle Fleet Identification"                                },
        { 24, item_type_compound,    0,      0, "I062/110 Mode 5 Data reports & Extended Mode 1 Code",  &cat062_110_uap},
        { 25, item_type_fixed,       0,      2, "I062/120 Track Mode 2 Code"                                           },
        { 26, item_type_variable,    3,      3, "I062/510 Composed Track Number"                                       },
        { 27, item_type_compound,    0,      0, "I062/500 Estimated Accuracies",                        &cat062_500_uap},
        { 28, item_type_compound,    0,      0, "I062/340 Measured Information",                        &cat062_340_uap},

        // FSPEC Byte 5
        // FRN 29 to 33 are Spares
        { 34, item_type_explicit,    0,      0, "RE Reserved Expansion Field",                          &cat062_ref_uap},
        { 35, item_type_explicit,    0,      0, "SP Special Purpose Field"                                             }
    };

    uap cat062_uap(62, cat062_items, sizeof(cat062_items) / sizeof(field_spec));

    // Sub-UAP for I062/110 Mode 5 Data reports & Extended Mode 1 Code (Compound)
    const field_spec cat062_110_items[] =
    {
        {  1, item_type_fixed,       0,      1, "SUM - Mode 5 Summary"                                                 },
        {  2, item_type_fixed,       0,      4, "PMN - Mode 5 PIN / National Origin / Mission Code"                    },
        {  3, item_type_fixed,       0,      6, "POS - Mode 5 Reported Position"                                       },
        {  4, item_type_fixed,       0,      2, "GA - Mode 5 GNSS-derived Altitude"                                    },
        {  5, item_type_fixed,       0,      2, "EM1 - Extended Mode 1 Code in Octal Representation"                   },
        {  6, item_type_fixed,       0,      1, "TOS - Time Offset for POS and GA"                                     },
        {  7, item_type_fixed,       0,      1, "XP - X Pulse Presence"                                                }
    };

    uap cat062_110_uap(0, cat062_110_items, sizeof(cat062_110_items) / sizeof(field_spec));

    // Sub-UAP for I062/290 System Track Update Ages (Compound)
    const field_spec cat062_290_items[] =
    {
        {  1, item_type_fixed,       0,      1, "TRK - Track age"                                                      },
        {  2, item_type_fixed,       0,      1, "PSR - PSR age"                                                        },
        {  3, item_type_fixed,       0,      1, "SSR - SSR age"                                                        },
        {  4, item_type_fixed,       0,      1, "MDS - Mode S age"                                                     },
        {  5, item_type_fixed,       0,      1, "ADS - ADS-C age"                                                      },
        {  6, item_type_fixed,       0,      1, "ES - ADS-B Extended Squitter age"                                     },
        {  7, item_type_fixed,       0,      1, "VDL - ADS-B VDL Mode 4 age"                                           },
        {  8, item_type_fixed,       0,      1, "UAT - ADS-B UAT age"                                                  },
        {  9, item_type_fixed,       0,      1, "LOP - Loop age"                                                       },
        { 10, item_type_fixed,       0,      1, "MLT - Multilateration age"                                            }
    };

    uap cat062_290_uap(0, cat062_290_items, sizeof(cat062_290_items) / sizeof(field_spec));

    // Sub-UAP for I062/295 Track Data Ages (Compound)
    const field_spec cat062_295_items[] =
    {
        {  1, item_type_fixed,       0,      1, "MFL - Measured Flight Level age"                                      },
        {  2, item_type_fixed,       0,      1, "MD1 - Mode 1 age"                                                     },
        {  3, item_type_fixed,       0,      1, "MD2 - Mode 2 age"                                                     },
        {  4, item_type_fixed,       0,      1, "MDA - Mode 3/A age"                                                   },
        {  5, item_type_fixed,       0,      1, "MD4 - Mode 4 age"                                                     },
        {  6, item_type_fixed,       0,      1, "MD5 - Mode 5 age"                                                     },
        {  7, item_type_fixed,       0,      1, "MHG - Magnetic Heading age"                                           },
        {  8, item_type_fixed,       0,      1, "IAS - Indicated Airspeed/Mach Nb age"                                 },
        {  9, item_type_fixed,       0,      1, "TAS - True Airspeed age"                                              },
        { 10, item_type_fixed,       0,      1, "SAL - Selected Altitude age"                                          },
        { 11, item_type_fixed,       0,      1, "FSS - Final State Selected Altitude age"                              },
        { 12, item_type_fixed,       0,      1, "TID - Trajectory Intent Data age"                                     },
        { 13, item_type_fixed,       0,      1, "COM - Communications / ACAS Capability and Flight Status age"         },
        { 14, item_type_fixed,       0,      1, "SAB - Status Reported by ADS-B age"                                   },
        { 15, item_type_fixed,       0,      1, "ACS - ACAS Resolution Advisory Report age"                            },
        { 16, item_type_fixed,       0,      1, "BVR - Barometric Vertical Rate age"                                   },
        { 17, item_type_fixed,       0,      1, "GVR - Geometric Vertical Rate age"                                    },
        { 18, item_type_fixed,       0,      1, "RAN - Roll Angle age"                                                 },
        { 19, item_type_fixed,       0,      1, "TAR - Track Angle Rate age"                                           },
        { 20, item_type_fixed,       0,      1, "TAN - Track Angle age"                                                },
        { 21, item_type_fixed,       0,      1, "GSP - Ground Speed age"                                               },
        { 22, item_type_fixed,       0,      1, "VUN - Velocity Uncertainty age"                                       },
        { 23, item_type_fixed,       0,      1, "MET - Meteorological Data age"                                        },
        { 24, item_type_fixed,       0,      1, "EMC - Emitter Category age"                                           },
        { 25, item_type_fixed,       0,      1, "POS - Position Data age"                                              },
        { 26, item_type_fixed,       0,      1, "GAL - Geometric Altitude Data age"                                    },
        { 27, item_type_fixed,       0,      1, "PUN - Position Uncertainty Data age"                                  },
        { 28, item_type_fixed,       0,      1, "MB - BDS Register Data age"                                           },
        { 29, item_type_fixed,       0,      1, "IAR - Indicated Airspeed age"                                         },
        { 30, item_type_fixed,       0,      1, "MAC - Mach Number age"                                                },
        { 31, item_type_fixed,       0,      1, "BPS - Barometric Pressure Setting age"                                }
    };

    uap cat062_295_uap(0, cat062_295_items, sizeof(cat062_295_items) / sizeof(field_spec));

    // Sub-UAP for I062/340 Measured Information (Compound)
    const field_spec cat062_340_items[] =
    {
        {  1, item_type_fixed,       0,      2, "SID - Sensor Identification"                                          },
        {  2, item_type_fixed,       0,      4, "POS - Measured Position"                                              },
        {  3, item_type_fixed,       0,      2, "HEI - Measured 3-D Height"                                            },
        {  4, item_type_fixed,       0,      2, "MDC - Last Measured Mode C code"                                      },
        {  5, item_type_fixed,       0,      2, "MDA - Last Measured Mode 3/A code"                                    },
        {  6, item_type_fixed,       0,      1, "TYP - Report Type"                                                    }
    };

    uap cat062_340_uap(0, cat062_340_items, sizeof(cat062_340_items) / sizeof(field_spec));

    // Sub-UAP for I062/380 Aircraft Derived Data (Compound)
    const field_spec cat062_380_items[] =
    {
        {  1, item_type_fixed,       0,      3, "ADR - Target Address"                                                 },
        {  2, item_type_fixed,       0,      6, "ID - Target Identification"                                           },
        {  3, item_type_fixed,       0,      2, "MHG - Magnetic Heading"                                               },
        {  4, item_type_fixed,       0,      2, "IAS - Indicated Airspeed / Mach Number"                               },
        {  5, item_type_fixed,       0,      2, "TAS - True Airspeed"                                                  },
        {  6, item_type_fixed,       0,      2, "SAL - Selected Altitude"                                              },
        {  7, item_type_fixed,       0,      2, "FSS - Final State Selected Altitude"                                  },
        {  8, item_type_fixed,       0,      1, "TIS - Trajectory Intent Status"                                       },
        {  9, item_type_repetetive,  1,     15, "TID - Trajectory Intent Data"                                         },
        { 10, item_type_fixed,       0,      2, "COM - Communications / ACAS Capability and Flight Status"             },
        { 11, item_type_fixed,       0,      2, "SAB - Status Reported by ADS-B"                                       },
        { 12, item_type_fixed,       0,      7, "ACS - ACAS Resolution Advisory Report"                                },
        { 13, item_type_fixed,       0,      2, "BVR - Barometric Vertical Rate"                                       },
        { 14, item_type_fixed,       0,      2, "GVR - Geometric Vertical Rate"                                        },
        { 15, item_type_fixed,       0,      2, "RAN - Roll Angle"                                                     },
        { 16, item_type_fixed,       0,      2, "TAR - Track Angle Rate"                                               },
        { 17, item_type_fixed,       0,      2, "TAN - Track Angle"                                                    },
        { 18, item_type_fixed,       0,      2, "GSP - Ground Speed"                                                   },
        { 19, item_type_fixed,       0,      1, "VUN - Velocity Uncertainty"                                           },
        { 20, item_type_fixed,       0,      8, "MET - Meteorological Data"                                            },
        { 21, item_type_fixed,       0,      1, "EMC - Emitter Category"                                               },
        { 22, item_type_fixed,       0,      6, "POS - Position Data"                                                  },
        { 23, item_type_fixed,       0,      2, "GAL - Geometric Altitude Data"                                        },
        { 24, item_type_fixed,       0,      1, "PUN - Position Uncertainty Data"                                      },
        { 25, item_type_repetetive,  1,      8, "MB - BDS Register Data"                                               },
        { 26, item_type_fixed,       0,      2, "IAR - Indicated Airspeed"                                             },
        { 27, item_type_fixed,       0,      2, "MAC - Mach Number"                                                    },
        { 28, item_type_fixed,       0,      2, "BPS - Barometric Pressure Setting"                                    }
    };

    uap cat062_380_uap(0, cat062_380_items, sizeof(cat062_380_items) / sizeof(field_spec));

    // Sub-UAP for I062/390 Flight Plan Related Data (Compound)
    const field_spec cat062_390_items[] =
    {
        {  1, item_type_fixed,       0,      2, "TAG - FPPS Identification Tag"                                        },
        {  2, item_type_fixed,       0,      6, "CSN - Callsign"                                                       },
        {  3, item_type_fixed,       0,      4, "IFI - IFPS_FLIGHT_ID"                                                 },
        {  4, item_type_fixed,       0,      1, "FCT - Flight Category"                                                },
        {  5, item_type_fixed,       0,      4, "TAC - Type of Aircraft"                                               },
        {  6, item_type_fixed,       0,      1, "WTC - Wake Turbulence Category"                                       },
        {  7, item_type_fixed,       0,      4, "DEP - Departure Airport"                                              },
        {  8, item_type_fixed,       0,      4, "DST - Destination Airport"                                            },
        {  9, item_type_fixed,       0,      3, "RDS - Runway Designation"                                             },
        { 10, item_type_fixed,       0,      2, "CFL - Current Cleared Flight Level"                                   },
        { 11, item_type_fixed,       0,      2, "CTL - Current Control Position"                                       },
        { 12, item_type_repetetive,  1,      4, "TOD - Time of Departure / Arrival"                                    },
        { 13, item_type_fixed,       0,      6, "AST - Aircraft Stand"                                                 },
        { 14, item_type_fixed,       0,      1, "STS - Stand Status"                                                   },
        { 15, item_type_fixed,       0,      7, "STD - Standard Instrument Departure"                                  },
        { 16, item_type_fixed,       0,      7, "STA - Standard Instrument Arrival"                                    },
        { 17, item_type_fixed,       0,      2, "PEM - Pre-emergency Mode 3/A code"                                    },
        { 18, item_type_fixed,       0,      6, "PEC - Pre-emergency Callsign"                                         }
    };

    uap cat062_390_uap(0, cat062_390_items, sizeof(cat062_390_items) / sizeof(field_spec));

    // Sub-UAP for I062/500 Estimated Accuracies (Compound)
    const field_spec cat062_500_items[] =
    {
        {  1, item_type_fixed,       0,      4, "APC - Estimated Accuracy Of Track Position (Cartesian)"               },
        {  2, item_type_fixed,       0,      2, "COV - XY Covariance"                                                  },
        {  3, item_type_fixed,       0,      4, "APW - Estimated Accuracy of Track Position (WGS-84)"                  },
        {  4, item_type_fixed,       0,      1, "AGA - Estimated Accuracy Of Calculated Track Geometric Altitude"      },
        {  5, item_type_fixed,       0,      1, "ABA - Estimated Accuracy Of Calculated Track Barometric Altitude"     },
        {  6, item_type_fixed,       0,      2, "ATV - Estimated Accuracy Of Track Velocity (Cartesian)"               },
        {  7, item_type_fixed,       0,      2, "AA - Estimated Accuracy Of Acceleration (Cartesian)"                  },
        {  8, item_type_fixed,       0,      1, "ARC - Estimated Accuracy Of Rate Of Climb/Descent"                    }
    };

    uap cat062_500_uap(0, cat062_500_items, sizeof(cat062_500_items) / sizeof(field_spec));

    // Forward declarations for REF sub-UAPs
    extern uap cat062_ref_av3_uap;
    extern uap cat062_ref_moi_uap;
    extern uap cat062_ref_mti_uap;
    extern uap cat062_ref_g62_uap;

    // Sub-UAP for Reserved Expansion Field (REF) - Appendix A
    const field_spec cat062_ref_items[] =
    {
        {  1, item_type_repetetive,  1,      5, "CST - Contributing Sensors with local tracknumber"                    },
        {  2, item_type_repetetive,  1,      3, "CSN - Contributing Sensors No local tracknumber"                      },
        {  3, item_type_fixed,       0,      4, "TVS - Calculated Track Velocity relative to System Reference Point"   },
        {  4, item_type_variable,    1,      1, "STS - Supplementary Track Status"                                     },
        {  5, item_type_compound,    0,      0, "V3 - ADS-B Version 3 Data",                        &cat062_ref_av3_uap},
        {  6, item_type_explicit,    0,      0, "MOI - Miscellaneous Operational Items",            &cat062_ref_moi_uap},
        {  7, item_type_explicit,    0,      0, "MTI - Miscellaneous Technical Items",              &cat062_ref_mti_uap},
        {  8, item_type_compound,    0,      0, "GEN62 - Generic Category 062 Data",                &cat062_ref_g62_uap}
    };

    uap cat062_ref_uap(0, cat062_ref_items, sizeof(cat062_ref_items) / sizeof(cat062_ref_items[0]));

    // Sub-UAP for V3 (ADS-B Version 3 Data)
    const field_spec cat062_ref_v3_items[] =
    {
        {  1, item_type_fixed,       0,      1, "PS3 - Priority Status ADS-B Version 3"                                },
        {  2, item_type_fixed,       0,      3, "AS - Aircraft Status"                                                 },
        {  3, item_type_fixed,       0,      1, "UAS - UAS/RPAS Status"                                                },
        {  4, item_type_fixed,       0,      1, "CASS - Collision Avoidance System Status"                             }
    };

    uap cat062_ref_av3_uap(0, cat062_ref_v3_items, sizeof(cat062_ref_v3_items) / sizeof(field_spec));

    // Sub-UAP for MOI (Miscellaneous Operational Items)
    const field_spec cat062_ref_moi_items[] =
    {
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
    };

    uap cat062_ref_moi_uap(0, cat062_ref_moi_items, sizeof(cat062_ref_moi_items) / sizeof(field_spec));

    // Sub-UAP for MTI (Miscellaneous Technical Items)
    const field_spec cat062_ref_mti_items[] =
    {
        {  1, item_type_fixed,       0,      4, "DATE - Date of Track Information"                                     },
        {  2, item_type_fixed,       0,      2, "TENTU - Time to Expected Next Track Update"                           },
        {  3, item_type_fixed,       0,      4, "IPMC - Inverse Point Scale Factor and Meridian Convergence"           },
        {  4, item_type_repetetive,  1,      2, "IMP - IMM Model Probability"                                          },
        {  5, item_type_repetetive,  1,     15, "IST - IMM State"                                                      },
        {  6, item_type_fixed,       0,      3, "TTT - Time of Track Termination"                                      },
        {  7, item_type_fixed,       0,      2, "EXM3A - Expired Mode 3/A Code"                                        },
        {  8, item_type_fixed,       0,      3, "EXADDR - Expired Target Address"                                      },
        {  9, item_type_fixed,       0,      6, "EXTID - Expired Target Identification"                                }
    };

    uap cat062_ref_mti_uap(0, cat062_ref_mti_items, sizeof(cat062_ref_mti_items) / sizeof(field_spec));

    // Sub-UAP for GEN62 (Generic Category 062 Data) - Placeholder
    const field_spec cat062_ref_gen62_items[] =
    {
        {  1, item_type_variable,    0,      0, "Spare/Placeholder"                                                    }
    };

    uap cat062_ref_g62_uap(0, cat062_ref_gen62_items, sizeof(cat062_ref_gen62_items) / sizeof(field_spec));

    uap& get_uap()
    {
        return cat062_uap;
    }
}
