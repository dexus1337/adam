#include "data/profiles/mercedes/w209/w209-can-b.hpp"

#include <array>

/**
 * @file    w209-can-b.cpp
 * @author  dexus1337
 * @brief   Implements the complete Mercedes-Benz W209 Interior CAN-B profile definitions.
 * @version 1.0
 * @date    24.08.2026
 */

namespace adam::modules::can::profiles::mercedes::w209
{
    using namespace adam::string_hashed_ct_literals;

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: UNKNOWN (ID: 0x0000) - Central Locking & Terminal Status (35 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto unknown_0000_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KG_KL_AKT",         "Keyless Go terminal control active",                                            0,   1 },
        {   1, "KL_50_EIN",         "Terminal 50 is turned on",                                                      3,   1 },
        {   2, "KL_15X_EIN",        "Terminal 15X is turned on",                                                     4,   1 },
        {   3, "KL_15_EIN",         "Terminal 15 is turned on",                                                      5,   1 },
        {   4, "KL_15R_EIN",        "Terminal 15R is turned on",                                                     6,   1 },
        {   5, "FZG_RECH",          "Message: \"Vehicle calculating, please wait\"",                                 8,   1 },
        {   6, "DIAG_TGL",          "Diagnostic toggle bit",                                                         9,   1 },
        {   7, "APPL_AUS",          "Do not send application IDs, only NM IDs",                                     10,   1 },
        {   8, "PNK_ALM_AUS",       "Panic alarm via key off",                                                      11,   1 },
        {   9, "PNK_ALM_EIN",       "Panic alarm via key on",                                                       12,   1 },
        {  10, "FERN_ALARM",        "Remote triggering MSS alarm",                                                  13,   1 },
        {  11, "SCHLUE_NEU",        "Message: Renew key",                                                           16,   1 },
        {  12, "ZV_PASSIV",         "Passive locking",                                                              17,   1 },
        {  13, "ZV_SPIEL",          "Play protection active",                                                       18,   1 },
        {  14, "HD_STOPP",          "Trunk lid stop",                                                               19,   1 },
        {  15, "SPEI_NR",           "Current memory block number",                                                  21,   3 },
        {  16, "AUSS_SICH",         "Exterior lock",                                                                24,   1 },
        {  17, "AUSS_ENTSI",        "Exterior unlock",                                                              25,   1 },
        {  18, "BLI_SICH",          "Central locking blinker feedback lock",                                        26,   1 },
        {  19, "BLI_ENTSI",         "Central locking blinker feedback unlock",                                      27,   1 },
        {  20, "HFE_EZS",           "Remote trunk lid release",                                                     32,   1 },
        {  21, "HD_SICH",           "Trunk lid lock",                                                               33,   1 },
        {  22, "HD_ENTSI",          "Trunk lid unlock",                                                             34,   1 },
        {  23, "TD_VERRI",          "Lock fuel flap (glove box / storage compartments)",                            35,   1 },
        {  24, "TD_ENTRI",          "Unlock fuel flap (glove box / storage compartments)",                          36,   1 },
        {  25, "ZV_NV",             "Central locking auto-relock",                                                  38,   1 },
        {  26, "SCHL_BEF",          "Mechanical / remote key active",                                               39,   1 },
        {  27, "THR_VERRI",         "Lock rear right door",                                                         40,   1 },
        {  28, "THR_ENTRI",         "Unlock rear right door",                                                       41,   1 },
        {  29, "THL_VERRI",         "Lock rear left door",                                                          42,   1 },
        {  30, "THL_ENTRI",         "Unlock rear left door",                                                        43,   1 },
        {  31, "TVR_VERRI",         "Lock front right door",                                                        44,   1 },
        {  32, "TVR_ENTRI",         "Unlock front right door",                                                      45,   1 },
        {  33, "TVL_VERRI",         "Lock front left door",                                                         46,   1 },
        {  34, "TVL_ENTRI",         "Unlock front left door",                                                       47,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A10 (ID: 0x010A) - Rear Wheel Speed & Direction (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a10_010a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "RIZ_HL",            "Pulse ring counter rear left wheel (48/rev) (pulses)",                          0,   8 },
        {   1, "RIZ_HR",            "Pulse ring counter rear right wheel (48/rev) (pulses)",                         8,   8 },
        {   2, "DRTGHR",            "Direction of rotation rear right wheel",                                       16,   2 },
        {   3, "DHR",               "Rear right wheel speed (1/min)",                                               18,  14 },
        {   4, "DRTGHL",            "Direction of rotation rear left wheel",                                        32,   2 },
        {   5, "DHL",               "Rear left wheel speed (1/min)",                                                34,  14 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A11 (ID: 0x0016) - Battery Voltage (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a11_0016_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "U_BATT",            "Battery voltage (V)",                                                           0,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A2 (ID: 0x0002) - Front Wheel Speed & Engine Data (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a2_0002_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "RIZ_VL",            "Pulse ring counter front left wheel (48/rev) (pulses)",                         0,   8 },
        {   1, "RIZ_VR",            "Pulse ring counter front right wheel (48/rev) (pulses)",                        8,   8 },
        {   2, "N_MOT",             "Engine speed (1/min)",                                                         16,  16 },
        {   3, "T_MOT",             "Engine coolant temperature (°C)",                                              32,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A4 (ID: 0x0058) - Key Identification & Mileage (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a4_0058_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SCHLUE_ID",         "Key identification for pre-filtering",                                          0,  32 },
        {   1, "KM_EZS",            "Odometer reading (km)",                                                        32,  24 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A5 (ID: 0x001F) - Special Equipment & Features Coding (45 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a5_001f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LAND",              "Country-specific special equipment coding",                                     0,   4 },
        {   1, "LL_RL",             "Left/right hand drive",                                                         6,   2 },
        {   2, "GUARD_B4",          "Special protection Guard B4",                                                   8,   1 },
        {   3, "BEHI_FZG",          "Disabled vehicle conversion (tester only)",                                     9,   1 },
        {   4, "TAXI_FUNKAUF",      "Taxi radio connection",                                                        10,   1 },
        {   5, "SO_FZG",            "Special purpose vehicle",                                                      11,   1 },
        {   6, "TAXI_HIRU",         "Taxi distress call",                                                           12,   1 },
        {   7, "TAXI_DZ",           "Connection for roof sign",                                                     13,   1 },
        {   8, "TAXI_NOTALM",       "Taxi emergency alarm system",                                                  14,   1 },
        {   9, "TAXI_INT",          "Taxi international",                                                           15,   1 },
        {  10, "KB_SPERR_KLA",      "Lock recirculation convenience operation",                                     16,   1 },
        {  11, "KB_MAN_KLA",        "Recirculation convenience run mode",                                           17,   1 },
        {  12, "KB_AUTO",           "Convenience run mode",                                                         18,   1 },
        {  13, "FH_SPERR_VO",       "Lock automatic roll-up front power windows",                                   19,   1 },
        {  14, "FH_SPERR_HI",       "Lock automatic roll-up rear power windows",                                    20,   1 },
        {  15, "FL_ZU_MS",          "Close fresh air flap on engine start",                                         21,   1 },
        {  16, "DATENF",            "Data radio present",                                                           22,   1 },
        {  17, "GUARD_B6",          "Special protection Guard B6/7",                                                23,   1 },
        {  18, "FCOD_KAR",          "Vehicle body code (203/209)",                                                  24,   3 },
        {  19, "FCOD_BR",           "Vehicle model series code",                                                    27,   5 },
        {  20, "PRW_VH",            "Flat tyre warning present",                                                    32,   1 },
        {  21, "FCOD_MOT7",         "Vehicle engine code",                                                          33,   7 },
        {  22, "RS_VH",             "Rain sensor present",                                                          40,   1 },
        {  23, "XEN_VH",            "Xenon light present",                                                          41,   1 },
        {  24, "SRA_VH",            "Headlamp cleaning system present",                                             42,   1 },
        {  25, "KLA_VH",            "Air conditioning system present",                                              43,   1 },
        {  26, "NAG_VH",            "Automatic transmission present",                                               44,   1 },
        {  27, "KSG_VH",            "Convenience manual transmission present",                                      45,   1 },
        {  28, "MEMORY_VH",         "Driver seat memory present",                                                   46,   1 },
        {  29, "KP_VH",             "Communication platform present",                                               47,   1 },
        {  30, "ART_VH",            "Distronic (ART) present",                                                      48,   1 },
        {  31, "CVT_VH",            "CVT transmission present",                                                     49,   1 },
        {  32, "FSB_HZG_VH",        "Heated windshield present",                                                    50,   1 },
        {  33, "FUK_SCHL",          "Close footwell flaps in cooling mode (G463 only)",                             51,   1 },
        {  34, "BOOSTER_NVH",       "Booster blower not present",                                                   52,   1 },
        {  35, "NIV_VH",            "Level control present",                                                        53,   1 },
        {  36, "SOUND_VH",          "Sound system present",                                                         55,   1 },
        {  37, "PTS_VH",            "Parktronic system present",                                                    56,   1 },
        {  38, "AHK_VH",            "Trailer hitch present",                                                        57,   1 },
        {  39, "HR_VH",             "Rear roller blind present",                                                    58,   1 },
        {  40, "EDW_VH",            "Anti-theft alarm system present",                                              59,   1 },
        {  41, "IRS_VH",            "Interior protection present",                                                  60,   1 },
        {  42, "KG_VH",             "Keyless Go present",                                                           61,   1 },
        {  43, "ERS_LICHT",         "Complete replacement light permitted",                                         62,   1 },
        {  44, "SWB_VH",            "Heated windshield washer system present",                                      63,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A6 (ID: 0x001E) - Model Year & TPM Status (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a6_001e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VER_JAHR",          "Year specification",                                                            1,   5 },
        {   1, "VER_AE",            "Modification year",                                                             6,   2 },
        {   2, "TPM_VH",            "Tire pressure module present",                                                 30,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A7 (ID: 0x0003) - Engine Status, Wheel Speed & Lamp Faults (26 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a7_0003_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LL_STBL",           "Idle speed is stable",                                                          0,   1 },
        {   1, "KOMP_BAUS",         "A/C compressor off: Acceleration",                                              1,   1 },
        {   2, "KOMP_NOTAUS",       "A/C compressor emergency shutoff",                                              2,   1 },
        {   3, "LUEFT_MOT_KL",      "Engine cooling fan defective indicator lamp",                                   3,   1 },
        {   4, "ZWP_EIN_MS",        "Auxiliary water pump switch on",                                                4,   1 },
        {   5, "BLS_UNT",           "Brake light suppression",                                                       5,   1 },
        {   6, "BLS_ST",            "Brake light switch status",                                                     6,   2 },
        {   7, "RG",                "Reverse gear engaged (all transmissions)",                                      8,   1 },
        {   8, "P",                 "Park position engaged",                                                         9,   1 },
        {   9, "HZL_ST",            "Heating output status",                                                        10,   2 },
        {  10, "WHC",               "Transmission selector lever position (NAG only)",                              12,   4 },
        {  11, "DRTGVL",            "Direction of rotation front left wheel",                                       16,   2 },
        {  12, "DVL",               "Front left wheel speed (1/min)",                                               18,  14 },
        {  13, "OEL_KL",            "Engine oil level / oil pressure warning lamp",                                 32,   1 },
        {  14, "DIAG_KL",           "Diagnostic check engine lamp (OBD II)",                                        33,   1 },
        {  15, "BAS_KL",            "Brake Assist (BAS) defective warning lamp",                                    34,   1 },
        {  16, "ESP_KL",            "ESP defective warning lamp",                                                   35,   1 },
        {  17, "ABS_KL",            "ABS defective warning lamp",                                                   36,   1 },
        {  18, "BBV_KL",            "Brake pad wear warning lamp",                                                  37,   1 },
        {  19, "UEHITZ",            "Engine oil temperature too high (overheating)",                                38,   1 },
        {  20, "KPL",               "Clutch depressed",                                                             39,   1 },
        {  21, "WHST",              "Transmission selector lever position (NAG, KSG, CVT)",                         40,   3 },
        {  22, "NOTBRE",            "Emergency braking (flashing brake lights)",                                    43,   1 },
        {  23, "ART_ABW_AKT",       "Distronic (ART) distance warning switched on",                                 44,   1 },
        {  24, "SUB_ABL_L",         "Low beam substitution left",                                                   45,   1 },
        {  25, "SUB_ABL_R",         "Low beam substitution right",                                                  46,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A8 (ID: 0x0390) - Climate Control Configuration & Characteristics (27 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a8_0390_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZWP_NVH",           "Auxiliary water pump not installed",                                            0,   1 },
        {   1, "UMLUFT_UBG",        "Manual recirculation unlimited duration",                                       1,   1 },
        {   2, "RHT_EIN",           "Always REHEAT operation",                                                       2,   1 },
        {   3, "GBA_MAN",           "Blower bar display only in manual mode",                                        3,   1 },
        {   4, "GBL_40",            "40% base ventilation",                                                          4,   1 },
        {   5, "SUS_EIN",           "Pollutant-dependent recirculation generally on",                                5,   1 },
        {   6, "SUS_AUS",           "Pollutant-dependent recirculation off",                                         6,   1 },
        {   7, "KFK_AUS",           "Refrigerant fill level check inactive",                                         7,   1 },
        {   8, "KALTLAND_1",        "+1°C increase",                                                                 8,   1 },
        {   9, "HEISSLAND_2",       "-2°C decrease",                                                                 9,   1 },
        {  10, "KALTLAND_2",        "+2°C increase",                                                                10,   1 },
        {  11, "UMLUFT_EIN",        "Close recirculation flap completely from < 20%",                               11,   1 },
        {  12, "ESAUGBEL_EIN",      "20% base ventilation electric suction fan on",                                 12,   1 },
        {  13, "UMLUFT_AUS",        "Recirculation flap open in OFF mode",                                          13,   1 },
        {  14, "SOL_AUS",           "Solar influence not active",                                                   14,   1 },
        {  15, "HEISSLAND_1",       "-1°C hot countries",                                                           15,   1 },
        {  16, "WUESTENLAND",       "Desert countries with sand",                                                   16,   1 },
        {  17, "GBL_KNL",           "Base ventilation characteristic curve",                                        17,   3 },
        {  18, "P_KNL",             "Pressure characteristic curve",                                                20,   4 },
        {  19, "GSPA_KLA_KUEHL",    "Transmission shift point increase on cooling deficit",                         24,   1 },
        {  20, "GSPA_KLA_HEIZ",     "Transmission shift point increase on heating deficit",                         25,   1 },
        {  21, "TPS_NVH",           "Dew point sensor not present",                                                 26,   1 },
        {  22, "IFDBE_VH",          "Interior sensor in overhead control panel present",                            27,   1 },
        {  23, "REST_VH",           "Residual heat utilization present",                                            28,   1 },
        {  24, "MAXCOOL",           "Display indication \"MAXCOOL\" (USA only)",                                    29,   1 },
        {  25, "ASL_LVT",           "Automatic standard logic air distribution",                                    30,   1 },
        {  26, "ASL_GBL",           "Automatic standard logic blower",                                              31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A9 (ID: 0x00B2) - Vehicle Identification Number (VIN) (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a9_00b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VIN_MSG",           "VIN signal part",                                                               6,   2 },
        {   1, "VIN_DATA",          "VIN data",                                                                      8,  56 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KG_A1 (ID: 0x01B2) - Keyless Go Instrument Cluster Messages (15 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kg_a1_01b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "M5",                "Msg 5: \"Selector lever in P or N position, please\"",                          0,   1 },
        {   1, "M4",                "Msg 4: \"Check key card / key battery\" (white)",                               1,   1 },
        {   2, "M3",                "Msg 3: \"Selector lever to P\" (red, continuous tone)",                         2,   1 },
        {   3, "M2",                "Msg 2: \"Key card / key recognized in vehicle\" (white)",                       3,   1 },
        {   4, "M1",                "Msg 1: \"Key card / key not recognized\" (white)",                              4,   1 },
        {   5, "M0",                "Msg 0: \"Key card / key not recognized\" (red)",                                5,   1 },
        {   6, "WARNTON_KG",        "Warning tone switch on",                                                        6,   1 },
        {   7, "M12",               "Msg 12: \"Take key card / key with you!\"",                                     9,   1 },
        {   8, "M11",               "Msg 11: \"Please leave key inserted\"",                                        10,   1 },
        {   9, "M10",               "Msg 10: \"Key calculating\"",                                                  11,   1 },
        {  10, "M9",                "Msg 9: \"Keyless Go in diagnosis\"",                                           12,   1 },
        {  11, "M8",                "Msg 8: \"Door open\"",                                                         13,   1 },
        {  12, "M7",                "Msg 7: Reserved",                                                              14,   1 },
        {  13, "M6",                "Msg 6: \"No drive authorization\"",                                            15,   1 },
        {  14, "KM_REST_KG",        "Keyless Go distance reading (km)",                                             16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KG_A2 (ID: 0x0050) - Keyless Go Convenience Window & Roof Control (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kg_a2_0050_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_KG",            "Open / close rear right power window",                                          0,   1 },
        {   1, "FHL_KG",            "Open / close rear left power window",                                           1,   1 },
        {   2, "FVR_KG",            "Open / close front right power window",                                         2,   1 },
        {   3, "FVL_KG",            "Open / close front left power window",                                          3,   1 },
        {   4, "SHD_KG",            "Open / close sunroof / soft top",                                               4,   1 },
        {   5, "KB_RI_KG",          "Convenience operation direction",                                               5,   1 },
        {   6, "KB_MOD_KG",         "Convenience operation mode",                                                    6,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_A2 (ID: 0x018D) - TeleAid Authorization & Status (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_a2_018d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MK_ATRSRT",         "Mobility account authorized",                                                   0,   1 },
        {   1, "LIVE_TELEAID",      "TeleAid heartbeat alive message",                                               4,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS1 (ID: 0x03E5) - TeleAid GPS Latitude & Longitude (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos1_03e5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_LAT",           "GPS latitude (- means south) (mas)",                                            0,  32 },
        {   1, "GPS_LONG",          "GPS longitude (- means west) (mas)",                                           32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS2 (ID: 0x03E6) - TeleAid GPS Velocity, Heading & Altitude (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos2_03e6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_VEL",           "GPS velocity (cm/s)",                                                           0,  16 },
        {   1, "GPS_HEAD",          "GPS heading (°)",                                                              16,  16 },
        {   2, "GPS_ELLIP",         "GPS ellipsoid height (m)",                                                     32,  16 },
        {   3, "GPS_ALT",           "GPS altitude (m)",                                                             48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS3 (ID: 0x03E7) - TeleAid GPS Date & UTC Time (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos3_03e7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_DATE_YEAR",     "GPS date year (years)",                                                         0,  16 },
        {   1, "GPS_DATE_MONTH",    "GPS date month (months)",                                                      16,   8 },
        {   2, "GPS_DATE_DAY",      "GPS date day (days)",                                                          24,   8 },
        {   3, "GPS_UTC_HOUR",      "GPS UTC hour (h)",                                                             32,   8 },
        {   4, "GPS_UTC_MINUTE",    "GPS UTC minute (min)",                                                         40,   8 },
        {   5, "GPS_UTC_SECOND",    "GPS UTC second (s)",                                                           48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS4 (ID: 0x03E8) - TeleAid Dead Reckoning / Map Matching Position (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos4_03e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DR_MM_LAT",         "Dead reckoning / map matching latitude (- means south) (mas)",                  0,  32 },
        {   1, "DR_MM_LONG",        "Dead reckoning / map matching longitude (- means west) (mas)",                 32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS5 (ID: 0x03E9) - TeleAid Satellite & Map Matching Status (11 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos5_03e9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_TRCK_SAT",      "GPS tracked satellites",                                                        0,   4 },
        {   1, "GPS_VSBL_SAT",      "GPS visible satellites",                                                        4,   4 },
        {   2, "GPS_VDOP",          "GPS vertical dilution of position",                                             8,   8 },
        {   3, "GPS_HDOP",          "GPS horizontal dilution of position",                                          16,   8 },
        {   4, "GPS_PDOP",          "GPS dilution of position",                                                     24,   8 },
        {   5, "GPS_FIX",           "GPS fix",                                                                      36,   4 },
        {   6, "DR_MM_REL",         "Dead reckoning / map matching position reliability (%)",                       40,   8 },
        {   7, "MM_MAP_STAT",       "Map matching map state",                                                       48,   2 },
        {   8, "MM_ROAD_STAT",      "Map matching road state",                                                      50,   2 },
        {   9, "MM_ROUTE_STAT",     "Map matching route state",                                                     52,   2 },
        {  10, "DR_MM_STAT",        "Dead reckoning / map matching state",                                          54,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GW_C_B7 (ID: 0x0005) - Gateway CAN-C to CAN-B Front Right Wheel Speed (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gw_c_b7_0005_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DRTGVR",            "Direction of rotation front right wheel",                                      32,   2 },
        {   1, "DVR",               "Front right wheel speed (1/min)",                                              34,  14 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_TELEAID_AGW6 (ID: 0x0209) - Transport Protocol TeleAid to AGW (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_teleaid_agw6_0209_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_TELEAID_AGW",    "Transport protocol TeleAid to AGW",                                             0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_TELEAID_KOMBI4 (ID: 0x01A1) - Transport Protocol TeleAid to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_teleaid_kombi4_01a1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_TELEAID_KOMBI",  "Transport protocol TeleAid to Instrument Cluster",                              0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_EZS (ID: 0x0400) - Network Management Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_ezs_0400_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EZS (ID: 0x05FF) - Diagnostic Response Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ezs_05ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_EZS (ID: 0x0760) - Application Interface Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_ezs_0760_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_AAG (ID: 0x0730) - Diagnostic Request Trailer Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_aag_0730_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_AGW (ID: 0x05D6) - Diagnostic Request Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_agw_05d6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ARMADA (ID: 0x06BC) - Diagnostic Request Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_armada_06bc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_DBE (ID: 0x0667) - Diagnostic Request Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_dbe_0667_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FDSVL (ID: 0x06BE) - Diagnostic Request Dynamic Seat Front Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fdsvl_06be_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FDSVR (ID: 0x06BF) - Diagnostic Request Dynamic Seat Front Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fdsvr_06bf_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_HFS (ID: 0x0577) - Diagnostic Request Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_hfs_0577_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ICANI (ID: 0x07DA) - Diagnostic Request CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_icani_07da_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_ICANI",        "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_KLA (ID: 0x0791) - Diagnostic Request Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_kla_0791_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_KOMBI (ID: 0x05B4) - Diagnostic Request Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_kombi_05b4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_LRK (ID: 0x06AF) - Diagnostic Request Heated Steering Wheel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_lrk_06af_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST0 (ID: 0x0640) - Diagnostic Request MOST Gateway 0 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most0_0640_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST1 (ID: 0x0641) - Diagnostic Request MOST Gateway 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most1_0641_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST10 (ID: 0x064A) - Diagnostic Request MOST Gateway 10 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most10_064a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST11 (ID: 0x064B) - Diagnostic Request MOST Gateway 11 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most11_064b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST12 (ID: 0x064C) - Diagnostic Request MOST Gateway 12 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most12_064c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST13 (ID: 0x064D) - Diagnostic Request MOST Gateway 13 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most13_064d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST14 (ID: 0x064E) - Diagnostic Request MOST Gateway 14 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most14_064e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST15 (ID: 0x064F) - Diagnostic Request MOST Gateway 15 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most15_064f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST2 (ID: 0x0642) - Diagnostic Request MOST Gateway 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most2_0642_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST3 (ID: 0x0643) - Diagnostic Request MOST Gateway 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most3_0643_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST4 (ID: 0x0644) - Diagnostic Request MOST Gateway 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most4_0644_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST5 (ID: 0x0645) - Diagnostic Request MOST Gateway 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most5_0645_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST6 (ID: 0x0646) - Diagnostic Request MOST Gateway 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most6_0646_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST7 (ID: 0x0647) - Diagnostic Request MOST Gateway 7 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most7_0647_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST8 (ID: 0x0648) - Diagnostic Request MOST Gateway 8 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most8_0648_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST9 (ID: 0x0649) - Diagnostic Request MOST Gateway 9 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most9_0649_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MRM (ID: 0x06D5) - Diagnostic Request Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_mrm_06d5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MSS (ID: 0x0726) - Diagnostic Request Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_mss_0726_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_NAVI (ID: 0x054A) - Diagnostic Request Navigation System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_navi_054a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_NAVI",         "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_OBF (ID: 0x06A5) - Diagnostic Request Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_obf_06a5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_PFDS (ID: 0x072E) - Diagnostic Request Pneumatic Dynamic Seat (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_pfds_072e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_PTS (ID: 0x0733) - Diagnostic Request Parktronic (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_pts_0733_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SAM_H (ID: 0x0563) - Diagnostic Request Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sam_h_0563_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SAM_V (ID: 0x0662) - Diagnostic Request Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sam_v_0662_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SB (ID: 0x06AD) - Diagnostic Request Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sb_06ad_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SF (ID: 0x06AC) - Diagnostic Request Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sf_06ac_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SHZ (ID: 0x057B) - Diagnostic Request Seat Heating (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_shz_057b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_STH (ID: 0x0739) - Diagnostic Request Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sth_0739_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_THL (ID: 0x0749) - Diagnostic Request Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_thl_0749_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_THR (ID: 0x074B) - Diagnostic Request Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_thr_074b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TLM (ID: 0x05DA) - Diagnostic Request Telematics Control Unit (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tlm_05da_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TPM (ID: 0x06B8) - Diagnostic Request Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tpm_06b8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_TPM",          "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TVL (ID: 0x06C8) - Diagnostic Request Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tvl_06c8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TVR (ID: 0x06CA) - Diagnostic Request Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tvr_06ca_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_UBF (ID: 0x073D) - Diagnostic Request Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ubf_073d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_WSS (ID: 0x06A8) - Diagnostic Request Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_wss_06a8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_GLOBAL (ID: 0x001C) - Global Diagnostic Request (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_global_001c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "Global KWP2000 diagnostic request",                                             0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A1 (ID: 0x000C) - Instrument Cluster Display & Convenience Status (28 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a1_000c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KL_58D_B",          "Instrument illumination brightness (%)",                                        0,   8 },
        {   1, "V_SIGNAL",          "Vehicle speed (km/h)",                                                          8,   8 },
        {   2, "DZ_EIN",            "Roof sign switch on (Taxi)",                                                   16,   1 },
        {   3, "TFSM_B",            "Fuel level minimum",                                                           17,   1 },
        {   4, "AUTO_TUER",         "Automatic door locking",                                                       18,   1 },
        {   5, "T_C",               "Temperature unit",                                                             19,   1 },
        {   6, "TFL_EIN",           "Daytime running lamps on",                                                     20,   1 },
        {   7, "ANH_UEBW",          "Trailer monitoring switch on",                                                 21,   1 },
        {   8, "SCHLUE_ABH_EIN",    "Key dependency on",                                                            22,   1 },
        {   9, "SP_PARK_SPERR",     "Mirror in park position",                                                      23,   1 },
        {  10, "ESH_POS_SP",        "Store seat longitudinal position for entry/exit aid",                          24,   1 },
        {  11, "SP_ANKL_SPERR",     "Fold mirror on vehicle locking",                                               25,   1 },
        {  12, "ESH_POS_STD",       "Seat adjustment travel for entry/exit aid to standard",                        28,   1 },
        {  13, "ESH_SITZ_EIN",      "Seat adjustment for entry/exit aid on",                                        29,   1 },
        {  14, "ESH_LENK_EIN",      "Steering column adjustment for entry/exit aid on",                             30,   1 },
        {  15, "ESH_AUTO_EIN",      "Entry aid / auto positioning on",                                              31,   1 },
        {  16, "SLF",               "Seek mode",                                                                    32,   1 },
        {  17, "RR_KM",             "Trip computer distance unit",                                                  33,   1 },
        {  18, "FL_OK",             "High beam enable permitted",                                                   34,   1 },
        {  19, "UFB_EIN",           "Surround lighting on",                                                         35,   1 },
        {  20, "SPRACHE",           "Language",                                                                     36,   4 },
        {  21, "STHL_EIN_KOMBI",    "Stationary heater / ventilation switch on",                                    40,   1 },
        {  22, "VWZ_AKT",           "Preset time activated (LED on)",                                               41,   1 },
        {  23, "VWZ_AUS_MFL",       "Preset time deactivated via steering wheel buttons (LED off)",                 42,   1 },
        {  24, "IRS_VDK_EIN",       "Interior protection on with soft top open",                                    46,   1 },
        {  25, "RDK_AKT",           "Tire pressure monitor activate",                                               47,   1 },
        {  26, "INLI_NLZ",          "Interior lighting delay time (s)",                                             48,   8 },
        {  27, "ABL_NLZ",           "Standing / fog light delay time (headlamp assist) (s)",                        56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A3 (ID: 0x00D4) - Time, Odometer & Range (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a3_00d4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "A_ZEIT",            "Current time (s)",                                                              0,  16 },
        {   1, "KM",                "Odometer reading (km)",                                                        16,  24 },
        {   2, "RW",                "Cruising range (km)",                                                          40,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A5 (ID: 0x01CA) - Multifunction Steering Wheel Button Events (25 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a5_01ca_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KI_STAT",           "Instrument cluster status",                                                     0,   8 },
        {   1, "BUTTON_4_2",        "Telephone End",                                                                 8,   1 },
        {   2, "BUTTON_4_1",        "Telephone Send",                                                                9,   1 },
        {   3, "BUTTON_3_2",        "Button \"-\"",                                                                 10,   1 },
        {   4, "BUTTON_3_1",        "Button \"+\"",                                                                 11,   1 },
        {   5, "BUTTON_2_2",        "Reserved",                                                                     12,   1 },
        {   6, "BUTTON_2_1",        "Reserved",                                                                     13,   1 },
        {   7, "BUTTON_1_2",        "Previous display",                                                             14,   1 },
        {   8, "BUTTON_1_1",        "Next display",                                                                 15,   1 },
        {   9, "BUTTON_8_2",        "Reserved",                                                                     16,   1 },
        {  10, "BUTTON_8_1",        "Reserved",                                                                     17,   1 },
        {  11, "BUTTON_7_2",        "Reserved",                                                                     18,   1 },
        {  12, "BUTTON_7_1",        "Reserved",                                                                     19,   1 },
        {  13, "BUTTON_6_2",        "Reserved",                                                                     20,   1 },
        {  14, "BUTTON_6_1",        "Reserved",                                                                     21,   1 },
        {  15, "BUTTON_5_2",        "Reserved",                                                                     22,   1 },
        {  16, "BUTTON_5_1",        "Reserved",                                                                     23,   1 },
        {  17, "PTT_4_2",           "Reserved",                                                                     24,   1 },
        {  18, "PTT_4_1",           "Reserved",                                                                     25,   1 },
        {  19, "PTT_3_2",           "Reserved",                                                                     26,   1 },
        {  20, "PTT_3_1",           "Reserved",                                                                     27,   1 },
        {  21, "PTT_2_2",           "Reserved",                                                                     28,   1 },
        {  22, "PTT_2_1",           "Reserved",                                                                     29,   1 },
        {  23, "PTT_1_2",           "Deactivate Linguatronic voice control",                                        30,   1 },
        {  24, "PTT_1_1",           "Activate Linguatronic voice control",                                          31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A6 (ID: 0x009E) - Key Identification & Mileage Redundant (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a6_009e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SCHLUE_ID_KI",      "Key identification for pre-filtering",                                          0,  32 },
        {   1, "KM_KI",             "Odometer reading (km)",                                                        32,  24 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A7 (ID: 0x0194) - Display Dimming & Trunk Limiter (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a7_0194_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DISP_DIMM",         "Display dimming (%)",                                                           0,   8 },
        {   1, "DATENF_MENU_AKT",   "Data radio menu activated",                                                     9,   1 },
        {   2, "HD_BEGRENZ",        "Trunk lid limiter on",                                                         10,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A8 (ID: 0x032A) - Special Vehicle Steering Wheel Button Events (25 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a8_032a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KI_STAT_MSS",       "Instrument cluster status (MSS)",                                               0,   8 },
        {   1, "BUTTON_4_2_MSS",    "Telephone End",                                                                 8,   1 },
        {   2, "BUTTON_4_1_MSS",    "Telephone Send",                                                                9,   1 },
        {   3, "BUTTON_3_2_MSS",    "Button \"-\"",                                                                 10,   1 },
        {   4, "BUTTON_3_1_MSS",    "Button \"+\"",                                                                 11,   1 },
        {   5, "BUTTON_2_2_MSS",    "Reserved",                                                                     12,   1 },
        {   6, "BUTTON_2_1_MSS",    "Reserved",                                                                     13,   1 },
        {   7, "BUTTON_1_2_MSS",    "Previous display",                                                             14,   1 },
        {   8, "BUTTON_1_1_MSS",    "Next display",                                                                 15,   1 },
        {   9, "BUTTON_8_2_MSS",    "Reserved",                                                                     16,   1 },
        {  10, "BUTTON_8_1_MSS",    "Reserved",                                                                     17,   1 },
        {  11, "BUTTON_7_2_MSS",    "Reserved",                                                                     18,   1 },
        {  12, "BUTTON_7_1_MSS",    "Reserved",                                                                     19,   1 },
        {  13, "BUTTON_6_2_MSS",    "Reserved",                                                                     20,   1 },
        {  14, "BUTTON_6_1_MSS",    "Reserved",                                                                     21,   1 },
        {  15, "BUTTON_5_2_MSS",    "Reserved",                                                                     22,   1 },
        {  16, "BUTTON_5_1_MSS",    "Reserved",                                                                     23,   1 },
        {  17, "PTT_4_2_MSS",       "Reserved",                                                                     24,   1 },
        {  18, "PTT_4_1_MSS",       "Reserved",                                                                     25,   1 },
        {  19, "PTT_3_2_MSS",       "Reserved",                                                                     26,   1 },
        {  20, "PTT_3_1_MSS",       "Reserved",                                                                     27,   1 },
        {  21, "PTT_2_2_MSS",       "Reserved",                                                                     28,   1 },
        {  22, "PTT_2_1_MSS",       "Reserved",                                                                     29,   1 },
        {  23, "PTT_1_2_MSS",       "Deactivate Linguatronic voice control",                                        30,   1 },
        {  24, "PTT_1_1_MSS",       "Activate Linguatronic voice control",                                          31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_KOMBI_AGW1 (ID: 0x01D0) - Transport Protocol Instrument Cluster to AGW (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_kombi_agw1_01d0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_KOMBI_AGW",      "Transport protocol Instrument Cluster to AGW",                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_KOMBI_MSS2 (ID: 0x0330) - Transport Protocol Instrument Cluster to MSS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_kombi_mss2_0330_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_KOMBI_MSS",      "Transport protocol Instrument Cluster to MSS",                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_KOMBI_TELEAID4 (ID: 0x03E1) - Transport Protocol Instrument Cluster to TeleAid (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_kombi_teleaid4_03e1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_KOMBI_TELEAID",  "Transport protocol Instrument Cluster to TeleAid",                              0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_KOMBI (ID: 0x0414) - Network Management Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_kombi_0414_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_KOMBI (ID: 0x04F4) - Diagnostic Response Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_kombi_04f4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_KOMBI (ID: 0x0774) - Application Interface Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_kombi_0774_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_A1 (ID: 0x0006) - Steering Column Switch Positions & Steering Angle (22 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_a1_0006_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SGH_EIN_LR",        "Horn switch on",                                                                0,   1 },
        {   1, "LHP_EIN",           "Headlamp flasher switch on",                                                    1,   1 },
        {   2, "FL_EIN",            "High beam switch on",                                                           2,   1 },
        {   3, "BLI_RE",            "Turn signal right",                                                             3,   1 },
        {   4, "BLI_LI",            "Turn signal left",                                                              4,   1 },
        {   5, "SCH_WI_2",          "Combination switch position III (speed 2)",                                     8,   1 },
        {   6, "SCH_WI_1",          "Combination switch position II (speed 1)",                                      9,   1 },
        {   7, "SCH_WI_INT",        "Combination switch position I (rain sensor mode)",                             10,   1 },
        {   8, "WASCHEN",           "Washing operated",                                                             11,   1 },
        {   9, "TIPP_WISCH",        "Touch wipe operated",                                                          12,   1 },
        {  10, "HECK_INT_MRM",      "Rear window intermittent wipe",                                                13,   1 },
        {  11, "HECK_WISCH_MRM",    "Rear window wipe / wash",                                                      14,   1 },
        {  12, "LS_ST_VER",         "Steering column is locked [0] (USA only)",                                     16,   1 },
        {  13, "ESH_EIN_MRM",       "Entry aid switch on (if rotary knob)",                                         17,   1 },
        {  14, "SBS_EIN",           "Voice control system on (push-to-talk)",                                       22,   1 },
        {  15, "SBS_AUS",           "Voice control system off (abort)",                                             23,   1 },
        {  16, "LW_PA_B",           "Steering angle parity bit (even parity)",                                      24,   1 },
        {  17, "LW_OV_B",           "Steering angle sensor: Overflow",                                              25,   1 },
        {  18, "LW_CF_B",           "Steering angle sensor: Code error",                                            26,   1 },
        {  19, "LW_INI_B",          "Steering angle sensor: Not initialized",                                       27,   1 },
        {  20, "LW_VZ_B",           "Steering angle sign",                                                          28,   1 },
        {  21, "LW_B",              "Steering angle (°)",                                                           29,  11 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_A2 (ID: 0x01A8) - Steering Wheel Rocker Switches (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_a2_01a8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "WIPPE_4_2",         "Button lower right downward",                                                   0,   1 },
        {   1, "WIPPE_4_1",         "Button lower right upward",                                                     1,   1 },
        {   2, "WIPPE_3_2",         "Button upper right downward",                                                   2,   1 },
        {   3, "WIPPE_3_1",         "Button upper right upward",                                                     3,   1 },
        {   4, "WIPPE_2_2",         "Button lower left downward",                                                    4,   1 },
        {   5, "WIPPE_2_1",         "Button lower left upward",                                                      5,   1 },
        {   6, "WIPPE_1_2",         "Button upper left downward",                                                    6,   1 },
        {   7, "WIPPE_1_1",         "Button upper left upward",                                                      7,   1 },
        {   8, "WIPPE_8_2",         "Reserved",                                                                      8,   1 },
        {   9, "WIPPE_8_1",         "Reserved",                                                                      9,   1 },
        {  10, "WIPPE_7_2",         "Reserved",                                                                     10,   1 },
        {  11, "WIPPE_7_1",         "Reserved",                                                                     11,   1 },
        {  12, "WIPPE_6_2",         "Reserved",                                                                     12,   1 },
        {  13, "WIPPE_6_1",         "Reserved",                                                                     13,   1 },
        {  14, "WIPPE_5_2",         "Reserved",                                                                     14,   1 },
        {  15, "WIPPE_5_1",         "Reserved",                                                                     15,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_A3 (ID: 0x0296) - Steering Column Lever Adjustment (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_a3_0296_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LS_ZUR_MRM",        "Steering column backward (towards driver)",                                     0,   1 },
        {   1, "LS_VOR_MRM",        "Steering column forward",                                                       1,   1 },
        {   2, "LS_AB_MRM",         "Steering column downward",                                                      2,   1 },
        {   3, "LS_AUF_MRM",        "Steering column upward",                                                        3,   1 },
        {   4, "LSVH_UN",           "Steering column adjustment lever rotated downward",                             4,   1 },
        {   5, "LSVH_OB",           "Steering column adjustment lever rotated upward",                               5,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_MRM (ID: 0x0415) - Network Management Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_mrm_0415_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MRM (ID: 0x04F5) - Diagnostic Response Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_mrm_04f5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_MRM (ID: 0x0775) - Application Interface Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_mrm_0775_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_MRM (ID: 0x07D5) - System Diagnostic Response Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_mrm_07d5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A1 (ID: 0x000A) - Front Lighting, Terminal Status & Defect Indicators (43 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a1_000a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KL_61_EIN",         "Terminal 61",                                                                   0,   1 },
        {   1, "SWA_AKT",           "Headlamp activation active",                                                    1,   1 },
        {   2, "RG_SAM_V",          "Reverse gear engaged (NSG/KSG only)",                                           2,   1 },
        {   3, "SPVS_ST_R",         "Mirror adjustment switch in right position",                                    3,   1 },
        {   4, "SGH_ST_EIN",        "Horn switched on",                                                              4,   1 },
        {   5, "FL_ST_EIN",         "High beam switched on",                                                         5,   1 },
        {   6, "NSW_ST_EIN",        "Fog lights switched on",                                                        6,   1 },
        {   7, "AFL_AKT",           "Exterior light activated by light sensor",                                      7,   1 },
        {   8, "NSL_EIN",           "Rear fog light switch on",                                                      8,   1 },
        {   9, "ABL_EIN",           "Low beam switch on",                                                           10,   1 },
        {  10, "STL_EIN",           "Standing light switch on",                                                     11,   1 },
        {  11, "PL_RE_EIN",         "Parking light right switch on",                                                13,   1 },
        {  12, "PL_LI_EIN",         "Parking light left switch on",                                                 14,   1 },
        {  13, "ZWP_LFT",           "Auxiliary water pump running",                                                 16,   1 },
        {  14, "KOMP_LFT",          "A/C compressor running",                                                       17,   1 },
        {  15, "HAS_KL",            "Parking brake applied (warning lamp)",                                         18,   1 },
        {  16, "KOMP_EIN",          "A/C compressor switched on",                                                   19,   1 },
        {  17, "KOMP_DEF",          "A/C compressor control current output defective",                              20,   1 },
        {  18, "DIAG_15_EIN",       "Terminal 15 activated via diagnosis",                                          21,   1 },
        {  19, "DIAG_15R_EIN",      "Terminal 15R activated via diagnosis",                                         22,   1 },
        {  20, "BFL_KL",            "Brake fluid level warning lamp",                                               24,   1 },
        {  21, "WWS_KL",            "Washer fluid level low warning lamp",                                          25,   1 },
        {  22, "KWS_KL",            "Coolant level low warning lamp",                                               26,   1 },
        {  23, "NSW_DEF_L",         "Fog lamp left defective",                                                      32,   1 },
        {  24, "FL_DEF_L",          "High beam left defective",                                                     33,   1 },
        {  25, "ABL_DEF_L",         "Low beam left defective",                                                      34,   1 },
        {  26, "PL_DEF_VL",         "Parking light front left defective",                                           35,   1 },
        {  27, "BLI_DEF_VL",        "Turn signal front left defective",                                             36,   1 },
        {  28, "SM_DEF_VL",         "Side marker front left defective",                                             37,   1 },
        {  29, "INSTR_AUS",         "Instrument illumination off",                                                  39,   1 },
        {  30, "NSW_DEF_R",         "Fog lamp right defective",                                                     40,   1 },
        {  31, "FL_DEF_R",          "High beam right defective",                                                    41,   1 },
        {  32, "ABL_DEF_R",         "Low beam right defective",                                                     42,   1 },
        {  33, "PL_DEF_VR",         "Parking light front right defective",                                          43,   1 },
        {  34, "BLI_DEF_VR",        "Turn signal front right defective",                                            44,   1 },
        {  35, "SM_DEF_VR",         "Side marker front right defective",                                            45,   1 },
        {  36, "LENK_OEL_KL",       "Msg: Steering fluid level low",                                                47,   1 },
        {  37, "BLI_ERS_VL",        "Backup turn signal front left active",                                         48,   1 },
        {  38, "PL_ERS_VL",         "Backup parking light front left active",                                       49,   1 },
        {  39, "DIAG_X4_F",         "Start Xenon 4 diagnosis procedure driver side",                                50,   1 },
        {  40, "BLI_ERS_VR",        "Backup turn signal front right active",                                        52,   1 },
        {  41, "PL_ERS_VR",         "Backup parking light front right active",                                      53,   1 },
        {  42, "DIAG_X4_B",         "Start Xenon 4 diagnosis procedure passenger side",                             54,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A2 (ID: 0x0017) - Outside Temperature & Refrigerant Pressure (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a2_0017_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "T_AUSSEN_B",        "Outside ambient air temperature (°C)",                                          0,   8 },
        {   1, "P_KAELTE",          "Refrigerant R134a pressure (bar)",                                              8,  16 },
        {   2, "T_KAELTE",          "Refrigerant R134a temperature (°C)",                                           24,  16 },
        {   3, "I_KOMP",            "Compressor main control valve current (mA)",                                   40,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A3 (ID: 0x0070) - Rain Sensor & Wiper Status (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a3_0070_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PARITY_SAM_V",      "Parity from bit 0 to 6 (even)",                                                 0,   1 },
        {   1, "KONFIG_RS",         "Rain sensor configuration",                                                     2,   3 },
        {   2, "KL_86_EIN",         "Washing operated",                                                              5,   1 },
        {   3, "KL_31B_EIN",        "Wiper outside park position",                                                   6,   1 },
        {   4, "RS_AKT",            "Rain sensor activated",                                                         7,   1 },
        {   5, "BYTE_KENN",         "Byte identifier",                                                               8,   2 },
        {   6, "DIAG_RS",           "Rain sensor diagnosis",                                                        10,   1 },
        {   7, "RS_NM",             "Rain sensor operation not possible",                                           11,   1 },
        {   8, "SAM_V_INIT",        "Front SAM initialization",                                                     12,   1 },
        {   9, "KL_86_RS",          "Washing operated",                                                             13,   1 },
        {  10, "KL_31B_RS",         "Wiper outside park position",                                                  14,   1 },
        {  11, "RS_INT",            "Rain sensor on/off (intermittent position)",                                   15,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A4 (ID: 0x02CC) - Outside Mirror Control Signals (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a4_02cc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SPVS_ST",           "Mirror adjustment switch position",                                             0,   1 },
        {   1, "SP_FAHREN",         "Outside mirror to driving position",                                            2,   1 },
        {   2, "SP_GARAGE",         "Outside mirror to garage position",                                             3,   1 },
        {   3, "SP_N_UN",           "Outside mirror glass downward",                                                 4,   1 },
        {   4, "SP_N_OB",           "Outside mirror glass upward",                                                   5,   1 },
        {   5, "SP_N_RE",           "Outside mirror glass to the right",                                             6,   1 },
        {   6, "SP_N_LI",           "Outside mirror glass to the left",                                              7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SAM_V (ID: 0x0402) - Network Management Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sam_v_0402_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SAM_V (ID: 0x04E2) - Diagnostic Response Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sam_v_04e2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SAM_V (ID: 0x0762) - Application Interface Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sam_v_0762_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A1 (ID: 0x0004) - Rear Lighting, Contact Switches & Alarm Status (47 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a1_0004_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PNK_AKT",           "Panic alarm is active",                                                         0,   1 },
        {   1, "KL54_RM",           "Terminal 54 hardware active",                                                   1,   1 },
        {   2, "HDK_BET",           "Trunk lid contact pressed",                                                     2,   1 },
        {   3, "HD_AUF",            "Trunk lid is open",                                                             3,   1 },
        {   4, "THR_AUF",           "Rear right door is open",                                                       4,   1 },
        {   5, "THL_AUF",           "Rear left door is open",                                                        5,   1 },
        {   6, "TVR_AUF",           "Front right door is open",                                                      6,   1 },
        {   7, "TVL_AUF",           "Front left door is open",                                                       7,   1 },
        {   8, "MOT_AUF",           "Engine hood is open",                                                           8,   1 },
        {   9, "HVST_BF_ENT",       "Passenger height adjuster unlocked",                                            9,   1 },
        {  10, "HVST_F_ENT",        "Driver height adjuster unlocked",                                              10,   1 },
        {  11, "HHS_ST_USPG",       "Heated rear window off due to undervoltage",                                   11,   1 },
        {  12, "HHS_ST_EIN",        "Heated rear window switched on",                                               12,   1 },
        {  13, "HSCHL_ST_SICH",     "Rear lock secured",                                                            13,   1 },
        {  14, "HSCHL_ZU",          "Rear lock in 90° position",                                                    14,   1 },
        {  15, "HD_SK_SAM_H",       "Trunk lid pawl operated",                                                      15,   1 },
        {  16, "EDW_IL_EIN",        "Anti-theft alarm interior light on",                                           16,   1 },
        {  17, "EDW_AKT",           "Anti-theft alarm armed",                                                       17,   1 },
        {  18, "EDW_IRS_AKT",       "Anti-theft alarm interior protection activate",                                18,   1 },
        {  19, "EDW_AAG_AKT",       "Anti-theft alarm trailer monitoring activate",                                 19,   1 },
        {  20, "EDW_ALARM",         "Anti-theft alarm triggered",                                                   20,   1 },
        {  21, "KZL_DEF_L",         "License plate lamp left defective",                                            24,   1 },
        {  22, "RFL_DEF_L",         "Reversing light left defective",                                               25,   1 },
        {  23, "BL_DEF_L",          "Brake light left defective",                                                   26,   1 },
        {  24, "SL_DEF_L",          "Tail light left defective",                                                    27,   1 },
        {  25, "BLI_DEF_HL",        "Turn signal rear left defective",                                              28,   1 },
        {  26, "NSL_DEF",           "Rear fog light defective",                                                     29,   1 },
        {  27, "BL3_DEF",           "3rd brake lamp defective",                                                     30,   1 },
        {  28, "KL_54_DEF",         "Terminal 54 fault",                                                            31,   1 },
        {  29, "KZL_DEF_R",         "License plate lamp right defective",                                           32,   1 },
        {  30, "RFL_DEF_R",         "Reversing light right defective",                                              33,   1 },
        {  31, "BL_DEF_R",          "Brake light right defective",                                                  34,   1 },
        {  32, "SL_DEF_R",          "Tail light right defective",                                                   35,   1 },
        {  33, "BLI_DEF_HR",        "Turn signal rear right defective",                                             36,   1 },
        {  34, "SM_DEF_HR",         "Side marker rear right defective",                                             38,   1 },
        {  35, "SM_DEF_HL",         "Side marker rear left defective",                                              39,   1 },
        {  36, "NSL_ERS",           "Backup rear fog light(s) active",                                              40,   1 },
        {  37, "SL_ERS_HR",         "Backup tail light rear right active",                                          42,   1 },
        {  38, "BLI_ERS_HR",        "Backup turn signal rear right active",                                         43,   1 },
        {  39, "SL_ERS_HL",         "Backup tail light rear left active",                                           46,   1 },
        {  40, "BLI_ERS_HL",        "Backup turn signal rear left active",                                          47,   1 },
        {  41, "HW_INT_AKT",        "Rear wiper in intermittent mode",                                              48,   1 },
        {  42, "GURT_KL_HW",        "Seat belt warning lamp switch on (for G463)",                                  49,   1 },
        {  43, "SRS_KL_HW",         "SRS warning lamp (for G463)",                                                  50,   1 },
        {  44, "HFS_SB_EIN",        "Trunk remote locator lighting switch on",                                      51,   1 },
        {  45, "HD_SCHLIESS_SAM_H", "Trunk close button operated",                                                  52,   1 },
        {  46, "HD_SICH_SAM_H",     "Trunk close & lock button operated",                                           53,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A2 (ID: 0x0090) - Fuel Tank Levels (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a2_0090_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TANK_FS_B",         "Fuel tank level (%)",                                                           0,   8 },
        {   1, "TANK_GE_RE",        "Fuel sender value right (%)",                                                   8,   8 },
        {   2, "TANK_GE_LI",        "Fuel sender value left (%)",                                                   16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A3 (ID: 0x000E) - Turn Signal & Hazard Flash Control (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a3_000e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "BLI_RE_EIN",        "Turn signal right switch on",                                                   0,   1 },
        {   1, "BLI_LI_EIN",        "Turn signal left switch on",                                                    1,   1 },
        {   2, "WARN_AKT",          "Hazard warning flashers active",                                                2,   1 },
        {   3, "HELL_BLINK",        "Turn signal light bright phase duration (ms)",                                  8,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A4 (ID: 0x0041) - Emergency Central Locking Release (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a4_0041_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SN1_SAM_H",         "Lock follower 1 (unlock)",                                                      1,   1 },
        {   1, "ZV_NOTOEFF",        "Central locking emergency opening",                                             7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A5 (ID: 0x0230) - Anti-Theft Alarm Lighting Control (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a5_0230_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NSW_EIN_EDW",       "Fog lamps switch on (anti-theft alarm)",                                        0,   1 },
        {   1, "ABL_EIN_EDW",       "Low beam switch on (anti-theft alarm)",                                         1,   1 },
        {   2, "SL_EIN_EDW",        "Tail light switch on (anti-theft alarm)",                                       2,   1 },
        {   3, "HELL_EDW",          "Light bright phase duration (anti-theft alarm) (ms)",                           8,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A6 (ID: 0x00CC) - Access Authorization Code (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a6_00cc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZBC_SAM_H",         "Access authorization code",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SAM_H (ID: 0x0403) - Network Management Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sam_h_0403_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SAM_H (ID: 0x04E3) - Diagnostic Response Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sam_h_04e3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SAM_H (ID: 0x0763) - Application Interface Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sam_h_0763_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: AAG_A1 (ID: 0x0130) - Trailer Detection & Lamp Status (9 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto aag_a1_0130_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ANH_ERK",           "Trailer detected",                                                              0,   1 },
        {   1, "AHK_NOK",           "Trailer hitch not locked",                                                      1,   1 },
        {   2, "ANHKL_54_DEF",      "Trailer terminal 54 fault",                                                     2,   1 },
        {   3, "EDW_ANH_ALM",       "Anti-theft alarm trailer monitoring alarm triggered",                           3,   1 },
        {   4, "ANHBL_DEF",         "Trailer brake light defective",                                                 7,   1 },
        {   5, "ANHSL_DEF_L",       "Trailer tail light left defective",                                             8,   1 },
        {   6, "ANHBLI_DEF_L",      "Trailer turn signal left defective",                                            9,   1 },
        {   7, "ANHSL_DEF_R",       "Trailer tail light right defective",                                           12,   1 },
        {   8, "ANHBLI_DEF_R",      "Trailer turn signal right defective",                                          13,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_AAG (ID: 0x0410) - Network Management Trailer Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_aag_0410_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_AAG (ID: 0x04F0) - Diagnostic Response Trailer Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_aag_04f0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_AAG (ID: 0x0770) - Application Interface Trailer Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_aag_0770_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_AAG (ID: 0x07D0) - System Diagnostic Response Trailer Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_aag_07d0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A1 (ID: 0x028C) - Driver Door Seat, Mirror & Steering Adjustment & Memory (30 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a1_028c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SVL_TGL",           "Front left seat - toggle bit",                                                  0,   1 },
        {   1, "SVL_HI_AB",         "Front left seat - rear height down",                                            4,   1 },
        {   2, "SVL_HI_AUF",        "Front left seat - rear height up",                                              5,   1 },
        {   3, "SVL_ZUR",           "Front left seat - longitudinal backward",                                       6,   1 },
        {   4, "SVL_VOR",           "Front left seat - longitudinal forward",                                        7,   1 },
        {   5, "SVL_KST_AB",        "Front left seat - headrest down",                                               8,   1 },
        {   6, "SVL_KST_AUF",       "Front left seat - headrest up",                                                 9,   1 },
        {   7, "SVL_VO_AB",         "Front left seat - front height down",                                          10,   1 },
        {   8, "SVL_VO_AUF",        "Front left seat - front height up",                                            11,   1 },
        {   9, "SVL_LE_ZUR",        "Front left seat - backrest backward",                                          12,   1 },
        {  10, "SVL_LE_VOR",        "Front left seat - backrest forward",                                           13,   1 },
        {  11, "LS_ZURUECK_LL",     "Steering column backward (towards driver)",                                    16,   1 },
        {  12, "LS_VOR_LL",         "Steering column forward",                                                      17,   1 },
        {  13, "LS_AB_LL",          "Steering column downward",                                                     18,   1 },
        {  14, "LS_AUF_LL",         "Steering column upward",                                                       19,   1 },
        {  15, "MVL_TGL",           "Memory front left - toggle bit",                                               24,   1 },
        {  16, "SPI_RE_SP",         "Store outside mirror right park position",                                     25,   1 },
        {  17, "MVL_P3_SP",         "Memory front left - store position 3",                                         26,   1 },
        {  18, "MVL_P2_SP",         "Memory front left - store position 2",                                         27,   1 },
        {  19, "MVL_P1_SP",         "Memory front left - store position 1",                                         28,   1 },
        {  20, "MVL_P3_EN",         "Memory front left - recall position 3",                                        29,   1 },
        {  21, "MVL_P2_EN",         "Memory front left - recall position 2",                                        30,   1 },
        {  22, "MVL_P1_EN",         "Memory front left - recall position 1",                                        31,   1 },
        {  23, "SPVS_BET_LL",       "Mirror adjustment switch operated",                                            32,   1 },
        {  24, "SPI_RE_FAHREN",     "Outside mirror right to driving position (not 203)",                           34,   1 },
        {  25, "SPI_RE_GARAGE",     "Outside mirror right to garage position (not 203)",                            35,   1 },
        {  26, "SPI_RE_N_UN",       "Outside mirror right downward (not 203)",                                      36,   1 },
        {  27, "SPI_RE_N_OB",       "Outside mirror right upward (not 203)",                                        37,   1 },
        {  28, "SPI_RE_N_RE",       "Outside mirror right to the right (not 203)",                                  38,   1 },
        {  29, "SPI_RE_N_LI",       "Outside mirror right to the left (not 203)",                                   39,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A2 (ID: 0x0044) - Driver Door Power Windows & Convenience Control (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a2_0044_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_TVL",           "Open / close rear right power window",                                          8,   1 },
        {   1, "FHL_TVL",           "Open / close rear left power window",                                           9,   1 },
        {   2, "FVR_TVL",           "Open / close front right power window",                                        10,   1 },
        {   3, "FVL_TVL",           "Open / close front left power window",                                         11,   1 },
        {   4, "SHD_TVL",           "Open / close sunroof / soft top",                                              12,   1 },
        {   5, "KB_RI_TVL",         "Convenience operation direction",                                              13,   1 },
        {   6, "KB_MOD_TVL",        "Convenience operation mode",                                                   14,   1 },
        {   7, "FHR_AS_LL",         "Rear right power window - auto close",                                         16,   1 },
        {   8, "FHR_MS_LL",         "Rear right power window - manual close",                                       17,   1 },
        {   9, "FHR_MOE_LL",        "Rear right power window - manual open",                                        18,   1 },
        {  10, "FHR_AOE_LL",        "Rear right power window - auto open",                                          19,   1 },
        {  11, "FHL_AS_LL",         "Rear left power window - auto close",                                          20,   1 },
        {  12, "FHL_MS_LL",         "Rear left power window - manual close",                                        21,   1 },
        {  13, "FHL_MOE_LL",        "Rear left power window - manual open",                                         22,   1 },
        {  14, "FHL_AOE_LL",        "Rear left power window - auto open",                                           23,   1 },
        {  15, "FVR_AS",            "Front right power window - auto close",                                        24,   1 },
        {  16, "FVR_MS",            "Front right power window - manual close",                                      25,   1 },
        {  17, "FVR_MOE",           "Front right power window - manual open",                                       26,   1 },
        {  18, "FVR_AOE",           "Front right power window - auto open",                                         27,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A3 (ID: 0x0018) - Driver Door Window Status & Lock Commands (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a3_0018_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SPVS_BF_LL",        "Mirror adjustment switch in right position (not 203)",                          0,   1 },
        {   1, "HFE_LL",            "Remote trunk lid release",                                                      1,   1 },
        {   2, "KISI_EIN_LL",       "Child safety lock on",                                                          2,   1 },
        {   3, "ZBLL_DEF",          "Auxiliary turn signal left defective",                                          3,   1 },
        {   4, "HFS_LL",            "Trunk lid remote closing",                                                      4,   1 },
        {   5, "FVL_NORM",          "Front left power window normalized",                                            8,   1 },
        {   6, "FVL_BLOCK",         "Front left power window blocked",                                               9,   1 },
        {   7, "FVL_AUF",           "Front left window open",                                                       10,   1 },
        {   8, "FVL_KZHB",          "Front left power window past short-stroke position",                           11,   1 },
        {   9, "FESTE_VL",          "Front left window position (1/armature rev)",                                  12,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A4 (ID: 0x00E8) - Driver Door Access Authorization Code (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a4_00e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZBC_TVL",           "Access authorization code",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_TVL (ID: 0x0408) - Network Management Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_tvl_0408_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TVL (ID: 0x04E8) - Diagnostic Response Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tvl_04e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_TVL (ID: 0x0768) - Application Interface Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_tvl_0768_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A1 (ID: 0x0290) - Passenger Door Seat, Mirror & Steering Adjustment & Memory (30 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a1_0290_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SVR_TGL",           "Front right seat - toggle bit",                                                 0,   1 },
        {   1, "SVR_HI_AB",         "Front right seat - rear height down",                                           4,   1 },
        {   2, "SVR_HI_AUF",        "Front right seat - rear height up",                                             5,   1 },
        {   3, "SVR_ZUR",           "Front right seat - longitudinal backward",                                      6,   1 },
        {   4, "SVR_VOR",           "Front right seat - longitudinal forward",                                       7,   1 },
        {   5, "SVR_KST_AB",        "Front right seat - headrest down",                                              8,   1 },
        {   6, "SVR_KST_AUF",       "Front right seat - headrest up",                                                9,   1 },
        {   7, "SVR_VO_AB",         "Front right seat - front height down",                                         10,   1 },
        {   8, "SVR_VO_AUF",        "Front right seat - front height up",                                           11,   1 },
        {   9, "SVR_LE_ZUR",        "Front right seat - backrest backward",                                         12,   1 },
        {  10, "SVR_LE_VOR",        "Front right seat - backrest forward",                                          13,   1 },
        {  11, "LS_ZURUECK_RL",     "Steering column backward (towards driver)",                                    16,   1 },
        {  12, "LS_VOR_RL",         "Steering column forward",                                                      17,   1 },
        {  13, "LS_AB_RL",          "Steering column downward",                                                     18,   1 },
        {  14, "LS_AUF_RL",         "Steering column upward",                                                       19,   1 },
        {  15, "MVR_TGL",           "Memory front right - toggle bit",                                              24,   1 },
        {  16, "SPI_LI_SP",         "Store outside mirror left park position",                                      25,   1 },
        {  17, "MVR_P3_SP",         "Memory front right - store position 3",                                        26,   1 },
        {  18, "MVR_P2_SP",         "Memory front right - store position 2",                                        27,   1 },
        {  19, "MVR_P1_SP",         "Memory front right - store position 1",                                        28,   1 },
        {  20, "MVR_P3_EN",         "Memory front right - recall position 3",                                       29,   1 },
        {  21, "MVR_P2_EN",         "Memory front right - recall position 2",                                       30,   1 },
        {  22, "MVR_P1_EN",         "Memory front right - recall position 1",                                       31,   1 },
        {  23, "SPVS_BET_RL",       "Mirror adjustment switch operated",                                            32,   1 },
        {  24, "SPI_LI_FAHREN",     "Outside mirror left to driving position (not 203)",                            34,   1 },
        {  25, "SPI_LI_GARAGE",     "Outside mirror left to garage position (not 203)",                             35,   1 },
        {  26, "SPI_LI_N_UN",       "Outside mirror left downward (not 203)",                                       36,   1 },
        {  27, "SPI_LI_N_OB",       "Outside mirror left upward (not 203)",                                         37,   1 },
        {  28, "SPI_LI_N_RE",       "Outside mirror left to the right (not 203)",                                   38,   1 },
        {  29, "SPI_LI_N_LI",       "Outside mirror left to the left (not 203)",                                    39,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A2 (ID: 0x0045) - Passenger Door Power Windows & Convenience Control (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a2_0045_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_TVR",           "Open / close rear right power window",                                          8,   1 },
        {   1, "FHL_TVR",           "Open / close rear left power window",                                           9,   1 },
        {   2, "FVR_TVR",           "Open / close front right power window",                                        10,   1 },
        {   3, "FVL_TVR",           "Open / close front left power window",                                         11,   1 },
        {   4, "SHD_TVR",           "Open / close sunroof / soft top",                                              12,   1 },
        {   5, "KB_RI_TVR",         "Convenience operation direction",                                              13,   1 },
        {   6, "KB_MOD_TVR",        "Convenience operation mode",                                                   14,   1 },
        {   7, "FHR_AS_RL",         "Rear right power window - auto close",                                         16,   1 },
        {   8, "FHR_MS_RL",         "Rear right power window - manual close",                                       17,   1 },
        {   9, "FHR_MOE_RL",        "Rear right power window - manual open",                                        18,   1 },
        {  10, "FHR_AOE_RL",        "Rear right power window - auto open",                                          19,   1 },
        {  11, "FHL_AS_RL",         "Rear left power window - auto close",                                          20,   1 },
        {  12, "FHL_MS_RL",         "Rear left power window - manual close",                                        21,   1 },
        {  13, "FHL_MOE_RL",        "Rear left power window - manual open",                                         22,   1 },
        {  14, "FHL_AOE_RL",        "Rear left power window - auto open",                                           23,   1 },
        {  15, "FVL_AS",            "Front left power window - auto close",                                         28,   1 },
        {  16, "FVL_MS",            "Front left power window - manual close",                                       29,   1 },
        {  17, "FVL_MOE",           "Front left power window - manual open",                                        30,   1 },
        {  18, "FVL_AOE",           "Front left power window - auto open",                                          31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A3 (ID: 0x0019) - Passenger Door Window Status & Lock Commands (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a3_0019_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SPVS_BF_RL",        "Mirror adjustment switch in left position (not 203)",                           0,   1 },
        {   1, "HFE_RL",            "Remote trunk lid release",                                                      1,   1 },
        {   2, "KISI_EIN_RL",       "Child safety lock on",                                                          2,   1 },
        {   3, "ZBLR_DEF",          "Auxiliary turn signal right defective",                                         3,   1 },
        {   4, "HFS_RL",            "Trunk lid remote closing",                                                      4,   1 },
        {   5, "FVR_NORM",          "Front right power window normalized",                                           8,   1 },
        {   6, "FVR_BLOCK",         "Front right power window blocked",                                              9,   1 },
        {   7, "FVR_AUF",           "Front right window open",                                                      10,   1 },
        {   8, "FVR_KZHB",          "Front right power window past short-stroke position",                          11,   1 },
        {   9, "FESTE_VR",          "Front right window position (1/armature rev)",                                 12,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A4 (ID: 0x00EC) - Passenger Door Access Authorization Code (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a4_00ec_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZBC_TVR",           "Access authorization code",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_TVR (ID: 0x040A) - Network Management Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_tvr_040a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TVR (ID: 0x04EA) - Diagnostic Response Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tvr_04ea_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_TVR (ID: 0x076A) - Application Interface Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_tvr_076a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: THL_A1 (ID: 0x009A) - Rear Left Door Power Window Status (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto thl_a1_009a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHL_NORM",          "Rear left power window normalized",                                             0,   1 },
        {   1, "FHL_BLOCK",         "Rear left power window blocked",                                                1,   1 },
        {   2, "FHL_AUF",           "Rear left window open",                                                         2,   1 },
        {   3, "FHL_KZHB",          "Rear left power window past short-stroke position",                             3,   1 },
        {   4, "FESTE_HL",          "Rear left window position (1/armature rev)",                                    4,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_THL (ID: 0x0409) - Network Management Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_thl_0409_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_THL (ID: 0x04E9) - Diagnostic Response Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_thl_04e9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_THL (ID: 0x0769) - Application Interface Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_thl_0769_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: THR_A1 (ID: 0x009C) - Rear Right Door Power Window Status (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto thr_a1_009c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_NORM",          "Rear right power window normalized",                                            0,   1 },
        {   1, "FHR_BLOCK",         "Rear right power window blocked",                                               1,   1 },
        {   2, "FHR_AUF",           "Rear right window open",                                                        2,   1 },
        {   3, "FHR_KZHB",          "Rear right power window past short-stroke position",                            3,   1 },
        {   4, "FESTE_HR",          "Rear right window position (1/armature rev)",                                   4,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_THR (ID: 0x040B) - Network Management Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_thr_040b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_THR (ID: 0x04EB) - Diagnostic Response Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_thr_04eb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_THR (ID: 0x076B) - Application Interface Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_thr_076b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: HFS_A1 (ID: 0x0078) - Trunk Remote Control Status & Lock Buttons (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto hfs_a1_0078_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HD_ST",             "Trunk lid status",                                                              0,   3 },
        {   1, "HD_SICH_HFS",       "Trunk lid close & lock operated",                                               3,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_HFS (ID: 0x0417) - Network Management Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_hfs_0417_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_HFS (ID: 0x04F7) - Diagnostic Response Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_hfs_04f7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_HFS (ID: 0x0777) - Application Interface Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_hfs_0777_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_HFS (ID: 0x07D7) - System Diagnostic Response Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_hfs_07d7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_VS (ID: 0x00FD) - Diagnostic Response Soft Top Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_vs_00fd_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VS_A1 (ID: 0x000B) - Soft Top Operation, Rollover Bar & Messages (24 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vs_a1_000b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FH_VR_KH",          "Front right power window short-stroke move",                                    0,   1 },
        {   1, "HD_SPERR",          "Trunk lid opening locked",                                                      1,   1 },
        {   2, "UERB_KL",           "Rollover bar warning lamp",                                                     2,   1 },
        {   3, "VDK_WARN",          "Warning tone switch on",                                                        3,   1 },
        {   4, "FH_VL_KH",          "Front left power window short-stroke move",                                     4,   1 },
        {   5, "VDK_AKTIV",         "Soft top is active",                                                            5,   1 },
        {   6, "VDK_STAT",          "Soft top status",                                                               6,   2 },
        {   7, "VS_M8",             "Msg 8: \"Soft top lowering\"",                                                  8,   1 },
        {   8, "VS_M7",             "Msg 7: \"Soft top in operation\"",                                              9,   1 },
        {   9, "VS_M6",             "Msg 6: \"Lower rollover bar\"",                                                10,   1 },
        {  10, "VS_M5",             "Msg 5: \"Deploy rollover bar\"",                                               11,   1 },
        {  11, "VS_M4",             "Msg 4: \"Lock soft top\"",                                                     12,   1 },
        {  12, "VS_M3",             "Msg 3: \"Start engine for soft top operation\"",                               13,   1 },
        {  13, "VS_M2",             "Msg 2: \"Close trunk partition / ski bag\"",                                   14,   1 },
        {  14, "VS_M1",             "Msg 1: \"Close trunk lid\"",                                                   15,   1 },
        {  15, "FHS_V_SPERR",       "Lock front power window switch commands",                                      16,   1 },
        {  16, "HD_SK_VS",          "Trunk lid pawl operated",                                                      17,   1 },
        {  17, "VS_M14",            "Msg 14: \"Please close ski bag\"",                                             18,   1 },
        {  18, "VS_M13",            "Msg 13: \"Soft top closed\"",                                                  19,   1 },
        {  19, "VS_M12",            "Msg 12: \"Soft top open\"",                                                    20,   1 },
        {  20, "VS_M11",            "Msg 11: \"Soft top operation, please wait\"",                                  21,   1 },
        {  21, "VS_M10",            "Msg 10: \"Soft top locked due to vehicle motion\"",                            22,   1 },
        {  22, "VS_M9",             "Msg 9: \"Soft top visit workshop\"",                                           23,   1 },
        {  23, "VDK_KL_ANF",        "Soft top warning lamp request",                                                30,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VS_A2 (ID: 0x0010) - Soft Top Window Commands & Rollover Detection (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vs_a2_0010_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FVR_VS",            "Open / close front right power window",                                         2,   1 },
        {   1, "FVL_VS",            "Open / close front left power window",                                          3,   1 },
        {   2, "FH_RI_VS",          "Power window operation direction",                                              5,   1 },
        {   3, "FH_MOD_VS",         "Power window run mode",                                                         6,   1 },
        {   4, "FH_LH_BEGR",        "Long-stroke travel limitation active",                                          7,   1 },
        {   5, "UEB_ERK",           "Rollover detected",                                                             8,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A1 (ID: 0x0014) - Overhead Control Panel Lighting, Ambient Sensors & Sunroof (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a1_0014_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AL_VL",             "Exit lamp front left switch on",                                                0,   1 },
        {   1, "AL_VR",             "Exit lamp front right switch on",                                               1,   1 },
        {   2, "AL_HL",             "Exit lamp rear left switch on",                                                 2,   1 },
        {   3, "AL_HR",             "Exit lamp rear right switch on",                                                3,   1 },
        {   4, "T_INNEN",           "Interior temperature (°C)",                                                     8,   8 },
        {   5, "AFL_ABL_EIN",       "Headlamp assist request: Low beam switch on",                                  16,   1 },
        {   6, "NACHT",             "Day / night signal",                                                           17,   1 },
        {   7, "LISR_DEF",          "Light sensor defective",                                                       18,   1 },
        {   8, "TUNNEL",            "Light sensor: Tunnel detected",                                                20,   1 },
        {   9, "DAEMMER",           "Light sensor: Twilight detected",                                              21,   1 },
        {  10, "INIT_LS_AKT",       "Light sensor initialization active",                                           23,   1 },
        {  11, "VERD_FANGPOS",      "Soft top in catch position",                                                   25,   1 },
        {  12, "VERD_ZU",           "Soft top locked (when W,S,C,CL=[1])",                                          26,   1 },
        {  13, "LADE_EIN",          "Charging lamp switch on",                                                      27,   1 },
        {  14, "SHD_ST",            "Sunroof status",                                                               29,   3 },
        {  15, "IRS_ALM",           "Interior protection triggered",                                                32,   1 },
        {  16, "IRS_GB",            "Interior protection glass breakage triggered",                                 33,   1 },
        {  17, "FRBL_VL",           "Footwell lighting front left on",                                              52,   1 },
        {  18, "FRBL_HELL",         "Footwell lighting brightness (%)",                                             56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A2 (ID: 0x0270) - Rain & Light Sensor Protocol (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a2_0270_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PARITY_DBE",        "Parity from bit 0 to bit 6 (even)",                                             0,   1 },
        {   1, "SENDE_WIEDER",      "Request not understood",                                                        2,   1 },
        {   2, "NEU_INI_FERTIG",    "Re-initialization complete",                                                    3,   1 },
        {   3, "FEHLER_RS",         "Error coding",                                                                  4,   3 },
        {   4, "WISCHER_EIN",       "Wiper request",                                                                 7,   1 },
        {   5, "KENN_RS",           "Rain sensor byte identifier",                                                   8,   1 },
        {   6, "RS_DEF",            "Rain sensor defective",                                                         9,   1 },
        {   7, "SCHWALL",           "Water splash detection",                                                       11,   1 },
        {   8, "WISCHER_ST",        "Wiper stages (stages)",                                                        12,   4 },
        {   9, "MESS_RLS_NV",       "Rain/light sensor measurements not available",                                 21,   1 },
        {  10, "KOM_RLS_FEHL",      "Communication error to rain/light sensor",                                     22,   1 },
        {  11, "DIAG_RLS_EIN",      "Rain/light sensor diagnosis on",                                               23,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A3 (ID: 0x02D4) - Automatic Dimming Rearview Mirror (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a3_02d4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SP_ABBLEND",        "Mirror automatic dimming level (stages)",                                       3,   5 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A4 (ID: 0x0174) - Sunroof Rain Close (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a4_0174_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SHD_ZU_RS",         "Close sunroof on rain",                                                         0,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_DBE (ID: 0x0407) - Network Management Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_dbe_0407_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_DBE (ID: 0x04E7) - Diagnostic Response Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_dbe_04e7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_DBE (ID: 0x0767) - Application Interface Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_dbe_0767_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: LRK_A1 (ID: 0x0288) - Heated Steering Wheel Status (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto lrk_a1_0288_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LHZG_LED_EIN",      "Heated steering wheel LED switch on",                                           1,   1 },
        {   1, "LRK_STOERG",        "Steering wheel LEDs flashing due to fault",                                     2,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: OBF_A1 (ID: 0x002C) - Upper Control Panel Switch Operations (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto obf_a1_002c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZV_VERRI_IS",       "Central locking lock (interior switch operated)",                               0,   1 },
        {   1, "ZV_ENTRI_IS",       "Central locking unlock (interior switch operated)",                             1,   1 },
        {   2, "HR_BET",            "Rear roller blind button operated",                                             2,   1 },
        {   3, "FKS_BET",           "Rear headrest raise/lower button operated",                                     4,   1 },
        {   4, "ESP_BET",           "ESP on/off button operated",                                                    6,   2 },
        {   5, "WBL_EIN",           "Hazard warning flashers on",                                                   10,   1 },
        {   6, "EDW_AS_ABW",        "Anti-theft alarm tow-away protection deselect",                                13,   1 },
        {   7, "EDW_IRS_ABW",       "Anti-theft alarm interior protection deselect",                                14,   1 },
        {   8, "EDW_HAND_AUF",      "Anti-theft alarm glove box contact triggered",                                 15,   1 },
        {   9, "PTS_BET",           "Parktronic button operated",                                                   16,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_OBF (ID: 0x0405) - Network Management Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_obf_0405_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_OBF (ID: 0x04E5) - Diagnostic Response Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_obf_04e5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_OBF (ID: 0x0765) - Application Interface Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_obf_0765_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: UBF_A1 (ID: 0x001A) - Lower Control Panel Switch Operations & ART Spacing (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ubf_a1_001a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ART_ABW_BET",       "Distronic (ART) distance warning on/off operated",                              2,   2 },
        {   1, "ASG_SPORT_BET",     "Automated transmission Sport mode on/off operated",                             4,   1 },
        {   2, "FU_FRSP_BET",       "Radio connection button operated",                                              7,   1 },
        {   3, "ART_ABSTAND",       "Distronic (ART) distance factor",                                               8,   8 },
        {   4, "BH_FUNK_BET",       "Authority radio button operated",                                              18,   1 },
        {   5, "STHL_BET",          "Auxiliary heater switch operated",                                             24,   1 },
        {   6, "LED_STH_DEF",       "Auxiliary heater LEDs defective",                                              26,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: UBF_A2 (ID: 0x035B) - Soft Top Operation Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ubf_a2_035b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VDK_ANF",           "Soft top operation request",                                                    0,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_UBF (ID: 0x041D) - Network Management Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_ubf_041d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_UBF (ID: 0x04FD) - Diagnostic Response Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ubf_04fd_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_UBF (ID: 0x077D) - Application Interface Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_ubf_077d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ARMADA_A1 (ID: 0x0012) - Restraint Systems & Occupant Classification (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto armada_a1_0012_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AKSE_EIN",          "Automatic child seat recognition lamp on",                                      0,   1 },
        {   1, "AKSE_BLINK",        "Automatic child seat recognition lamp flashing",                                1,   1 },
        {   2, "SRS_KL",            "SRS warning lamp",                                                              4,   1 },
        {   3, "SRS_BLINK",         "SRS warning lamp flashing",                                                     5,   1 },
        {   4, "SRS_SERV",          "SRS warning lamp (Service)",                                                    6,   1 },
        {   5, "SRS_WERK",          "SRS warning lamp (Workshop)",                                                   7,   1 },
        {   6, "KISI_ST",           "Child seat status",                                                            13,   3 },
        {   7, "GS_BF",             "Seat belt buckle passenger",                                                   16,   2 },
        {   8, "PSG_DETEC_FAST",    "Passenger detection fast",                                                     19,   2 },
        {   9, "SBE_BF",            "Seat occupancy classification passenger",                                      21,   3 },
        {  10, "GS_F",              "Seat belt buckle driver",                                                      24,   2 },
        {  11, "SBE_F",             "Seat occupancy classification driver",                                         29,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ARMADA_A2 (ID: 0x0040) - Crash Event Confirmation & Triggers (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto armada_a2_0040_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "CONF_CRASH",        "Confirmation bit for all crash events (toggling)",                              0,   1 },
        {   1, "CRASH_G",           "Rollover event 1",                                                              1,   1 },
        {   2, "CRASH_F",           "Frontal impact event 2",                                                        2,   1 },
        {   3, "CRASH_E",           "Rear impact event 2",                                                           3,   1 },
        {   4, "CRASH_D",           "Side impact event 1",                                                           4,   1 },
        {   5, "CRASH_C",           "Frontal impact event 5",                                                        5,   1 },
        {   6, "CRASH_B",           "Rear impact event 1",                                                           6,   1 },
        {   7, "CRASH_A",           "Frontal impact event 1",                                                        7,   1 },
        {   8, "X_CRASH",           "Any crash event",                                                               8,   1 },
        {   9, "CRASH_O",           "Crash event TBD",                                                               9,   1 },
        {  10, "CRASH_N",           "Crash event TBD",                                                              10,   1 },
        {  11, "CRASH_M",           "Crash event TBD",                                                              11,   1 },
        {  12, "CRASH_L",           "Side impact event 2",                                                          12,   1 },
        {  13, "CRASH_K",           "Rear impact event 3",                                                          13,   1 },
        {  14, "CRASH_I",           "Rollover event 3",                                                             14,   1 },
        {  15, "CRASH_H",           "Rollover event 2",                                                             15,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_ARMADA (ID: 0x041C) - Network Management Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_armada_041c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ARMADA (ID: 0x04FC) - Diagnostic Response Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_armada_04fc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_ARMADA (ID: 0x077C) - Application Interface Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_armada_077c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: WSS_A1 (ID: 0x02A4) - Weight Sensing System Classification (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto wss_a1_02a4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PSG_DETECT_FAST",   "Passenger detection fast",                                                      2,   2 },
        {   1, "WSS_PSG_FAULT",     "Weight classification passenger fault",                                         4,   1 },
        {   2, "WSS_PSG",           "Weight classification passenger",                                               5,   3 },
        {   3, "WSS_ID",            "WSS identification",                                                           16,   8 },
        {   4, "WSS_ANZ",           "WSS display in instrument cluster",                                            30,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_WSS (ID: 0x0426) - Network Management Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_wss_0426_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_WSS (ID: 0x04C6) - Diagnostic Response Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_wss_04c6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_WSS (ID: 0x0706) - Application Interface Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_wss_0706_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SF_A1 (ID: 0x01AC) - Driver Seat Position & Backrest Status (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sf_a1_01ac_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MF_MAN_SP1",        "Store manually set position (redundant)",                                       0,   1 },
        {   1, "LE_F_ENT",          "Driver seat backrest unlocked",                                                 4,   1 },
        {   2, "SF_POS",            "Driver seat position (steps)",                                                  8,   8 },
        {   3, "SF_EA_DEF",         "Default position for driver seat entry/exit aid (steps)",                      16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SF_A2 (ID: 0x02D0) - Driver Seat Memory & Entry/Exit Positioning (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sf_a2_02d0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ESH_AKT",           "Move to entry/exit position active",                                            0,   1 },
        {   1, "AUTO_AKT",          "Move to driving position active",                                               1,   1 },
        {   2, "MF_MAN_SP",         "Store manually set position",                                                   2,   1 },
        {   3, "ESH_AUTO_REST",     "Execute mirror positioning",                                                    5,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SF (ID: 0x040C) - Network Management Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sf_040c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SF (ID: 0x04EC) - Diagnostic Response Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sf_04ec_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SF (ID: 0x076C) - Application Interface Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sf_076c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_SF (ID: 0x07CC) - System Diagnostic Response Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_sf_07cc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SB_A1 (ID: 0x01B6) - Passenger Seat Backrest Status (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sb_a1_01b6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LE_B_ENT",          "Passenger seat backrest unlocked",                                              4,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SB (ID: 0x040D) - Network Management Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sb_040d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SB (ID: 0x04ED) - Diagnostic Response Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sb_04ed_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SB (ID: 0x076D) - Application Interface Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sb_076d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_SB (ID: 0x07CD) - System Diagnostic Response Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_sb_07cd_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: STH_A1 (ID: 0x0094) - Auxiliary Heater Operation & Status (8 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sth_a1_0094_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "STHL_EIN",          "Stationary heater / ventilation switch on",                                     0,   1 },
        {   1, "STHL_AUS",          "Stationary heater / ventilation switch off",                                    1,   1 },
        {   2, "STLFT_EIN",         "Switch on ventilation manually",                                                2,   1 },
        {   3, "STHZG_EIN",         "Switch on heating manually",                                                    3,   1 },
        {   4, "GEBLAESE_EIN",      "Vehicle blower switch on",                                                      4,   1 },
        {   5, "VWZ_MENUE",         "Open preset time menu",                                                         5,   1 },
        {   6, "ZH_LED_EIN",        "Auxiliary heater LED switch on",                                                6,   1 },
        {   7, "SENDLM_EIN",        "Transmitter learning mode on",                                                  7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_STH (ID: 0x0419) - Network Management Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sth_0419_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_STH (ID: 0x04F9) - Diagnostic Response Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sth_04f9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_STH (ID: 0x0779) - Application Interface Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sth_0779_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_A1 (ID: 0x0030) - Climate Control Compressor, Fan & Flaps (20 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_a1_0030_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HHS_EIN",           "Heated rear window switch on",                                                  0,   1 },
        {   1, "EC_AKT",            "Economy (EC) mode active",                                                      1,   1 },
        {   2, "IFG_EIN",           "Interior temperature sensor blower switch on",                                  2,   1 },
        {   3, "ZWP_EIN",           "Auxiliary water pump switch on",                                                3,   1 },
        {   4, "ZH_EIN_OK",         "Auxiliary heater enable permitted",                                             4,   1 },
        {   5, "LL_DZA",            "Idle speed increase for cooling capacity",                                      5,   1 },
        {   6, "HEIZEN",            "Auxiliary heater heating",                                                      6,   1 },
        {   7, "LUEFTEN",           "Auxiliary heater ventilating",                                                  7,   1 },
        {   8, "NLFTS",             "Engine cooling fan target speed (%)",                                           8,   8 },
        {   9, "M_KOMP",            "A/C compressor torque absorption (Nm)",                                        16,   8 },
        {  10, "KOMP_STELL",        "A/C compressor control signal (%)",                                            24,   8 },
        {  11, "FSB_HZG_EIN",       "Heated windshield switch on (for G463)",                                       32,   1 },
        {  12, "G_ANF_KUEHL_KLA",   "Shift point increase on cooling deficit",                                      33,   1 },
        {  13, "GEB_LSTG",          "Blower power (%)",                                                             40,   8 },
        {  14, "UL_AKT_KLA",        "Air recirculation active",                                                     48,   1 },
        {  15, "G_ANF_KLA",         "Shift point increase on heating deficit",                                      49,   1 },
        {  16, "LKO_VORN",          "Ventilation flap position top",                                                50,   2 },
        {  17, "LKM_VORN",          "Ventilation flap position center",                                             52,   2 },
        {  18, "LKU_VORN",          "Ventilation flap position bottom",                                             54,   2 },
        {  19, "T_INNEN_KLA",       "Interior temperature (°C)",                                                    56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_A2 (ID: 0x0250) - Climate Control Window & Roof Requests (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_a2_0250_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_KLA",           "Open / close rear right power window",                                          0,   1 },
        {   1, "FHL_KLA",           "Open / close rear left power window",                                           1,   1 },
        {   2, "FVR_KLA",           "Open / close front right power window",                                         2,   1 },
        {   3, "FVL_KLA",           "Open / close front left power window",                                          3,   1 },
        {   4, "SHD_KLA",           "Open / close sunroof / soft top",                                               4,   1 },
        {   5, "KB_RI_KLA",         "Convenience operation direction",                                               5,   1 },
        {   6, "KB_MOD_KLA",        "Convenience operation mode",                                                    6,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_A3 (ID: 0x00F1) - Heating Demand & Temperature (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_a3_00f1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HZL_ANF",           "Heating output demand (%)",                                                     0,   8 },
        {   1, "T_AUSSEN_WM",       "Outside temperature for thermal management (°C)",                               8,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_KLA (ID: 0x0411) - Network Management Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_kla_0411_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_KLA (ID: 0x04F1) - Diagnostic Response Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_kla_04f1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_KLA (ID: 0x0771) - Application Interface Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_kla_0771_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A1 (ID: 0x0015) - Special Vehicle Sirens, Lights & Audio (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a1_0015_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FL_EIN_MSS",        "High beam switch on",                                                           0,   1 },
        {   1, "NSW_EIN_MSS",       "Fog lamps switch on",                                                           1,   1 },
        {   2, "SGH_EIN_MSS",       "Horn switch on",                                                                2,   1 },
        {   3, "BLI_EIN_MSS",       "MSS flasher on",                                                                3,   1 },
        {   4, "T_HIRU_EIN",        "Send taxi distress call",                                                       4,   1 },
        {   5, "UMLUFT_MSS",        "Close air recirculation flaps",                                                 5,   1 },
        {   6, "IL_EIN_MSS",        "Interior lighting switch on",                                                   6,   1 },
        {   7, "ZV_ZU_MSS",         "Close central locking",                                                         7,   1 },
        {   8, "BLI_HELL_MSS",      "Turn signal bright phase duration (ms)",                                        8,   8 },
        {   9, "SGH_AN_MSS",        "Horn sounding duration (ms)",                                                  16,   8 },
        {  10, "NSW_HELL_MSS",      "Fog lamp bright phase duration (ms)",                                          24,   8 },
        {  11, "FL_HELL_MSS",       "High beam bright phase duration (ms)",                                         32,   8 },
        {  12, "FU_FRSP_AKT",       "Radio connection active",                                                      40,   1 },
        {  13, "ANF_FBAS",          "Request composite video input (Comand)",                                       41,   1 },
        {  14, "BHF_LED_AKT",       "Authority radio LED switch on",                                                42,   1 },
        {  15, "ANF_ZT",            "Numeric keypad request (Headunit)",                                            43,   1 },
        {  16, "LADEN_AKT",         "220V external charging connected",                                             44,   1 },
        {  17, "AUDIO_MUTE2",       "Mute audio source on radio reception",                                         45,   1 },
        {  18, "AUDIO_MUTE1",       "Audio mute",                                                                   46,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A2 (ID: 0x01AE) - Roof Sign, Oxygen & Mist System Status (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a2_01ae_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DZ_KL",             "Roof sign indicator lamp on",                                                   0,   1 },
        {   1, "DZ_LA_DEF",         "Roof sign lamp defective",                                                      1,   1 },
        {   2, "DZ_PRF",            "Check roof sign (open circuit)",                                                2,   1 },
        {   3, "DZ_DEF",            "Roof sign defective (short circuit)",                                           3,   1 },
        {   4, "MSS_USPG",          "MSS detects undervoltage",                                                      4,   1 },
        {   5, "MSS_ALM",           "Silent alarm triggered",                                                        5,   1 },
        {   6, "NOTALM_DEF",        "Emergency alarm system defective",                                              6,   1 },
        {   7, "MSS_EE_DEF",        "MSS electrics defective",                                                       7,   1 },
        {   8, "FNK_STAT",          "Radio set status",                                                              8,   1 },
        {   9, "MSS_SUMMER",        "Buzzer control in instrument cluster",                                          9,   1 },
        {  10, "O2_AUS",            "Fresh air system out of operation",                                            10,   1 },
        {  11, "O2_AKT",            "Fresh air system activated",                                                   11,   1 },
        {  12, "O2_LEER",           "Fresh air bottle empty",                                                       12,   1 },
        {  13, "NEBEL_AUS",         "Mist system out of operation",                                                 13,   1 },
        {  14, "NEBEL_AKT",         "Mist system activated",                                                        14,   1 },
        {  15, "NP_LEER",           "Mist cartridge empty",                                                         15,   1 },
        {  16, "MSS_USPG_MO",       "MSS detects undervoltage while engine running",                                21,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A3 (ID: 0x01CE) - Special Vehicle Destination Coordinates (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a3_01ce_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DEST_LAT",          "GPS latitude (- means south) (ms)",                                             0,  32 },
        {   1, "DEST_LONG",         "GPS longitude (- means west) (ms)",                                            32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A4 (ID: 0x0248) - Emergency Window & Sunroof Close (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a4_0248_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_ALARM",         "Close rear right power window",                                                 0,   1 },
        {   1, "FHL_ALARM",         "Close rear left power window",                                                  1,   1 },
        {   2, "FVR_ALARM",         "Close front right power window",                                                2,   1 },
        {   3, "FVL_ALARM",         "Close front left power window",                                                 3,   1 },
        {   4, "SHD_ALARM",         "Close sunroof",                                                                 4,   1 },
        {   5, "RI_ALARM",          "Alarm actuation direction",                                                     5,   1 },
        {   6, "MOD_ALARM",         "Sunroof / window run mode",                                                     6,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSSK_A1 (ID: 0x0046) - Special Vehicle Switch Console 1 (21 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mssk_a1_0046_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SGH_EIN_K",         "Horn switch on",                                                                0,   1 },
        {   1, "LHP_EIN_K",         "Headlamp flasher switch on",                                                    1,   1 },
        {   2, "FL_EIN_K",          "High beam switch on",                                                           2,   1 },
        {   3, "BLI_RE_K",          "Turn signal right",                                                             3,   1 },
        {   4, "BLI_LI_K",          "Turn signal left",                                                              4,   1 },
        {   5, "SCH_WI_2_K",        "MSSK switch in position III (speed 2)",                                         8,   1 },
        {   6, "SCH_WI_1_K",        "MSSK switch in position II (speed 1)",                                          9,   1 },
        {   7, "SCH_WI_INT_K",      "MSSK switch in position I (rain sensor mode)",                                 10,   1 },
        {   8, "WASCHEN_K",         "Washing switch on",                                                            11,   1 },
        {   9, "TIPP_WISCH_K",      "Touch wipe switch on",                                                         12,   1 },
        {  10, "HECK_INT_K",        "Rear window intermittent wipe",                                                16,   1 },
        {  11, "HECK_WISCH_K",      "Rear window wipe / wash",                                                      17,   1 },
        {  12, "WBL_EIN_K",         "Hazard warning flashers on",                                                   18,   1 },
        {  13, "STL_EIN_K",         "Standing light switch on",                                                     19,   1 },
        {  14, "ABL_EIN_K",         "Low beam switch on",                                                           20,   1 },
        {  15, "NSW_EIN_K",         "Fog lamps switch on",                                                          21,   1 },
        {  16, "NSL_EIN_K",         "Rear fog light switch on",                                                     22,   1 },
        {  17, "SHD_STOP",          "Sunroof stop",                                                                 24,   1 },
        {  18, "HD_AUF_K",          "Open lifting roof",                                                            25,   1 },
        {  19, "SHD_ZU_K",          "Close sunroof",                                                                26,   1 },
        {  20, "SHD_AUF_K",         "Open sunroof",                                                                 27,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSSK_A2 (ID: 0x0208) - Special Vehicle Passenger Seat Control (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mssk_a2_0208_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SBF_K_TGL",         "Passenger seat - toggle bit",                                                   0,   1 },
        {   1, "SBF_ZUR_K",         "Passenger seat - longitudinal backward",                                        6,   1 },
        {   2, "SBF_VOR_K",         "Passenger seat - longitudinal forward",                                         7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_MSS_AGW3 (ID: 0x01D8) - Transport Protocol MSS to AGW (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_mss_agw3_01d8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_MSS_AGW3",       "Transport protocol MSS to AGW",                                                 0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_MSS_KOMBI2 (ID: 0x01A6) - Transport Protocol MSS to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_mss_kombi2_01a6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_MSS_KOMBI",      "Transport protocol MSS to Instrument Cluster",                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_MSS (ID: 0x0406) - Network Management Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_mss_0406_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MSS (ID: 0x04E6) - Diagnostic Response Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_mss_04e6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_MSS (ID: 0x0766) - Application Interface Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_mss_0766_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: PTS_A1 (ID: 0x02B0) - Parktronic System State (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto pts_a1_02b0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PTS_ST_AUS",        "Parktronic system completely switched off",                                     0,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_PTS (ID: 0x0413) - Network Management Parktronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_pts_0413_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_PTS (ID: 0x04F3) - Diagnostic Response Parktronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_pts_04f3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_PTS (ID: 0x0773) - Application Interface Parktronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_pts_0773_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TPM_A1 (ID: 0x02FF) - Tire Pressures & Warning Status (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tpm_a1_02ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TPM_MsgDisp_Rq",    "TPM message display request",                                                   0,   4 },
        {   1, "TPM_Stat",          "TPM state",                                                                     4,   4 },
        {   2, "TPM_Disp_On_Rq",    "Tire pressure module display on request",                                       9,   1 },
        {   3, "TPM_MalfLmp_On_Rq", "Tire pressure module malfunction lamp on request",                             10,   1 },
        {   4, "Tire_LHOM",         "Tire in limp-home operation mode",                                             11,   1 },
        {   5, "TPM_IndLmp_On_Rq",  "Tire pressure module indication lamp on request",                              12,   1 },
        {   6, "TPM_WarnDisp_Rq",   "TPM warning display request",                                                  13,   3 },
        {   7, "Tire_Spr",          "Spare tire",                                                                   16,   1 },
        {   8, "Tire_RR",           "Tire rear right",                                                              17,   1 },
        {   9, "Tire_RL",           "Tire rear left",                                                               18,   1 },
        {  10, "Tire_FR",           "Tire front right",                                                             19,   1 },
        {  11, "Tire_FL",           "Tire front left",                                                              20,   1 },
        {  12, "TirePress_FL",      "Tire pressure front left (bar)",                                               24,   8 },
        {  13, "TirePress_FR",      "Tire pressure front right (bar)",                                              32,   8 },
        {  14, "TirePress_RL",      "Tire pressure rear left (bar)",                                                40,   8 },
        {  15, "TirePress_RR",      "Tire pressure rear right (bar)",                                               48,   8 },
        {  16, "TirePress_Spr",     "Tire pressure spare tire (bar)",                                               56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_TPM (ID: 0x0418) - Network Management Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_tpm_0418_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TPM (ID: 0x04F8) - Diagnostic Response Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tpm_04f8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_TPM",          "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_TPM (ID: 0x0778) - Application Interface Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_tpm_0778_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MESS_TPM1 (ID: 0x0607) - Tire Pressure Measurement Data (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mess_tpm1_0607_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS_TPM1",         "Measurement data",                                                              0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: AGW_A3 (ID: 0x0138) - Audio Gateway Keypad & Function Keys (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto agw_a3_0138_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "CANCEL_HU",         "Reject digit input (Headunit)",                                                 1,   1 },
        {   1, "STERN_HU",          "Star button operated",                                                          2,   1 },
        {   2, "RAUTE_HU",          "Hash button operated",                                                          3,   1 },
        {   3, "ZIFFER_HU",         "Numeric keypad (Headunit)",                                                     4,   4 },
        {   4, "FUNKTIONSTASTEN",   "Function keys",                                                                10,   3 },
        {   5, "AKT_SYS",           "Active application",                                                           13,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GPS_A1 (ID: 0x0338) - GPS Latitude & Longitude (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gps_a1_0338_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LATITUDE",          "GPS latitude (+ = North) (ms)",                                                 0,  32 },
        {   1, "LONGITUDE",         "GPS longitude (+ = East) (ms)",                                                32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GPS_A2 (ID: 0x0339) - GPS Date & UTC Time (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gps_a2_0339_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UTC_YEARS",         "UTC years (years)",                                                             0,  16 },
        {   1, "UTC_MONTHS",        "UTC months (months)",                                                          16,   8 },
        {   2, "UTC_DAYS",          "UTC days (days)",                                                              24,   8 },
        {   3, "UTC_HOURS",         "UTC hours (h)",                                                                32,   8 },
        {   4, "UTC_MINUTES",       "UTC minutes (min)",                                                            40,   8 },
        {   5, "UTC_SECONDS",       "UTC seconds (s)",                                                              48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GPS_A3 (ID: 0x033A) - GPS Dynamics, Fix & Dilution of Precision (13 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gps_a3_033a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_SPEED",         "GPS speed (cm/s)",                                                              0,  16 },
        {   1, "GPS_HEADING",       "GPS heading (0° = North) (°)",                                                 16,   8 },
        {   2, "GPS_HEIGHT",        "GPS height (m)",                                                               24,   8 },
        {   3, "GPS_FIX",           "GPS fix",                                                                      37,   3 },
        {   4, "POS_AVLB",          "GPS position available",                                                       40,   1 },
        {   5, "DIFF_POS_AVLB",     "Differential GPS position data available",                                     41,   1 },
        {   6, "DEAD_RCK",          "Dead reckoning available",                                                     42,   1 },
        {   7, "IDG",               "Inside digitized area (on map)",                                               43,   1 },
        {   8, "FDG",               "Fully digitized area",                                                         44,   1 },
        {   9, "MDM",               "Matched to digital map",                                                       45,   1 },
        {  10, "CALI",              "Calibrated",                                                                   46,   1 },
        {  11, "V_DOP",             "Vertical dilution of position",                                                48,   8 },
        {  12, "H_DOP",             "Horizontal dilution of position",                                              56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_AGW_KOMBI1 (ID: 0x01A4) - Transport Protocol AGW to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_agw_kombi1_01a4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_AGW_KOMBI",      "Transport protocol AGW to Instrument Cluster",                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_AGW_MSS3 (ID: 0x0334) - Transport Protocol AGW to MSS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_agw_mss3_0334_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_AGW_MSS3",       "Transport protocol AGW to MSS",                                                 0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_AGW_TELEAID6 (ID: 0x03E3) - Transport Protocol AGW to TeleAid (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_agw_teleaid6_03e3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_AGW_TELEAID",    "Transport protocol AGW to TeleAid",                                             0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_AGW (ID: 0x0416) - Network Management Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_agw_0416_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_AGW (ID: 0x04F6) - Diagnostic Response Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_agw_04f6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST0 (ID: 0x0680) - Diagnostic Response MOST Gateway 0 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most0_0680_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST1 (ID: 0x0681) - Diagnostic Response MOST Gateway 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most1_0681_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST10 (ID: 0x068A) - Diagnostic Response MOST Gateway 10 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most10_068a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST11 (ID: 0x068B) - Diagnostic Response MOST Gateway 11 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most11_068b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST12 (ID: 0x068C) - Diagnostic Response MOST Gateway 12 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most12_068c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST13 (ID: 0x068D) - Diagnostic Response MOST Gateway 13 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most13_068d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST14 (ID: 0x068E) - Diagnostic Response MOST Gateway 14 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most14_068e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST15 (ID: 0x068F) - Diagnostic Response MOST Gateway 15 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most15_068f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST2 (ID: 0x0682) - Diagnostic Response MOST Gateway 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most2_0682_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST3 (ID: 0x0683) - Diagnostic Response MOST Gateway 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most3_0683_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST4 (ID: 0x0684) - Diagnostic Response MOST Gateway 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most4_0684_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST5 (ID: 0x0685) - Diagnostic Response MOST Gateway 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most5_0685_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST6 (ID: 0x0686) - Diagnostic Response MOST Gateway 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most6_0686_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST7 (ID: 0x0687) - Diagnostic Response MOST Gateway 7 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most7_0687_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST8 (ID: 0x0688) - Diagnostic Response MOST Gateway 8 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most8_0688_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST9 (ID: 0x0689) - Diagnostic Response MOST Gateway 9 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most9_0689_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_NAVI (ID: 0x058A) - Diagnostic Response Navigation System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_navi_058a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_NAVI",         "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_AGW (ID: 0x0776) - Application Interface Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_agw_0776_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_AGW (ID: 0x07D6) - System Diagnostic Response Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_agw_07d6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_ICANI (ID: 0x043B) - Network Management CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_icani_043b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Network management",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ICANI (ID: 0x04BB) - Diagnostic Response CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_icani_04bb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_ICANI",        "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_ICANI (ID: 0x05BB) - System Diagnostic Response CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_icani_05bb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS_ICANI",       "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A3 (ID: 0x01B8) - Odometer Value Legacy (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a3_01b8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KM_EZS_ALT",        "Odometer reading (up to MY 2002/1) (km)",                                       0,  24 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FDSVL (ID: 0x04FE) - Diagnostic Response Dynamic Seat Front Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fdsvl_04fe_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FDSVR (ID: 0x04FF) - Diagnostic Response Dynamic Seat Front Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fdsvr_04ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_LRK (ID: 0x04EF) - Diagnostic Response Heated Steering Wheel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_lrk_04ef_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_PFDS (ID: 0x04EE) - Diagnostic Response Pneumatic Dynamic Seat (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_pfds_04ee_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SHZ (ID: 0x04FB) - Diagnostic Response Seat Heating (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_shz_04fb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TLM (ID: 0x04FA) - Diagnostic Response Telematics Control Unit (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tlm_04fa_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_AAG (ID: 0x072F) - Application Interface to Trailer Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_aag_072f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_AGW (ID: 0x05C9) - Application Interface to Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_agw_05c9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_ARMADA (ID: 0x06A3) - Application Interface to Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_armada_06a3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_DBE (ID: 0x0678) - Application Interface to Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_dbe_0678_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_EZS (ID: 0x04DF) - Application Interface to Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_ezs_04df_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_HFS (ID: 0x0568) - Application Interface to Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_hfs_0568_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_KLA (ID: 0x078E) - Application Interface to Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_kla_078e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_KOMBI (ID: 0x05AB) - Application Interface to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_kombi_05ab_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_MRM (ID: 0x06C1) - Application Interface to Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_mrm_06c1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_MSS (ID: 0x0720) - Application Interface to Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_mss_0720_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_OBF (ID: 0x06BA) - Application Interface to Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_obf_06ba_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_PTS (ID: 0x072C) - Application Interface to Parktronic (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_pts_072c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SAM_H (ID: 0x057C) - Application Interface to Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sam_h_057c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SAM_V (ID: 0x067D) - Application Interface to Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sam_v_067d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SB (ID: 0x06B2) - Application Interface to Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sb_06b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SF (ID: 0x06B3) - Application Interface to Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sf_06b3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_STH (ID: 0x073F) - Application Interface to Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sth_073f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_THL (ID: 0x0756) - Application Interface to Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_thl_0756_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_THR (ID: 0x0754) - Application Interface to Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_thr_0754_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_TPM (ID: 0x06A7) - Application Interface to Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_tpm_06a7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_TVL (ID: 0x06D7) - Application Interface to Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_tvl_06d7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_TVR (ID: 0x06C0) - Application Interface to Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_tvr_06c0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_UBF (ID: 0x0722) - Application Interface to Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_ubf_0722_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_WSS (ID: 0x06B7) - Application Interface to Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_wss_06b7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EZS (ID: 0x04E0) - Diagnostic Request Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ezs_04e0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // CAN-B Message Registry (272 Message(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto w209_messages = std::to_array<const can_message_spec>
    ({
        { 0x0000, false, 8, "UNKNOWN", "Central Locking & Terminal Status", unknown_0000_signals.data(), unknown_0000_signals.size() },
        { 0x010A, false, 8, "EZS_A10", "Rear Wheel Speed & Direction", ezs_a10_010a_signals.data(), ezs_a10_010a_signals.size() },
        { 0x0016, false, 8, "EZS_A11", "Battery Voltage", ezs_a11_0016_signals.data(), ezs_a11_0016_signals.size() },
        { 0x0002, false, 8, "EZS_A2", "Front Wheel Speed & Engine Data", ezs_a2_0002_signals.data(), ezs_a2_0002_signals.size() },
        { 0x0058, false, 8, "EZS_A4", "Key Identification & Mileage", ezs_a4_0058_signals.data(), ezs_a4_0058_signals.size() },
        { 0x001F, false, 8, "EZS_A5", "Special Equipment & Features Coding", ezs_a5_001f_signals.data(), ezs_a5_001f_signals.size() },
        { 0x001E, false, 8, "EZS_A6", "Model Year & TPM Status", ezs_a6_001e_signals.data(), ezs_a6_001e_signals.size() },
        { 0x0003, false, 8, "EZS_A7", "Engine Status, Wheel Speed & Lamp Faults", ezs_a7_0003_signals.data(), ezs_a7_0003_signals.size() },
        { 0x0390, false, 8, "EZS_A8", "Climate Control Configuration & Characteristics", ezs_a8_0390_signals.data(), ezs_a8_0390_signals.size() },
        { 0x00B2, false, 8, "EZS_A9", "Vehicle Identification Number (VIN)", ezs_a9_00b2_signals.data(), ezs_a9_00b2_signals.size() },
        { 0x01B2, false, 8, "KG_A1", "Keyless Go Instrument Cluster Messages", kg_a1_01b2_signals.data(), kg_a1_01b2_signals.size() },
        { 0x0050, false, 8, "KG_A2", "Keyless Go Convenience Window & Roof Control", kg_a2_0050_signals.data(), kg_a2_0050_signals.size() },
        { 0x018D, false, 8, "TELEAID_A2", "TeleAid Authorization & Status", teleaid_a2_018d_signals.data(), teleaid_a2_018d_signals.size() },
        { 0x03E5, false, 8, "TELEAID_POS1", "TeleAid GPS Latitude & Longitude", teleaid_pos1_03e5_signals.data(), teleaid_pos1_03e5_signals.size() },
        { 0x03E6, false, 8, "TELEAID_POS2", "TeleAid GPS Velocity, Heading & Altitude", teleaid_pos2_03e6_signals.data(), teleaid_pos2_03e6_signals.size() },
        { 0x03E7, false, 8, "TELEAID_POS3", "TeleAid GPS Date & UTC Time", teleaid_pos3_03e7_signals.data(), teleaid_pos3_03e7_signals.size() },
        { 0x03E8, false, 8, "TELEAID_POS4", "TeleAid Dead Reckoning / Map Matching Position", teleaid_pos4_03e8_signals.data(), teleaid_pos4_03e8_signals.size() },
        { 0x03E9, false, 8, "TELEAID_POS5", "TeleAid Satellite & Map Matching Status", teleaid_pos5_03e9_signals.data(), teleaid_pos5_03e9_signals.size() },
        { 0x0005, false, 8, "GW_C_B7", "Gateway CAN-C to CAN-B Front Right Wheel Speed", gw_c_b7_0005_signals.data(), gw_c_b7_0005_signals.size() },
        { 0x0209, false, 8, "TP_TELEAID_AGW6", "Transport Protocol TeleAid to AGW", tp_teleaid_agw6_0209_signals.data(), tp_teleaid_agw6_0209_signals.size() },
        { 0x01A1, false, 8, "TP_TELEAID_KOMBI4", "Transport Protocol TeleAid to Instrument Cluster", tp_teleaid_kombi4_01a1_signals.data(), tp_teleaid_kombi4_01a1_signals.size() },
        { 0x0400, false, 8, "NM_EZS", "Network Management Ignition Switch", nm_ezs_0400_signals.data(), nm_ezs_0400_signals.size() },
        { 0x05FF, false, 8, "D_RS_EZS", "Diagnostic Response Ignition Switch", d_rs_ezs_05ff_signals.data(), d_rs_ezs_05ff_signals.size() },
        { 0x0760, false, 8, "SG_APPL_EZS", "Application Interface Ignition Switch", sg_appl_ezs_0760_signals.data(), sg_appl_ezs_0760_signals.size() },
        { 0x0730, false, 8, "D_RQ_AAG", "Diagnostic Request Trailer Control", d_rq_aag_0730_signals.data(), d_rq_aag_0730_signals.size() },
        { 0x05D6, false, 8, "D_RQ_AGW", "Diagnostic Request Audio Gateway", d_rq_agw_05d6_signals.data(), d_rq_agw_05d6_signals.size() },
        { 0x06BC, false, 8, "D_RQ_ARMADA", "Diagnostic Request Restraint System", d_rq_armada_06bc_signals.data(), d_rq_armada_06bc_signals.size() },
        { 0x0667, false, 8, "D_RQ_DBE", "Diagnostic Request Overhead Control Panel", d_rq_dbe_0667_signals.data(), d_rq_dbe_0667_signals.size() },
        { 0x06BE, false, 8, "D_RQ_FDSVL", "Diagnostic Request Dynamic Seat Front Left", d_rq_fdsvl_06be_signals.data(), d_rq_fdsvl_06be_signals.size() },
        { 0x06BF, false, 8, "D_RQ_FDSVR", "Diagnostic Request Dynamic Seat Front Right", d_rq_fdsvr_06bf_signals.data(), d_rq_fdsvr_06bf_signals.size() },
        { 0x0577, false, 8, "D_RQ_HFS", "Diagnostic Request Trunk Remote Control", d_rq_hfs_0577_signals.data(), d_rq_hfs_0577_signals.size() },
        { 0x07DA, false, 8, "D_RQ_ICANI", "Diagnostic Request CAN Interface", d_rq_icani_07da_signals.data(), d_rq_icani_07da_signals.size() },
        { 0x0791, false, 8, "D_RQ_KLA", "Diagnostic Request Climate Control", d_rq_kla_0791_signals.data(), d_rq_kla_0791_signals.size() },
        { 0x05B4, false, 8, "D_RQ_KOMBI", "Diagnostic Request Instrument Cluster", d_rq_kombi_05b4_signals.data(), d_rq_kombi_05b4_signals.size() },
        { 0x06AF, false, 8, "D_RQ_LRK", "Diagnostic Request Heated Steering Wheel", d_rq_lrk_06af_signals.data(), d_rq_lrk_06af_signals.size() },
        { 0x0640, false, 8, "D_RQ_MOST0", "Diagnostic Request MOST Gateway 0", d_rq_most0_0640_signals.data(), d_rq_most0_0640_signals.size() },
        { 0x0641, false, 8, "D_RQ_MOST1", "Diagnostic Request MOST Gateway 1", d_rq_most1_0641_signals.data(), d_rq_most1_0641_signals.size() },
        { 0x064A, false, 8, "D_RQ_MOST10", "Diagnostic Request MOST Gateway 10", d_rq_most10_064a_signals.data(), d_rq_most10_064a_signals.size() },
        { 0x064B, false, 8, "D_RQ_MOST11", "Diagnostic Request MOST Gateway 11", d_rq_most11_064b_signals.data(), d_rq_most11_064b_signals.size() },
        { 0x064C, false, 8, "D_RQ_MOST12", "Diagnostic Request MOST Gateway 12", d_rq_most12_064c_signals.data(), d_rq_most12_064c_signals.size() },
        { 0x064D, false, 8, "D_RQ_MOST13", "Diagnostic Request MOST Gateway 13", d_rq_most13_064d_signals.data(), d_rq_most13_064d_signals.size() },
        { 0x064E, false, 8, "D_RQ_MOST14", "Diagnostic Request MOST Gateway 14", d_rq_most14_064e_signals.data(), d_rq_most14_064e_signals.size() },
        { 0x064F, false, 8, "D_RQ_MOST15", "Diagnostic Request MOST Gateway 15", d_rq_most15_064f_signals.data(), d_rq_most15_064f_signals.size() },
        { 0x0642, false, 8, "D_RQ_MOST2", "Diagnostic Request MOST Gateway 2", d_rq_most2_0642_signals.data(), d_rq_most2_0642_signals.size() },
        { 0x0643, false, 8, "D_RQ_MOST3", "Diagnostic Request MOST Gateway 3", d_rq_most3_0643_signals.data(), d_rq_most3_0643_signals.size() },
        { 0x0644, false, 8, "D_RQ_MOST4", "Diagnostic Request MOST Gateway 4", d_rq_most4_0644_signals.data(), d_rq_most4_0644_signals.size() },
        { 0x0645, false, 8, "D_RQ_MOST5", "Diagnostic Request MOST Gateway 5", d_rq_most5_0645_signals.data(), d_rq_most5_0645_signals.size() },
        { 0x0646, false, 8, "D_RQ_MOST6", "Diagnostic Request MOST Gateway 6", d_rq_most6_0646_signals.data(), d_rq_most6_0646_signals.size() },
        { 0x0647, false, 8, "D_RQ_MOST7", "Diagnostic Request MOST Gateway 7", d_rq_most7_0647_signals.data(), d_rq_most7_0647_signals.size() },
        { 0x0648, false, 8, "D_RQ_MOST8", "Diagnostic Request MOST Gateway 8", d_rq_most8_0648_signals.data(), d_rq_most8_0648_signals.size() },
        { 0x0649, false, 8, "D_RQ_MOST9", "Diagnostic Request MOST Gateway 9", d_rq_most9_0649_signals.data(), d_rq_most9_0649_signals.size() },
        { 0x06D5, false, 8, "D_RQ_MRM", "Diagnostic Request Steering Column Module", d_rq_mrm_06d5_signals.data(), d_rq_mrm_06d5_signals.size() },
        { 0x0726, false, 8, "D_RQ_MSS", "Diagnostic Request Special Vehicle Control", d_rq_mss_0726_signals.data(), d_rq_mss_0726_signals.size() },
        { 0x054A, false, 8, "D_RQ_NAVI", "Diagnostic Request Navigation System", d_rq_navi_054a_signals.data(), d_rq_navi_054a_signals.size() },
        { 0x06A5, false, 8, "D_RQ_OBF", "Diagnostic Request Upper Control Panel", d_rq_obf_06a5_signals.data(), d_rq_obf_06a5_signals.size() },
        { 0x072E, false, 8, "D_RQ_PFDS", "Diagnostic Request Pneumatic Dynamic Seat", d_rq_pfds_072e_signals.data(), d_rq_pfds_072e_signals.size() },
        { 0x0733, false, 8, "D_RQ_PTS", "Diagnostic Request Parktronic", d_rq_pts_0733_signals.data(), d_rq_pts_0733_signals.size() },
        { 0x0563, false, 8, "D_RQ_SAM_H", "Diagnostic Request Rear SAM", d_rq_sam_h_0563_signals.data(), d_rq_sam_h_0563_signals.size() },
        { 0x0662, false, 8, "D_RQ_SAM_V", "Diagnostic Request Front SAM", d_rq_sam_v_0662_signals.data(), d_rq_sam_v_0662_signals.size() },
        { 0x06AD, false, 8, "D_RQ_SB", "Diagnostic Request Passenger Seat Module", d_rq_sb_06ad_signals.data(), d_rq_sb_06ad_signals.size() },
        { 0x06AC, false, 8, "D_RQ_SF", "Diagnostic Request Driver Seat Module", d_rq_sf_06ac_signals.data(), d_rq_sf_06ac_signals.size() },
        { 0x057B, false, 8, "D_RQ_SHZ", "Diagnostic Request Seat Heating", d_rq_shz_057b_signals.data(), d_rq_shz_057b_signals.size() },
        { 0x0739, false, 8, "D_RQ_STH", "Diagnostic Request Auxiliary Heater", d_rq_sth_0739_signals.data(), d_rq_sth_0739_signals.size() },
        { 0x0749, false, 8, "D_RQ_THL", "Diagnostic Request Rear Left Door Module", d_rq_thl_0749_signals.data(), d_rq_thl_0749_signals.size() },
        { 0x074B, false, 8, "D_RQ_THR", "Diagnostic Request Rear Right Door Module", d_rq_thr_074b_signals.data(), d_rq_thr_074b_signals.size() },
        { 0x05DA, false, 8, "D_RQ_TLM", "Diagnostic Request Telematics Control Unit", d_rq_tlm_05da_signals.data(), d_rq_tlm_05da_signals.size() },
        { 0x06B8, false, 8, "D_RQ_TPM", "Diagnostic Request Tire Pressure Monitor", d_rq_tpm_06b8_signals.data(), d_rq_tpm_06b8_signals.size() },
        { 0x06C8, false, 8, "D_RQ_TVL", "Diagnostic Request Driver Door Module", d_rq_tvl_06c8_signals.data(), d_rq_tvl_06c8_signals.size() },
        { 0x06CA, false, 8, "D_RQ_TVR", "Diagnostic Request Passenger Door Module", d_rq_tvr_06ca_signals.data(), d_rq_tvr_06ca_signals.size() },
        { 0x073D, false, 8, "D_RQ_UBF", "Diagnostic Request Lower Control Panel", d_rq_ubf_073d_signals.data(), d_rq_ubf_073d_signals.size() },
        { 0x06A8, false, 8, "D_RQ_WSS", "Diagnostic Request Weight Sensing System", d_rq_wss_06a8_signals.data(), d_rq_wss_06a8_signals.size() },
        { 0x001C, false, 8, "D_RQ_GLOBAL", "Global Diagnostic Request", d_rq_global_001c_signals.data(), d_rq_global_001c_signals.size() },
        { 0x000C, false, 8, "KOMBI_A1", "Instrument Cluster Display & Convenience Status", kombi_a1_000c_signals.data(), kombi_a1_000c_signals.size() },
        { 0x00D4, false, 8, "KOMBI_A3", "Time, Odometer & Range", kombi_a3_00d4_signals.data(), kombi_a3_00d4_signals.size() },
        { 0x01CA, false, 8, "KOMBI_A5", "Multifunction Steering Wheel Button Events", kombi_a5_01ca_signals.data(), kombi_a5_01ca_signals.size() },
        { 0x009E, false, 8, "KOMBI_A6", "Key Identification & Mileage Redundant", kombi_a6_009e_signals.data(), kombi_a6_009e_signals.size() },
        { 0x0194, false, 8, "KOMBI_A7", "Display Dimming & Trunk Limiter", kombi_a7_0194_signals.data(), kombi_a7_0194_signals.size() },
        { 0x032A, false, 8, "KOMBI_A8", "Special Vehicle Steering Wheel Button Events", kombi_a8_032a_signals.data(), kombi_a8_032a_signals.size() },
        { 0x01D0, false, 8, "TP_KOMBI_AGW1", "Transport Protocol Instrument Cluster to AGW", tp_kombi_agw1_01d0_signals.data(), tp_kombi_agw1_01d0_signals.size() },
        { 0x0330, false, 8, "TP_KOMBI_MSS2", "Transport Protocol Instrument Cluster to MSS", tp_kombi_mss2_0330_signals.data(), tp_kombi_mss2_0330_signals.size() },
        { 0x03E1, false, 8, "TP_KOMBI_TELEAID4", "Transport Protocol Instrument Cluster to TeleAid", tp_kombi_teleaid4_03e1_signals.data(), tp_kombi_teleaid4_03e1_signals.size() },
        { 0x0414, false, 8, "NM_KOMBI", "Network Management Instrument Cluster", nm_kombi_0414_signals.data(), nm_kombi_0414_signals.size() },
        { 0x04F4, false, 8, "D_RS_KOMBI", "Diagnostic Response Instrument Cluster", d_rs_kombi_04f4_signals.data(), d_rs_kombi_04f4_signals.size() },
        { 0x0774, false, 8, "SG_APPL_KOMBI", "Application Interface Instrument Cluster", sg_appl_kombi_0774_signals.data(), sg_appl_kombi_0774_signals.size() },
        { 0x0006, false, 8, "MRM_A1", "Steering Column Switch Positions & Steering Angle", mrm_a1_0006_signals.data(), mrm_a1_0006_signals.size() },
        { 0x01A8, false, 8, "MRM_A2", "Steering Wheel Rocker Switches", mrm_a2_01a8_signals.data(), mrm_a2_01a8_signals.size() },
        { 0x0296, false, 8, "MRM_A3", "Steering Column Lever Adjustment", mrm_a3_0296_signals.data(), mrm_a3_0296_signals.size() },
        { 0x0415, false, 8, "NM_MRM", "Network Management Steering Column Module", nm_mrm_0415_signals.data(), nm_mrm_0415_signals.size() },
        { 0x04F5, false, 8, "D_RS_MRM", "Diagnostic Response Steering Column Module", d_rs_mrm_04f5_signals.data(), d_rs_mrm_04f5_signals.size() },
        { 0x0775, false, 8, "SG_APPL_MRM", "Application Interface Steering Column Module", sg_appl_mrm_0775_signals.data(), sg_appl_mrm_0775_signals.size() },
        { 0x07D5, false, 8, "SD_RS_MRM", "System Diagnostic Response Steering Column Module", sd_rs_mrm_07d5_signals.data(), sd_rs_mrm_07d5_signals.size() },
        { 0x000A, false, 8, "SAM_V_A1", "Front Lighting, Terminal Status & Defect Indicators", sam_v_a1_000a_signals.data(), sam_v_a1_000a_signals.size() },
        { 0x0017, false, 8, "SAM_V_A2", "Outside Temperature & Refrigerant Pressure", sam_v_a2_0017_signals.data(), sam_v_a2_0017_signals.size() },
        { 0x0070, false, 8, "SAM_V_A3", "Rain Sensor & Wiper Status", sam_v_a3_0070_signals.data(), sam_v_a3_0070_signals.size() },
        { 0x02CC, false, 8, "SAM_V_A4", "Outside Mirror Control Signals", sam_v_a4_02cc_signals.data(), sam_v_a4_02cc_signals.size() },
        { 0x0402, false, 8, "NM_SAM_V", "Network Management Front SAM", nm_sam_v_0402_signals.data(), nm_sam_v_0402_signals.size() },
        { 0x04E2, false, 8, "D_RS_SAM_V", "Diagnostic Response Front SAM", d_rs_sam_v_04e2_signals.data(), d_rs_sam_v_04e2_signals.size() },
        { 0x0762, false, 8, "SG_APPL_SAM_V", "Application Interface Front SAM", sg_appl_sam_v_0762_signals.data(), sg_appl_sam_v_0762_signals.size() },
        { 0x0004, false, 8, "SAM_H_A1", "Rear Lighting, Contact Switches & Alarm Status", sam_h_a1_0004_signals.data(), sam_h_a1_0004_signals.size() },
        { 0x0090, false, 8, "SAM_H_A2", "Fuel Tank Levels", sam_h_a2_0090_signals.data(), sam_h_a2_0090_signals.size() },
        { 0x000E, false, 8, "SAM_H_A3", "Turn Signal & Hazard Flash Control", sam_h_a3_000e_signals.data(), sam_h_a3_000e_signals.size() },
        { 0x0041, false, 8, "SAM_H_A4", "Emergency Central Locking Release", sam_h_a4_0041_signals.data(), sam_h_a4_0041_signals.size() },
        { 0x0230, false, 8, "SAM_H_A5", "Anti-Theft Alarm Lighting Control", sam_h_a5_0230_signals.data(), sam_h_a5_0230_signals.size() },
        { 0x00CC, false, 8, "SAM_H_A6", "Access Authorization Code", sam_h_a6_00cc_signals.data(), sam_h_a6_00cc_signals.size() },
        { 0x0403, false, 8, "NM_SAM_H", "Network Management Rear SAM", nm_sam_h_0403_signals.data(), nm_sam_h_0403_signals.size() },
        { 0x04E3, false, 8, "D_RS_SAM_H", "Diagnostic Response Rear SAM", d_rs_sam_h_04e3_signals.data(), d_rs_sam_h_04e3_signals.size() },
        { 0x0763, false, 8, "SG_APPL_SAM_H", "Application Interface Rear SAM", sg_appl_sam_h_0763_signals.data(), sg_appl_sam_h_0763_signals.size() },
        { 0x0130, false, 8, "AAG_A1", "Trailer Detection & Lamp Status", aag_a1_0130_signals.data(), aag_a1_0130_signals.size() },
        { 0x0410, false, 8, "NM_AAG", "Network Management Trailer Control", nm_aag_0410_signals.data(), nm_aag_0410_signals.size() },
        { 0x04F0, false, 8, "D_RS_AAG", "Diagnostic Response Trailer Control", d_rs_aag_04f0_signals.data(), d_rs_aag_04f0_signals.size() },
        { 0x0770, false, 8, "SG_APPL_AAG", "Application Interface Trailer Control", sg_appl_aag_0770_signals.data(), sg_appl_aag_0770_signals.size() },
        { 0x07D0, false, 8, "SD_RS_AAG", "System Diagnostic Response Trailer Control", sd_rs_aag_07d0_signals.data(), sd_rs_aag_07d0_signals.size() },
        { 0x028C, false, 8, "TVL_A1", "Driver Door Seat, Mirror & Steering Adjustment & Memory", tvl_a1_028c_signals.data(), tvl_a1_028c_signals.size() },
        { 0x0044, false, 8, "TVL_A2", "Driver Door Power Windows & Convenience Control", tvl_a2_0044_signals.data(), tvl_a2_0044_signals.size() },
        { 0x0018, false, 8, "TVL_A3", "Driver Door Window Status & Lock Commands", tvl_a3_0018_signals.data(), tvl_a3_0018_signals.size() },
        { 0x00E8, false, 8, "TVL_A4", "Driver Door Access Authorization Code", tvl_a4_00e8_signals.data(), tvl_a4_00e8_signals.size() },
        { 0x0408, false, 8, "NM_TVL", "Network Management Driver Door Module", nm_tvl_0408_signals.data(), nm_tvl_0408_signals.size() },
        { 0x04E8, false, 8, "D_RS_TVL", "Diagnostic Response Driver Door Module", d_rs_tvl_04e8_signals.data(), d_rs_tvl_04e8_signals.size() },
        { 0x0768, false, 8, "SG_APPL_TVL", "Application Interface Driver Door Module", sg_appl_tvl_0768_signals.data(), sg_appl_tvl_0768_signals.size() },
        { 0x0290, false, 8, "TVR_A1", "Passenger Door Seat, Mirror & Steering Adjustment & Memory", tvr_a1_0290_signals.data(), tvr_a1_0290_signals.size() },
        { 0x0045, false, 8, "TVR_A2", "Passenger Door Power Windows & Convenience Control", tvr_a2_0045_signals.data(), tvr_a2_0045_signals.size() },
        { 0x0019, false, 8, "TVR_A3", "Passenger Door Window Status & Lock Commands", tvr_a3_0019_signals.data(), tvr_a3_0019_signals.size() },
        { 0x00EC, false, 8, "TVR_A4", "Passenger Door Access Authorization Code", tvr_a4_00ec_signals.data(), tvr_a4_00ec_signals.size() },
        { 0x040A, false, 8, "NM_TVR", "Network Management Passenger Door Module", nm_tvr_040a_signals.data(), nm_tvr_040a_signals.size() },
        { 0x04EA, false, 8, "D_RS_TVR", "Diagnostic Response Passenger Door Module", d_rs_tvr_04ea_signals.data(), d_rs_tvr_04ea_signals.size() },
        { 0x076A, false, 8, "SG_APPL_TVR", "Application Interface Passenger Door Module", sg_appl_tvr_076a_signals.data(), sg_appl_tvr_076a_signals.size() },
        { 0x009A, false, 8, "THL_A1", "Rear Left Door Power Window Status", thl_a1_009a_signals.data(), thl_a1_009a_signals.size() },
        { 0x0409, false, 8, "NM_THL", "Network Management Rear Left Door Module", nm_thl_0409_signals.data(), nm_thl_0409_signals.size() },
        { 0x04E9, false, 8, "D_RS_THL", "Diagnostic Response Rear Left Door Module", d_rs_thl_04e9_signals.data(), d_rs_thl_04e9_signals.size() },
        { 0x0769, false, 8, "SG_APPL_THL", "Application Interface Rear Left Door Module", sg_appl_thl_0769_signals.data(), sg_appl_thl_0769_signals.size() },
        { 0x009C, false, 8, "THR_A1", "Rear Right Door Power Window Status", thr_a1_009c_signals.data(), thr_a1_009c_signals.size() },
        { 0x040B, false, 8, "NM_THR", "Network Management Rear Right Door Module", nm_thr_040b_signals.data(), nm_thr_040b_signals.size() },
        { 0x04EB, false, 8, "D_RS_THR", "Diagnostic Response Rear Right Door Module", d_rs_thr_04eb_signals.data(), d_rs_thr_04eb_signals.size() },
        { 0x076B, false, 8, "SG_APPL_THR", "Application Interface Rear Right Door Module", sg_appl_thr_076b_signals.data(), sg_appl_thr_076b_signals.size() },
        { 0x0078, false, 8, "HFS_A1", "Trunk Remote Control Status & Lock Buttons", hfs_a1_0078_signals.data(), hfs_a1_0078_signals.size() },
        { 0x0417, false, 8, "NM_HFS", "Network Management Trunk Remote Control", nm_hfs_0417_signals.data(), nm_hfs_0417_signals.size() },
        { 0x04F7, false, 8, "D_RS_HFS", "Diagnostic Response Trunk Remote Control", d_rs_hfs_04f7_signals.data(), d_rs_hfs_04f7_signals.size() },
        { 0x0777, false, 8, "SG_APPL_HFS", "Application Interface Trunk Remote Control", sg_appl_hfs_0777_signals.data(), sg_appl_hfs_0777_signals.size() },
        { 0x07D7, false, 8, "SD_RS_HFS", "System Diagnostic Response Trunk Remote Control", sd_rs_hfs_07d7_signals.data(), sd_rs_hfs_07d7_signals.size() },
        { 0x00FD, false, 8, "D_RS_VS", "Diagnostic Response Soft Top Control", d_rs_vs_00fd_signals.data(), d_rs_vs_00fd_signals.size() },
        { 0x000B, false, 8, "VS_A1", "Soft Top Operation, Rollover Bar & Messages", vs_a1_000b_signals.data(), vs_a1_000b_signals.size() },
        { 0x0010, false, 8, "VS_A2", "Soft Top Window Commands & Rollover Detection", vs_a2_0010_signals.data(), vs_a2_0010_signals.size() },
        { 0x0014, false, 8, "DBE_A1", "Overhead Control Panel Lighting, Ambient Sensors & Sunroof", dbe_a1_0014_signals.data(), dbe_a1_0014_signals.size() },
        { 0x0270, false, 8, "DBE_A2", "Rain & Light Sensor Protocol", dbe_a2_0270_signals.data(), dbe_a2_0270_signals.size() },
        { 0x02D4, false, 8, "DBE_A3", "Automatic Dimming Rearview Mirror", dbe_a3_02d4_signals.data(), dbe_a3_02d4_signals.size() },
        { 0x0174, false, 8, "DBE_A4", "Sunroof Rain Close", dbe_a4_0174_signals.data(), dbe_a4_0174_signals.size() },
        { 0x0407, false, 8, "NM_DBE", "Network Management Overhead Control Panel", nm_dbe_0407_signals.data(), nm_dbe_0407_signals.size() },
        { 0x04E7, false, 8, "D_RS_DBE", "Diagnostic Response Overhead Control Panel", d_rs_dbe_04e7_signals.data(), d_rs_dbe_04e7_signals.size() },
        { 0x0767, false, 8, "SG_APPL_DBE", "Application Interface Overhead Control Panel", sg_appl_dbe_0767_signals.data(), sg_appl_dbe_0767_signals.size() },
        { 0x0288, false, 8, "LRK_A1", "Heated Steering Wheel Status", lrk_a1_0288_signals.data(), lrk_a1_0288_signals.size() },
        { 0x002C, false, 8, "OBF_A1", "Upper Control Panel Switch Operations", obf_a1_002c_signals.data(), obf_a1_002c_signals.size() },
        { 0x0405, false, 8, "NM_OBF", "Network Management Upper Control Panel", nm_obf_0405_signals.data(), nm_obf_0405_signals.size() },
        { 0x04E5, false, 8, "D_RS_OBF", "Diagnostic Response Upper Control Panel", d_rs_obf_04e5_signals.data(), d_rs_obf_04e5_signals.size() },
        { 0x0765, false, 8, "SG_APPL_OBF", "Application Interface Upper Control Panel", sg_appl_obf_0765_signals.data(), sg_appl_obf_0765_signals.size() },
        { 0x001A, false, 8, "UBF_A1", "Lower Control Panel Switch Operations & ART Spacing", ubf_a1_001a_signals.data(), ubf_a1_001a_signals.size() },
        { 0x035B, false, 8, "UBF_A2", "Soft Top Operation Switch", ubf_a2_035b_signals.data(), ubf_a2_035b_signals.size() },
        { 0x041D, false, 8, "NM_UBF", "Network Management Lower Control Panel", nm_ubf_041d_signals.data(), nm_ubf_041d_signals.size() },
        { 0x04FD, false, 8, "D_RS_UBF", "Diagnostic Response Lower Control Panel", d_rs_ubf_04fd_signals.data(), d_rs_ubf_04fd_signals.size() },
        { 0x077D, false, 8, "SG_APPL_UBF", "Application Interface Lower Control Panel", sg_appl_ubf_077d_signals.data(), sg_appl_ubf_077d_signals.size() },
        { 0x0012, false, 8, "ARMADA_A1", "Restraint Systems & Occupant Classification", armada_a1_0012_signals.data(), armada_a1_0012_signals.size() },
        { 0x0040, false, 8, "ARMADA_A2", "Crash Event Confirmation & Triggers", armada_a2_0040_signals.data(), armada_a2_0040_signals.size() },
        { 0x041C, false, 8, "NM_ARMADA", "Network Management Restraint System", nm_armada_041c_signals.data(), nm_armada_041c_signals.size() },
        { 0x04FC, false, 8, "D_RS_ARMADA", "Diagnostic Response Restraint System", d_rs_armada_04fc_signals.data(), d_rs_armada_04fc_signals.size() },
        { 0x077C, false, 8, "SG_APPL_ARMADA", "Application Interface Restraint System", sg_appl_armada_077c_signals.data(), sg_appl_armada_077c_signals.size() },
        { 0x02A4, false, 8, "WSS_A1", "Weight Sensing System Classification", wss_a1_02a4_signals.data(), wss_a1_02a4_signals.size() },
        { 0x0426, false, 8, "NM_WSS", "Network Management Weight Sensing System", nm_wss_0426_signals.data(), nm_wss_0426_signals.size() },
        { 0x04C6, false, 8, "D_RS_WSS", "Diagnostic Response Weight Sensing System", d_rs_wss_04c6_signals.data(), d_rs_wss_04c6_signals.size() },
        { 0x0706, false, 8, "SG_APPL_WSS", "Application Interface Weight Sensing System", sg_appl_wss_0706_signals.data(), sg_appl_wss_0706_signals.size() },
        { 0x01AC, false, 8, "SF_A1", "Driver Seat Position & Backrest Status", sf_a1_01ac_signals.data(), sf_a1_01ac_signals.size() },
        { 0x02D0, false, 8, "SF_A2", "Driver Seat Memory & Entry/Exit Positioning", sf_a2_02d0_signals.data(), sf_a2_02d0_signals.size() },
        { 0x040C, false, 8, "NM_SF", "Network Management Driver Seat Module", nm_sf_040c_signals.data(), nm_sf_040c_signals.size() },
        { 0x04EC, false, 8, "D_RS_SF", "Diagnostic Response Driver Seat Module", d_rs_sf_04ec_signals.data(), d_rs_sf_04ec_signals.size() },
        { 0x076C, false, 8, "SG_APPL_SF", "Application Interface Driver Seat Module", sg_appl_sf_076c_signals.data(), sg_appl_sf_076c_signals.size() },
        { 0x07CC, false, 8, "SD_RS_SF", "System Diagnostic Response Driver Seat Module", sd_rs_sf_07cc_signals.data(), sd_rs_sf_07cc_signals.size() },
        { 0x01B6, false, 8, "SB_A1", "Passenger Seat Backrest Status", sb_a1_01b6_signals.data(), sb_a1_01b6_signals.size() },
        { 0x040D, false, 8, "NM_SB", "Network Management Passenger Seat Module", nm_sb_040d_signals.data(), nm_sb_040d_signals.size() },
        { 0x04ED, false, 8, "D_RS_SB", "Diagnostic Response Passenger Seat Module", d_rs_sb_04ed_signals.data(), d_rs_sb_04ed_signals.size() },
        { 0x076D, false, 8, "SG_APPL_SB", "Application Interface Passenger Seat Module", sg_appl_sb_076d_signals.data(), sg_appl_sb_076d_signals.size() },
        { 0x07CD, false, 8, "SD_RS_SB", "System Diagnostic Response Passenger Seat Module", sd_rs_sb_07cd_signals.data(), sd_rs_sb_07cd_signals.size() },
        { 0x0094, false, 8, "STH_A1", "Auxiliary Heater Operation & Status", sth_a1_0094_signals.data(), sth_a1_0094_signals.size() },
        { 0x0419, false, 8, "NM_STH", "Network Management Auxiliary Heater", nm_sth_0419_signals.data(), nm_sth_0419_signals.size() },
        { 0x04F9, false, 8, "D_RS_STH", "Diagnostic Response Auxiliary Heater", d_rs_sth_04f9_signals.data(), d_rs_sth_04f9_signals.size() },
        { 0x0779, false, 8, "SG_APPL_STH", "Application Interface Auxiliary Heater", sg_appl_sth_0779_signals.data(), sg_appl_sth_0779_signals.size() },
        { 0x0030, false, 8, "KLA_A1", "Climate Control Compressor, Fan & Flaps", kla_a1_0030_signals.data(), kla_a1_0030_signals.size() },
        { 0x0250, false, 8, "KLA_A2", "Climate Control Window & Roof Requests", kla_a2_0250_signals.data(), kla_a2_0250_signals.size() },
        { 0x00F1, false, 8, "KLA_A3", "Heating Demand & Temperature", kla_a3_00f1_signals.data(), kla_a3_00f1_signals.size() },
        { 0x0411, false, 8, "NM_KLA", "Network Management Climate Control", nm_kla_0411_signals.data(), nm_kla_0411_signals.size() },
        { 0x04F1, false, 8, "D_RS_KLA", "Diagnostic Response Climate Control", d_rs_kla_04f1_signals.data(), d_rs_kla_04f1_signals.size() },
        { 0x0771, false, 8, "SG_APPL_KLA", "Application Interface Climate Control", sg_appl_kla_0771_signals.data(), sg_appl_kla_0771_signals.size() },
        { 0x0015, false, 8, "MSS_A1", "Special Vehicle Sirens, Lights & Audio", mss_a1_0015_signals.data(), mss_a1_0015_signals.size() },
        { 0x01AE, false, 8, "MSS_A2", "Roof Sign, Oxygen & Mist System Status", mss_a2_01ae_signals.data(), mss_a2_01ae_signals.size() },
        { 0x01CE, false, 8, "MSS_A3", "Special Vehicle Destination Coordinates", mss_a3_01ce_signals.data(), mss_a3_01ce_signals.size() },
        { 0x0248, false, 8, "MSS_A4", "Emergency Window & Sunroof Close", mss_a4_0248_signals.data(), mss_a4_0248_signals.size() },
        { 0x0046, false, 8, "MSSK_A1", "Special Vehicle Switch Console 1", mssk_a1_0046_signals.data(), mssk_a1_0046_signals.size() },
        { 0x0208, false, 8, "MSSK_A2", "Special Vehicle Passenger Seat Control", mssk_a2_0208_signals.data(), mssk_a2_0208_signals.size() },
        { 0x01D8, false, 8, "TP_MSS_AGW3", "Transport Protocol MSS to AGW", tp_mss_agw3_01d8_signals.data(), tp_mss_agw3_01d8_signals.size() },
        { 0x01A6, false, 8, "TP_MSS_KOMBI2", "Transport Protocol MSS to Instrument Cluster", tp_mss_kombi2_01a6_signals.data(), tp_mss_kombi2_01a6_signals.size() },
        { 0x0406, false, 8, "NM_MSS", "Network Management Special Vehicle Control", nm_mss_0406_signals.data(), nm_mss_0406_signals.size() },
        { 0x04E6, false, 8, "D_RS_MSS", "Diagnostic Response Special Vehicle Control", d_rs_mss_04e6_signals.data(), d_rs_mss_04e6_signals.size() },
        { 0x0766, false, 8, "SG_APPL_MSS", "Application Interface Special Vehicle Control", sg_appl_mss_0766_signals.data(), sg_appl_mss_0766_signals.size() },
        { 0x02B0, false, 8, "PTS_A1", "Parktronic System State", pts_a1_02b0_signals.data(), pts_a1_02b0_signals.size() },
        { 0x0413, false, 8, "NM_PTS", "Network Management Parktronic System", nm_pts_0413_signals.data(), nm_pts_0413_signals.size() },
        { 0x04F3, false, 8, "D_RS_PTS", "Diagnostic Response Parktronic System", d_rs_pts_04f3_signals.data(), d_rs_pts_04f3_signals.size() },
        { 0x0773, false, 8, "SG_APPL_PTS", "Application Interface Parktronic System", sg_appl_pts_0773_signals.data(), sg_appl_pts_0773_signals.size() },
        { 0x02FF, false, 8, "TPM_A1", "Tire Pressures & Warning Status", tpm_a1_02ff_signals.data(), tpm_a1_02ff_signals.size() },
        { 0x0418, false, 8, "NM_TPM", "Network Management Tire Pressure Monitor", nm_tpm_0418_signals.data(), nm_tpm_0418_signals.size() },
        { 0x04F8, false, 8, "D_RS_TPM", "Diagnostic Response Tire Pressure Monitor", d_rs_tpm_04f8_signals.data(), d_rs_tpm_04f8_signals.size() },
        { 0x0778, false, 8, "SG_APPL_TPM", "Application Interface Tire Pressure Monitor", sg_appl_tpm_0778_signals.data(), sg_appl_tpm_0778_signals.size() },
        { 0x0607, false, 8, "MESS_TPM1", "Tire Pressure Measurement Data", mess_tpm1_0607_signals.data(), mess_tpm1_0607_signals.size() },
        { 0x0138, false, 8, "AGW_A3", "Audio Gateway Keypad & Function Keys", agw_a3_0138_signals.data(), agw_a3_0138_signals.size() },
        { 0x0338, false, 8, "GPS_A1", "GPS Latitude & Longitude", gps_a1_0338_signals.data(), gps_a1_0338_signals.size() },
        { 0x0339, false, 8, "GPS_A2", "GPS Date & UTC Time", gps_a2_0339_signals.data(), gps_a2_0339_signals.size() },
        { 0x033A, false, 8, "GPS_A3", "GPS Dynamics, Fix & Dilution of Precision", gps_a3_033a_signals.data(), gps_a3_033a_signals.size() },
        { 0x01A4, false, 8, "TP_AGW_KOMBI1", "Transport Protocol AGW to Instrument Cluster", tp_agw_kombi1_01a4_signals.data(), tp_agw_kombi1_01a4_signals.size() },
        { 0x0334, false, 8, "TP_AGW_MSS3", "Transport Protocol AGW to MSS", tp_agw_mss3_0334_signals.data(), tp_agw_mss3_0334_signals.size() },
        { 0x03E3, false, 8, "TP_AGW_TELEAID6", "Transport Protocol AGW to TeleAid", tp_agw_teleaid6_03e3_signals.data(), tp_agw_teleaid6_03e3_signals.size() },
        { 0x0416, false, 8, "NM_AGW", "Network Management Audio Gateway", nm_agw_0416_signals.data(), nm_agw_0416_signals.size() },
        { 0x04F6, false, 8, "D_RS_AGW", "Diagnostic Response Audio Gateway", d_rs_agw_04f6_signals.data(), d_rs_agw_04f6_signals.size() },
        { 0x0680, false, 8, "D_RS_MOST0", "Diagnostic Response MOST Gateway 0", d_rs_most0_0680_signals.data(), d_rs_most0_0680_signals.size() },
        { 0x0681, false, 8, "D_RS_MOST1", "Diagnostic Response MOST Gateway 1", d_rs_most1_0681_signals.data(), d_rs_most1_0681_signals.size() },
        { 0x068A, false, 8, "D_RS_MOST10", "Diagnostic Response MOST Gateway 10", d_rs_most10_068a_signals.data(), d_rs_most10_068a_signals.size() },
        { 0x068B, false, 8, "D_RS_MOST11", "Diagnostic Response MOST Gateway 11", d_rs_most11_068b_signals.data(), d_rs_most11_068b_signals.size() },
        { 0x068C, false, 8, "D_RS_MOST12", "Diagnostic Response MOST Gateway 12", d_rs_most12_068c_signals.data(), d_rs_most12_068c_signals.size() },
        { 0x068D, false, 8, "D_RS_MOST13", "Diagnostic Response MOST Gateway 13", d_rs_most13_068d_signals.data(), d_rs_most13_068d_signals.size() },
        { 0x068E, false, 8, "D_RS_MOST14", "Diagnostic Response MOST Gateway 14", d_rs_most14_068e_signals.data(), d_rs_most14_068e_signals.size() },
        { 0x068F, false, 8, "D_RS_MOST15", "Diagnostic Response MOST Gateway 15", d_rs_most15_068f_signals.data(), d_rs_most15_068f_signals.size() },
        { 0x0682, false, 8, "D_RS_MOST2", "Diagnostic Response MOST Gateway 2", d_rs_most2_0682_signals.data(), d_rs_most2_0682_signals.size() },
        { 0x0683, false, 8, "D_RS_MOST3", "Diagnostic Response MOST Gateway 3", d_rs_most3_0683_signals.data(), d_rs_most3_0683_signals.size() },
        { 0x0684, false, 8, "D_RS_MOST4", "Diagnostic Response MOST Gateway 4", d_rs_most4_0684_signals.data(), d_rs_most4_0684_signals.size() },
        { 0x0685, false, 8, "D_RS_MOST5", "Diagnostic Response MOST Gateway 5", d_rs_most5_0685_signals.data(), d_rs_most5_0685_signals.size() },
        { 0x0686, false, 8, "D_RS_MOST6", "Diagnostic Response MOST Gateway 6", d_rs_most6_0686_signals.data(), d_rs_most6_0686_signals.size() },
        { 0x0687, false, 8, "D_RS_MOST7", "Diagnostic Response MOST Gateway 7", d_rs_most7_0687_signals.data(), d_rs_most7_0687_signals.size() },
        { 0x0688, false, 8, "D_RS_MOST8", "Diagnostic Response MOST Gateway 8", d_rs_most8_0688_signals.data(), d_rs_most8_0688_signals.size() },
        { 0x0689, false, 8, "D_RS_MOST9", "Diagnostic Response MOST Gateway 9", d_rs_most9_0689_signals.data(), d_rs_most9_0689_signals.size() },
        { 0x058A, false, 8, "D_RS_NAVI", "Diagnostic Response Navigation System", d_rs_navi_058a_signals.data(), d_rs_navi_058a_signals.size() },
        { 0x0776, false, 8, "SG_APPL_AGW", "Application Interface Audio Gateway", sg_appl_agw_0776_signals.data(), sg_appl_agw_0776_signals.size() },
        { 0x07D6, false, 8, "SD_RS_AGW", "System Diagnostic Response Audio Gateway", sd_rs_agw_07d6_signals.data(), sd_rs_agw_07d6_signals.size() },
        { 0x043B, false, 8, "NM_ICANI", "Network Management CAN Interface", nm_icani_043b_signals.data(), nm_icani_043b_signals.size() },
        { 0x04BB, false, 8, "D_RS_ICANI", "Diagnostic Response CAN Interface", d_rs_icani_04bb_signals.data(), d_rs_icani_04bb_signals.size() },
        { 0x05BB, false, 8, "SD_RS_ICANI", "System Diagnostic Response CAN Interface", sd_rs_icani_05bb_signals.data(), sd_rs_icani_05bb_signals.size() },
        { 0x01B8, false, 8, "EZS_A3", "Odometer Value Legacy", ezs_a3_01b8_signals.data(), ezs_a3_01b8_signals.size() },
        { 0x04FE, false, 8, "D_RS_FDSVL", "Diagnostic Response Dynamic Seat Front Left", d_rs_fdsvl_04fe_signals.data(), d_rs_fdsvl_04fe_signals.size() },
        { 0x04FF, false, 8, "D_RS_FDSVR", "Diagnostic Response Dynamic Seat Front Right", d_rs_fdsvr_04ff_signals.data(), d_rs_fdsvr_04ff_signals.size() },
        { 0x04EF, false, 8, "D_RS_LRK", "Diagnostic Response Heated Steering Wheel", d_rs_lrk_04ef_signals.data(), d_rs_lrk_04ef_signals.size() },
        { 0x04EE, false, 8, "D_RS_PFDS", "Diagnostic Response Pneumatic Dynamic Seat", d_rs_pfds_04ee_signals.data(), d_rs_pfds_04ee_signals.size() },
        { 0x04FB, false, 8, "D_RS_SHZ", "Diagnostic Response Seat Heating", d_rs_shz_04fb_signals.data(), d_rs_shz_04fb_signals.size() },
        { 0x04FA, false, 8, "D_RS_TLM", "Diagnostic Response Telematics Control Unit", d_rs_tlm_04fa_signals.data(), d_rs_tlm_04fa_signals.size() },
        { 0x072F, false, 8, "APPL_SG_AAG", "Application Interface to Trailer Control", appl_sg_aag_072f_signals.data(), appl_sg_aag_072f_signals.size() },
        { 0x05C9, false, 8, "APPL_SG_AGW", "Application Interface to Audio Gateway", appl_sg_agw_05c9_signals.data(), appl_sg_agw_05c9_signals.size() },
        { 0x06A3, false, 8, "APPL_SG_ARMADA", "Application Interface to Restraint System", appl_sg_armada_06a3_signals.data(), appl_sg_armada_06a3_signals.size() },
        { 0x0678, false, 8, "APPL_SG_DBE", "Application Interface to Overhead Control Panel", appl_sg_dbe_0678_signals.data(), appl_sg_dbe_0678_signals.size() },
        { 0x04DF, false, 8, "APPL_SG_EZS", "Application Interface to Ignition Switch", appl_sg_ezs_04df_signals.data(), appl_sg_ezs_04df_signals.size() },
        { 0x0568, false, 8, "APPL_SG_HFS", "Application Interface to Trunk Remote Control", appl_sg_hfs_0568_signals.data(), appl_sg_hfs_0568_signals.size() },
        { 0x078E, false, 8, "APPL_SG_KLA", "Application Interface to Climate Control", appl_sg_kla_078e_signals.data(), appl_sg_kla_078e_signals.size() },
        { 0x05AB, false, 8, "APPL_SG_KOMBI", "Application Interface to Instrument Cluster", appl_sg_kombi_05ab_signals.data(), appl_sg_kombi_05ab_signals.size() },
        { 0x06C1, false, 8, "APPL_SG_MRM", "Application Interface to Steering Column Module", appl_sg_mrm_06c1_signals.data(), appl_sg_mrm_06c1_signals.size() },
        { 0x0720, false, 8, "APPL_SG_MSS", "Application Interface to Special Vehicle Control", appl_sg_mss_0720_signals.data(), appl_sg_mss_0720_signals.size() },
        { 0x06BA, false, 8, "APPL_SG_OBF", "Application Interface to Upper Control Panel", appl_sg_obf_06ba_signals.data(), appl_sg_obf_06ba_signals.size() },
        { 0x072C, false, 8, "APPL_SG_PTS", "Application Interface to Parktronic", appl_sg_pts_072c_signals.data(), appl_sg_pts_072c_signals.size() },
        { 0x057C, false, 8, "APPL_SG_SAM_H", "Application Interface to Rear SAM", appl_sg_sam_h_057c_signals.data(), appl_sg_sam_h_057c_signals.size() },
        { 0x067D, false, 8, "APPL_SG_SAM_V", "Application Interface to Front SAM", appl_sg_sam_v_067d_signals.data(), appl_sg_sam_v_067d_signals.size() },
        { 0x06B2, false, 8, "APPL_SG_SB", "Application Interface to Passenger Seat Module", appl_sg_sb_06b2_signals.data(), appl_sg_sb_06b2_signals.size() },
        { 0x06B3, false, 8, "APPL_SG_SF", "Application Interface to Driver Seat Module", appl_sg_sf_06b3_signals.data(), appl_sg_sf_06b3_signals.size() },
        { 0x073F, false, 8, "APPL_SG_STH", "Application Interface to Auxiliary Heater", appl_sg_sth_073f_signals.data(), appl_sg_sth_073f_signals.size() },
        { 0x0756, false, 8, "APPL_SG_THL", "Application Interface to Rear Left Door Module", appl_sg_thl_0756_signals.data(), appl_sg_thl_0756_signals.size() },
        { 0x0754, false, 8, "APPL_SG_THR", "Application Interface to Rear Right Door Module", appl_sg_thr_0754_signals.data(), appl_sg_thr_0754_signals.size() },
        { 0x06A7, false, 8, "APPL_SG_TPM", "Application Interface to Tire Pressure Monitor", appl_sg_tpm_06a7_signals.data(), appl_sg_tpm_06a7_signals.size() },
        { 0x06D7, false, 8, "APPL_SG_TVL", "Application Interface to Driver Door Module", appl_sg_tvl_06d7_signals.data(), appl_sg_tvl_06d7_signals.size() },
        { 0x06C0, false, 8, "APPL_SG_TVR", "Application Interface to Passenger Door Module", appl_sg_tvr_06c0_signals.data(), appl_sg_tvr_06c0_signals.size() },
        { 0x0722, false, 8, "APPL_SG_UBF", "Application Interface to Lower Control Panel", appl_sg_ubf_0722_signals.data(), appl_sg_ubf_0722_signals.size() },
        { 0x06B7, false, 8, "APPL_SG_WSS", "Application Interface to Weight Sensing System", appl_sg_wss_06b7_signals.data(), appl_sg_wss_06b7_signals.size() },
        { 0x04E0, false, 8, "D_RQ_EZS", "Diagnostic Request Ignition Switch", d_rq_ezs_04e0_signals.data(), d_rq_ezs_04e0_signals.size() },
    });

    can_profile& get_profile()
    {
        static can_profile profile
        (
            "Mercedes W209 CAN-B"_ct,
            "Mercedes-Benz W209 Interior CAN-B Bus"_ct,
            w209_messages.data(),
            w209_messages.size()
        );
        return profile;
    }
}
