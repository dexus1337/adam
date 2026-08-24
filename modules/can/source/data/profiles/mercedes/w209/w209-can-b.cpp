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
        {   2, "KL_15X_EIN",        "15X terminal is turned on",                                                     4,   1 },
        {   3, "KL_15_EIN",         "Terminal 15 is turned on",                                                      5,   1 },
        {   4, "KL_15R_EIN",        "Terminal 15R is turned on",                                                     6,   1 },
        {   5, "FZG_RECH",          "Message: \"vehicle calculated Wait\"",                                          8,   1 },
        {   6, "DIAG_TGL",          "diagnosis toggle",                                                              9,   1 },
        {   7, "APPL_AUS",          "Application ID's do not send just NM IDs",                                     10,   1 },
        {   8, "PNK_ALM_AUS",       "Panic alarm by key",                                                           11,   1 },
        {   9, "PNK_ALM_EIN",       "Panic alarm by key",                                                           12,   1 },
        {  10, "FERN_ALARM",        "Remote triggering MSS alarm",                                                  13,   1 },
        {  11, "SCHLUE_NEU",        "Message: renew key",                                                           16,   1 },
        {  12, "ZV_PASSIV",         "closing passive",                                                              17,   1 },
        {  13, "ZV_SPIEL",          "Game Protection active",                                                       18,   1 },
        {  14, "HD_STOPP",          "Trunk lid stop",                                                               19,   1 },
        {  15, "SPEI_NR",           "Current memory block number",                                                  21,   3 },
        {  16, "AUSS_SICH",         "perimeter security",                                                           24,   1 },
        {  17, "AUSS_ENTSI",        "Außenentsicherung",                                                            25,   1 },
        {  18, "BLI_SICH",          "secure ZV flasher feedback",                                                   26,   1 },
        {  19, "BLI_ENTSI",         "ZV flasher feedback unlock",                                                   27,   1 },
        {  20, "HFE_EZS",           "Heckdeckelfernentriegelung",                                                   32,   1 },
        {  21, "HD_SICH",           "secure boot lid",                                                              33,   1 },
        {  22, "HD_ENTSI",          "Unlock the boot lid",                                                          34,   1 },
        {  23, "TD_VERRI",          "locking gas cap (HSF / stackers)",                                             35,   1 },
        {  24, "TD_ENTRI",          "unlock tank cap (HSF / trays)",                                                36,   1 },
        {  25, "ZV_NV",             "ZV Nachverriegelung",                                                          38,   1 },
        {  26, "SCHL_BEF",          "Mechanical / FB key active",                                                   39,   1 },
        {  27, "THR_VERRI",         "Right rear door lock",                                                         40,   1 },
        {  28, "THR_ENTRI",         "Unlocking doors, rear right",                                                  41,   1 },
        {  29, "THL_VERRI",         "Left rear door lock",                                                          42,   1 },
        {  30, "THL_ENTRI",         "Unlocking doors, rear left",                                                   43,   1 },
        {  31, "TVR_VERRI",         "Right front door lock",                                                        44,   1 },
        {  32, "TVR_ENTRI",         "Unlock Right front door",                                                      45,   1 },
        {  33, "TVL_VERRI",         "Left front door lock",                                                         46,   1 },
        {  34, "TVL_ENTRI",         "Unlock Left front door",                                                       47,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A10 (ID: 0x010A) - Rear Wheel Speed & Direction (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a10_010a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "RIZ_HL",            "(Impulse) (Pulses) pulse ring counter left rear wheel (48 per revolution)",     0,   8 },
        {   1, "RIZ_HR",            "(Impulse) (Pulses) pulse ring counter right rear wheel (48 per revolution)",    8,   8 },
        {   2, "DRTGHR",            "Direction of rotation right rear wheel",                                       16,   2 },
        {   3, "DHR",               "(1/min) (1 / min), rear right wheel speed",                                    18,  14 },
        {   4, "DRTGHL",            "Rotation direction left rear wheel",                                           32,   2 },
        {   5, "DHL",               "(1/min) (1 / min), rear left wheel speed",                                     34,  14 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A11 (ID: 0x0016) - Battery Voltage (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a11_0016_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "U_BATT",            "(V) Batteriespannung",                                                          0,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A2 (ID: 0x0002) - Front Wheel Speed & Engine Data (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a2_0002_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "RIZ_VL",            "(Impulse) (Pulses) pulse ring counter left front wheel (48 per revolution)",    0,   8 },
        {   1, "RIZ_VR",            "(Impulse) (Pulses) pulse ring counter right front wheel (48 per revolution)",   8,   8 },
        {   2, "N_MOT",             "(1/min) (1 / min) engine speed",                                               16,  16 },
        {   3, "T_MOT",             "(°C) (° C) engine coolant temperature",                                        32,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A4 (ID: 0x0058) - Key Identification & Mileage (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a4_0058_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SCHLUE_ID",         "Identification key for pre-filtering",                                          0,  32 },
        {   1, "KM_EZS",            "(km) (Km) Mileage",                                                            32,  24 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A5 (ID: 0x001F) - Vehicle Variant & Equipment Coding (45 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a5_001f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LAND",              "Country-specific coding SA",                                                    0,   4 },
        {   1, "LL_RL",             "Left / right-hand drive",                                                       6,   2 },
        {   2, "GUARD_B4",          "Special Protection Guard B4",                                                   8,   1 },
        {   3, "BEHI_FZG",          "Conversion for disabled (only tester)",                                         9,   1 },
        {   4, "TAXI_FUNKAUF",      "Radio override taxi",                                                          10,   1 },
        {   5, "SO_FZG",            "special vehicle",                                                              11,   1 },
        {   6, "TAXI_HIRU",         "Taxi call for help",                                                           12,   1 },
        {   7, "TAXI_DZ",           "Connection for roof sign",                                                     13,   1 },
        {   8, "TAXI_NOTALM",       "Taxi emergency alarm system",                                                  14,   1 },
        {   9, "TAXI_INT",          "taxi International",                                                           15,   1 },
        {  10, "KB_SPERR_KLA",      "Lock convection-touch control",                                                16,   1 },
        {  11, "KB_MAN_KLA",        "Mode convection Comfort Free",                                                 17,   1 },
        {  12, "KB_AUTO",           "Mode Comfort Free",                                                            18,   1 },
        {  13, "FH_SPERR_VO",       "Automatic startup lock FH-front",                                              19,   1 },
        {  14, "FH_SPERR_HI",       "Automatic startup lock FH-back",                                               20,   1 },
        {  15, "FL_ZU_MS",          "Fresh air damper close at engine start",                                       21,   1 },
        {  16, "DATENF",            "Radio data available",                                                         22,   1 },
        {  17, "GUARD_B6",          "Special Protection Guard B6 / 7",                                              23,   1 },
        {  18, "FCOD_KAR",          "Vehicle code body (203/209)",                                                  24,   3 },
        {  19, "FCOD_BR",           "Vehicle Code series",                                                          27,   5 },
        {  20, "PRW_VH",            "Platt Roll Warner available",                                                  32,   1 },
        {  21, "FCOD_MOT7",         "Motor vehicle code",                                                           33,   7 },
        {  22, "RS_VH",             "Rain sensor available",                                                        40,   1 },
        {  23, "XEN_VH",            "Xenon light available",                                                        41,   1 },
        {  24, "SRA_VH",            "Headlight washers available",                                                  42,   1 },
        {  25, "KLA_VH",            "Air conditioning available",                                                   43,   1 },
        {  26, "NAG_VH",            "available automatic transmission",                                             44,   1 },
        {  27, "KSG_VH",            "Comfort manual available",                                                     45,   1 },
        {  28, "MEMORY_VH",         "Driver seat memory available",                                                 46,   1 },
        {  29, "KP_VH",             "Communication platform available",                                             47,   1 },
        {  30, "ART_VH",            "ART available",                                                                48,   1 },
        {  31, "CVT_VH",            "CVT available",                                                                49,   1 },
        {  32, "FSB_HZG_VH",        "Frontscheibenhzg. prev.",                                                      50,   1 },
        {  33, "FUK_SCHL",          "Footwell flaps in the cooling mode. Keys (Only G463)",                         51,   1 },
        {  34, "BOOSTER_NVH",       "Booster blower No",                                                            52,   1 },
        {  35, "NIV_VH",            "Level control available",                                                      53,   1 },
        {  36, "SOUND_VH",          "Sound System",                                                                 55,   1 },
        {  37, "PTS_VH",            "Parktronic System",                                                            56,   1 },
        {  38, "AHK_VH",            "Trailer hitch available",                                                      57,   1 },
        {  39, "HR_VH",             "Rear blind available",                                                         58,   1 },
        {  40, "EDW_VH",            "Theft alarm system available",                                                 59,   1 },
        {  41, "IRS_VH",            "Interior protection available",                                                60,   1 },
        {  42, "KG_VH",             "Keyless Go there",                                                             61,   1 },
        {  43, "ERS_LICHT",         "Complete replacement of light allowed",                                        62,   1 },
        {  44, "SWB_VH",            "Heated washer nozzles available",                                              63,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A6 (ID: 0x001E) - Model Year & Variant Coding (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a6_001e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VER_JAHR",          "year specification",                                                            1,   5 },
        {   1, "VER_AE",            "change year",                                                                   6,   2 },
        {   2, "TPM_VH",            "Tire pressure modules available",                                              30,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A7 (ID: 0x0003) - Drive Status & Warnings (26 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a7_0003_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LL_STBL",           "Idle is stable",                                                                0,   1 },
        {   1, "KOMP_BAUS",         "turn air compressor: Acceleration",                                             1,   1 },
        {   2, "KOMP_NOTAUS",       "Air compressor Emergency Shutdown",                                             2,   1 },
        {   3, "LUEFT_MOT_KL",      "Motor fan defective indicator light",                                           3,   1 },
        {   4, "ZWP_EIN_MS",        "Switch on auxiliary water pump",                                                4,   1 },
        {   5, "BLS_UNT",           "Brake light suppression",                                                       5,   1 },
        {   6, "BLS_ST",            "Status Brake Light Switch",                                                     6,   2 },
        {   7, "RG",                "engaged reverse gear (all transmission)",                                       8,   1 },
        {   8, "P",                 "Park position inserted",                                                        9,   1 },
        {   9, "HZL_ST",            "status heating",                                                               10,   2 },
        {  10, "WHC",               "Gear selector lever position (NOS only)",                                      12,   4 },
        {  11, "DRTGVL",            "Rotation direction left front wheel",                                          16,   2 },
        {  12, "DVL",               "(1/min) (1 / min) Left Front wheel",                                           18,  14 },
        {  13, "OEL_KL",            "Oil level / oil pressure warning light",                                       32,   1 },
        {  14, "DIAG_KL",           "Diagnostic indicator (OBD II)",                                                33,   1 },
        {  15, "BAS_KL",            "BAS defect indicator",                                                         34,   1 },
        {  16, "ESP_KL",            "ESP faulty warning light",                                                     35,   1 },
        {  17, "ABS_KL",            "ABS warning light defective",                                                  36,   1 },
        {  18, "BBV_KL",            "Brake pad wear warning light",                                                 37,   1 },
        {  19, "UEHITZ",            "Engine oil temperature too high (overheating)",                                38,   1 },
        {  20, "KPL",               "clutch is pressed",                                                            39,   1 },
        {  21, "WHST",              "Gear selector lever position (NAG, KSG, CVT)",                                 40,   3 },
        {  22, "NOTBRE",            "Emergency braking (brake light flashing)",                                     43,   1 },
        {  23, "ART_ABW_AKT",       "ART-distance warning is switched on",                                          44,   1 },
        {  24, "SUB_ABL_L",         "Substitution low beam left",                                                   45,   1 },
        {  25, "SUB_ABL_R",         "Substitution low beam right",                                                  46,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A8 (ID: 0x0390) - Climate Control Coding & Defaults (27 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a8_0390_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZWP_NVH",           "not installed auxiliary water pump",                                            0,   1 },
        {   1, "UMLUFT_UBG",        "manual air recirculation circuit indefinite period of time",                    1,   1 },
        {   2, "RHT_EIN",           "always REHEAT-operation",                                                       2,   1 },
        {   3, "GBA_MAN",           "Blower bar display only in the manual mode",                                    3,   1 },
        {   4, "GBL_40",            "40% basic ventilation",                                                         4,   1 },
        {   5, "SUS_EIN",           "Noxious gas-dependent air recirculation generally a",                           5,   1 },
        {   6, "SUS_AUS",           "Noxious gas-dependent air recirculation from",                                  6,   1 },
        {   7, "KFK_AUS",           "Refrigerant level control inactive",                                            7,   1 },
        {   8, "KALTLAND_1",        "\"+ 1 ° C\" increase",                                                          8,   1 },
        {   9, "HEISSLAND_2",       "\"- 2 ° C\" Reduced",                                                           9,   1 },
        {  10, "KALTLAND_2",        "\"+ 2 ° C\" increase",                                                         10,   1 },
        {  11, "UMLUFT_EIN",        "close air recirculation flap from <20% throughout",                            11,   1 },
        {  12, "ESAUGBEL_EIN",      "20% basic ventilation E-aspirator a",                                          12,   1 },
        {  13, "UMLUFT_AUS",        "Recirculation damper in OFF mode open",                                        13,   1 },
        {  14, "SOL_AUS",           "Solar influence not active",                                                   14,   1 },
        {  15, "HEISSLAND_1",       "\"-1 ° C\" Hot Country",                                                       15,   1 },
        {  16, "WUESTENLAND",       "Desert lands with sand",                                                       16,   1 },
        {  17, "GBL_KNL",           "Basic ventilation characteristics",                                            17,   3 },
        {  18, "P_KNL",             "Pressure characteristic curve",                                                20,   4 },
        {  19, "GSPA_KLA_KUEHL",    "Transmission shift point increase in cooling power deficit",                   24,   1 },
        {  20, "GSPA_KLA_HEIZ",     "Transmission shift point increase in heating power deficit",                   25,   1 },
        {  21, "TPS_NVH",           "Dew Point No",                                                                 26,   1 },
        {  22, "IFDBE_VH",          "Room sensor in DBE available",                                                 27,   1 },
        {  23, "REST_VH",           "Utilization of residual heat available",                                       28,   1 },
        {  24, "MAXCOOL",           "Display \"Maxcool\" (US only)",                                                29,   1 },
        {  25, "ASL_LVT",           "Autom. Default logic air distribution",                                        30,   1 },
        {  26, "ASL_GBL",           "Autom. Default logic blower",                                                  31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A9 (ID: 0x00B2) - Vehicle Identification Number (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a9_00b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VIN_MSG",           "VIN Control unit",                                                              6,   2 },
        {   1, "VIN_DATA",          "VIN data",                                                                      8,  56 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KG_A1 (ID: 0x01B2) - Keyless Go Status & Alerts (15 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kg_a1_01b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "M5",                "Messg. 5: \"selector lever in P please od N position.\"",                       0,   1 },
        {   1, "M4",                "Messg. 4: \"Check chip card / key battery\" (white)",                           1,   1 },
        {   2, "M3",                "Messg. 3: \"selector lever to the P\" (red tone)",                              2,   1 },
        {   3, "M2",                "Messg. 2 \"/ recognized chip card keys in the vehicle\" (white)",               3,   1 },
        {   4, "M1",                "Messg. 1 \"/ not recognized chip card key\" (White)",                           4,   1 },
        {   5, "M0",                "Messg. 0 \"chip card / key is not recognized\" (red)",                          5,   1 },
        {   6, "WARNTON_KG",        "switch on warning",                                                             6,   1 },
        {   7, "M12",               "Messg. \"Take chip card / key!\": 12",                                          9,   1 },
        {   8, "M11",               "Messg. \"Please leave keys stuck\": 11",                                       10,   1 },
        {   9, "M10",               "Messg. 10: \"Key anticipates\"",                                               11,   1 },
        {  10, "M9",                "Messg. 9: \"Keyless Go in diagnosis\"",                                        12,   1 },
        {  11, "M8",                "Messg. 8: \"Door open\"",                                                      13,   1 },
        {  12, "M7",                "Messg. 7: Reserved",                                                           14,   1 },
        {  13, "M6",                "Messg. 6: \"No driver authorization\"",                                        15,   1 },
        {  14, "KM_REST_KG",        "(km) (Km) way indication Keyless Go",                                          16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KG_A2 (ID: 0x0050) - Keyless Go Window & Roof Control (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kg_a2_0050_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_KG",            "rear window open / close right",                                                0,   1 },
        {   1, "FHL_KG",            "Open windows, rear left / Close",                                               1,   1 },
        {   2, "FVR_KG",            "Front windows open / close right",                                              2,   1 },
        {   3, "FVL_KG",            "Open windows front left / Close",                                               3,   1 },
        {   4, "SHD_KG",            "Open / close SHD / top",                                                        4,   1 },
        {   5, "KB_RI_KG",          "Direction touch control",                                                       5,   1 },
        {   6, "KB_MOD_KG",         "Mode-touch control",                                                            6,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_A2 (ID: 0x018D) - TeleAid Status & Heartbeat (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_a2_018d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MK_ATRSRT",         "Mobility account authorized",                                                   0,   1 },
        {   1, "LIVE_TELEAID",      "Alive message TELEAID",                                                         4,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS1 (ID: 0x03E5) - TeleAid GPS Coordinates (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos1_03e5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_LAT",           "(mas) (Mas) GPS latitude (- means south)",                                      0,  32 },
        {   1, "GPS_LONG",          "(mas) (Mas) GPS longitude (- WEST Means)",                                     32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS2 (ID: 0x03E6) - TeleAid GPS Dynamics & Altitude (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos2_03e6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_VEL",           "(cm/s) (Cm / s) GPS velocity",                                                  0,  16 },
        {   1, "GPS_HEAD",          "(°) (°) GPS heading",                                                          16,  16 },
        {   2, "GPS_ELLIP",         "(m) (M) GPS ellipsoid height",                                                 32,  16 },
        {   3, "GPS_ALT",           "(m) (M) GPS altitude",                                                         48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS3 (ID: 0x03E7) - TeleAid GPS Date & UTC Time (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos3_03e7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_DATE_YEAR",     "(years) (Years) GPS dateyear",                                                  0,  16 },
        {   1, "GPS_DATE_MONTH",    "(months) (Months) GPS datemonth",                                              16,   8 },
        {   2, "GPS_DATE_DAY",      "(days) (Days) GPS date day",                                                   24,   8 },
        {   3, "GPS_UTC_HOUR",      "(h) (H) GPS UTC hour",                                                         32,   8 },
        {   4, "GPS_UTC_MINUTE",    "(min) (Min) GPS UTC minute",                                                   40,   8 },
        {   5, "GPS_UTC_SECOND",    "(s) (S) GPS UTC second",                                                       48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS4 (ID: 0x03E8) - TeleAid Dead Reckoning Coordinates (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos4_03e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DR_MM_LAT",         "(mas) (Mas) Dead reckoning / map matching latitude (- means south)",            0,  32 },
        {   1, "DR_MM_LONG",        "(mas) (What) Dead reckoning / map matching longitude (- WEST Means)",          32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TELEAID_POS5 (ID: 0x03E9) - TeleAid GPS Satellite & Navigation Status (11 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto teleaid_pos5_03e9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_TRCK_SAT",      "GPS satellites tracked",                                                        0,   4 },
        {   1, "GPS_VSBL_SAT",      "GPS satellites visible",                                                        4,   4 },
        {   2, "GPS_VDOP",          "GPS vertical dilution of position",                                             8,   8 },
        {   3, "GPS_HDOP",          "GPS horizontal dilution of position",                                          16,   8 },
        {   4, "GPS_PDOP",          "GPS Dilution Of Position",                                                     24,   8 },
        {   5, "GPS_FIX",           "GPS fix",                                                                      36,   4 },
        {   6, "DR_MM_REL",         "(%) (%) Dead reckoning / map matching position reliablity",                    40,   8 },
        {   7, "MM_MAP_STAT",       "Map matching map state",                                                       48,   2 },
        {   8, "MM_ROAD_STAT",      "Map matching state road",                                                      50,   2 },
        {   9, "MM_ROUTE_STAT",     "Map matching route state",                                                     52,   2 },
        {  10, "DR_MM_STAT",        "Dead reckoning / map matching state",                                          54,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GW_C_B7 (ID: 0x0005) - Gateway Front Right Wheel Speed (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gw_c_b7_0005_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DRTGVR",            "Direction of rotation right front wheel",                                      32,   2 },
        {   1, "DVR",               "(1/min) (1 / min) the right front wheel speed",                                34,  14 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_TELEAID_AGW6 (ID: 0x0209) - Transport Protocol TeleAid to AGW (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_teleaid_agw6_0209_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_TELEAID_AGW",    "Kommunikation TELEAID zum AGW",                                                 0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_TELEAID_KOMBI4 (ID: 0x01A1) - Transport Protocol TeleAid to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_teleaid_kombi4_01a1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_TELEAID_KOMBI",  "Kommunikation TELEAID zum KOMBI",                                               0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_EZS (ID: 0x0400) - Network Management Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_ezs_0400_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EZS (ID: 0x05FF) - Diagnostic Response Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ezs_05ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_EZS (ID: 0x0760) - Application Interface Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_ezs_0760_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_AAG (ID: 0x0730) - Diagnostic Request Trailer Recognition (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_aag_0730_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_AGW (ID: 0x05D6) - Diagnostic Request Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_agw_05d6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ARMADA (ID: 0x06BC) - Diagnostic Request Restraint Systems Airbag (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_armada_06bc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_DBE (ID: 0x0667) - Diagnostic Request Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_dbe_0667_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FDSVL (ID: 0x06BE) - Diagnostic Request Dynamic Seat Front Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fdsvl_06be_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FDSVR (ID: 0x06BF) - Diagnostic Request Dynamic Seat Front Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fdsvr_06bf_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_HFS (ID: 0x0577) - Diagnostic Request Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_hfs_0577_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ICANI (ID: 0x07DA) - Diagnostic Request CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_icani_07da_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_ICANI",        "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_KLA (ID: 0x0791) - Diagnostic Request Automatic Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_kla_0791_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_KOMBI (ID: 0x05B4) - Diagnostic Request Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_kombi_05b4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_LRK (ID: 0x06AF) - Diagnostic Request Heated Steering Wheel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_lrk_06af_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST0 (ID: 0x0640) - Diagnostic Request MOST Gateway 0 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most0_0640_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST1 (ID: 0x0641) - Diagnostic Request MOST Gateway 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most1_0641_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST10 (ID: 0x064A) - Diagnostic Request MOST Gateway 10 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most10_064a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST11 (ID: 0x064B) - Diagnostic Request MOST Gateway 11 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most11_064b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST12 (ID: 0x064C) - Diagnostic Request MOST Gateway 12 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most12_064c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST13 (ID: 0x064D) - Diagnostic Request MOST Gateway 13 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most13_064d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST14 (ID: 0x064E) - Diagnostic Request MOST Gateway 14 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most14_064e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST15 (ID: 0x064F) - Diagnostic Request MOST Gateway 15 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most15_064f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST2 (ID: 0x0642) - Diagnostic Request MOST Gateway 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most2_0642_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST3 (ID: 0x0643) - Diagnostic Request MOST Gateway 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most3_0643_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST4 (ID: 0x0644) - Diagnostic Request MOST Gateway 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most4_0644_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST5 (ID: 0x0645) - Diagnostic Request MOST Gateway 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most5_0645_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST6 (ID: 0x0646) - Diagnostic Request MOST Gateway 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most6_0646_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST7 (ID: 0x0647) - Diagnostic Request MOST Gateway 7 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most7_0647_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST8 (ID: 0x0648) - Diagnostic Request MOST Gateway 8 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most8_0648_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MOST9 (ID: 0x0649) - Diagnostic Request MOST Gateway 9 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_most9_0649_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MRM (ID: 0x06D5) - Diagnostic Request Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_mrm_06d5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MSS (ID: 0x0726) - Diagnostic Request Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_mss_0726_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_NAVI (ID: 0x054A) - Diagnostic Request Navigation System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_navi_054a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_NAVI",         "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_OBF (ID: 0x06A5) - Diagnostic Request Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_obf_06a5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_PFDS (ID: 0x072E) - Diagnostic Request Pneumatic Dynamic Seat (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_pfds_072e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_PTS (ID: 0x0733) - Diagnostic Request Parktronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_pts_0733_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SAM_H (ID: 0x0563) - Diagnostic Request Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sam_h_0563_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SAM_V (ID: 0x0662) - Diagnostic Request Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sam_v_0662_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SB (ID: 0x06AD) - Diagnostic Request Passenger Seat Adjustment (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sb_06ad_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SF (ID: 0x06AC) - Diagnostic Request Driver Seat Adjustment (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sf_06ac_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_SHZ (ID: 0x057B) - Diagnostic Request Seat Heating (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_shz_057b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_STH (ID: 0x0739) - Diagnostic Request Stationary Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_sth_0739_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_THL (ID: 0x0749) - Diagnostic Request Rear Left Door Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_thl_0749_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_THR (ID: 0x074B) - Diagnostic Request Rear Right Door Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_thr_074b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TLM (ID: 0x05DA) - Diagnostic Request Telematics Control Unit (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tlm_05da_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TPM (ID: 0x06B8) - Diagnostic Request Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tpm_06b8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_TPM",          "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TVL (ID: 0x06C8) - Diagnostic Request Front Left Door Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tvl_06c8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_TVR (ID: 0x06CA) - Diagnostic Request Front Right Door Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_tvr_06ca_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_UBF (ID: 0x073D) - Diagnostic Request Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ubf_073d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_WSS (ID: 0x06A8) - Diagnostic Request Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_wss_06a8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_GLOBAL (ID: 0x001C) - Global Diagnostic Request (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_global_001c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "Globaler KWP2000 Diagnose-Request",                                             0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A1 (ID: 0x000C) - Lighting, Speed & Cluster Config (28 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a1_000c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KL_58D_B",          "(%) (%) Brightness meter lighting",                                             0,   8 },
        {   1, "V_SIGNAL",          "(km/h) (Km / h) vehicle speed",                                                 8,   8 },
        {   2, "DZ_EIN",            "Roof sign on (taxi)",                                                          16,   1 },
        {   3, "TFSM_B",            "Tank level minimum",                                                           17,   1 },
        {   4, "AUTO_TUER",         "automatic door lock",                                                          18,   1 },
        {   5, "T_C",               "temperature unit",                                                             19,   1 },
        {   6, "TFL_EIN",           "daytime running lights",                                                       20,   1 },
        {   7, "ANH_UEBW",          "Switch trailer monitoring",                                                    21,   1 },
        {   8, "SCHLUE_ABH_EIN",    "Key dependence a",                                                             22,   1 },
        {   9, "SP_PARK_SPERR",     "Mirror in park",                                                               23,   1 },
        {  10, "ESH_POS_SP",        "Seat longitudinal position for I / O help save",                               24,   1 },
        {  11, "SP_ANKL_SPERR",     "Spiegelanklappen at Fzg. lock",                                                25,   1 },
        {  12, "ESH_POS_STD",       "Sitzverstellweg at I / O help to default",                                     28,   1 },
        {  13, "ESH_SITZ_EIN",      "Seat adjustment with I / O help a",                                            29,   1 },
        {  14, "ESH_LENK_EIN",      "Adjustable steering column for I / O help a",                                  30,   1 },
        {  15, "ESH_AUTO_EIN",      "Easy Entry / automatic. Positionierg. on",                                     31,   1 },
        {  16, "SLF",               "search",                                                                       32,   1 },
        {  17, "RR_KM",             "Trip computer unit distance",                                                  33,   1 },
        {  18, "FL_OK",             "High beam switch permits",                                                     34,   1 },
        {  19, "UFB_EIN",           "Ambient lighting a",                                                           35,   1 },
        {  20, "SPRACHE",           "language",                                                                     36,   4 },
        {  21, "STHL_EIN_KOMBI",    "/ Off auxiliary heating independent ventilation",                              40,   1 },
        {  22, "VWZ_AKT",           "Preset time is enabled (LED on)",                                              41,   1 },
        {  23, "VWZ_AUS_MFL",       "Preselection time off (from LED) over MFL",                                    42,   1 },
        {  24, "IRS_VDK_EIN",       "Interior protection with the roof one",                                        46,   1 },
        {  25, "RDK_AKT",           "enable RDK",                                                                   47,   1 },
        {  26, "INLI_NLZ",          "(s) (S) Interior lighting afterglow",                                          48,   8 },
        {  27, "ABL_NLZ",           "(s) (S) standing or fog light afterglow (SWA)",                                56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A3 (ID: 0x00D4) - Clock, Odometer & Cruising Range (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a3_00d4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "A_ZEIT",            "(s) (S) Current time",                                                          0,  16 },
        {   1, "KM",                "(km) (Km) Mileage",                                                            16,  24 },
        {   2, "RW",                "(km(miles)) (Km (miles)) Range",                                               40,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A5 (ID: 0x01CA) - Multifunction Steering Wheel Buttons (25 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a5_01ca_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KI_STAT",           "status combi",                                                                  0,   8 },
        {   1, "BUTTON_4_2",        "phone End",                                                                     8,   1 },
        {   2, "BUTTON_4_1",        "phone Send",                                                                    9,   1 },
        {   3, "BUTTON_3_2",        "\"-\" key",                                                                    10,   1 },
        {   4, "BUTTON_3_1",        "\"+\" Key",                                                                    11,   1 },
        {   5, "BUTTON_2_2",        "reserve",                                                                      12,   1 },
        {   6, "BUTTON_2_1",        "reserve",                                                                      13,   1 },
        {   7, "BUTTON_1_2",        "Previous display",                                                             14,   1 },
        {   8, "BUTTON_1_1",        "Next display",                                                                 15,   1 },
        {   9, "BUTTON_8_2",        "reserve",                                                                      16,   1 },
        {  10, "BUTTON_8_1",        "reserve",                                                                      17,   1 },
        {  11, "BUTTON_7_2",        "reserve",                                                                      18,   1 },
        {  12, "BUTTON_7_1",        "reserve",                                                                      19,   1 },
        {  13, "BUTTON_6_2",        "reserve",                                                                      20,   1 },
        {  14, "BUTTON_6_1",        "reserve",                                                                      21,   1 },
        {  15, "BUTTON_5_2",        "reserve",                                                                      22,   1 },
        {  16, "BUTTON_5_1",        "reserve",                                                                      23,   1 },
        {  17, "PTT_4_2",           "reserve",                                                                      24,   1 },
        {  18, "PTT_4_1",           "reserve",                                                                      25,   1 },
        {  19, "PTT_3_2",           "reserve",                                                                      26,   1 },
        {  20, "PTT_3_1",           "reserve",                                                                      27,   1 },
        {  21, "PTT_2_2",           "reserve",                                                                      28,   1 },
        {  22, "PTT_2_1",           "reserve",                                                                      29,   1 },
        {  23, "PTT_1_2",           "disable Linguatronic",                                                         30,   1 },
        {  24, "PTT_1_1",           "Enable Linguatronic",                                                          31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A6 (ID: 0x009E) - Key ID & Redundant Odometer (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a6_009e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SCHLUE_ID_KI",      "Identification key for pre-filtering",                                          0,  32 },
        {   1, "KM_KI",             "(km) (Km) Mileage",                                                            32,  24 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A7 (ID: 0x0194) - Display Dimming & Tailgate Setting (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a7_0194_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DISP_DIMM",         "(%) (%) Display dimming",                                                       0,   8 },
        {   1, "DATENF_MENU_AKT",   "Data radio menu activated",                                                     9,   1 },
        {   2, "HD_BEGRENZ",        "Tailgate limiting a",                                                          10,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_A8 (ID: 0x032A) - Multifunction Steering Wheel MSS Buttons (25 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_a8_032a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KI_STAT_MSS",       "status combi",                                                                  0,   8 },
        {   1, "BUTTON_4_2_MSS",    "phone End",                                                                     8,   1 },
        {   2, "BUTTON_4_1_MSS",    "phone Send",                                                                    9,   1 },
        {   3, "BUTTON_3_2_MSS",    "\"-\" key",                                                                    10,   1 },
        {   4, "BUTTON_3_1_MSS",    "\"+\" Key",                                                                    11,   1 },
        {   5, "BUTTON_2_2_MSS",    "reserve",                                                                      12,   1 },
        {   6, "BUTTON_2_1_MSS",    "reserve",                                                                      13,   1 },
        {   7, "BUTTON_1_2_MSS",    "Previous display",                                                             14,   1 },
        {   8, "BUTTON_1_1_MSS",    "Next display",                                                                 15,   1 },
        {   9, "BUTTON_8_2_MSS",    "reserve",                                                                      16,   1 },
        {  10, "BUTTON_8_1_MSS",    "reserve",                                                                      17,   1 },
        {  11, "BUTTON_7_2_MSS",    "reserve",                                                                      18,   1 },
        {  12, "BUTTON_7_1_MSS",    "reserve",                                                                      19,   1 },
        {  13, "BUTTON_6_2_MSS",    "reserve",                                                                      20,   1 },
        {  14, "BUTTON_6_1_MSS",    "reserve",                                                                      21,   1 },
        {  15, "BUTTON_5_2_MSS",    "reserve",                                                                      22,   1 },
        {  16, "BUTTON_5_1_MSS",    "reserve",                                                                      23,   1 },
        {  17, "PTT_4_2_MSS",       "reserve",                                                                      24,   1 },
        {  18, "PTT_4_1_MSS",       "reserve",                                                                      25,   1 },
        {  19, "PTT_3_2_MSS",       "reserve",                                                                      26,   1 },
        {  20, "PTT_3_1_MSS",       "reserve",                                                                      27,   1 },
        {  21, "PTT_2_2_MSS",       "reserve",                                                                      28,   1 },
        {  22, "PTT_2_1_MSS",       "reserve",                                                                      29,   1 },
        {  23, "PTT_1_2_MSS",       "disable Linguatronic",                                                         30,   1 },
        {  24, "PTT_1_1_MSS",       "Enable Linguatronic",                                                          31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_KOMBI_AGW1 (ID: 0x01D0) - Transport Protocol Instrument Cluster to AGW (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_kombi_agw1_01d0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_KOMBI_AGW",      "Kommunikation KOMBI zum AGW",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_KOMBI_MSS2 (ID: 0x0330) - Transport Protocol Instrument Cluster to MSS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_kombi_mss2_0330_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_KOMBI_MSS",      "Kommunikation KOMBI zum MSS",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_KOMBI_TELEAID4 (ID: 0x03E1) - Transport Protocol Instrument Cluster to TeleAid (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_kombi_teleaid4_03e1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_KOMBI_TELEAID",  "Kommunikation KOMBI zum TELEAID",                                               0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_KOMBI (ID: 0x0414) - Network Management Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_kombi_0414_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_KOMBI (ID: 0x04F4) - Diagnostic Response Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_kombi_04f4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_KOMBI (ID: 0x0774) - Application Interface Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_kombi_0774_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_A1 (ID: 0x0006) - Steering Column Switches & Steering Angle (22 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_a1_0006_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SGH_EIN_LR",        "Switch Bugle",                                                                  0,   1 },
        {   1, "LHP_EIN",           "turn flasher",                                                                  1,   1 },
        {   2, "FL_EIN",            "Turn on high beams",                                                            2,   1 },
        {   3, "BLI_RE",            "Directional blinking right",                                                    3,   1 },
        {   4, "BLI_LI",            "Directional blinking left",                                                     4,   1 },
        {   5, "SCH_WI_2",          "LSS III in position (step 2)",                                                  8,   1 },
        {   6, "SCH_WI_1",          "LSS in position II (Step 1)",                                                   9,   1 },
        {   7, "SCH_WI_INT",        "LSS in position I (the rain sensor operation)",                                10,   1 },
        {   8, "WASCHEN",           "washing actuated",                                                             11,   1 },
        {   9, "TIPP_WISCH",        "Brief wipe operated",                                                          12,   1 },
        {  10, "HECK_INT_MRM",      "Rear window Intermittent wipe",                                                13,   1 },
        {  11, "HECK_WISCH_MRM",    "Rear window wipe / wash",                                                      14,   1 },
        {  12, "LS_ST_VER",         "Steering column is locked [0] (US only)",                                      16,   1 },
        {  13, "ESH_EIN_MRM",       "Entry aid (if this knob)",                                                     17,   1 },
        {  14, "SBS_EIN",           "Voice control switch (push to talk)",                                          22,   1 },
        {  15, "SBS_AUS",           "Voice control OFF (Cancel)",                                                   23,   1 },
        {  16, "LW_PA_B",           "Steering angle parity bit (even parity)",                                      24,   1 },
        {  17, "LW_OV_B",           "Steering angle sensor: Overflow",                                              25,   1 },
        {  18, "LW_CF_B",           "Steering angle sensor: Code Error",                                            26,   1 },
        {  19, "LW_INI_B",          "Steering angle sensor: not initialized",                                       27,   1 },
        {  20, "LW_VZ_B",           "Steering angle sign",                                                          28,   1 },
        {  21, "LW_B",              "(°) (°) steering angle",                                                       29,  11 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_A2 (ID: 0x01A8) - Steering Wheel Rocker Switches (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_a2_01a8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "WIPPE_4_2",         "Button on the bottom right to bottom",                                          0,   1 },
        {   1, "WIPPE_4_1",         "Button on the right bottom to top",                                             1,   1 },
        {   2, "WIPPE_3_2",         "Button on the right top to bottom",                                             2,   1 },
        {   3, "WIPPE_3_1",         "Button on the top right to the top",                                            3,   1 },
        {   4, "WIPPE_2_2",         "Button on the bottom left to bottom",                                           4,   1 },
        {   5, "WIPPE_2_1",         "Button on the left bottom to top",                                              5,   1 },
        {   6, "WIPPE_1_2",         "Button top left to bottom",                                                     6,   1 },
        {   7, "WIPPE_1_1",         "Button top left to top",                                                        7,   1 },
        {   8, "WIPPE_8_2",         "reserve",                                                                       8,   1 },
        {   9, "WIPPE_8_1",         "reserve",                                                                       9,   1 },
        {  10, "WIPPE_7_2",         "reserve",                                                                      10,   1 },
        {  11, "WIPPE_7_1",         "reserve",                                                                      11,   1 },
        {  12, "WIPPE_6_2",         "reserve",                                                                      12,   1 },
        {  13, "WIPPE_6_1",         "reserve",                                                                      13,   1 },
        {  14, "WIPPE_5_2",         "reserve",                                                                      14,   1 },
        {  15, "WIPPE_5_1",         "reserve",                                                                      15,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_A3 (ID: 0x0296) - Steering Column Adjustment Lever (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_a3_0296_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LS_ZUR_MRM",        "Steering column to the rear (toward the driver)",                               0,   1 },
        {   1, "LS_VOR_MRM",        "Steering column to the front",                                                  1,   1 },
        {   2, "LS_AB_MRM",         "Steering column downward",                                                      2,   1 },
        {   3, "LS_AUF_MRM",        "Steering column according to above",                                            3,   1 },
        {   4, "LSVH_UN",           "Steering column adjustment lever is turned down",                               4,   1 },
        {   5, "LSVH_OB",           "Steering column adjustment lever rotated upward",                               5,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_MRM (ID: 0x0415) - Network Management Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_mrm_0415_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MRM (ID: 0x04F5) - Diagnostic Response Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_mrm_04f5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_MRM (ID: 0x0775) - Application Interface Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_mrm_0775_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_MRM (ID: 0x07D5) - System Diagnostic Response Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_mrm_07d5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "Systemdiagnose-Response",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A1 (ID: 0x000A) - Front Lighting & Exterior Status (43 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a1_000a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KL_61_EIN",         "terminal 61",                                                                   0,   1 },
        {   1, "SWA_AKT",           "Injection active headlamps",                                                    1,   1 },
        {   2, "RG_SAM_V",          "Reverse gear engaged (for NSG / KSG)",                                          2,   1 },
        {   3, "SPVS_ST_R",         "Mirror adjustment in position right",                                           3,   1 },
        {   4, "SGH_ST_EIN",        "Signal horn is activated",                                                      4,   1 },
        {   5, "FL_ST_EIN",         "High beam is turned on",                                                        5,   1 },
        {   6, "NSW_ST_EIN",        "Fog lights are turned on",                                                      6,   1 },
        {   7, "AFL_AKT",           "Outside light activated by light sensor",                                       7,   1 },
        {   8, "NSL_EIN",           "Switch rear fog light",                                                         8,   1 },
        {   9, "ABL_EIN",           "switch on low beam",                                                           10,   1 },
        {  10, "STL_EIN",           "Switch on parking lights",                                                     11,   1 },
        {  11, "PL_RE_EIN",         "Turn right parking light",                                                     13,   1 },
        {  12, "PL_LI_EIN",         "Left parking light switch",                                                    14,   1 },
        {  13, "ZWP_LFT",           "Auxiliary water pump runs",                                                    16,   1 },
        {  14, "KOMP_LFT",          "Refrigeration compressor runs",                                                17,   1 },
        {  15, "HAS_KL",            "Hand brake applied (control light)",                                           18,   1 },
        {  16, "KOMP_EIN",          "Air compressor switched",                                                      19,   1 },
        {  17, "KOMP_DEF",          "C compressor drive current output defective",                                  20,   1 },
        {  18, "DIAG_15_EIN",       "Terminal 15 is activated via diagnostics",                                     21,   1 },
        {  19, "DIAG_15R_EIN",      "Terminal 15R via diagnostic activated",                                        22,   1 },
        {  20, "BFL_KL",            "Brake fluid level warning light",                                              24,   1 },
        {  21, "WWS_KL",            "Wash water level too low pilot light",                                         25,   1 },
        {  22, "KWS_KL",            "Cooling water level too low pilot light",                                      26,   1 },
        {  23, "NSW_DEF_L",         "Left fog defective",                                                           32,   1 },
        {  24, "FL_DEF_L",          "High beam left defective",                                                     33,   1 },
        {  25, "ABL_DEF_L",         "Low beam left defective",                                                      34,   1 },
        {  26, "PL_DEF_VL",         "Front left parking light defective",                                           35,   1 },
        {  27, "BLI_DEF_VL",        "Indicator front left defective",                                               36,   1 },
        {  28, "SM_DEF_VL",         "Side Marker front left defective",                                             37,   1 },
        {  29, "INSTR_AUS",         "Instrument illumination from",                                                 39,   1 },
        {  30, "NSW_DEF_R",         "Right fog defective",                                                          40,   1 },
        {  31, "FL_DEF_R",          "Right high beam defect",                                                       41,   1 },
        {  32, "ABL_DEF_R",         "Low beam right defective",                                                     42,   1 },
        {  33, "PL_DEF_VR",         "front right parking light defective",                                          43,   1 },
        {  34, "BLI_DEF_VR",        "Indicator front left defective",                                               44,   1 },
        {  35, "SM_DEF_VR",         "Side Marker front right defective",                                            45,   1 },
        {  36, "LENK_OEL_KL",       "Messg. Steering fluid too low",                                                47,   1 },
        {  37, "BLI_ERS_VL",        "Replacement Indicator front left active",                                      48,   1 },
        {  38, "PL_ERS_VL",         "Replacement Parking light front left active",                                  49,   1 },
        {  39, "DIAG_X4_F",         "Start Xenon4 diagnostic procedure driver's side",                              50,   1 },
        {  40, "BLI_ERS_VR",        "Replacement Indicator front left active",                                      52,   1 },
        {  41, "PL_ERS_VR",         "Replacement parking light front right active",                                 53,   1 },
        {  42, "DIAG_X4_B",         "Start Xenon4 diagnostic procedure passenger side",                             54,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A2 (ID: 0x0017) - Outside Temperature & AC Refrigerant Data (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a2_0017_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "T_AUSSEN_B",        "(°C) (° C) Outside air temperature",                                            0,   8 },
        {   1, "P_KAELTE",          "(bar) (Bar) pressure refrigerant R134a",                                        8,  16 },
        {   2, "T_KAELTE",          "(°C) (° C) temperature refrigerant R134a",                                     24,  16 },
        {   3, "I_KOMP",            "(mA) (MA) current compressor main control valve",                              40,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A3 (ID: 0x0070) - Rain Sensor & Wiper Status (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a3_0070_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PARITY_SAM_V",      "Parity (even) of bit 0 to 6",                                                   0,   1 },
        {   1, "KONFIG_RS",         "Rain sensor configuration",                                                     2,   3 },
        {   2, "KL_86_EIN",         "washing actuated",                                                              5,   1 },
        {   3, "KL_31B_EIN",        "Wiper park position outside",                                                   6,   1 },
        {   4, "RS_AKT",            "Rain sensor activated",                                                         7,   1 },
        {   5, "BYTE_KENN",         "a byte code",                                                                   8,   2 },
        {   6, "DIAG_RS",           "Diagnosis rain sensor",                                                        10,   1 },
        {   7, "RS_NM",             "Rain sensor operation is not possible",                                        11,   1 },
        {   8, "SAM_V_INIT",        "SAM_V initialization",                                                         12,   1 },
        {   9, "KL_86_RS",          "washing actuated",                                                             13,   1 },
        {  10, "KL_31B_RS",         "Wiper outside parking position",                                               14,   1 },
        {  11, "RS_INT",            "Rain sensor on / off (position interval)",                                     15,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_V_A4 (ID: 0x02CC) - Exterior Mirror Adjustment (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_v_a4_02cc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SPVS_ST",           "Spiegelverstellschalterstellung",                                               0,   1 },
        {   1, "SP_FAHREN",         "Exterior mirrors to the driving position",                                      2,   1 },
        {   2, "SP_GARAGE",         "An exterior mirror according Garage position",                                  3,   1 },
        {   3, "SP_N_UN",           "Outside mirror glass down",                                                     4,   1 },
        {   4, "SP_N_OB",           "Outside mirror glass upwards",                                                  5,   1 },
        {   5, "SP_N_RE",           "Outside mirror glass to the right",                                             6,   1 },
        {   6, "SP_N_LI",           "Outside mirror glass to the left",                                              7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SAM_V (ID: 0x0402) - Network Management Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sam_v_0402_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SAM_V (ID: 0x04E2) - Diagnostic Response Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sam_v_04e2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SAM_V (ID: 0x0762) - Application Interface Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sam_v_0762_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A1 (ID: 0x0004) - Rear Lighting, Doors & Alarm Status (47 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a1_0004_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PNK_AKT",           "Panic alarm is active",                                                         0,   1 },
        {   1, "KL54_RM",           "Terminal 54 Hardware enabled",                                                  1,   1 },
        {   2, "HDK_BET",           "down tailgate Contact",                                                         2,   1 },
        {   3, "HD_AUF",            "Boot lid is on",                                                                3,   1 },
        {   4, "THR_AUF",           "Rear door is right",                                                            4,   1 },
        {   5, "THL_AUF",           "Door rear left on",                                                             5,   1 },
        {   6, "TVR_AUF",           "Front door is right",                                                           6,   1 },
        {   7, "TVL_AUF",           "Door front left",                                                               7,   1 },
        {   8, "MOT_AUF",           "Bonnet is on",                                                                  8,   1 },
        {   9, "HVST_BF_ENT",       "Height adjuster passenger unlocked",                                            9,   1 },
        {  10, "HVST_F_ENT",        "Height adjuster driver unlocks",                                               10,   1 },
        {  11, "HHS_ST_USPG",       "Heizb. Rear window is switched off due to undervoltage.",                      11,   1 },
        {  12, "HHS_ST_EIN",        "Heated rear window is on",                                                     12,   1 },
        {  13, "HSCHL_ST_SICH",     "Rear lock is secured",                                                         13,   1 },
        {  14, "HSCHL_ZU",          "Rear lock in 90 ° position,",                                                  14,   1 },
        {  15, "HD_SK_SAM_H",       "Rear lid lock pawl actuated",                                                  15,   1 },
        {  16, "EDW_IL_EIN",        "Switch EDW interior light",                                                    16,   1 },
        {  17, "EDW_AKT",           "EDW sharpened",                                                                17,   1 },
        {  18, "EDW_IRS_AKT",       "Enable EDW interior protection",                                               18,   1 },
        {  19, "EDW_AAG_AKT",       "Enable EDW-trailer monitoring",                                                19,   1 },
        {  20, "EDW_ALARM",         "EDW alarm triggered",                                                          20,   1 },
        {  21, "KZL_DEF_L",         "Plate light left broken",                                                      24,   1 },
        {  22, "RFL_DEF_L",         "Reversing light Left defective",                                               25,   1 },
        {  23, "BL_DEF_L",          "Left brake light is defective",                                                26,   1 },
        {  24, "SL_DEF_L",          "Bottom left defective",                                                        27,   1 },
        {  25, "BLI_DEF_HL",        "Indicator rear left defective",                                                28,   1 },
        {  26, "NSL_DEF",           "Rear fog light broken",                                                        29,   1 },
        {  27, "BL3_DEF",           "3rd brake light is defective",                                                 30,   1 },
        {  28, "KL_54_DEF",         "Terminal 54 errors",                                                           31,   1 },
        {  29, "KZL_DEF_R",         "Plate light right defective",                                                  32,   1 },
        {  30, "RFL_DEF_R",         "Reversing light Right defective",                                              33,   1 },
        {  31, "BL_DEF_R",          "Stoplight right defective",                                                    34,   1 },
        {  32, "SL_DEF_R",          "Bottom right defective",                                                       35,   1 },
        {  33, "BLI_DEF_HR",        "Rear right turn signal defective",                                             36,   1 },
        {  34, "SM_DEF_HR",         "Side marker defective rear right",                                             38,   1 },
        {  35, "SM_DEF_HL",         "Sidemarker rear left defective",                                               39,   1 },
        {  36, "NSL_ERS",           "Replacement light rear fog lamp (s) active",                                   40,   1 },
        {  37, "SL_ERS_HR",         "Replacement light tail light, rear right active",                              42,   1 },
        {  38, "BLI_ERS_HR",        "Replacement light Rear right turn signal active",                              43,   1 },
        {  39, "SL_ERS_HL",         "Replacement light rear lights, left active",                                   46,   1 },
        {  40, "BLI_ERS_HL",        "Replacement light Indicator rear left active",                                 47,   1 },
        {  41, "HW_INT_AKT",        "Rear wiper interval operation",                                                48,   1 },
        {  42, "GURT_KL_HW",        "Belt KL on (for G463)",                                                        49,   1 },
        {  43, "SRS_KL_HW",         "SRS warning light (for G463)",                                                 50,   1 },
        {  44, "HFS_SB_EIN",        "Switch HFS search lights",                                                     51,   1 },
        {  45, "HD_SCHLIESS_SAM_H", "Button Heckd. operated close",                                                 52,   1 },
        {  46, "HD_SICH_SAM_H",     "Button Heckd. Close & Secure operated",                                        53,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A2 (ID: 0x0090) - Fuel Tank Level & Sensors (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a2_0090_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TANK_FS_B",         "(%) (%) Tank Level",                                                            0,   8 },
        {   1, "TANK_GE_RE",        "(%) (%) Tank encoder value right",                                              8,   8 },
        {   2, "TANK_GE_LI",        "(%) (%) Tank timer value left",                                                16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A3 (ID: 0x000E) - Turn Signals & Hazard Warning Flasher (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a3_000e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "BLI_RE_EIN",        "Turn Right indicator",                                                          0,   1 },
        {   1, "BLI_LI_EIN",        "Turn left turn signal",                                                         1,   1 },
        {   2, "WARN_AKT",          "Hazard lights active",                                                          2,   1 },
        {   3, "HELL_BLINK",        "(ms) (Ms) Flashing-light light phase",                                          8,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A4 (ID: 0x0041) - Central Locking Emergency Opening (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a4_0041_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SN1_SAM_H",         "Lock nut 1 (unlock)",                                                           1,   1 },
        {   1, "ZV_NOTOEFF",        "ZV emergency opening",                                                          7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A5 (ID: 0x0230) - Alarm Lighting Activation (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a5_0230_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NSW_EIN_EDW",       "switch fog lights",                                                             0,   1 },
        {   1, "ABL_EIN_EDW",       "switch on low beam",                                                            1,   1 },
        {   2, "SL_EIN_EDW",        "Turn tail light",                                                               2,   1 },
        {   3, "HELL_EDW",          "(ms) (Ms) continuous light light phase",                                        8,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SAM_H_A6 (ID: 0x00CC) - Access Authorization Code (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sam_h_a6_00cc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZBC_SAM_H",         "Code Zugangberechtigung",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SAM_H (ID: 0x0403) - Network Management Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sam_h_0403_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SAM_H (ID: 0x04E3) - Diagnostic Response Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sam_h_04e3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SAM_H (ID: 0x0763) - Application Interface Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sam_h_0763_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: AAG_A1 (ID: 0x0130) - Trailer Recognition & Lighting Status (9 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto aag_a1_0130_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ANH_ERK",           "Towing recognized",                                                             0,   1 },
        {   1, "AHK_NOK",           "not locked trailer coupling",                                                   1,   1 },
        {   2, "ANHKL_54_DEF",      "Trailer Terminal 54 errors",                                                    2,   1 },
        {   3, "EDW_ANH_ALM",       "EDW trailer monitoring alarm triggered",                                        3,   1 },
        {   4, "ANHBL_DEF",         "Trailer brake light broken",                                                    7,   1 },
        {   5, "ANHSL_DEF_L",       "Trailer left tail light broken",                                                8,   1 },
        {   6, "ANHBLI_DEF_L",      "Trailer left turn signal defective",                                            9,   1 },
        {   7, "ANHSL_DEF_R",       "Trailer tail lamp right defective",                                            12,   1 },
        {   8, "ANHBLI_DEF_R",      "Trailer right turn signal defective",                                          13,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_AAG (ID: 0x0410) - Network Management Trailer Recognition (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_aag_0410_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_AAG (ID: 0x04F0) - Diagnostic Response Trailer Recognition (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_aag_04f0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_AAG (ID: 0x0770) - Application Interface Trailer Recognition (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_aag_0770_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_AAG (ID: 0x07D0) - System Diagnostic Response Trailer Recognition (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_aag_07d0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "Systemdiagnose-Response",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A1 (ID: 0x028C) - Driver Seat Adjustment & Memory Switch (30 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a1_028c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SVL_TGL",           "Sitting front left - toggle",                                                   0,   1 },
        {   1, "SVL_HI_AB",         "Sitting front left - rear height from",                                         4,   1 },
        {   2, "SVL_HI_AUF",        "Sitting front left - rear height on",                                           5,   1 },
        {   3, "SVL_ZUR",           "Seat left front - along back",                                                  6,   1 },
        {   4, "SVL_VOR",           "Seat left front - along ago",                                                   7,   1 },
        {   5, "SVL_KST_AB",        "Seat left front - head restraint down",                                         8,   1 },
        {   6, "SVL_KST_AUF",       "Seat left front - head restraint upwards",                                      9,   1 },
        {   7, "SVL_VO_AB",         "Sitting front left - front height from",                                       10,   1 },
        {   8, "SVL_VO_AUF",        "Sitting front left - front height",                                            11,   1 },
        {   9, "SVL_LE_ZUR",        "Seat left front - back to back",                                               12,   1 },
        {  10, "SVL_LE_VOR",        "Sitting front left - back forward",                                            13,   1 },
        {  11, "LS_ZURUECK_LL",     "Steering column to the rear (toward the driver)",                              16,   1 },
        {  12, "LS_VOR_LL",         "Steering column to the front",                                                 17,   1 },
        {  13, "LS_AB_LL",          "Steering column downward",                                                     18,   1 },
        {  14, "LS_AUF_LL",         "Steering column according to above",                                           19,   1 },
        {  15, "MVL_TGL",           "Memory left front - toggle",                                                   24,   1 },
        {  16, "SPI_RE_SP",         "Mirrors right store parking position",                                         25,   1 },
        {  17, "MVL_P3_SP",         "Memory front left - save position 3",                                          26,   1 },
        {  18, "MVL_P2_SP",         "Memory left front - Save position 2",                                          27,   1 },
        {  19, "MVL_P1_SP",         "Memory front left - save position 1",                                          28,   1 },
        {  20, "MVL_P3_EN",         "Memory left front - taking position 3",                                        29,   1 },
        {  21, "MVL_P2_EN",         "Memory left front - taking position 2",                                        30,   1 },
        {  22, "MVL_P1_EN",         "Memory left front - occupy position 1",                                        31,   1 },
        {  23, "SPVS_BET_LL",       "mirror adjustment operated",                                                   32,   1 },
        {  24, "SPI_RE_FAHREN",     "Mirrors right to drive position (not 203)",                                    34,   1 },
        {  25, "SPI_RE_GARAGE",     "Mirrors right to Garage position (not 203)",                                   35,   1 },
        {  26, "SPI_RE_N_UN",       "Mirrors right down (not 203)",                                                 36,   1 },
        {  27, "SPI_RE_N_OB",       "Mirrors right to the top (not 203)",                                           37,   1 },
        {  28, "SPI_RE_N_RE",       "Mirrors right to right (not 203)",                                             38,   1 },
        {  29, "SPI_RE_N_LI",       "Mirrors right to left (not 203)",                                              39,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A2 (ID: 0x0044) - Power Window Controls Driver Door (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a2_0044_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_TVL",           "rear window open / close right",                                                8,   1 },
        {   1, "FHL_TVL",           "Open windows, rear left / Close",                                               9,   1 },
        {   2, "FVR_TVL",           "Front windows open / close right",                                             10,   1 },
        {   3, "FVL_TVL",           "Open windows front left / Close",                                              11,   1 },
        {   4, "SHD_TVL",           "Open / close SHD / top",                                                       12,   1 },
        {   5, "KB_RI_TVL",         "Direction touch control",                                                      13,   1 },
        {   6, "KB_MOD_TVL",        "Mode-touch control",                                                           14,   1 },
        {   7, "FHR_AS_LL",         "automatically close - window rear right",                                      16,   1 },
        {   8, "FHR_MS_LL",         "manually close - window rear right",                                           17,   1 },
        {   9, "FHR_MOE_LL",        "manually open - window rear right",                                            18,   1 },
        {  10, "FHR_AOE_LL",        "Automatically open - window rear right",                                       19,   1 },
        {  11, "FHL_AS_LL",         "Left rear windows - automatically close",                                      20,   1 },
        {  12, "FHL_MS_LL",         "rear left window - manually close",                                            21,   1 },
        {  13, "FHL_MOE_LL",        "Left rear window - manually open",                                             22,   1 },
        {  14, "FHL_AOE_LL",        "Left rear windows - automatically open",                                       23,   1 },
        {  15, "FVR_AS",            "automatically close - window front right",                                     24,   1 },
        {  16, "FVR_MS",            "manually close - window front right",                                          25,   1 },
        {  17, "FVR_MOE",           "manually open - window front right",                                           26,   1 },
        {  18, "FVR_AOE",           "Automatically Open - front right window",                                      27,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A3 (ID: 0x0018) - Driver Window Position & Mirror Switch (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a3_0018_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SPVS_BF_LL",        "Spiegelverstellsch. in Stellg. right (not 203)",                                0,   1 },
        {   1, "HFE_LL",            "Heckdeckelfernentriegelung",                                                    1,   1 },
        {   2, "KISI_EIN_LL",       "Parental Control",                                                              2,   1 },
        {   3, "ZBLL_DEF",          "Additional left turn signal defective",                                         3,   1 },
        {   4, "HFS_LL",            "Remote boot lid closing",                                                       4,   1 },
        {   5, "FVL_NORM",          "normalized window front left",                                                  8,   1 },
        {   6, "FVL_BLOCK",         "blocked windows front left",                                                    9,   1 },
        {   7, "FVL_AUF",           "open window",                                                                  10,   1 },
        {   8, "FVL_KZHB",          "Windows, front left short-stroke greater",                                     11,   1 },
        {   9, "FESTE_VL",          "(1/Ankerumdre) (1 / Ankerumdre) window position front left",                   12,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVL_A4 (ID: 0x00E8) - Driver Door Access Authorization Code (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvl_a4_00e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZBC_TVL",           "Code Zugangberechtigung",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_TVL (ID: 0x0408) - Network Management Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_tvl_0408_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TVL (ID: 0x04E8) - Diagnostic Response Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tvl_04e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_TVL (ID: 0x0768) - Application Interface Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_tvl_0768_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A1 (ID: 0x0290) - Passenger Seat Adjustment & Memory Switch (30 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a1_0290_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SVR_TGL",           "Seat front right - toggle",                                                     0,   1 },
        {   1, "SVR_HI_AB",         "Seat front right - rear height from",                                           4,   1 },
        {   2, "SVR_HI_AUF",        "Seat front right - rear height on",                                             5,   1 },
        {   3, "SVR_ZUR",           "Seat front right - along back",                                                 6,   1 },
        {   4, "SVR_VOR",           "Seat front right - along ago",                                                  7,   1 },
        {   5, "SVR_KST_AB",        "Seat front right - the head restraint down",                                    8,   1 },
        {   6, "SVR_KST_AUF",       "Seat front right - head restraint upwards",                                     9,   1 },
        {   7, "SVR_VO_AB",         "Seat front right - front height from",                                         10,   1 },
        {   8, "SVR_VO_AUF",        "Seat front right - front height",                                              11,   1 },
        {   9, "SVR_LE_ZUR",        "Seat front right - back to back",                                              12,   1 },
        {  10, "SVR_LE_VOR",        "Seat front right - back forward",                                              13,   1 },
        {  11, "LS_ZURUECK_RL",     "Steering column to the rear (toward the driver)",                              16,   1 },
        {  12, "LS_VOR_RL",         "Steering column to the front",                                                 17,   1 },
        {  13, "LS_AB_RL",          "Steering column downward",                                                     18,   1 },
        {  14, "LS_AUF_RL",         "Steering column according to above",                                           19,   1 },
        {  15, "MVR_TGL",           "Memory Front right - toggle",                                                  24,   1 },
        {  16, "SPI_LI_SP",         "Exterior mirrors store parking position",                                      25,   1 },
        {  17, "MVR_P3_SP",         "Memory front right - save position 3",                                         26,   1 },
        {  18, "MVR_P2_SP",         "Memory front right - save position 2",                                         27,   1 },
        {  19, "MVR_P1_SP",         "Memory front right - save position 1",                                         28,   1 },
        {  20, "MVR_P3_EN",         "Memory Front right - taking position 3",                                       29,   1 },
        {  21, "MVR_P2_EN",         "Memory Front right - taking position 2",                                       30,   1 },
        {  22, "MVR_P1_EN",         "Memory Front right - occupy position 1",                                       31,   1 },
        {  23, "SPVS_BET_RL",       "mirror adjustment operated",                                                   32,   1 },
        {  24, "SPI_LI_FAHREN",     "Mirrors left to the drive position (not 203)",                                 34,   1 },
        {  25, "SPI_LI_GARAGE",     "Mirrors left to garage position (not 203)",                                    35,   1 },
        {  26, "SPI_LI_N_UN",       "Mirrors left to bottom (not 203)",                                             36,   1 },
        {  27, "SPI_LI_N_OB",       "Mirrors left to top (not 203)",                                                37,   1 },
        {  28, "SPI_LI_N_RE",       "Mirrors left to right (not 203)",                                              38,   1 },
        {  29, "SPI_LI_N_LI",       "Mirrors left to the left (not 203)",                                           39,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A2 (ID: 0x0045) - Power Window Controls Passenger Door (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a2_0045_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_TVR",           "rear window open / close right",                                                8,   1 },
        {   1, "FHL_TVR",           "Open windows, rear left / Close",                                               9,   1 },
        {   2, "FVR_TVR",           "Front windows open / close right",                                             10,   1 },
        {   3, "FVL_TVR",           "Open windows front left / Close",                                              11,   1 },
        {   4, "SHD_TVR",           "Open / close SHD / top",                                                       12,   1 },
        {   5, "KB_RI_TVR",         "Direction touch control",                                                      13,   1 },
        {   6, "KB_MOD_TVR",        "Mode-touch control",                                                           14,   1 },
        {   7, "FHR_AS_RL",         "automatically close - window rear right",                                      16,   1 },
        {   8, "FHR_MS_RL",         "manually close - window rear right",                                           17,   1 },
        {   9, "FHR_MOE_RL",        "manually open - window rear right",                                            18,   1 },
        {  10, "FHR_AOE_RL",        "Automatically open - window rear right",                                       19,   1 },
        {  11, "FHL_AS_RL",         "Left rear windows - automatically close",                                      20,   1 },
        {  12, "FHL_MS_RL",         "rear left window - manually close",                                            21,   1 },
        {  13, "FHL_MOE_RL",        "Left rear window - manually open",                                             22,   1 },
        {  14, "FHL_AOE_RL",        "Left rear windows - automatically open",                                       23,   1 },
        {  15, "FVL_AS",            "the left window front - automatically close",                                  28,   1 },
        {  16, "FVL_MS",            "the left window front - manually close",                                       29,   1 },
        {  17, "FVL_MOE",           "the left window front - manually open",                                        30,   1 },
        {  18, "FVL_AOE",           "the left window front - automatically open",                                   31,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A3 (ID: 0x0019) - Passenger Window Position & Controls (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a3_0019_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SPVS_BF_RL",        "Spiegelverstellsch. in Stellg. left (not 203)",                                 0,   1 },
        {   1, "HFE_RL",            "Heckdeckelfernentriegelung",                                                    1,   1 },
        {   2, "KISI_EIN_RL",       "Parental Control",                                                              2,   1 },
        {   3, "ZBLR_DEF",          "Additional right turn signal defective",                                        3,   1 },
        {   4, "HFS_RL",            "Remote boot lid closing",                                                       4,   1 },
        {   5, "FVR_NORM",          "Window normalized front right",                                                 8,   1 },
        {   6, "FVR_BLOCK",         "Front windows blocked right",                                                   9,   1 },
        {   7, "FVR_AUF",           "open window",                                                                  10,   1 },
        {   8, "FVR_KZHB",          "Power windows front right higher short-stroke",                                11,   1 },
        {   9, "FESTE_VR",          "(1/Ankerumdre) (1 / Ankerumdre) Window position front right",                  12,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TVR_A4 (ID: 0x00EC) - Passenger Door Access Authorization Code (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tvr_a4_00ec_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZBC_TVR",           "Code Zugangberechtigung",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_TVR (ID: 0x040A) - Network Management Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_tvr_040a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TVR (ID: 0x04EA) - Diagnostic Response Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tvr_04ea_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_TVR (ID: 0x076A) - Application Interface Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_tvr_076a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: THL_A1 (ID: 0x009A) - Rear Left Window Position & Controls (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto thl_a1_009a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHL_NORM",          "normalized power windows, rear left",                                           0,   1 },
        {   1, "FHL_BLOCK",         "Rear windows blocked the left",                                                 1,   1 },
        {   2, "FHL_AUF",           "open window, rear left",                                                        2,   1 },
        {   3, "FHL_KZHB",          "Windows rear left short-stroke greater",                                        3,   1 },
        {   4, "FESTE_HL",          "(1/Ankerumdre) (1 / Ankerumdre) window position rear left",                     4,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_THL (ID: 0x0409) - Network Management Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_thl_0409_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_THL (ID: 0x04E9) - Diagnostic Response Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_thl_04e9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_THL (ID: 0x0769) - Application Interface Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_thl_0769_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: THR_A1 (ID: 0x009C) - Rear Right Window Position & Controls (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto thr_a1_009c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_NORM",          "Windows normalized rear right",                                                 0,   1 },
        {   1, "FHR_BLOCK",         "Rear windows blocked right",                                                    1,   1 },
        {   2, "FHR_AUF",           "Window open rear right",                                                        2,   1 },
        {   3, "FHR_KZHB",          "Windows, rear right larger short-stroke",                                       3,   1 },
        {   4, "FESTE_HR",          "(1/Ankerumdre) (1 / Ankerumdre) Window position rear right",                    4,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_THR (ID: 0x040B) - Network Management Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_thr_040b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_THR (ID: 0x04EB) - Diagnostic Response Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_thr_04eb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_THR (ID: 0x076B) - Application Interface Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_thr_076b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: HFS_A1 (ID: 0x0078) - Trunk Remote Closing & Tailgate Status (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto hfs_a1_0078_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HD_ST",             "Status tailgate",                                                               0,   3 },
        {   1, "HD_SICH_HFS",       "Close rear lid and secure actuated",                                            3,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_HFS (ID: 0x0417) - Network Management Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_hfs_0417_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_HFS (ID: 0x04F7) - Diagnostic Response Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_hfs_04f7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_HFS (ID: 0x0777) - Application Interface Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_hfs_0777_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_HFS (ID: 0x07D7) - System Diagnostic Response Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_hfs_07d7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "Systemdiagnose-Response",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_VS (ID: 0x00FD) - Diagnostic Response Soft Top Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_vs_00fd_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VS_A1 (ID: 0x000B) - Soft Top & Roll Bar Status Messages (24 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vs_a1_000b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FH_VR_KH",          "Front windows move to the right short stroke",                                  0,   1 },
        {   1, "HD_SPERR",          "Open rear cover locked",                                                        1,   1 },
        {   2, "UERB_KL",           "Pilot roll bar",                                                                2,   1 },
        {   3, "VDK_WARN",          "switch on warning",                                                             3,   1 },
        {   4, "FH_VL_KH",          "Front windows left move to short stroke",                                       4,   1 },
        {   5, "VDK_AKTIV",         "Top is active",                                                                 5,   1 },
        {   6, "VDK_STAT",          "top status",                                                                    6,   2 },
        {   7, "VS_M8",             "Message 8: \"top drops\"",                                                      8,   1 },
        {   8, "VS_M7",             "Message 7: \"convertible into operation\"",                                     9,   1 },
        {   9, "VS_M6",             "Message 6: \"make roll bar low\"",                                             10,   1 },
        {  10, "VS_M5",             "Message 5: \"trigger roll bar\"",                                              11,   1 },
        {  11, "VS_M4",             "Message 4: \"top lock\"",                                                      12,   1 },
        {  12, "VS_M3",             "Message 3: \"Start For top operational engine\"",                              13,   1 },
        {  13, "VS_M2",             "Message 2: \"Kofferaumabtrennung / close ski bag\"",                           14,   1 },
        {  14, "VS_M1",             "Message 1: \"Close tailgate\"",                                                15,   1 },
        {  15, "FHS_V_SPERR",       "front FHS commands lock windows",                                              16,   1 },
        {  16, "HD_SK_VS",          "Rear lid lock pawl actuated",                                                  17,   1 },
        {  17, "VS_M14",            "Message 14: \"Please include ski bag\"",                                       18,   1 },
        {  18, "VS_M13",            "Message 13: \"convertible top closed\"",                                       19,   1 },
        {  19, "VS_M12",            "Message 12: \"convertible top open\"",                                         20,   1 },
        {  20, "VS_M11",            "Message 11: \"convertible top operation please wait\"",                        21,   1 },
        {  21, "VS_M10",            "Message 10: \"roof closed due to driving\"",                                   22,   1 },
        {  22, "VS_M9",             "Report 9: \"seek top Workshop\"",                                              23,   1 },
        {  23, "VDK_KL_ANF",        "Request top indicator light",                                                  30,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VS_A2 (ID: 0x0010) - Soft Top Window Movement & Rollover Status (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vs_a2_0010_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FVR_VS",            "Front windows open / close right",                                              2,   1 },
        {   1, "FVL_VS",            "Open windows front left / Close",                                               3,   1 },
        {   2, "FH_RI_VS",          "Direction operation windows",                                                   5,   1 },
        {   3, "FH_MOD_VS",         "Power window drive",                                                            6,   1 },
        {   4, "FH_LH_BEGR",        "FH-travel limit long throw active",                                             7,   1 },
        {   5, "UEB_ERK",           "Rollover identified",                                                           8,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A1 (ID: 0x0014) - Interior Lights, Rain Sensor & Sunroof Status (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a1_0014_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AL_VL",             "Turn courtesy lamp front left",                                                 0,   1 },
        {   1, "AL_VR",             "Exit light switch, front right",                                                1,   1 },
        {   2, "AL_HL",             "Turn courtesy lamp, rear left",                                                 2,   1 },
        {   3, "AL_HR",             "Exit light switch, rear right",                                                 3,   1 },
        {   4, "T_INNEN",           "(°C) (° C) indoor temperature",                                                 8,   8 },
        {   5, "AFL_ABL_EIN",       "AFL request: Switch low beam",                                                 16,   1 },
        {   6, "NACHT",             "Day / night signal",                                                           17,   1 },
        {   7, "LISR_DEF",          "Light sensor defective",                                                       18,   1 },
        {   8, "TUNNEL",            "Light sensor: Tunnel",                                                         20,   1 },
        {   9, "DAEMMER",           "Light sensor: Twilight",                                                       21,   1 },
        {  10, "INIT_LS_AKT",       "Initialization running LS",                                                    23,   1 },
        {  11, "VERD_FANGPOS",      "Top in tuck position",                                                         25,   1 },
        {  12, "VERD_ZU",           "Top lock (at W, S, C, CL = [1])",                                              26,   1 },
        {  13, "LADE_EIN",          "Switch charging light",                                                        27,   1 },
        {  14, "SHD_ST",            "Sunroof status",                                                               29,   3 },
        {  15, "IRS_ALM",           "Interior protection triggered",                                                32,   1 },
        {  16, "IRS_GB",            "triggered interior protection glass breakage",                                 33,   1 },
        {  17, "FRBL_VL",           "Footwell lighting front on the left",                                          52,   1 },
        {  18, "FRBL_HELL",         "(%) (%) Brightness footwell lighting",                                         56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A2 (ID: 0x0270) - Rain Sensor & Wiper Stages (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a2_0270_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PARITY_DBE",        "Parity (even) of bit 0 to bit 6",                                               0,   1 },
        {   1, "SENDE_WIEDER",      "not understood requirement",                                                    2,   1 },
        {   2, "NEU_INI_FERTIG",    "re-initialization completed",                                                   3,   1 },
        {   3, "FEHLER_RS",         "error coding",                                                                  4,   3 },
        {   4, "WISCHER_EIN",       "wiper request",                                                                 7,   1 },
        {   5, "KENN_RS",           "A byte code rain sensor",                                                       8,   1 },
        {   6, "RS_DEF",            "Rain sensor defective",                                                         9,   1 },
        {   7, "SCHWALL",           "Schwall recognition",                                                          11,   1 },
        {   8, "WISCHER_ST",        "(Stufen) (Stages) wiper stages",                                               12,   4 },
        {   9, "MESS_RLS_NV",       "Readings RLS unavailable",                                                     21,   1 },
        {  10, "KOM_RLS_FEHL",      "Communication error to rain / light sensor",                                   22,   1 },
        {  11, "DIAG_RLS_EIN",      "Diagnosis rain / light sensor, a",                                             23,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A3 (ID: 0x02D4) - Automatic Dimming Interior Mirror (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a3_02d4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SP_ABBLEND",        "(Stufen) Spiegel-Abblendung",                                                   3,   5 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DBE_A4 (ID: 0x0174) - Sunroof Auto-Close on Rain (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dbe_a4_0174_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SHD_ZU_RS",         "Schiebehebedach zu bei Regen",                                                  0,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_DBE (ID: 0x0407) - Network Management Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_dbe_0407_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_DBE (ID: 0x04E7) - Diagnostic Response Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_dbe_04e7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_DBE (ID: 0x0767) - Application Interface Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_dbe_0767_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: LRK_A1 (ID: 0x0288) - Heated Steering Wheel Status & LED (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto lrk_a1_0288_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LHZG_LED_EIN",      "Switch LED steering wheel heating",                                             1,   1 },
        {   1, "LRK_STOERG",        "LEDs blink LRK wg. disorder",                                                   2,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: OBF_A1 (ID: 0x002C) - Upper Control Panel Switch Status (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto obf_a1_002c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZV_VERRI_IS",       "ZV lock (internal switch operated)",                                            0,   1 },
        {   1, "ZV_ENTRI_IS",       "ZV unlock (internal switch operated)",                                          1,   1 },
        {   2, "HR_BET",            "Rear blind button actuated",                                                    2,   1 },
        {   3, "FKS_BET",           "Headrest rear lift / lower actuated",                                           4,   1 },
        {   4, "ESP_BET",           "ESP operated on / off",                                                         6,   2 },
        {   5, "WBL_EIN",           "Hazard lights a",                                                              10,   1 },
        {   6, "EDW_AS_ABW",        "deselect EDW tow-away protection",                                             13,   1 },
        {   7, "EDW_IRS_ABW",       "deselect EDW interior protection",                                             14,   1 },
        {   8, "EDW_HAND_AUF",      "EDW glovebox contact triggered",                                               15,   1 },
        {   9, "PTS_BET",           "Button actuated parking sensors",                                              16,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_OBF (ID: 0x0405) - Network Management Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_obf_0405_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_OBF (ID: 0x04E5) - Diagnostic Response Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_obf_04e5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_OBF (ID: 0x0765) - Application Interface Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_obf_0765_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: UBF_A1 (ID: 0x001A) - Lower Control Panel Switches & Distance Setting (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ubf_a1_001a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ART_ABW_BET",       "ART-distance warning actuated on / off",                                        2,   2 },
        {   1, "ASG_SPORT_BET",     "ASG Sport mode on / off operated (ST2_LED_DL when ABC available)",              4,   1 },
        {   2, "FU_FRSP_BET",       "Button radio override actuated",                                                7,   1 },
        {   3, "ART_ABSTAND",       "distance factor",                                                               8,   8 },
        {   4, "BH_FUNK_BET",       "Button authorities radio operated",                                            18,   1 },
        {   5, "STHL_BET",          "Switch operated heater",                                                       24,   1 },
        {   6, "LED_STH_DEF",       "LEDs defect for auxiliary heating",                                            26,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: UBF_A2 (ID: 0x035B) - Lower Control Panel Soft Top Operation (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ubf_a2_035b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VDK_ANF",           "Verdeckbetätigung",                                                             0,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_UBF (ID: 0x041D) - Network Management Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_ubf_041d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_UBF (ID: 0x04FD) - Diagnostic Response Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ubf_04fd_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_UBF (ID: 0x077D) - Application Interface Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_ubf_077d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ARMADA_A1 (ID: 0x0012) - Restraint Systems & Occupant Classification (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto armada_a1_0012_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AKSE_EIN",          "Switch ACFE light",                                                             0,   1 },
        {   1, "AKSE_BLINK",        "ACFE lights blink",                                                             1,   1 },
        {   2, "SRS_KL",            "SRS warning light",                                                             4,   1 },
        {   3, "SRS_BLINK",         "SRS warning light flashing",                                                    5,   1 },
        {   4, "SRS_SERV",          "SRS warning lamp (Service)",                                                    6,   1 },
        {   5, "SRS_WERK",          "SRS warning lamp (Werkstatt)",                                                  7,   1 },
        {   6, "KISI_ST",           "Status child seat",                                                            13,   3 },
        {   7, "GS_BF",             "buckle passenger",                                                             16,   2 },
        {   8, "PSG_DETEC_FAST",    "Passenger detection almost",                                                   19,   2 },
        {   9, "SBE_BF",            "SBE-class passenger",                                                          21,   3 },
        {  10, "GS_F",              "buckle driver",                                                                24,   2 },
        {  11, "SBE_F",             "SBE-Class driver",                                                             29,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ARMADA_A2 (ID: 0x0040) - Crash Event Confirmation & Triggers (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto armada_a2_0040_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "CONF_CRASH",        "Confirm bit for all crash events toggelnd",                                     0,   1 },
        {   1, "CRASH_G",           "Rollover event 1",                                                              1,   1 },
        {   2, "CRASH_F",           "Frontal Event 2",                                                               2,   1 },
        {   3, "CRASH_E",           "Heck Event 2",                                                                  3,   1 },
        {   4, "CRASH_D",           "Page Event 1",                                                                  4,   1 },
        {   5, "CRASH_C",           "Frontal Event 5",                                                               5,   1 },
        {   6, "CRASH_B",           "Heck Event 1",                                                                  6,   1 },
        {   7, "CRASH_A",           "Frontal Event 1",                                                               7,   1 },
        {   8, "X_CRASH",           "Any crash event",                                                               8,   1 },
        {   9, "CRASH_O",           "event tbd",                                                                     9,   1 },
        {  10, "CRASH_N",           "event tbd",                                                                    10,   1 },
        {  11, "CRASH_M",           "event tbd",                                                                    11,   1 },
        {  12, "CRASH_L",           "Page Event 2",                                                                 12,   1 },
        {  13, "CRASH_K",           "Heck Event 3",                                                                 13,   1 },
        {  14, "CRASH_I",           "Rollover event 3",                                                             14,   1 },
        {  15, "CRASH_H",           "Rollover event 2",                                                             15,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_ARMADA (ID: 0x041C) - Network Management Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_armada_041c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ARMADA (ID: 0x04FC) - Diagnostic Response Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_armada_04fc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_ARMADA (ID: 0x077C) - Application Interface Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_armada_077c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: WSS_A1 (ID: 0x02A4) - Weight Sensing System Classification (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto wss_a1_02a4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PSG_DETECT_FAST",   "Passenger Detection Fast",                                                      2,   2 },
        {   1, "WSS_PSG_FAULT",     "Weight classification passenger fault",                                         4,   1 },
        {   2, "WSS_PSG",           "Weight classification passenger",                                               5,   3 },
        {   3, "WSS_ID",            "WSS Identification",                                                           16,   8 },
        {   4, "WSS_ANZ",           "WSS display in the dash",                                                      30,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_WSS (ID: 0x0426) - Network Management Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_wss_0426_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_WSS (ID: 0x04C6) - Diagnostic Response Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_wss_04c6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_WSS (ID: 0x0706) - Application Interface Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_wss_0706_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SF_A1 (ID: 0x01AC) - Driver Seat Position & Backrest Status (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sf_a1_01ac_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MF_MAN_SP1",        "save manually set position (redundant)",                                        0,   1 },
        {   1, "LE_F_ENT",          "Back seat driver unlocked",                                                     4,   1 },
        {   2, "SF_POS",            "(Abschnitte) (Sections) driver seat position",                                  8,   8 },
        {   3, "SF_EA_DEF",         "(Abschnitte) (Sections) default position for driver's seat at I / O help",     16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SF_A2 (ID: 0x02D0) - Driver Seat Memory & Entry/Exit Positioning (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sf_a2_02d0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ESH_AKT",           "Entry / exit is approaching a position actively",                               0,   1 },
        {   1, "AUTO_AKT",          "Driving position to approach active",                                           1,   1 },
        {   2, "MF_MAN_SP",         "save manually set position",                                                    2,   1 },
        {   3, "ESH_AUTO_REST",     "perform positioning mirror",                                                    5,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SF (ID: 0x040C) - Network Management Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sf_040c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SF (ID: 0x04EC) - Diagnostic Response Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sf_04ec_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SF (ID: 0x076C) - Application Interface Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sf_076c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_SF (ID: 0x07CC) - System Diagnostic Response Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_sf_07cc_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "Systemdiagnose-Response",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SB_A1 (ID: 0x01B6) - Passenger Seat Backrest Status (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sb_a1_01b6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LE_B_ENT",          "Lehne Beifahrersitz entriegelt",                                                4,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_SB (ID: 0x040D) - Network Management Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sb_040d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SB (ID: 0x04ED) - Diagnostic Response Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sb_04ed_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_SB (ID: 0x076D) - Application Interface Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sb_076d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_SB (ID: 0x07CD) - System Diagnostic Response Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_sb_07cd_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "Systemdiagnose-Response",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: STH_A1 (ID: 0x0094) - Auxiliary Heater Operation & Status (8 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sth_a1_0094_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "STHL_EIN",          "/ Off auxiliary heating independent ventilation",                               0,   1 },
        {   1, "STHL_AUS",          "/ Off auxiliary heater vent",                                                   1,   1 },
        {   2, "STLFT_EIN",         "Switch airing manually",                                                        2,   1 },
        {   3, "STHZG_EIN",         "Switch heating manually",                                                       3,   1 },
        {   4, "GEBLAESE_EIN",      "Switch Vehicle fan",                                                            4,   1 },
        {   5, "VWZ_MENUE",         "Open code-time menu",                                                           5,   1 },
        {   6, "ZH_LED_EIN",        "Switch on heater LED",                                                          6,   1 },
        {   7, "SENDLM_EIN",        "Transmitter learning mode",                                                     7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_STH (ID: 0x0419) - Network Management Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_sth_0419_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_STH (ID: 0x04F9) - Diagnostic Response Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_sth_04f9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_STH (ID: 0x0779) - Application Interface Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_sth_0779_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_A1 (ID: 0x0030) - Climate Control Compressor, Fan & Flaps (20 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_a1_0030_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HHS_EIN",           "Switch on heated rear window",                                                  0,   1 },
        {   1, "EC_AKT",            "EC mode active",                                                                1,   1 },
        {   2, "IFG_EIN",           "Switch internal sensor Fan",                                                    2,   1 },
        {   3, "ZWP_EIN",           "Switch on auxiliary water pump",                                                3,   1 },
        {   4, "ZH_EIN_OK",         "Heater switch allows",                                                          4,   1 },
        {   5, "LL_DZA",            "Idle speed increase for the cooling capacity increase",                         5,   1 },
        {   6, "HEIZEN",            "heating heater",                                                                6,   1 },
        {   7, "LUEFTEN",           "Heater vent",                                                                   7,   1 },
        {   8, "NLFTS",             "(%) (%) Motor fan setpoint speed",                                              8,   8 },
        {   9, "M_KOMP",            "(Nm) (Nm) torque absorption chiller",                                          16,   8 },
        {  10, "KOMP_STELL",        "(%) (%) Refrigeration compressor control signal",                              24,   8 },
        {  11, "FSB_HZG_EIN",       "Frontscheibenhzg. Switch (for G463)",                                          32,   1 },
        {  12, "G_ANF_KUEHL_KLA",   "Switching point increase in cooling power deficit",                            33,   1 },
        {  13, "GEB_LSTG",          "(%) (%) Fan power",                                                            40,   8 },
        {  14, "UL_AKT_KLA",        "convection active",                                                            48,   1 },
        {  15, "G_ANF_KLA",         "Switching point increase in heating power deficit",                            49,   1 },
        {  16, "LKO_VORN",          "Position ventilation flap above",                                              50,   2 },
        {  17, "LKM_VORN",          "Position ventilation flap center",                                             52,   2 },
        {  18, "LKU_VORN",          "Position ventilation flap down",                                               54,   2 },
        {  19, "T_INNEN_KLA",       "(°C) (° C) indoor temperature",                                                56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_A2 (ID: 0x0250) - Climate Control Window & Roof Requests (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_a2_0250_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_KLA",           "rear window open / close right",                                                0,   1 },
        {   1, "FHL_KLA",           "Open windows, rear left / Close",                                               1,   1 },
        {   2, "FVR_KLA",           "Front windows open / close right",                                              2,   1 },
        {   3, "FVL_KLA",           "Open windows front left / Close",                                               3,   1 },
        {   4, "SHD_KLA",           "Open / close SHD / top",                                                        4,   1 },
        {   5, "KB_RI_KLA",         "Direction touch control",                                                       5,   1 },
        {   6, "KB_MOD_KLA",        "Mode-touch control",                                                            6,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_A3 (ID: 0x00F1) - Heating Demand & Temperature (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_a3_00f1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HZL_ANF",           "(%) (%) Heating power requirement",                                             0,   8 },
        {   1, "T_AUSSEN_WM",       "(°C) (° C) Outside air temperature for heat management",                        8,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_KLA (ID: 0x0411) - Network Management Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_kla_0411_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_KLA (ID: 0x04F1) - Diagnostic Response Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_kla_04f1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_KLA (ID: 0x0771) - Application Interface Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_kla_0771_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A1 (ID: 0x0015) - Special Vehicle Sirens, Lights & Audio (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a1_0015_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FL_EIN_MSS",        "Turn on high beams",                                                            0,   1 },
        {   1, "NSW_EIN_MSS",       "switch fog lights",                                                             1,   1 },
        {   2, "SGH_EIN_MSS",       "Switch Bugle",                                                                  2,   1 },
        {   3, "BLI_EIN_MSS",       "MSS flashing on",                                                               3,   1 },
        {   4, "T_HIRU_EIN",        "Taxi call for help settle",                                                     4,   1 },
        {   5, "UMLUFT_MSS",        "close air recirculation flap",                                                  5,   1 },
        {   6, "IL_EIN_MSS",        "Switch interior light",                                                         6,   1 },
        {   7, "ZV_ZU_MSS",         "close Central",                                                                 7,   1 },
        {   8, "BLI_HELL_MSS",      "(ms) (Ms) Duration Indicator light phase",                                      8,   8 },
        {   9, "SGH_AN_MSS",        "(ms) (Ms) duration of Hupens",                                                 16,   8 },
        {  10, "NSW_HELL_MSS",      "(ms) (Ms) duration fog lamp light phase",                                      24,   8 },
        {  11, "FL_HELL_MSS",       "(ms) (Ms) duration high-beam light phase",                                     32,   8 },
        {  12, "FU_FRSP_AKT",       "Radio override active",                                                        40,   1 },
        {  13, "ANF_FBAS",          "Requirements Composite Input Comand",                                          41,   1 },
        {  14, "BHF_LED_AKT",       "Switch LED Radio Communications",                                              42,   1 },
        {  15, "ANF_ZT",            "Request numeric keypad HU",                                                    43,   1 },
        {  16, "LADEN_AKT",         "220V external load is connected",                                              44,   1 },
        {  17, "AUDIO_MUTE2",       "Muting the audio source for radio reception",                                  45,   1 },
        {  18, "AUDIO_MUTE1",       "Audio Mute",                                                                   46,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A2 (ID: 0x01AE) - Roof Sign, Oxygen & Mist System Status (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a2_01ae_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DZ_KL",             "Roof sign warning light a",                                                     0,   1 },
        {   1, "DZ_LA_DEF",         "Roof sign lamp defect",                                                         1,   1 },
        {   2, "DZ_PRF",            "Check roof sign (interruption)",                                                2,   1 },
        {   3, "DZ_DEF",            "Roof sign defective (short circuit)",                                           3,   1 },
        {   4, "MSS_USPG",          "MSS detects undervoltage",                                                      4,   1 },
        {   5, "MSS_ALM",           "Silent alarm triggered",                                                        5,   1 },
        {   6, "NOTALM_DEF",        "Emergency alarm system is defective",                                           6,   1 },
        {   7, "MSS_EE_DEF",        "Electrical on MSS defective",                                                   7,   1 },
        {   8, "FNK_STAT",          "Radio Status",                                                                  8,   1 },
        {   9, "MSS_SUMMER",        "Summer drive in combination",                                                   9,   1 },
        {  10, "O2_AUS",            "Fresh air system out of service",                                              10,   1 },
        {  11, "O2_AKT",            "Fresh air system activated",                                                   11,   1 },
        {  12, "O2_LEER",           "Fresh air bottle is empty",                                                    12,   1 },
        {  13, "NEBEL_AUS",         "Fog system out of service",                                                    13,   1 },
        {  14, "NEBEL_AKT",         "Mist system activated",                                                        14,   1 },
        {  15, "NP_LEER",           "Fog cartridge empty",                                                          15,   1 },
        {  16, "MSS_USPG_MO",       "MSS detects under-voltage when the engine is running",                         21,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A3 (ID: 0x01CE) - Special Vehicle Destination Coordinates (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a3_01ce_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DEST_LAT",          "(ms) (Ms) GPS Latitude, South = [-]; North = [+]",                              0,  32 },
        {   1, "DEST_LONG",         "(ms) (Ms) GPS Longitude, West = [-]; East = [+]",                              32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSS_A4 (ID: 0x0248) - Emergency Window & Sunroof Close (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mss_a4_0248_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FHR_ALARM",         "Close rear right",                                                              0,   1 },
        {   1, "FHL_ALARM",         "Close rear left",                                                               1,   1 },
        {   2, "FVR_ALARM",         "Close window front right",                                                      2,   1 },
        {   3, "FVL_ALARM",         "Close front left",                                                              3,   1 },
        {   4, "SHD_ALARM",         "close SHD",                                                                     4,   1 },
        {   5, "RI_ALARM",          "Line Alert operation",                                                          5,   1 },
        {   6, "MOD_ALARM",         "SHD / window Run",                                                              6,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSSK_A1 (ID: 0x0046) - Special Vehicle Switch Console 1 (21 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mssk_a1_0046_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SGH_EIN_K",         "Switch Bugle",                                                                  0,   1 },
        {   1, "LHP_EIN_K",         "turn flasher",                                                                  1,   1 },
        {   2, "FL_EIN_K",          "Turn on high beams",                                                            2,   1 },
        {   3, "BLI_RE_K",          "Directional blinking right",                                                    3,   1 },
        {   4, "BLI_LI_K",          "Directional blinking left",                                                     4,   1 },
        {   5, "SCH_WI_2_K",        "MSSK III in position (step 2)",                                                 8,   1 },
        {   6, "SCH_WI_1_K",        "MSSK in position II (Step 1)",                                                  9,   1 },
        {   7, "SCH_WI_INT_K",      "MSSK in position I (the rain sensor operation)",                               10,   1 },
        {   8, "WASCHEN_K",         "turn wash",                                                                    11,   1 },
        {   9, "TIPP_WISCH_K",      "Switch Brief wipe",                                                            12,   1 },
        {  10, "HECK_INT_K",        "Rear window Intermittent wipe",                                                16,   1 },
        {  11, "HECK_WISCH_K",      "Rear window wipe / wash",                                                      17,   1 },
        {  12, "WBL_EIN_K",         "Hazard lights a",                                                              18,   1 },
        {  13, "STL_EIN_K",         "Switch on parking lights",                                                     19,   1 },
        {  14, "ABL_EIN_K",         "switch on low beam",                                                           20,   1 },
        {  15, "NSW_EIN_K",         "switch fog lights",                                                            21,   1 },
        {  16, "NSL_EIN_K",         "Switch rear fog light",                                                        22,   1 },
        {  17, "SHD_STOP",          "sunroof stop",                                                                 24,   1 },
        {  18, "HD_AUF_K",          "open Hubdach",                                                                 25,   1 },
        {  19, "SHD_ZU_K",          "sunroof close",                                                                26,   1 },
        {  20, "SHD_AUF_K",         "open sunroof",                                                                 27,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MSSK_A2 (ID: 0x0208) - Special Vehicle Passenger Seat Control (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mssk_a2_0208_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SBF_K_TGL",         "Passenger seat - toggle",                                                       0,   1 },
        {   1, "SBF_ZUR_K",         "Passenger seat - back along",                                                   6,   1 },
        {   2, "SBF_VOR_K",         "Passenger seat - along ago",                                                    7,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_MSS_AGW3 (ID: 0x01D8) - Transport Protocol MSS to AGW (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_mss_agw3_01d8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_MSS_AGW3",       "Kommunikation vom MSS zum AGW",                                                 0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_MSS_KOMBI2 (ID: 0x01A6) - Transport Protocol MSS to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_mss_kombi2_01a6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_MSS_KOMBI",      "Kommunikation vom MSS zum KOMBI",                                               0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_MSS (ID: 0x0406) - Network Management Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_mss_0406_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MSS (ID: 0x04E6) - Diagnostic Response Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_mss_04e6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_MSS (ID: 0x0766) - Application Interface Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_mss_0766_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: PTS_A1 (ID: 0x02B0) - Parktronic System State (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto pts_a1_02b0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PTS_ST_AUS",        "PTS ist komplett ausgeschaltet",                                                0,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_PTS (ID: 0x0413) - Network Management Parktronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_pts_0413_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_PTS (ID: 0x04F3) - Diagnostic Response Parktronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_pts_04f3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_PTS (ID: 0x0773) - Application Interface Parktronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_pts_0773_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TPM_A1 (ID: 0x02FF) - Tire Pressures & Warning Status (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tpm_a1_02ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TPM_MsgDisp_Rq",    "TPM message display request",                                                   0,   4 },
        {   1, "TPM_Stat",          "TPM state",                                                                     4,   4 },
        {   2, "TPM_Disp_On_Rq",    "Tire pressure modules display on request",                                      9,   1 },
        {   3, "TPM_MalfLmp_On_Rq", "Tire pressure module malfunction lamp on request",                             10,   1 },
        {   4, "Tire_LHOM",         "Tire in limp-home operation mode",                                             11,   1 },
        {   5, "TPM_IndLmp_On_Rq",  "Tire pressure module indication lamp on request",                              12,   1 },
        {   6, "TPM_WarnDisp_Rq",   "TPM warning display request",                                                  13,   3 },
        {   7, "Tire_Spr",          "spare tire",                                                                   16,   1 },
        {   8, "Tire_RR",           "Tire rear right",                                                              17,   1 },
        {   9, "Tire_RL",           "Tire rear left",                                                               18,   1 },
        {  10, "Tire_FR",           "Tire front right",                                                             19,   1 },
        {  11, "Tire_FL",           "Tire front left",                                                              20,   1 },
        {  12, "TirePress_FL",      "(bar) (Bar) Tire pressure front left",                                         24,   8 },
        {  13, "TirePress_FR",      "(bar) (Bar) Tire pressure front right",                                        32,   8 },
        {  14, "TirePress_RL",      "(bar) (Bar) Tire pressure left rear",                                          40,   8 },
        {  15, "TirePress_RR",      "(bar) (Bar) Tire pressure rear right",                                         48,   8 },
        {  16, "TirePress_Spr",     "(bar) (Bar) Tire pressure spare tire",                                         56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_TPM (ID: 0x0418) - Network Management Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_tpm_0418_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TPM (ID: 0x04F8) - Diagnostic Response Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tpm_04f8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_TPM",          "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_TPM (ID: 0x0778) - Application Interface Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_tpm_0778_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
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
        {   0, "CANCEL_HU",         "Rejection of numeric input HU",                                                 1,   1 },
        {   1, "STERN_HU",          "Star key operated",                                                             2,   1 },
        {   2, "RAUTE_HU",          "Argyle key is pressed",                                                         3,   1 },
        {   3, "ZIFFER_HU",         "Keypad Head Unit",                                                              4,   4 },
        {   4, "FUNKTIONSTASTEN",   "function keys",                                                                10,   3 },
        {   5, "AKT_SYS",           "active application",                                                           13,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GPS_A1 (ID: 0x0338) - GPS Latitude & Longitude (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gps_a1_0338_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LATITUDE",          "(ms) (Ms) GPS Latitude (North + =)",                                            0,  32 },
        {   1, "LONGITUDE",         "(ms) (Ms) GPS Longitude (East + =)",                                           32,  32 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GPS_A2 (ID: 0x0339) - GPS Date & UTC Time (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gps_a2_0339_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UTC_YEARS",         "(years) (Years) UTC years",                                                     0,  16 },
        {   1, "UTC_MONTHS",        "(months) (Months) UTC months",                                                 16,   8 },
        {   2, "UTC_DAYS",          "(days) (Days) UTC days",                                                       24,   8 },
        {   3, "UTC_HOURS",         "(h) (H) UTC hours",                                                            32,   8 },
        {   4, "UTC_MINUTES",       "(min) (Min) UTC minutes",                                                      40,   8 },
        {   5, "UTC_SECONDS",       "(s) (S) UTC seconds",                                                          48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GPS_A3 (ID: 0x033A) - GPS Dynamics, Fix & Dilution of Precision (13 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gps_a3_033a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GPS_SPEED",         "(cm/s) (Cm / s) GPS Speed",                                                     0,  16 },
        {   1, "GPS_HEADING",       "(°) (°) GPS Heading (0 ° = North)",                                            16,   8 },
        {   2, "GPS_HEIGHT",        "(m) (M) GPS height",                                                           24,   8 },
        {   3, "GPS_FIX",           "GPS fix",                                                                      37,   3 },
        {   4, "POS_AVLB",          "GPS position avilable",                                                        40,   1 },
        {   5, "DIFF_POS_AVLB",     "Differential GPS position data available",                                     41,   1 },
        {   6, "DEAD_RCK",          "Dead reckoning available",                                                     42,   1 },
        {   7, "IDG",               "Inside digitalized area (on map)",                                             43,   1 },
        {   8, "FDG",               "Fully digitalized area",                                                       44,   1 },
        {   9, "MDM",               "Matched to digitally map",                                                     45,   1 },
        {  10, "CALI",              "Calibrated",                                                                   46,   1 },
        {  11, "V_DOP",             "Vertical Dilution Of Position",                                                48,   8 },
        {  12, "H_DOP",             "Horizontal Dilution Of Position",                                              56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_AGW_KOMBI1 (ID: 0x01A4) - Transport Protocol AGW to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_agw_kombi1_01a4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_AGW_KOMBI",      "Kommunikation AGW zum KOMBI",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_AGW_MSS3 (ID: 0x0334) - Transport Protocol AGW to MSS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_agw_mss3_0334_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_AGW_MSS3",       "Kommunikation AGW zum MSS",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: TP_AGW_TELEAID6 (ID: 0x03E3) - Transport Protocol AGW to TeleAid (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto tp_agw_teleaid6_03e3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TP_AGW_TELEAID",    "Kommunikation AGW zum TELEAID",                                                 0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_AGW (ID: 0x0416) - Network Management Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_agw_0416_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_AGW (ID: 0x04F6) - Diagnostic Response Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_agw_04f6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST0 (ID: 0x0680) - Diagnostic Response MOST Gateway 0 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most0_0680_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST1 (ID: 0x0681) - Diagnostic Response MOST Gateway 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most1_0681_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST10 (ID: 0x068A) - Diagnostic Response MOST Gateway 10 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most10_068a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST11 (ID: 0x068B) - Diagnostic Response MOST Gateway 11 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most11_068b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST12 (ID: 0x068C) - Diagnostic Response MOST Gateway 12 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most12_068c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST13 (ID: 0x068D) - Diagnostic Response MOST Gateway 13 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most13_068d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST14 (ID: 0x068E) - Diagnostic Response MOST Gateway 14 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most14_068e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST15 (ID: 0x068F) - Diagnostic Response MOST Gateway 15 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most15_068f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST2 (ID: 0x0682) - Diagnostic Response MOST Gateway 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most2_0682_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST3 (ID: 0x0683) - Diagnostic Response MOST Gateway 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most3_0683_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST4 (ID: 0x0684) - Diagnostic Response MOST Gateway 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most4_0684_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST5 (ID: 0x0685) - Diagnostic Response MOST Gateway 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most5_0685_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST6 (ID: 0x0686) - Diagnostic Response MOST Gateway 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most6_0686_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST7 (ID: 0x0687) - Diagnostic Response MOST Gateway 7 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most7_0687_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST8 (ID: 0x0688) - Diagnostic Response MOST Gateway 8 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most8_0688_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MOST9 (ID: 0x0689) - Diagnostic Response MOST Gateway 9 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_most9_0689_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_NAVI (ID: 0x058A) - Diagnostic Response Navigation System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_navi_058a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_NAVI",         "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_AGW (ID: 0x0776) - Application Interface Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_agw_0776_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL",           "Steuergerät an externe Applikation",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_AGW (ID: 0x07D6) - System Diagnostic Response Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_agw_07d6_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "Systemdiagnose-Response",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: NM_ICANI (ID: 0x043B) - Network Management CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto nm_icani_043b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NM",                "Netzwerkmanagement",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ICANI (ID: 0x04BB) - Diagnostic Response CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_icani_04bb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_ICANI",        "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_ICANI (ID: 0x05BB) - System Diagnostic Response CAN Interface (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_icani_05bb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS_ICANI",       "Systemdiagnose-Response",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_A3 (ID: 0x01B8) - Odometer Value Legacy (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_a3_01b8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KM_EZS_ALT",        "(km) Kilometerstand (bis ÄJ 2002/1)",                                           0,  24 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FDSVL (ID: 0x04FE) - Diagnostic Response Dynamic Seat Front Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fdsvl_04fe_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FDSVR (ID: 0x04FF) - Diagnostic Response Dynamic Seat Front Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fdsvr_04ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_LRK (ID: 0x04EF) - Diagnostic Response Heated Steering Wheel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_lrk_04ef_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_PFDS (ID: 0x04EE) - Diagnostic Response Pneumatic Dynamic Seat (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_pfds_04ee_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_SHZ (ID: 0x04FB) - Diagnostic Response Seat Heating (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_shz_04fb_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_TLM (ID: 0x04FA) - Diagnostic Response Telematics Control Unit (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_tlm_04fa_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 Diagnose-Response",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_AAG (ID: 0x072F) - Application Interface to Trailer Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_aag_072f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_AGW (ID: 0x05C9) - Application Interface to Audio Gateway (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_agw_05c9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_ARMADA (ID: 0x06A3) - Application Interface to Restraint System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_armada_06a3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_DBE (ID: 0x0678) - Application Interface to Overhead Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_dbe_0678_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_EZS (ID: 0x04DF) - Application Interface to Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_ezs_04df_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_HFS (ID: 0x0568) - Application Interface to Trunk Remote Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_hfs_0568_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_KLA (ID: 0x078E) - Application Interface to Climate Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_kla_078e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_KOMBI (ID: 0x05AB) - Application Interface to Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_kombi_05ab_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_MRM (ID: 0x06C1) - Application Interface to Steering Column Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_mrm_06c1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_MSS (ID: 0x0720) - Application Interface to Special Vehicle Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_mss_0720_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_OBF (ID: 0x06BA) - Application Interface to Upper Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_obf_06ba_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_PTS (ID: 0x072C) - Application Interface to Parktronic (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_pts_072c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SAM_H (ID: 0x057C) - Application Interface to Rear SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sam_h_057c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SAM_V (ID: 0x067D) - Application Interface to Front SAM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sam_v_067d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SB (ID: 0x06B2) - Application Interface to Passenger Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sb_06b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_SF (ID: 0x06B3) - Application Interface to Driver Seat Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sf_06b3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_STH (ID: 0x073F) - Application Interface to Auxiliary Heater (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_sth_073f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_THL (ID: 0x0756) - Application Interface to Rear Left Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_thl_0756_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_THR (ID: 0x0754) - Application Interface to Rear Right Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_thr_0754_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_TPM (ID: 0x06A7) - Application Interface to Tire Pressure Monitor (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_tpm_06a7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_TVL (ID: 0x06D7) - Application Interface to Driver Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_tvl_06d7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_TVR (ID: 0x06C0) - Application Interface to Passenger Door Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_tvr_06c0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_UBF (ID: 0x0722) - Application Interface to Lower Control Panel (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_ubf_0722_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_WSS (ID: 0x06B7) - Application Interface to Weight Sensing System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_wss_06b7_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG",           "Externe Applikation zu Steuergerät",                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EZS (ID: 0x04E0) - Diagnostic Request Ignition Switch (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ezs_04e0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 Diagnose-Request",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // W209 Message Definitions Table
    // -------------------------------------------------------------------------------------------------------------- //
    const auto w209_messages = std::to_array<const can_message_spec>
    ({
        { 0x0000, false, 8, "UNKNOWN",           "Central Locking & Terminal Status",                  unknown_0000_signals.data(),           unknown_0000_signals.size()           },
        { 0x010A, false, 8, "EZS_A10",           "Rear Wheel Speed & Direction",                       ezs_a10_010a_signals.data(),           ezs_a10_010a_signals.size()           },
        { 0x0016, false, 8, "EZS_A11",           "Battery Voltage",                                    ezs_a11_0016_signals.data(),           ezs_a11_0016_signals.size()           },
        { 0x0002, false, 8, "EZS_A2",            "Front Wheel Speed & Engine Data",                    ezs_a2_0002_signals.data(),            ezs_a2_0002_signals.size()            },
        { 0x0058, false, 8, "EZS_A4",            "Key Identification & Mileage",                       ezs_a4_0058_signals.data(),            ezs_a4_0058_signals.size()            },
        { 0x001F, false, 8, "EZS_A5",            "Vehicle Variant & Equipment Coding",                 ezs_a5_001f_signals.data(),            ezs_a5_001f_signals.size()            },
        { 0x001E, false, 8, "EZS_A6",            "Model Year & Variant Coding",                        ezs_a6_001e_signals.data(),            ezs_a6_001e_signals.size()            },
        { 0x0003, false, 8, "EZS_A7",            "Drive Status & Warnings",                            ezs_a7_0003_signals.data(),            ezs_a7_0003_signals.size()            },
        { 0x0390, false, 8, "EZS_A8",            "Climate Control Coding & Defaults",                  ezs_a8_0390_signals.data(),            ezs_a8_0390_signals.size()            },
        { 0x00B2, false, 8, "EZS_A9",            "Vehicle Identification Number",                      ezs_a9_00b2_signals.data(),            ezs_a9_00b2_signals.size()            },
        { 0x01B2, false, 8, "KG_A1",             "Keyless Go Status & Alerts",                         kg_a1_01b2_signals.data(),             kg_a1_01b2_signals.size()             },
        { 0x0050, false, 8, "KG_A2",             "Keyless Go Window & Roof Control",                   kg_a2_0050_signals.data(),             kg_a2_0050_signals.size()             },
        { 0x018D, false, 8, "TELEAID_A2",        "TeleAid Status & Heartbeat",                         teleaid_a2_018d_signals.data(),        teleaid_a2_018d_signals.size()        },
        { 0x03E5, false, 8, "TELEAID_POS1",      "TeleAid GPS Coordinates",                            teleaid_pos1_03e5_signals.data(),      teleaid_pos1_03e5_signals.size()      },
        { 0x03E6, false, 8, "TELEAID_POS2",      "TeleAid GPS Dynamics & Altitude",                    teleaid_pos2_03e6_signals.data(),      teleaid_pos2_03e6_signals.size()      },
        { 0x03E7, false, 8, "TELEAID_POS3",      "TeleAid GPS Date & UTC Time",                        teleaid_pos3_03e7_signals.data(),      teleaid_pos3_03e7_signals.size()      },
        { 0x03E8, false, 8, "TELEAID_POS4",      "TeleAid Dead Reckoning Coordinates",                 teleaid_pos4_03e8_signals.data(),      teleaid_pos4_03e8_signals.size()      },
        { 0x03E9, false, 8, "TELEAID_POS5",      "TeleAid GPS Satellite & Navigation Status",          teleaid_pos5_03e9_signals.data(),      teleaid_pos5_03e9_signals.size()      },
        { 0x0005, false, 8, "GW_C_B7",           "Gateway Front Right Wheel Speed",                    gw_c_b7_0005_signals.data(),           gw_c_b7_0005_signals.size()           },
        { 0x0209, false, 8, "TP_TELEAID_AGW6",   "Transport Protocol TeleAid to AGW",                  tp_teleaid_agw6_0209_signals.data(),   tp_teleaid_agw6_0209_signals.size()   },
        { 0x01A1, false, 8, "TP_TELEAID_KOMBI4", "Transport Protocol TeleAid to Instrument Cluster",   tp_teleaid_kombi4_01a1_signals.data(), tp_teleaid_kombi4_01a1_signals.size() },
        { 0x0400, false, 8, "NM_EZS",            "Network Management Ignition Switch",                 nm_ezs_0400_signals.data(),            nm_ezs_0400_signals.size()            },
        { 0x05FF, false, 8, "D_RS_EZS",          "Diagnostic Response Ignition Switch",                d_rs_ezs_05ff_signals.data(),          d_rs_ezs_05ff_signals.size()          },
        { 0x0760, false, 8, "SG_APPL_EZS",       "Application Interface Ignition Switch",              sg_appl_ezs_0760_signals.data(),       sg_appl_ezs_0760_signals.size()       },
        { 0x0730, false, 8, "D_RQ_AAG",          "Diagnostic Request Trailer Recognition",             d_rq_aag_0730_signals.data(),          d_rq_aag_0730_signals.size()          },
        { 0x05D6, false, 8, "D_RQ_AGW",          "Diagnostic Request Audio Gateway",                   d_rq_agw_05d6_signals.data(),          d_rq_agw_05d6_signals.size()          },
        { 0x06BC, false, 8, "D_RQ_ARMADA",       "Diagnostic Request Restraint Systems Airbag",        d_rq_armada_06bc_signals.data(),       d_rq_armada_06bc_signals.size()       },
        { 0x0667, false, 8, "D_RQ_DBE",          "Diagnostic Request Overhead Control Panel",          d_rq_dbe_0667_signals.data(),          d_rq_dbe_0667_signals.size()          },
        { 0x06BE, false, 8, "D_RQ_FDSVL",        "Diagnostic Request Dynamic Seat Front Left",         d_rq_fdsvl_06be_signals.data(),        d_rq_fdsvl_06be_signals.size()        },
        { 0x06BF, false, 8, "D_RQ_FDSVR",        "Diagnostic Request Dynamic Seat Front Right",        d_rq_fdsvr_06bf_signals.data(),        d_rq_fdsvr_06bf_signals.size()        },
        { 0x0577, false, 8, "D_RQ_HFS",          "Diagnostic Request Trunk Remote Control",            d_rq_hfs_0577_signals.data(),          d_rq_hfs_0577_signals.size()          },
        { 0x07DA, false, 8, "D_RQ_ICANI",        "Diagnostic Request CAN Interface",                   d_rq_icani_07da_signals.data(),        d_rq_icani_07da_signals.size()        },
        { 0x0791, false, 8, "D_RQ_KLA",          "Diagnostic Request Automatic Climate Control",       d_rq_kla_0791_signals.data(),          d_rq_kla_0791_signals.size()          },
        { 0x05B4, false, 8, "D_RQ_KOMBI",        "Diagnostic Request Instrument Cluster",              d_rq_kombi_05b4_signals.data(),        d_rq_kombi_05b4_signals.size()        },
        { 0x06AF, false, 8, "D_RQ_LRK",          "Diagnostic Request Heated Steering Wheel",           d_rq_lrk_06af_signals.data(),          d_rq_lrk_06af_signals.size()          },
        { 0x0640, false, 8, "D_RQ_MOST0",        "Diagnostic Request MOST Gateway 0",                  d_rq_most0_0640_signals.data(),        d_rq_most0_0640_signals.size()        },
        { 0x0641, false, 8, "D_RQ_MOST1",        "Diagnostic Request MOST Gateway 1",                  d_rq_most1_0641_signals.data(),        d_rq_most1_0641_signals.size()        },
        { 0x064A, false, 8, "D_RQ_MOST10",       "Diagnostic Request MOST Gateway 10",                 d_rq_most10_064a_signals.data(),       d_rq_most10_064a_signals.size()       },
        { 0x064B, false, 8, "D_RQ_MOST11",       "Diagnostic Request MOST Gateway 11",                 d_rq_most11_064b_signals.data(),       d_rq_most11_064b_signals.size()       },
        { 0x064C, false, 8, "D_RQ_MOST12",       "Diagnostic Request MOST Gateway 12",                 d_rq_most12_064c_signals.data(),       d_rq_most12_064c_signals.size()       },
        { 0x064D, false, 8, "D_RQ_MOST13",       "Diagnostic Request MOST Gateway 13",                 d_rq_most13_064d_signals.data(),       d_rq_most13_064d_signals.size()       },
        { 0x064E, false, 8, "D_RQ_MOST14",       "Diagnostic Request MOST Gateway 14",                 d_rq_most14_064e_signals.data(),       d_rq_most14_064e_signals.size()       },
        { 0x064F, false, 8, "D_RQ_MOST15",       "Diagnostic Request MOST Gateway 15",                 d_rq_most15_064f_signals.data(),       d_rq_most15_064f_signals.size()       },
        { 0x0642, false, 8, "D_RQ_MOST2",        "Diagnostic Request MOST Gateway 2",                  d_rq_most2_0642_signals.data(),        d_rq_most2_0642_signals.size()        },
        { 0x0643, false, 8, "D_RQ_MOST3",        "Diagnostic Request MOST Gateway 3",                  d_rq_most3_0643_signals.data(),        d_rq_most3_0643_signals.size()        },
        { 0x0644, false, 8, "D_RQ_MOST4",        "Diagnostic Request MOST Gateway 4",                  d_rq_most4_0644_signals.data(),        d_rq_most4_0644_signals.size()        },
        { 0x0645, false, 8, "D_RQ_MOST5",        "Diagnostic Request MOST Gateway 5",                  d_rq_most5_0645_signals.data(),        d_rq_most5_0645_signals.size()        },
        { 0x0646, false, 8, "D_RQ_MOST6",        "Diagnostic Request MOST Gateway 6",                  d_rq_most6_0646_signals.data(),        d_rq_most6_0646_signals.size()        },
        { 0x0647, false, 8, "D_RQ_MOST7",        "Diagnostic Request MOST Gateway 7",                  d_rq_most7_0647_signals.data(),        d_rq_most7_0647_signals.size()        },
        { 0x0648, false, 8, "D_RQ_MOST8",        "Diagnostic Request MOST Gateway 8",                  d_rq_most8_0648_signals.data(),        d_rq_most8_0648_signals.size()        },
        { 0x0649, false, 8, "D_RQ_MOST9",        "Diagnostic Request MOST Gateway 9",                  d_rq_most9_0649_signals.data(),        d_rq_most9_0649_signals.size()        },
        { 0x06D5, false, 8, "D_RQ_MRM",          "Diagnostic Request Steering Column Module",          d_rq_mrm_06d5_signals.data(),          d_rq_mrm_06d5_signals.size()          },
        { 0x0726, false, 8, "D_RQ_MSS",          "Diagnostic Request Special Vehicle Control",         d_rq_mss_0726_signals.data(),          d_rq_mss_0726_signals.size()          },
        { 0x054A, false, 8, "D_RQ_NAVI",         "Diagnostic Request Navigation System",               d_rq_navi_054a_signals.data(),         d_rq_navi_054a_signals.size()         },
        { 0x06A5, false, 8, "D_RQ_OBF",          "Diagnostic Request Upper Control Panel",             d_rq_obf_06a5_signals.data(),          d_rq_obf_06a5_signals.size()          },
        { 0x072E, false, 8, "D_RQ_PFDS",         "Diagnostic Request Pneumatic Dynamic Seat",          d_rq_pfds_072e_signals.data(),         d_rq_pfds_072e_signals.size()         },
        { 0x0733, false, 8, "D_RQ_PTS",          "Diagnostic Request Parktronic System",               d_rq_pts_0733_signals.data(),          d_rq_pts_0733_signals.size()          },
        { 0x0563, false, 8, "D_RQ_SAM_H",        "Diagnostic Request Rear SAM",                        d_rq_sam_h_0563_signals.data(),        d_rq_sam_h_0563_signals.size()        },
        { 0x0662, false, 8, "D_RQ_SAM_V",        "Diagnostic Request Front SAM",                       d_rq_sam_v_0662_signals.data(),        d_rq_sam_v_0662_signals.size()        },
        { 0x06AD, false, 8, "D_RQ_SB",           "Diagnostic Request Passenger Seat Adjustment",       d_rq_sb_06ad_signals.data(),           d_rq_sb_06ad_signals.size()           },
        { 0x06AC, false, 8, "D_RQ_SF",           "Diagnostic Request Driver Seat Adjustment",          d_rq_sf_06ac_signals.data(),           d_rq_sf_06ac_signals.size()           },
        { 0x057B, false, 8, "D_RQ_SHZ",          "Diagnostic Request Seat Heating",                    d_rq_shz_057b_signals.data(),          d_rq_shz_057b_signals.size()          },
        { 0x0739, false, 8, "D_RQ_STH",          "Diagnostic Request Stationary Auxiliary Heater",     d_rq_sth_0739_signals.data(),          d_rq_sth_0739_signals.size()          },
        { 0x0749, false, 8, "D_RQ_THL",          "Diagnostic Request Rear Left Door Control Module",   d_rq_thl_0749_signals.data(),          d_rq_thl_0749_signals.size()          },
        { 0x074B, false, 8, "D_RQ_THR",          "Diagnostic Request Rear Right Door Control Module",  d_rq_thr_074b_signals.data(),          d_rq_thr_074b_signals.size()          },
        { 0x05DA, false, 8, "D_RQ_TLM",          "Diagnostic Request Telematics Control Unit",         d_rq_tlm_05da_signals.data(),          d_rq_tlm_05da_signals.size()          },
        { 0x06B8, false, 8, "D_RQ_TPM",          "Diagnostic Request Tire Pressure Monitor",           d_rq_tpm_06b8_signals.data(),          d_rq_tpm_06b8_signals.size()          },
        { 0x06C8, false, 8, "D_RQ_TVL",          "Diagnostic Request Front Left Door Control Module",  d_rq_tvl_06c8_signals.data(),          d_rq_tvl_06c8_signals.size()          },
        { 0x06CA, false, 8, "D_RQ_TVR",          "Diagnostic Request Front Right Door Control Module", d_rq_tvr_06ca_signals.data(),          d_rq_tvr_06ca_signals.size()          },
        { 0x073D, false, 8, "D_RQ_UBF",          "Diagnostic Request Lower Control Panel",             d_rq_ubf_073d_signals.data(),          d_rq_ubf_073d_signals.size()          },
        { 0x06A8, false, 8, "D_RQ_WSS",          "Diagnostic Request Weight Sensing System",           d_rq_wss_06a8_signals.data(),          d_rq_wss_06a8_signals.size()          },
        { 0x001C, false, 8, "D_RQ_GLOBAL",       "Global Diagnostic Request",                          d_rq_global_001c_signals.data(),       d_rq_global_001c_signals.size()       },
        { 0x000C, false, 8, "KOMBI_A1",          "Lighting, Speed & Cluster Config",                   kombi_a1_000c_signals.data(),          kombi_a1_000c_signals.size()          },
        { 0x00D4, false, 8, "KOMBI_A3",          "Clock, Odometer & Cruising Range",                   kombi_a3_00d4_signals.data(),          kombi_a3_00d4_signals.size()          },
        { 0x01CA, false, 8, "KOMBI_A5",          "Multifunction Steering Wheel Buttons",               kombi_a5_01ca_signals.data(),          kombi_a5_01ca_signals.size()          },
        { 0x009E, false, 8, "KOMBI_A6",          "Key ID & Redundant Odometer",                        kombi_a6_009e_signals.data(),          kombi_a6_009e_signals.size()          },
        { 0x0194, false, 8, "KOMBI_A7",          "Display Dimming & Tailgate Setting",                 kombi_a7_0194_signals.data(),          kombi_a7_0194_signals.size()          },
        { 0x032A, false, 8, "KOMBI_A8",          "Multifunction Steering Wheel MSS Buttons",           kombi_a8_032a_signals.data(),          kombi_a8_032a_signals.size()          },
        { 0x01D0, false, 8, "TP_KOMBI_AGW1",     "Transport Protocol Instrument Cluster to AGW",       tp_kombi_agw1_01d0_signals.data(),     tp_kombi_agw1_01d0_signals.size()     },
        { 0x0330, false, 8, "TP_KOMBI_MSS2",     "Transport Protocol Instrument Cluster to MSS",       tp_kombi_mss2_0330_signals.data(),     tp_kombi_mss2_0330_signals.size()     },
        { 0x03E1, false, 8, "TP_KOMBI_TELEAID4", "Transport Protocol Instrument Cluster to TeleAid",   tp_kombi_teleaid4_03e1_signals.data(), tp_kombi_teleaid4_03e1_signals.size() },
        { 0x0414, false, 8, "NM_KOMBI",          "Network Management Instrument Cluster",              nm_kombi_0414_signals.data(),          nm_kombi_0414_signals.size()          },
        { 0x04F4, false, 8, "D_RS_KOMBI",        "Diagnostic Response Instrument Cluster",             d_rs_kombi_04f4_signals.data(),        d_rs_kombi_04f4_signals.size()        },
        { 0x0774, false, 8, "SG_APPL_KOMBI",     "Application Interface Instrument Cluster",           sg_appl_kombi_0774_signals.data(),     sg_appl_kombi_0774_signals.size()     },
        { 0x0006, false, 8, "MRM_A1",            "Steering Column Switches & Steering Angle",          mrm_a1_0006_signals.data(),            mrm_a1_0006_signals.size()            },
        { 0x01A8, false, 8, "MRM_A2",            "Steering Wheel Rocker Switches",                     mrm_a2_01a8_signals.data(),            mrm_a2_01a8_signals.size()            },
        { 0x0296, false, 8, "MRM_A3",            "Steering Column Adjustment Lever",                   mrm_a3_0296_signals.data(),            mrm_a3_0296_signals.size()            },
        { 0x0415, false, 8, "NM_MRM",            "Network Management Steering Column Module",          nm_mrm_0415_signals.data(),            nm_mrm_0415_signals.size()            },
        { 0x04F5, false, 8, "D_RS_MRM",          "Diagnostic Response Steering Column Module",         d_rs_mrm_04f5_signals.data(),          d_rs_mrm_04f5_signals.size()          },
        { 0x0775, false, 8, "SG_APPL_MRM",       "Application Interface Steering Column Module",       sg_appl_mrm_0775_signals.data(),       sg_appl_mrm_0775_signals.size()       },
        { 0x07D5, false, 8, "SD_RS_MRM",         "System Diagnostic Response Steering Column Module",  sd_rs_mrm_07d5_signals.data(),         sd_rs_mrm_07d5_signals.size()         },
        { 0x000A, false, 8, "SAM_V_A1",          "Front Lighting & Exterior Status",                   sam_v_a1_000a_signals.data(),          sam_v_a1_000a_signals.size()          },
        { 0x0017, false, 8, "SAM_V_A2",          "Outside Temperature & AC Refrigerant Data",          sam_v_a2_0017_signals.data(),          sam_v_a2_0017_signals.size()          },
        { 0x0070, false, 8, "SAM_V_A3",          "Rain Sensor & Wiper Status",                         sam_v_a3_0070_signals.data(),          sam_v_a3_0070_signals.size()          },
        { 0x02CC, false, 8, "SAM_V_A4",          "Exterior Mirror Adjustment",                         sam_v_a4_02cc_signals.data(),          sam_v_a4_02cc_signals.size()          },
        { 0x0402, false, 8, "NM_SAM_V",          "Network Management Front SAM",                       nm_sam_v_0402_signals.data(),          nm_sam_v_0402_signals.size()          },
        { 0x04E2, false, 8, "D_RS_SAM_V",        "Diagnostic Response Front SAM",                      d_rs_sam_v_04e2_signals.data(),        d_rs_sam_v_04e2_signals.size()        },
        { 0x0762, false, 8, "SG_APPL_SAM_V",     "Application Interface Front SAM",                    sg_appl_sam_v_0762_signals.data(),     sg_appl_sam_v_0762_signals.size()     },
        { 0x0004, false, 8, "SAM_H_A1",          "Rear Lighting, Doors & Alarm Status",                sam_h_a1_0004_signals.data(),          sam_h_a1_0004_signals.size()          },
        { 0x0090, false, 8, "SAM_H_A2",          "Fuel Tank Level & Sensors",                          sam_h_a2_0090_signals.data(),          sam_h_a2_0090_signals.size()          },
        { 0x000E, false, 8, "SAM_H_A3",          "Turn Signals & Hazard Warning Flasher",              sam_h_a3_000e_signals.data(),          sam_h_a3_000e_signals.size()          },
        { 0x0041, false, 8, "SAM_H_A4",          "Central Locking Emergency Opening",                  sam_h_a4_0041_signals.data(),          sam_h_a4_0041_signals.size()          },
        { 0x0230, false, 8, "SAM_H_A5",          "Alarm Lighting Activation",                          sam_h_a5_0230_signals.data(),          sam_h_a5_0230_signals.size()          },
        { 0x00CC, false, 8, "SAM_H_A6",          "Access Authorization Code",                          sam_h_a6_00cc_signals.data(),          sam_h_a6_00cc_signals.size()          },
        { 0x0403, false, 8, "NM_SAM_H",          "Network Management Rear SAM",                        nm_sam_h_0403_signals.data(),          nm_sam_h_0403_signals.size()          },
        { 0x04E3, false, 8, "D_RS_SAM_H",        "Diagnostic Response Rear SAM",                       d_rs_sam_h_04e3_signals.data(),        d_rs_sam_h_04e3_signals.size()        },
        { 0x0763, false, 8, "SG_APPL_SAM_H",     "Application Interface Rear SAM",                     sg_appl_sam_h_0763_signals.data(),     sg_appl_sam_h_0763_signals.size()     },
        { 0x0130, false, 8, "AAG_A1",            "Trailer Recognition & Lighting Status",              aag_a1_0130_signals.data(),            aag_a1_0130_signals.size()            },
        { 0x0410, false, 8, "NM_AAG",            "Network Management Trailer Recognition",             nm_aag_0410_signals.data(),            nm_aag_0410_signals.size()            },
        { 0x04F0, false, 8, "D_RS_AAG",          "Diagnostic Response Trailer Recognition",            d_rs_aag_04f0_signals.data(),          d_rs_aag_04f0_signals.size()          },
        { 0x0770, false, 8, "SG_APPL_AAG",       "Application Interface Trailer Recognition",          sg_appl_aag_0770_signals.data(),       sg_appl_aag_0770_signals.size()       },
        { 0x07D0, false, 8, "SD_RS_AAG",         "System Diagnostic Response Trailer Recognition",     sd_rs_aag_07d0_signals.data(),         sd_rs_aag_07d0_signals.size()         },
        { 0x028C, false, 8, "TVL_A1",            "Driver Seat Adjustment & Memory Switch",             tvl_a1_028c_signals.data(),            tvl_a1_028c_signals.size()            },
        { 0x0044, false, 8, "TVL_A2",            "Power Window Controls Driver Door",                  tvl_a2_0044_signals.data(),            tvl_a2_0044_signals.size()            },
        { 0x0018, false, 8, "TVL_A3",            "Driver Window Position & Mirror Switch",             tvl_a3_0018_signals.data(),            tvl_a3_0018_signals.size()            },
        { 0x00E8, false, 8, "TVL_A4",            "Driver Door Access Authorization Code",              tvl_a4_00e8_signals.data(),            tvl_a4_00e8_signals.size()            },
        { 0x0408, false, 8, "NM_TVL",            "Network Management Driver Door Module",              nm_tvl_0408_signals.data(),            nm_tvl_0408_signals.size()            },
        { 0x04E8, false, 8, "D_RS_TVL",          "Diagnostic Response Driver Door Module",             d_rs_tvl_04e8_signals.data(),          d_rs_tvl_04e8_signals.size()          },
        { 0x0768, false, 8, "SG_APPL_TVL",       "Application Interface Driver Door Module",           sg_appl_tvl_0768_signals.data(),       sg_appl_tvl_0768_signals.size()       },
        { 0x0290, false, 8, "TVR_A1",            "Passenger Seat Adjustment & Memory Switch",          tvr_a1_0290_signals.data(),            tvr_a1_0290_signals.size()            },
        { 0x0045, false, 8, "TVR_A2",            "Power Window Controls Passenger Door",               tvr_a2_0045_signals.data(),            tvr_a2_0045_signals.size()            },
        { 0x0019, false, 8, "TVR_A3",            "Passenger Window Position & Controls",               tvr_a3_0019_signals.data(),            tvr_a3_0019_signals.size()            },
        { 0x00EC, false, 8, "TVR_A4",            "Passenger Door Access Authorization Code",           tvr_a4_00ec_signals.data(),            tvr_a4_00ec_signals.size()            },
        { 0x040A, false, 8, "NM_TVR",            "Network Management Passenger Door Module",           nm_tvr_040a_signals.data(),            nm_tvr_040a_signals.size()            },
        { 0x04EA, false, 8, "D_RS_TVR",          "Diagnostic Response Passenger Door Module",          d_rs_tvr_04ea_signals.data(),          d_rs_tvr_04ea_signals.size()          },
        { 0x076A, false, 8, "SG_APPL_TVR",       "Application Interface Passenger Door Module",        sg_appl_tvr_076a_signals.data(),       sg_appl_tvr_076a_signals.size()       },
        { 0x009A, false, 8, "THL_A1",            "Rear Left Window Position & Controls",               thl_a1_009a_signals.data(),            thl_a1_009a_signals.size()            },
        { 0x0409, false, 8, "NM_THL",            "Network Management Rear Left Door Module",           nm_thl_0409_signals.data(),            nm_thl_0409_signals.size()            },
        { 0x04E9, false, 8, "D_RS_THL",          "Diagnostic Response Rear Left Door Module",          d_rs_thl_04e9_signals.data(),          d_rs_thl_04e9_signals.size()          },
        { 0x0769, false, 8, "SG_APPL_THL",       "Application Interface Rear Left Door Module",        sg_appl_thl_0769_signals.data(),       sg_appl_thl_0769_signals.size()       },
        { 0x009C, false, 8, "THR_A1",            "Rear Right Window Position & Controls",              thr_a1_009c_signals.data(),            thr_a1_009c_signals.size()            },
        { 0x040B, false, 8, "NM_THR",            "Network Management Rear Right Door Module",          nm_thr_040b_signals.data(),            nm_thr_040b_signals.size()            },
        { 0x04EB, false, 8, "D_RS_THR",          "Diagnostic Response Rear Right Door Module",         d_rs_thr_04eb_signals.data(),          d_rs_thr_04eb_signals.size()          },
        { 0x076B, false, 8, "SG_APPL_THR",       "Application Interface Rear Right Door Module",       sg_appl_thr_076b_signals.data(),       sg_appl_thr_076b_signals.size()       },
        { 0x0078, false, 8, "HFS_A1",            "Trunk Remote Closing & Tailgate Status",             hfs_a1_0078_signals.data(),            hfs_a1_0078_signals.size()            },
        { 0x0417, false, 8, "NM_HFS",            "Network Management Trunk Remote Control",            nm_hfs_0417_signals.data(),            nm_hfs_0417_signals.size()            },
        { 0x04F7, false, 8, "D_RS_HFS",          "Diagnostic Response Trunk Remote Control",           d_rs_hfs_04f7_signals.data(),          d_rs_hfs_04f7_signals.size()          },
        { 0x0777, false, 8, "SG_APPL_HFS",       "Application Interface Trunk Remote Control",         sg_appl_hfs_0777_signals.data(),       sg_appl_hfs_0777_signals.size()       },
        { 0x07D7, false, 8, "SD_RS_HFS",         "System Diagnostic Response Trunk Remote Control",    sd_rs_hfs_07d7_signals.data(),         sd_rs_hfs_07d7_signals.size()         },
        { 0x00FD, false, 8, "D_RS_VS",           "Diagnostic Response Soft Top Control",               d_rs_vs_00fd_signals.data(),           d_rs_vs_00fd_signals.size()           },
        { 0x000B, false, 8, "VS_A1",             "Soft Top & Roll Bar Status Messages",                vs_a1_000b_signals.data(),             vs_a1_000b_signals.size()             },
        { 0x0010, false, 8, "VS_A2",             "Soft Top Window Movement & Rollover Status",         vs_a2_0010_signals.data(),             vs_a2_0010_signals.size()             },
        { 0x0014, false, 8, "DBE_A1",            "Interior Lights, Rain Sensor & Sunroof Status",      dbe_a1_0014_signals.data(),            dbe_a1_0014_signals.size()            },
        { 0x0270, false, 8, "DBE_A2",            "Rain Sensor & Wiper Stages",                         dbe_a2_0270_signals.data(),            dbe_a2_0270_signals.size()            },
        { 0x02D4, false, 8, "DBE_A3",            "Automatic Dimming Interior Mirror",                  dbe_a3_02d4_signals.data(),            dbe_a3_02d4_signals.size()            },
        { 0x0174, false, 8, "DBE_A4",            "Sunroof Auto-Close on Rain",                         dbe_a4_0174_signals.data(),            dbe_a4_0174_signals.size()            },
        { 0x0407, false, 8, "NM_DBE",            "Network Management Overhead Control Panel",          nm_dbe_0407_signals.data(),            nm_dbe_0407_signals.size()            },
        { 0x04E7, false, 8, "D_RS_DBE",          "Diagnostic Response Overhead Control Panel",         d_rs_dbe_04e7_signals.data(),          d_rs_dbe_04e7_signals.size()          },
        { 0x0767, false, 8, "SG_APPL_DBE",       "Application Interface Overhead Control Panel",       sg_appl_dbe_0767_signals.data(),       sg_appl_dbe_0767_signals.size()       },
        { 0x0288, false, 8, "LRK_A1",            "Heated Steering Wheel Status & LED",                 lrk_a1_0288_signals.data(),            lrk_a1_0288_signals.size()            },
        { 0x002C, false, 8, "OBF_A1",            "Upper Control Panel Switch Status",                  obf_a1_002c_signals.data(),            obf_a1_002c_signals.size()            },
        { 0x0405, false, 8, "NM_OBF",            "Network Management Upper Control Panel",             nm_obf_0405_signals.data(),            nm_obf_0405_signals.size()            },
        { 0x04E5, false, 8, "D_RS_OBF",          "Diagnostic Response Upper Control Panel",            d_rs_obf_04e5_signals.data(),          d_rs_obf_04e5_signals.size()          },
        { 0x0765, false, 8, "SG_APPL_OBF",       "Application Interface Upper Control Panel",          sg_appl_obf_0765_signals.data(),       sg_appl_obf_0765_signals.size()       },
        { 0x001A, false, 8, "UBF_A1",            "Lower Control Panel Switches & Distance Setting",    ubf_a1_001a_signals.data(),            ubf_a1_001a_signals.size()            },
        { 0x035B, false, 8, "UBF_A2",            "Lower Control Panel Soft Top Operation",             ubf_a2_035b_signals.data(),            ubf_a2_035b_signals.size()            },
        { 0x041D, false, 8, "NM_UBF",            "Network Management Lower Control Panel",             nm_ubf_041d_signals.data(),            nm_ubf_041d_signals.size()            },
        { 0x04FD, false, 8, "D_RS_UBF",          "Diagnostic Response Lower Control Panel",            d_rs_ubf_04fd_signals.data(),          d_rs_ubf_04fd_signals.size()          },
        { 0x077D, false, 8, "SG_APPL_UBF",       "Application Interface Lower Control Panel",          sg_appl_ubf_077d_signals.data(),       sg_appl_ubf_077d_signals.size()       },
        { 0x0012, false, 8, "ARMADA_A1",         "Restraint Systems & Occupant Classification",        armada_a1_0012_signals.data(),         armada_a1_0012_signals.size()         },
        { 0x0040, false, 8, "ARMADA_A2",         "Crash Event Confirmation & Triggers",                armada_a2_0040_signals.data(),         armada_a2_0040_signals.size()         },
        { 0x041C, false, 8, "NM_ARMADA",         "Network Management Restraint System",                nm_armada_041c_signals.data(),         nm_armada_041c_signals.size()         },
        { 0x04FC, false, 8, "D_RS_ARMADA",       "Diagnostic Response Restraint System",               d_rs_armada_04fc_signals.data(),       d_rs_armada_04fc_signals.size()       },
        { 0x077C, false, 8, "SG_APPL_ARMADA",    "Application Interface Restraint System",             sg_appl_armada_077c_signals.data(),    sg_appl_armada_077c_signals.size()    },
        { 0x02A4, false, 8, "WSS_A1",            "Weight Sensing System Classification",               wss_a1_02a4_signals.data(),            wss_a1_02a4_signals.size()            },
        { 0x0426, false, 8, "NM_WSS",            "Network Management Weight Sensing System",           nm_wss_0426_signals.data(),            nm_wss_0426_signals.size()            },
        { 0x04C6, false, 8, "D_RS_WSS",          "Diagnostic Response Weight Sensing System",          d_rs_wss_04c6_signals.data(),          d_rs_wss_04c6_signals.size()          },
        { 0x0706, false, 8, "SG_APPL_WSS",       "Application Interface Weight Sensing System",        sg_appl_wss_0706_signals.data(),       sg_appl_wss_0706_signals.size()       },
        { 0x01AC, false, 8, "SF_A1",             "Driver Seat Position & Backrest Status",             sf_a1_01ac_signals.data(),             sf_a1_01ac_signals.size()             },
        { 0x02D0, false, 8, "SF_A2",             "Driver Seat Memory & Entry/Exit Positioning",        sf_a2_02d0_signals.data(),             sf_a2_02d0_signals.size()             },
        { 0x040C, false, 8, "NM_SF",             "Network Management Driver Seat Module",              nm_sf_040c_signals.data(),             nm_sf_040c_signals.size()             },
        { 0x04EC, false, 8, "D_RS_SF",           "Diagnostic Response Driver Seat Module",             d_rs_sf_04ec_signals.data(),           d_rs_sf_04ec_signals.size()           },
        { 0x076C, false, 8, "SG_APPL_SF",        "Application Interface Driver Seat Module",           sg_appl_sf_076c_signals.data(),        sg_appl_sf_076c_signals.size()        },
        { 0x07CC, false, 8, "SD_RS_SF",          "System Diagnostic Response Driver Seat Module",      sd_rs_sf_07cc_signals.data(),          sd_rs_sf_07cc_signals.size()          },
        { 0x01B6, false, 8, "SB_A1",             "Passenger Seat Backrest Status",                     sb_a1_01b6_signals.data(),             sb_a1_01b6_signals.size()             },
        { 0x040D, false, 8, "NM_SB",             "Network Management Passenger Seat Module",           nm_sb_040d_signals.data(),             nm_sb_040d_signals.size()             },
        { 0x04ED, false, 8, "D_RS_SB",           "Diagnostic Response Passenger Seat Module",          d_rs_sb_04ed_signals.data(),           d_rs_sb_04ed_signals.size()           },
        { 0x076D, false, 8, "SG_APPL_SB",        "Application Interface Passenger Seat Module",        sg_appl_sb_076d_signals.data(),        sg_appl_sb_076d_signals.size()        },
        { 0x07CD, false, 8, "SD_RS_SB",          "System Diagnostic Response Passenger Seat Module",   sd_rs_sb_07cd_signals.data(),          sd_rs_sb_07cd_signals.size()          },
        { 0x0094, false, 8, "STH_A1",            "Auxiliary Heater Operation & Status",                sth_a1_0094_signals.data(),            sth_a1_0094_signals.size()            },
        { 0x0419, false, 8, "NM_STH",            "Network Management Auxiliary Heater",                nm_sth_0419_signals.data(),            nm_sth_0419_signals.size()            },
        { 0x04F9, false, 8, "D_RS_STH",          "Diagnostic Response Auxiliary Heater",               d_rs_sth_04f9_signals.data(),          d_rs_sth_04f9_signals.size()          },
        { 0x0779, false, 8, "SG_APPL_STH",       "Application Interface Auxiliary Heater",             sg_appl_sth_0779_signals.data(),       sg_appl_sth_0779_signals.size()       },
        { 0x0030, false, 8, "KLA_A1",            "Climate Control Compressor, Fan & Flaps",            kla_a1_0030_signals.data(),            kla_a1_0030_signals.size()            },
        { 0x0250, false, 8, "KLA_A2",            "Climate Control Window & Roof Requests",             kla_a2_0250_signals.data(),            kla_a2_0250_signals.size()            },
        { 0x00F1, false, 8, "KLA_A3",            "Heating Demand & Temperature",                       kla_a3_00f1_signals.data(),            kla_a3_00f1_signals.size()            },
        { 0x0411, false, 8, "NM_KLA",            "Network Management Climate Control",                 nm_kla_0411_signals.data(),            nm_kla_0411_signals.size()            },
        { 0x04F1, false, 8, "D_RS_KLA",          "Diagnostic Response Climate Control",                d_rs_kla_04f1_signals.data(),          d_rs_kla_04f1_signals.size()          },
        { 0x0771, false, 8, "SG_APPL_KLA",       "Application Interface Climate Control",              sg_appl_kla_0771_signals.data(),       sg_appl_kla_0771_signals.size()       },
        { 0x0015, false, 8, "MSS_A1",            "Special Vehicle Sirens, Lights & Audio",             mss_a1_0015_signals.data(),            mss_a1_0015_signals.size()            },
        { 0x01AE, false, 8, "MSS_A2",            "Roof Sign, Oxygen & Mist System Status",             mss_a2_01ae_signals.data(),            mss_a2_01ae_signals.size()            },
        { 0x01CE, false, 8, "MSS_A3",            "Special Vehicle Destination Coordinates",            mss_a3_01ce_signals.data(),            mss_a3_01ce_signals.size()            },
        { 0x0248, false, 8, "MSS_A4",            "Emergency Window & Sunroof Close",                   mss_a4_0248_signals.data(),            mss_a4_0248_signals.size()            },
        { 0x0046, false, 8, "MSSK_A1",           "Special Vehicle Switch Console 1",                   mssk_a1_0046_signals.data(),           mssk_a1_0046_signals.size()           },
        { 0x0208, false, 8, "MSSK_A2",           "Special Vehicle Passenger Seat Control",             mssk_a2_0208_signals.data(),           mssk_a2_0208_signals.size()           },
        { 0x01D8, false, 8, "TP_MSS_AGW3",       "Transport Protocol MSS to AGW",                      tp_mss_agw3_01d8_signals.data(),       tp_mss_agw3_01d8_signals.size()       },
        { 0x01A6, false, 8, "TP_MSS_KOMBI2",     "Transport Protocol MSS to Instrument Cluster",       tp_mss_kombi2_01a6_signals.data(),     tp_mss_kombi2_01a6_signals.size()     },
        { 0x0406, false, 8, "NM_MSS",            "Network Management Special Vehicle Control",         nm_mss_0406_signals.data(),            nm_mss_0406_signals.size()            },
        { 0x04E6, false, 8, "D_RS_MSS",          "Diagnostic Response Special Vehicle Control",        d_rs_mss_04e6_signals.data(),          d_rs_mss_04e6_signals.size()          },
        { 0x0766, false, 8, "SG_APPL_MSS",       "Application Interface Special Vehicle Control",      sg_appl_mss_0766_signals.data(),       sg_appl_mss_0766_signals.size()       },
        { 0x02B0, false, 8, "PTS_A1",            "Parktronic System State",                            pts_a1_02b0_signals.data(),            pts_a1_02b0_signals.size()            },
        { 0x0413, false, 8, "NM_PTS",            "Network Management Parktronic System",               nm_pts_0413_signals.data(),            nm_pts_0413_signals.size()            },
        { 0x04F3, false, 8, "D_RS_PTS",          "Diagnostic Response Parktronic System",              d_rs_pts_04f3_signals.data(),          d_rs_pts_04f3_signals.size()          },
        { 0x0773, false, 8, "SG_APPL_PTS",       "Application Interface Parktronic System",            sg_appl_pts_0773_signals.data(),       sg_appl_pts_0773_signals.size()       },
        { 0x02FF, false, 8, "TPM_A1",            "Tire Pressures & Warning Status",                    tpm_a1_02ff_signals.data(),            tpm_a1_02ff_signals.size()            },
        { 0x0418, false, 8, "NM_TPM",            "Network Management Tire Pressure Monitor",           nm_tpm_0418_signals.data(),            nm_tpm_0418_signals.size()            },
        { 0x04F8, false, 8, "D_RS_TPM",          "Diagnostic Response Tire Pressure Monitor",          d_rs_tpm_04f8_signals.data(),          d_rs_tpm_04f8_signals.size()          },
        { 0x0778, false, 8, "SG_APPL_TPM",       "Application Interface Tire Pressure Monitor",        sg_appl_tpm_0778_signals.data(),       sg_appl_tpm_0778_signals.size()       },
        { 0x0607, false, 8, "MESS_TPM1",         "Tire Pressure Measurement Data",                     mess_tpm1_0607_signals.data(),         mess_tpm1_0607_signals.size()         },
        { 0x0138, false, 8, "AGW_A3",            "Audio Gateway Keypad & Function Keys",               agw_a3_0138_signals.data(),            agw_a3_0138_signals.size()            },
        { 0x0338, false, 8, "GPS_A1",            "GPS Latitude & Longitude",                           gps_a1_0338_signals.data(),            gps_a1_0338_signals.size()            },
        { 0x0339, false, 8, "GPS_A2",            "GPS Date & UTC Time",                                gps_a2_0339_signals.data(),            gps_a2_0339_signals.size()            },
        { 0x033A, false, 8, "GPS_A3",            "GPS Dynamics, Fix & Dilution of Precision",          gps_a3_033a_signals.data(),            gps_a3_033a_signals.size()            },
        { 0x01A4, false, 8, "TP_AGW_KOMBI1",     "Transport Protocol AGW to Instrument Cluster",       tp_agw_kombi1_01a4_signals.data(),     tp_agw_kombi1_01a4_signals.size()     },
        { 0x0334, false, 8, "TP_AGW_MSS3",       "Transport Protocol AGW to MSS",                      tp_agw_mss3_0334_signals.data(),       tp_agw_mss3_0334_signals.size()       },
        { 0x03E3, false, 8, "TP_AGW_TELEAID6",   "Transport Protocol AGW to TeleAid",                  tp_agw_teleaid6_03e3_signals.data(),   tp_agw_teleaid6_03e3_signals.size()   },
        { 0x0416, false, 8, "NM_AGW",            "Network Management Audio Gateway",                   nm_agw_0416_signals.data(),            nm_agw_0416_signals.size()            },
        { 0x04F6, false, 8, "D_RS_AGW",          "Diagnostic Response Audio Gateway",                  d_rs_agw_04f6_signals.data(),          d_rs_agw_04f6_signals.size()          },
        { 0x0680, false, 8, "D_RS_MOST0",        "Diagnostic Response MOST Gateway 0",                 d_rs_most0_0680_signals.data(),        d_rs_most0_0680_signals.size()        },
        { 0x0681, false, 8, "D_RS_MOST1",        "Diagnostic Response MOST Gateway 1",                 d_rs_most1_0681_signals.data(),        d_rs_most1_0681_signals.size()        },
        { 0x068A, false, 8, "D_RS_MOST10",       "Diagnostic Response MOST Gateway 10",                d_rs_most10_068a_signals.data(),       d_rs_most10_068a_signals.size()       },
        { 0x068B, false, 8, "D_RS_MOST11",       "Diagnostic Response MOST Gateway 11",                d_rs_most11_068b_signals.data(),       d_rs_most11_068b_signals.size()       },
        { 0x068C, false, 8, "D_RS_MOST12",       "Diagnostic Response MOST Gateway 12",                d_rs_most12_068c_signals.data(),       d_rs_most12_068c_signals.size()       },
        { 0x068D, false, 8, "D_RS_MOST13",       "Diagnostic Response MOST Gateway 13",                d_rs_most13_068d_signals.data(),       d_rs_most13_068d_signals.size()       },
        { 0x068E, false, 8, "D_RS_MOST14",       "Diagnostic Response MOST Gateway 14",                d_rs_most14_068e_signals.data(),       d_rs_most14_068e_signals.size()       },
        { 0x068F, false, 8, "D_RS_MOST15",       "Diagnostic Response MOST Gateway 15",                d_rs_most15_068f_signals.data(),       d_rs_most15_068f_signals.size()       },
        { 0x0682, false, 8, "D_RS_MOST2",        "Diagnostic Response MOST Gateway 2",                 d_rs_most2_0682_signals.data(),        d_rs_most2_0682_signals.size()        },
        { 0x0683, false, 8, "D_RS_MOST3",        "Diagnostic Response MOST Gateway 3",                 d_rs_most3_0683_signals.data(),        d_rs_most3_0683_signals.size()        },
        { 0x0684, false, 8, "D_RS_MOST4",        "Diagnostic Response MOST Gateway 4",                 d_rs_most4_0684_signals.data(),        d_rs_most4_0684_signals.size()        },
        { 0x0685, false, 8, "D_RS_MOST5",        "Diagnostic Response MOST Gateway 5",                 d_rs_most5_0685_signals.data(),        d_rs_most5_0685_signals.size()        },
        { 0x0686, false, 8, "D_RS_MOST6",        "Diagnostic Response MOST Gateway 6",                 d_rs_most6_0686_signals.data(),        d_rs_most6_0686_signals.size()        },
        { 0x0687, false, 8, "D_RS_MOST7",        "Diagnostic Response MOST Gateway 7",                 d_rs_most7_0687_signals.data(),        d_rs_most7_0687_signals.size()        },
        { 0x0688, false, 8, "D_RS_MOST8",        "Diagnostic Response MOST Gateway 8",                 d_rs_most8_0688_signals.data(),        d_rs_most8_0688_signals.size()        },
        { 0x0689, false, 8, "D_RS_MOST9",        "Diagnostic Response MOST Gateway 9",                 d_rs_most9_0689_signals.data(),        d_rs_most9_0689_signals.size()        },
        { 0x058A, false, 8, "D_RS_NAVI",         "Diagnostic Response Navigation System",              d_rs_navi_058a_signals.data(),         d_rs_navi_058a_signals.size()         },
        { 0x0776, false, 8, "SG_APPL_AGW",       "Application Interface Audio Gateway",                sg_appl_agw_0776_signals.data(),       sg_appl_agw_0776_signals.size()       },
        { 0x07D6, false, 8, "SD_RS_AGW",         "System Diagnostic Response Audio Gateway",           sd_rs_agw_07d6_signals.data(),         sd_rs_agw_07d6_signals.size()         },
        { 0x043B, false, 8, "NM_ICANI",          "Network Management CAN Interface",                   nm_icani_043b_signals.data(),          nm_icani_043b_signals.size()          },
        { 0x04BB, false, 8, "D_RS_ICANI",        "Diagnostic Response CAN Interface",                  d_rs_icani_04bb_signals.data(),        d_rs_icani_04bb_signals.size()        },
        { 0x05BB, false, 8, "SD_RS_ICANI",       "System Diagnostic Response CAN Interface",           sd_rs_icani_05bb_signals.data(),       sd_rs_icani_05bb_signals.size()       },
        { 0x01B8, false, 8, "EZS_A3",            "Odometer Value Legacy",                              ezs_a3_01b8_signals.data(),            ezs_a3_01b8_signals.size()            },
        { 0x04FE, false, 8, "D_RS_FDSVL",        "Diagnostic Response Dynamic Seat Front Left",        d_rs_fdsvl_04fe_signals.data(),        d_rs_fdsvl_04fe_signals.size()        },
        { 0x04FF, false, 8, "D_RS_FDSVR",        "Diagnostic Response Dynamic Seat Front Right",       d_rs_fdsvr_04ff_signals.data(),        d_rs_fdsvr_04ff_signals.size()        },
        { 0x04EF, false, 8, "D_RS_LRK",          "Diagnostic Response Heated Steering Wheel",          d_rs_lrk_04ef_signals.data(),          d_rs_lrk_04ef_signals.size()          },
        { 0x04EE, false, 8, "D_RS_PFDS",         "Diagnostic Response Pneumatic Dynamic Seat",         d_rs_pfds_04ee_signals.data(),         d_rs_pfds_04ee_signals.size()         },
        { 0x04FB, false, 8, "D_RS_SHZ",          "Diagnostic Response Seat Heating",                   d_rs_shz_04fb_signals.data(),          d_rs_shz_04fb_signals.size()          },
        { 0x04FA, false, 8, "D_RS_TLM",          "Diagnostic Response Telematics Control Unit",        d_rs_tlm_04fa_signals.data(),          d_rs_tlm_04fa_signals.size()          },
        { 0x072F, false, 8, "APPL_SG_AAG",       "Application Interface to Trailer Control",           appl_sg_aag_072f_signals.data(),       appl_sg_aag_072f_signals.size()       },
        { 0x05C9, false, 8, "APPL_SG_AGW",       "Application Interface to Audio Gateway",             appl_sg_agw_05c9_signals.data(),       appl_sg_agw_05c9_signals.size()       },
        { 0x06A3, false, 8, "APPL_SG_ARMADA",    "Application Interface to Restraint System",          appl_sg_armada_06a3_signals.data(),    appl_sg_armada_06a3_signals.size()    },
        { 0x0678, false, 8, "APPL_SG_DBE",       "Application Interface to Overhead Control Panel",    appl_sg_dbe_0678_signals.data(),       appl_sg_dbe_0678_signals.size()       },
        { 0x04DF, false, 8, "APPL_SG_EZS",       "Application Interface to Ignition Switch",           appl_sg_ezs_04df_signals.data(),       appl_sg_ezs_04df_signals.size()       },
        { 0x0568, false, 8, "APPL_SG_HFS",       "Application Interface to Trunk Remote Control",      appl_sg_hfs_0568_signals.data(),       appl_sg_hfs_0568_signals.size()       },
        { 0x078E, false, 8, "APPL_SG_KLA",       "Application Interface to Climate Control",           appl_sg_kla_078e_signals.data(),       appl_sg_kla_078e_signals.size()       },
        { 0x05AB, false, 8, "APPL_SG_KOMBI",     "Application Interface to Instrument Cluster",        appl_sg_kombi_05ab_signals.data(),     appl_sg_kombi_05ab_signals.size()     },
        { 0x06C1, false, 8, "APPL_SG_MRM",       "Application Interface to Steering Column Module",    appl_sg_mrm_06c1_signals.data(),       appl_sg_mrm_06c1_signals.size()       },
        { 0x0720, false, 8, "APPL_SG_MSS",       "Application Interface to Special Vehicle Control",   appl_sg_mss_0720_signals.data(),       appl_sg_mss_0720_signals.size()       },
        { 0x06BA, false, 8, "APPL_SG_OBF",       "Application Interface to Upper Control Panel",       appl_sg_obf_06ba_signals.data(),       appl_sg_obf_06ba_signals.size()       },
        { 0x072C, false, 8, "APPL_SG_PTS",       "Application Interface to Parktronic",                appl_sg_pts_072c_signals.data(),       appl_sg_pts_072c_signals.size()       },
        { 0x057C, false, 8, "APPL_SG_SAM_H",     "Application Interface to Rear SAM",                  appl_sg_sam_h_057c_signals.data(),     appl_sg_sam_h_057c_signals.size()     },
        { 0x067D, false, 8, "APPL_SG_SAM_V",     "Application Interface to Front SAM",                 appl_sg_sam_v_067d_signals.data(),     appl_sg_sam_v_067d_signals.size()     },
        { 0x06B2, false, 8, "APPL_SG_SB",        "Application Interface to Passenger Seat Module",     appl_sg_sb_06b2_signals.data(),        appl_sg_sb_06b2_signals.size()        },
        { 0x06B3, false, 8, "APPL_SG_SF",        "Application Interface to Driver Seat Module",        appl_sg_sf_06b3_signals.data(),        appl_sg_sf_06b3_signals.size()        },
        { 0x073F, false, 8, "APPL_SG_STH",       "Application Interface to Auxiliary Heater",          appl_sg_sth_073f_signals.data(),       appl_sg_sth_073f_signals.size()       },
        { 0x0756, false, 8, "APPL_SG_THL",       "Application Interface to Rear Left Door Module",     appl_sg_thl_0756_signals.data(),       appl_sg_thl_0756_signals.size()       },
        { 0x0754, false, 8, "APPL_SG_THR",       "Application Interface to Rear Right Door Module",    appl_sg_thr_0754_signals.data(),       appl_sg_thr_0754_signals.size()       },
        { 0x06A7, false, 8, "APPL_SG_TPM",       "Application Interface to Tire Pressure Monitor",     appl_sg_tpm_06a7_signals.data(),       appl_sg_tpm_06a7_signals.size()       },
        { 0x06D7, false, 8, "APPL_SG_TVL",       "Application Interface to Driver Door Module",        appl_sg_tvl_06d7_signals.data(),       appl_sg_tvl_06d7_signals.size()       },
        { 0x06C0, false, 8, "APPL_SG_TVR",       "Application Interface to Passenger Door Module",     appl_sg_tvr_06c0_signals.data(),       appl_sg_tvr_06c0_signals.size()       },
        { 0x0722, false, 8, "APPL_SG_UBF",       "Application Interface to Lower Control Panel",       appl_sg_ubf_0722_signals.data(),       appl_sg_ubf_0722_signals.size()       },
        { 0x06B7, false, 8, "APPL_SG_WSS",       "Application Interface to Weight Sensing System",     appl_sg_wss_06b7_signals.data(),       appl_sg_wss_06b7_signals.size()       },
        { 0x04E0, false, 8, "D_RQ_EZS",          "Diagnostic Request Ignition Switch",                 d_rq_ezs_04e0_signals.data(),          d_rq_ezs_04e0_signals.size()          },
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
