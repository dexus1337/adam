#include "data/profiles/mercedes/w209/w209-can-c.hpp"

#include <array>

/**
 * @file    w209-can-c.cpp
 * @author  dexus1337
 * @brief   Implements the complete Mercedes-Benz W209 Drivetrain CAN-C profile definitions.
 * @version 1.0
 * @date    24.08.2026
 */

namespace adam::modules::can::profiles::mercedes::w209::can_c
{
    using namespace adam::string_hashed_ct_literals;

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: UNKNOWN (ID: 0x0000) - Brake Status, ESP & Front Wheel Speeds (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto unknown_0000_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "BRE_KL",      "Brake defective warning lamp (EBV_KL at 463/461/NCV2)",   0,   1 },
        {   1, "BAS_KL",      "Brake Assist (BAS) defective warning lamp",               1,   1 },
        {   2, "ESP_INFO_BL", "ESP info lamp flashing light",                            2,   1 },
        {   3, "ESP_INFO_DL", "ESP info lamp steady light",                              3,   1 },
        {   4, "ESP_KL",      "ESP defective warning lamp",                              4,   1 },
        {   5, "ABS_KL",      "ABS defective warning lamp",                              5,   1 },
        {   6, "UNKNOWN_1",   "Unknown signal",                                          6,   2 },
        {   7, "BLS_UNT",     "Brake light suppression (EBV_KL at 163/T0/T1N)",          8,   1 },
        {   8, "BLS_PA",      "Brake light switch parity (even parity)",                 9,   1 },
        {   9, "BZ200h",      "Message counter",                                        10,   4 },
        {  10, "BLS",         "Brake light switch",                                     14,   2 },
        {  11, "DRTGVL",      "Direction of rotation front left wheel",                 16,   2 },
        {  12, "DVL",         "Front left wheel speed (1/min)",                         18,  14 },
        {  13, "DRTGVR",      "Direction of rotation front right wheel",                32,   2 },
        {  14, "DVR",         "Front right wheel speed (1/min)",                        34,  14 },
        {  15, "DRTGTM",      "Direction of rotation left wheel for cruise control",    48,   2 },
        {  16, "TM_DL",       "Wheel speed left for cruise control (1/min)",            50,  14 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_208h (ID: 0x0208) - ESP Brake Intervention & Rear Wheel Speeds (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_208h_0208_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AKT_R_ESP",         "ESP / ART request: \"Active downshift\"",                                       0,   1 },
        {   1, "MINMAX_ART",        "Target gear request from ART",                                                  1,   1 },
        {   2, "GMAX_ESP",          "Target gear, upper limit",                                                      2,   3 },
        {   3, "GMIN_ESP",          "Target gear, lower limit",                                                      5,   3 },
        {   4, "DDYN_UNT",          "Suppression of dynamic full-load downshift",                                    8,   1 },
        {   5, "SZS",               "System state",                                                                  9,   2 },
        {   6, "TM_AUS",            "Cruise control operation off",                                                 11,   1 },
        {   7, "SLV_ESP",           "Shift line offset ESP",                                                        12,   4 },
        {   8, "BRE_AKT_ESP",       "ESP brake intervention active",                                                16,   1 },
        {   9, "ANFN",              "ESP request: Engage \"N\"",                                                    17,   2 },
        {  10, "BRE_AKT_ART",       "Distronic (ART) brake intervention active",                                    19,   1 },
        {  11, "MBRE_ESP",          "Set brake torque (BR240 factor 1.8 larger) (Nm)",                              20,  12 },
        {  12, "DRTGHR",            "Direction of rotation rear right wheel",                                       32,   2 },
        {  13, "DHR",               "Rear right wheel speed (1/min)",                                               34,  14 },
        {  14, "DRTGHL",            "Direction of rotation rear left wheel",                                        48,   2 },
        {  15, "DHL",               "Rear left wheel speed (1/min)",                                                50,  14 },
        {  16, "SZS_NEU",           "System state (new)",                                                            9,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_270h (ID: 0x0270) - Rear Wheel Pulse Rings & Flat Tyre Warning (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_270h_0270_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "RIZ_HL",    "Pulse ring counter rear left wheel (48/rev) (pulses)",    0,   8 },
        {   1, "RIZ_HR",    "Pulse ring counter rear right wheel (48/rev) (pulses)",   8,   8 },
        {   2, "PRW_WARN",  "Flat tyre warning messages",                             16,   4 },
        {   3, "UNKNOWN_1", "Unknown signal",                                         20,   1 },
        {   4, "PRW_ST",    "Flat tyre warning status",                               21,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_300h (ID: 0x0300) - ESP & ART Torque Request & Yaw Rate (23 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_300h_0300_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DMPAR_ART",         "Engine torque request parity (even parity)",                                    0,   1 },
        {   1, "DMDYN_ART",         "Engine torque request dynamic",                                                 1,   1 },
        {   2, "BAS_AKT",           "BAS control active",                                                            2,   1 },
        {   3, "VOLLBRE",           "Emergency braking (ABS controls all 4 wheels)",                                 3,   1 },
        {   4, "ART_E",             "Enable Distronic (ART)",                                                        4,   1 },
        {   5, "ESP_GIER_AKT",      "ESP yaw moment control active",                                                 5,   1 },
        {   6, "LWS_INI_OK",        "Steering angle sensor initialization OK",                                       6,   1 },
        {   7, "LWS_INI_EIN",       "Steering angle sensor initialization possible",                                 7,   1 },
        {   8, "MPAR_ESP",          "Engine torque request parity (even parity)",                                    8,   1 },
        {   9, "MDYN_ESP",          "Engine torque request dynamic",                                                 9,   1 },
        {  10, "AMR_AKT_ESP",       "Drive torque control active",                                                  10,   1 },
        {  11, "T_Z",               "Transmission cycle time (ms)",                                                 11,   2 },
        {  12, "SFB_PA",            "Driver braking parity (even parity)",                                          13,   1 },
        {  13, "SFB",               "Driver braking",                                                               14,   2 },
        {  14, "DMTGL_ART",         "Engine torque request toggle 40ms +-10",                                       16,   1 },
        {  15, "DMMIN_ART",         "Engine torque request minimum",                                                17,   1 },
        {  16, "DMMAX_ART",         "Engine torque request maximum",                                                18,   1 },
        {  17, "DM_ART",            "Requested engine torque (Nm)",                                                 19,  13 },
        {  18, "MTGL_ESP",          "Engine torque request toggle 40ms +-10",                                       32,   1 },
        {  19, "MMIN_ESP",          "Engine torque request minimum",                                                33,   1 },
        {  20, "MMAX_ESP",          "Engine torque request maximum",                                                34,   1 },
        {  21, "M_ESP",             "Requested engine torque (Nm)",                                                 35,  13 },
        {  22, "GIER_ROH",          "Raw yaw rate signal without calibration/filtering (+ = left) (°/s)",           48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_328h (ID: 0x0328) - Roll Moment, Lateral Acceleration & Front Pulse Rings (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_328h_0328_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "WMS_PA",    "Target roll moment parity (even parity)",                               0,   1 },
        {   1, "WMS",       "Target roll moment change (Nm)",                                        1,  15 },
        {   2, "AY_S",      "Vehicle lateral acceleration at center of gravity (+ = left) (m/s²)",  16,   8 },
        {   3, "UNKNOWN_1", "Unknown signal",                                                       24,  11 },
        {   4, "ESP_DSPL",  "ESP display messages",                                                 35,   5 },
        {   5, "UNKNOWN_2", "Unknown signal",                                                       40,   1 },
        {   6, "NOTBRE",    "Emergency braking (flashing brake lights)",                            41,   1 },
        {   7, "UNKNOWN_3", "Unknown signal",                                                       42,   2 },
        {   8, "KPL_OEF",   "Open clutch",                                                          44,   1 },
        {   9, "BZ328h",    "Message counter",                                                      45,   3 },
        {  10, "RIZ_VL",    "Pulse ring counter front left wheel (48/rev) (pulses)",                48,   8 },
        {  11, "RIZ_VR",    "Pulse ring counter front right wheel (48/rev) (pulses)",               56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_BS (ID: 0x0785) - Diagnostic Response Brake System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_bs_0785_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_BS (ID: 0x0722) - System Diagnostic Response Brake System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_bs_0722_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_APPL2 (ID: 0x0635) - Application Interface Brake System 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_appl2_0635_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL2",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_100h (ID: 0x0100) - Drive Authorization FBS Message to EZS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_100h_0100_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS",               "FBS message to EZS (8 bytes)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_101h (ID: 0x0101) - Drive Authorization FBS Message to EZS (Alternate) (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_101h_0101_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MS_FBS",            "FBS message to EZS (8 bytes)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_210h (ID: 0x0210) - Engine Status, Accelerator Pedal & Cruise Control (36 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_210h_0210_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KOMP_NOTAUS", "A/C compressor emergency shutoff",                             0,   1 },
        {   1, "SLV_MS",      "Shift line offset MS",                                         1,   4 },
        {   2, "UNKNOWN_1",   "Unknown signal",                                               5,   1 },
        {   3, "KRIECH_AUS",  "Disable manual transmission creep",                            6,   1 },
        {   4, "ANF1",        "MS request: \"Drive away in 1st gear\"",                       7,   1 },
        {   5, "AKT_R_MS",    "MS request: \"Active downshift\"",                             8,   1 },
        {   6, "ZH_AUS_MS",   "Auxiliary heater switch off",                                  9,   1 },
        {   7, "GMAX_MS",     "Target gear, upper limit",                                    10,   3 },
        {   8, "GMIN_MS",     "Target gear, lower limit",                                    13,   3 },
        {   9, "PW",          "Accelerator pedal position (%)",                              16,   8 },
        {  10, "V_DSPL_NEU",  "Retrigger minimum display time",                              24,   1 },
        {  11, "LL_STBL",     "Idle speed is stable",                                        25,   1 },
        {  12, "VGL_ST",      "Preheating status",                                           26,   1 },
        {  13, "MSS_DEF",     "Engine Start/Stop system defective",                          27,   1 },
        {  14, "MSS_KL",      "Engine Start/Stop indicator lamp",                            28,   1 },
        {  15, "MSS_AKT",     "Engine Start/Stop system active",                             29,   1 },
        {  16, "KOMP_BAUS",   "A/C compressor off: Acceleration",                            30,   1 },
        {  17, "CRASH_MS",    "Crash signal from engine management",                         31,   1 },
        {  18, "PWG_ERR",     "Pedal value sensor fault",                                    32,   1 },
        {  19, "LL",          "Idle position",                                               33,   1 },
        {  20, "KUEB_S_A",    "Request torque converter lockup clutch \"slip\"",             34,   1 },
        {  21, "TM_REG",      "Cruise control controlling",                                  35,   1 },
        {  22, "V_MAX_EIN",   "Speed limiter switched on",                                   36,   1 },
        {  23, "KD_MS",       "Kickdown",                                                    37,   1 },
        {  24, "NOTL",        "Limp-home mode",                                              38,   1 },
        {  25, "V_MAX_SUM",   "Warning buzzer on",                                           39,   1 },
        {  26, "FBS_SE",      "Drive authorization: Start Error",                            40,   1 },
        {  27, "V_DSPL_PGB",  "Display \"Winter tyre limit reached\"",                       41,   1 },
        {  28, "TM_EIN",      "Cruise control switched on",                                  42,   1 },
        {  29, "V_MAX_REG",   "Speed limiter controlling",                                   43,   1 },
        {  30, "V_DSPL_LIM",  "Display \"Limit ?\" on screen",                               44,   1 },
        {  31, "V_DSPL_ERR",  "Display \"Error\" on screen",                                 45,   1 },
        {  32, "V_DSPL_BL",   "Display flashing",                                            46,   1 },
        {  33, "V_DSPL_EIN",  "Speed limiter / cruise control display on",                   47,   1 },
        {  34, "FMMOTMAX",    "Factor for derating max torque at reduced ambient pressure",  48,   8 },
        {  35, "V_MAX_TM",    "Set maximum or cruise control speed (km/h)",                  56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_212h (ID: 0x0212) - Engine Speed & Torque Requests (13 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_212h_0212_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NMOTS",       "Engine idle target speed (1/min)",           0,  16 },
        {   1, "UNKNOWN_1",   "Unknown signal",                            16,   1 },
        {   2, "TM_MS",       "Standard cruise control is variant-coded",  17,   1 },
        {   3, "M_ART_E",     "Enable torque request Distronic (ART)",     18,   1 },
        {   4, "M_FV",        "Driver demand torque (Nm)",                 19,  13 },
        {   5, "UNKNOWN_2",   "Unknown signal",                            32,   1 },
        {   6, "SME_E",       "Enable fast torque adjustment",             33,   1 },
        {   7, "M_ESP_E",     "Enable torque request ESP",                 34,   1 },
        {   8, "M_FEV",       "Driver substitute demand torque (Nm)",      35,  13 },
        {   9, "CALID_CVN_E", "Enable CALID/CVN transmission",             48,   1 },
        {  10, "M_EGS_Q",     "Acknowledge torque request EGS",            49,   1 },
        {  11, "M_EGS_E",     "Enable torque request EGS",                 50,   1 },
        {  12, "M_ESPV",      "Default target torque ESP (Nm)",            51,  13 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_268h (ID: 0x0268) - Gear Ratio Limits, Alternator Load & A/C Torque (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_268h_0268_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "IMIN_MS",      "Target gear ratio, lower limit (FCVT)",           0,   8 },
        {   1, "IMAX_MS",      "Target gear ratio, upper limit (FCVT)",           8,   8 },
        {   2, "KL_61_EIN",    "Terminal 61",                                    16,   1 },
        {   3, "OEL_INFO_169", "Oil info, reserved M266",                        17,   1 },
        {   4, "ASV_KKL_169",  "Cooling circuit shutoff valve M266 turbo",       18,   1 },
        {   5, "UNKNOWN_1",    "Unknown signal",                                 19,   3 },
        {   6, "HZL_ST",       "Heating output status",                          22,   2 },
        {   7, "KID_MS",       "Request power-free in \"D\" (FCVT)",             24,   1 },
        {   8, "LRS_MODE",     "Air control system mode",                        25,   1 },
        {   9, "LAST_GEN",     "Alternator load (LIN alternators only) (%)",     26,   6 },
        {  10, "M_KOMP_MAX",   "Max A/C compressor torque (Nm)",                 32,   8 },
        {  11, "PW_F",         "Driver accelerator pedal value (169 only) (%)",  40,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_2F3h (ID: 0x02F3) - Shift Recommendation (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_2f3h_02f3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FSC_IST",   "Shift recommendation current gear",   0,   8 },
        {   1, "UNKNOWN_1", "Unknown signal",                      8,  32 },
        {   2, "FSC_SOLL",  "Shift recommendation target gear",   40,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_308h (ID: 0x0308) - Engine RPM, Temperatures & Warning Lamps (28 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_308h_0308_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KPL",          "Clutch depressed",                                                 0,   1 },
        {   1, "KUEB_O_A",     "Request torque converter lockup clutch \"open\"",                  1,   1 },
        {   2, "N_MAX_BG",     "Engine speed limitation function active",                          2,   1 },
        {   3, "SAST",         "Partial overrun fuel cutoff",                                      3,   1 },
        {   4, "SASV",         "Full overrun fuel cutoff",                                         4,   1 },
        {   5, "KSF_KL",       "Fuel filter clogged warning lamp (CR2-USA only)",                  5,   1 },
        {   6, "WKS_KL",       "Water in fuel warning lamp (CR2-USA only)",                        6,   1 },
        {   7, "ZASBED",       "Cylinder shutoff conditions met",                                  7,   1 },
        {   8, "NMOT",         "Engine speed (1/min)",                                             8,  16 },
        {   9, "UNKNOWN_1",    "Unknown signal",                                                  24,   1 },
        {  10, "ELHP_WARN",    "Eco power steering pump warning",                                 25,   1 },
        {  11, "EOH",          "Ethanol operation detected",                                      26,   1 },
        {  12, "LUFI_KL",      "Air filter soiled warning lamp (Diesel only)",                    27,   1 },
        {  13, "VGL_KL",       "Preheating indicator lamp",                                       28,   1 },
        {  14, "OEL_KL",       "Engine oil level / oil pressure warning lamp",                    29,   1 },
        {  15, "DIAG_KL",      "Diagnostic check engine lamp (OBD II)",                           30,   1 },
        {  16, "TANK_KL",      "Fuel filler cap open warning lamp",                               31,   1 },
        {  17, "UEHITZ",       "Engine oil temperature too high (overheating)",                   32,   1 },
        {  18, "ZAS",          "Cylinder shutoff active",                                         33,   1 },
        {  19, "ADR_KL",       "Working speed control indicator lamp (commercial only)",          34,   1 },
        {  20, "ADR_DEF_KL",   "Working speed control defective warning lamp (commercial only)",  35,   1 },
        {  21, "ANL_LFT",      "Starter running",                                                 36,   1 },
        {  22, "LUEFT_MOT_KL", "Engine cooling fan defective indicator lamp",                     37,   1 },
        {  23, "DBAA",         "Speed limitation for display active (0 on CR)",                   38,   1 },
        {  24, "TEMP_KL",      "Coolant temperature too high",                                    39,   1 },
        {  25, "T_OEL",        "Engine oil temperature (°C)",                                     40,   8 },
        {  26, "OEL_FS",       "Engine oil level (mm)",                                           48,   8 },
        {  27, "OEL_QUAL",     "Engine oil quality index",                                        56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_312h (ID: 0x0312) - Engine Static, Max & Min Torque Limits (8 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_312h_0312_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1", "Unknown signal",                                           0,   3 },
        {   1, "M_STA",     "Static engine torque (Nm)",                                3,  13 },
        {   2, "UNKNOWN_2", "Unknown signal",                                          16,   3 },
        {   3, "M_MAX_ATL", "Max engine torque incl dynamic turbocharger boost (Nm)",  19,  13 },
        {   4, "UNKNOWN_3", "Unknown signal",                                          32,   3 },
        {   5, "M_MAX",     "Maximum engine torque (Nm)",                              35,  13 },
        {   6, "UNKNOWN_4", "Unknown signal",                                          48,   3 },
        {   7, "M_MIN",     "Minimum engine torque (Nm)",                              51,  13 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: AAD_580h (ID: 0x0580) - Adaptive Accelerator Pedal & Driver Behavior (9 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto aad_580h_0580_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FTK_BMI",   "Acceleration style index (>100: dynamic)",               0,   8 },
        {   1, "FTK_LMI",   "Lateral acceleration style index (>100: dynamic)",       8,   8 },
        {   2, "FTK_VMI",   "Braking style index (>100: dynamic)",                   16,   8 },
        {   3, "UNKNOWN_1", "Unknown signal",                                        24,   8 },
        {   4, "FTK_DPW",   "Max differential pedal angle rate per maneuver (%/s)",  32,   8 },
        {   5, "AADKB",     "Continuous driver observation index",                   40,   8 },
        {   6, "AADKBDYN",  "Spontaneous dynamic demand",                            48,   1 },
        {   7, "UNKNOWN_2", "Unknown signal",                                        49,   7 },
        {   8, "AADNT",     "Driver nervousness index",                              56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_608h (ID: 0x0608) - Coolant & Intake Air Temp, Consumption & Particulate Filter (13 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_608h_0608_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "T_MOT",             "Engine coolant temperature (°C)",                                               0,   8 },
        {   1, "T_LUFT",            "Intake air temperature (°C)",                                                   8,   8 },
        {   2, "FCOD_KAR",          "Vehicle body code",                                                            16,   3 },
        {   3, "FCOD_BR",           "Vehicle model series code",                                                    19,   5 },
        {   4, "FCOD_MOT6",         "Vehicle engine code (7-bit, bit 6)",                                           24,   1 },
        {   5, "GS_NVH",            "Transmission control module not present",                                      25,   1 },
        {   6, "FCOD_MOT",          "Vehicle engine code (7-bit, bits 0-5)",                                        26,   6 },
        {   7, "V_MAX_FIX",         "Fixed top speed limit (km/h)",                                                 32,   8 },
        {   8, "VB",                "Fuel consumption (l/h)",                                                       40,  16 },
        {   9, "ZWP_EIN_MS",        "Auxiliary water pump switch on",                                               56,   1 },
        {  10, "PFW",               "Diesel particulate filter warning",                                            57,   2 },
        {  11, "ZVB_EIN_MS",        "Auxiliary consumer switch on",                                                 59,   1 },
        {  12, "PFKO",              "Particulate filter correction offset FMMOTMAX",                                60,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FSCM (ID: 0x0779) - Diagnostic Response Fuel System Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fscm_0779_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_FSCM",         "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MS (ID: 0x07E8) - Diagnostic Response Engine Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ms_07e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_MS (ID: 0x0720) - System Diagnostic Response Engine Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_ms_0720_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_MS (ID: 0x0529) - Application Interface Engine Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_ms_0529_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL_MS",        "Control unit to external application",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL2 (ID: 0x04A9) - Application Interface Engine Control 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl2_04a9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL2",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL4 (ID: 0x0633) - Application Interface Engine Control 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl4_0633_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL4",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL5 (ID: 0x06A8) - Application Interface Engine Control 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl5_06a8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL5",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL6 (ID: 0x0610) - Application Interface Engine Control 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl6_0610_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL6",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL7 (ID: 0x0618) - Application Interface Engine Control 7 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl7_0618_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL7",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EDC_MESS1 (ID: 0x0670) - Electronic Diesel Control Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto edc_mess1_0670_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EDC_MESS2 (ID: 0x0671) - Electronic Diesel Control Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto edc_mess2_0671_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_218h (ID: 0x0218) - Transmission Shift Status, Current Gear & Torque Intervention (33 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_218h_0218_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MTGL_EGS",      "Engine torque request toggle 40ms +-10",                           0,   1 },
        {   1, "MMIN_EGS",      "Engine torque request minimum",                                    1,   1 },
        {   2, "MMAX_EGS",      "Engine torque request maximum",                                    2,   1 },
        {   3, "M_EGS",         "Requested engine torque (Nm)",                                     3,  13 },
        {   4, "GZC",           "Target gear",                                                     16,   4 },
        {   5, "I_IST_GET",     "Actual transmission ratio (FCVT only, else actual/target gear)",  16,   8 },
        {   6, "GIC",           "Actual gear",                                                     20,   4 },
        {   7, "K_S_B",         "Command torque converter lockup clutch \"slip\"",                 24,   1 },
        {   8, "K_O_B",         "Command torque converter lockup clutch \"open\"",                 25,   1 },
        {   9, "K_G_B",         "Command torque converter lockup clutch \"closed\"",               26,   1 },
        {  10, "G_G",           "Off-road low range gear engaged",                                 27,   1 },
        {  11, "GSP_OK",        "Base shift program OK",                                           28,   1 },
        {  12, "FW_HOCH",       "Driving resistance high",                                         29,   1 },
        {  13, "SCHALT",        "Shift in progress",                                               30,   1 },
        {  14, "HSM",           "Manual shift mode active",                                        31,   1 },
        {  15, "GET_OK",        "Transmission OK",                                                 32,   1 },
        {  16, "KS",            "Snap start / race launch",                                        33,   1 },
        {  17, "ALF",           "Starter release enable",                                          34,   1 },
        {  18, "GS_NOTL",       "Transmission in limp-home mode",                                  35,   1 },
        {  19, "UEHITZ_GET",    "Transmission overtemperature",                                    36,   1 },
        {  20, "KD",            "Kickdown active",                                                 37,   1 },
        {  21, "FPC_AAD",       "Driving program for AAD",                                         38,   2 },
        {  22, "MPAR_EGS",      "Engine torque request parity (even parity)",                      40,   1 },
        {  23, "DYN1_EGS",      "Intervention mode / drive torque control",                        41,   1 },
        {  24, "DYN0_AMR_EGS",  "Intervention mode / drive torque control",                        42,   1 },
        {  25, "UNKNOWN_1",     "Unknown signal",                                                  43,   2 },
        {  26, "K_LSTFR",       "Torque converter lockup clutch load-free",                        45,   1 },
        {  27, "MOT_NAUS_CNF",  "Engine emergency shutoff confirmation bit",                       46,   1 },
        {  28, "MOT_NAUS",      "Engine emergency shutoff",                                        47,   1 },
        {  29, "MKRIECH",       "Creep torque (FFh on EGS/CVT) or CALID/CVN (Nm)",                 48,   8 },
        {  30, "FEHLPRF_ST",    "Error check status",                                              56,   2 },
        {  31, "CALID_CVN_AKT", "CALID/CVN transmission active",                                   58,   1 },
        {  32, "FEHLER",        "Error number / counter for CALID/CVN transmission",               59,   5 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_338h (ID: 0x0338) - Transmission Output & Turbine RPM (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_338h_0338_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NAB",       "Transmission output shaft speed (463/461 only, else FFFFh) (1/min)",   0,  16 },
        {   1, "UNKNOWN_1", "Unknown signal",                                                      16,  32 },
        {   2, "NTURBINE",  "Turbine speed (EGS52-NAG, VGS-NAG2) (1/min)",                         48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_418h (ID: 0x0418) - Drive Program, Transmission Temp & Wheel Torque Factor (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_418h_0418_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FSC",               "Driving gear / shift position",                                                 0,   8 },
        {   1, "FPC",               "Driving program mode",                                                          8,   8 },
        {   2, "T_GET",             "Transmission oil temperature (°C)",                                            16,   8 },
        {   3, "ALLRAD",            "All-wheel drive active",                                                       24,   1 },
        {   4, "FRONT",             "Front-wheel drive [1], Rear-wheel drive [0]",                                  25,   1 },
        {   5, "SCHALT",            "Shift in progress",                                                            26,   1 },
        {   6, "CVT",               "Continuously variable transmission [1], Step gear [0]",                        27,   1 },
        {   7, "MECH",              "Transmission mechanical variant",                                              28,   2 },
        {   8, "ESV_BRE",           "Apply brakes during power-on shift process",                                   30,   1 },
        {   9, "KD",                "Kickdown active",                                                              31,   1 },
        {  10, "GZC",               "Target gear",                                                                  32,   4 },
        {  11, "GIC",               "Actual gear",                                                                  36,   4 },
        {  12, "M_VERL",            "Transmission loss torque (FFh on KSG) (Nm)",                                   40,   8 },
        {  13, "FMRADPAR",          "Wheel torque factor parity (even parity)",                                     48,   1 },
        {  14, "FMRADTGL",          "Wheel torque factor toggle 40ms +-10",                                         49,   1 },
        {  15, "WHST",              "Transmission selector lever position (NAG, KSG, CVT)",                         50,   3 },
        {  16, "FMRAD",             "Wheel torque factor (7FFh on KSG)",                                            53,  11 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_GS (ID: 0x07E9) - Diagnostic Response Transmission Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_gs_07e9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_GS (ID: 0x0723) - System Diagnostic Response Transmission Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_gs_0723_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS_GS",          "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_APPL1 (ID: 0x051C) - Application Interface Transmission Control 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_appl1_051c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSA (ID: 0x050A) - Test Bench Manual Control A (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsa_050a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSA",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSB (ID: 0x050B) - Test Bench Manual Control B (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsb_050b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSB",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSC (ID: 0x050C) - Test Bench Manual Control C (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsc_050c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSC",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSD (ID: 0x050D) - Test Bench Manual Control D (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsd_050d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSD",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSE (ID: 0x050E) - Test Bench Manual Control E (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hse_050e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSE",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_230h (ID: 0x0230) - Electronic Selector Lever Module Status (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_230h_0230_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "W_S",               "Driving program Winter / Standard",                                             0,   1 },
        {   1, "FPT",               "Driving program button pressed",                                                1,   1 },
        {   2, "KD",                "Kickdown switch active",                                                        2,   1 },
        {   3, "SPERR",             "Shift-lock solenoid energized",                                                 3,   1 },
        {   4, "WHC",               "Transmission selector lever position (NAG only)",                               4,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EWM (ID: 0x0789) - Diagnostic Response Electronic Selector Lever Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ewm_0789_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_EWM (ID: 0x0724) - System Diagnostic Response Electronic Selector Lever Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_ewm_0724_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",             "System diagnostic response",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_MESS1 (ID: 0x06F0) - Selector Lever Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_mess1_06f0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_MESS2 (ID: 0x06F1) - Selector Lever Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_mess2_06f1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_110h (ID: 0x0110) - Drive Authorization FBS Message to MS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_110h_0110_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "IFZ_ST",            "FBS message to MS (8 bytes)",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_111h (ID: 0x0111) - Drive Authorization FBS Message to MS (Alternate) (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_111h_0111_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS_MS",            "FBS message to MS (8 bytes)",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_112h (ID: 0x0112) - Drive Authorization FBS Message to GS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_112h_0112_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS_GS",            "FBS message to GS (8 bytes)",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_114h (ID: 0x0114) - Drive Authorization FBS Message to EWM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_114h_0114_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS_EWM",           "FBS message to EWM (8 bytes)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_240h (ID: 0x0240) - Ignition Switch, Cruise Control Lever & Terminal Status (40 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_240h_0240_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1",     "Unknown signal",                                                          0,   2 },
        {   1, "WH_UP",         "Cruise control lever implausible",                                        2,   1 },
        {   2, "VMAX_AKT",      "Variable speed limiter operation",                                        3,   1 },
        {   3, "S_MINUS_B",     "Cruise control lever: \"Set and decelerate stage 0\"",                    4,   1 },
        {   4, "S_PLUS_B",      "Cruise control lever: \"Set and accelerate stage 0\"",                    5,   1 },
        {   5, "WA",            "Cruise control lever: \"Resume\"",                                        6,   1 },
        {   6, "AUS",           "Cruise control lever: \"Off\"",                                           7,   1 },
        {   7, "KG_KL_AKT",     "Keyless Go terminal control active",                                      8,   1 },
        {   8, "KG_ALB_OK",     "Keyless Go start conditions met",                                         9,   1 },
        {   9, "LL_RLC",        "Left/right hand drive",                                                  10,   2 },
        {  10, "RG_SCHALT",     "Reverse gear engaged (manual transmission only)",                        12,   1 },
        {  11, "BS_SL",         "Brake switch for shift-lock",                                            13,   1 },
        {  12, "KL_15",         "Terminal 15",                                                            14,   1 },
        {  13, "KL_50",         "Terminal 50",                                                            15,   1 },
        {  14, "UNKNOWN_2",     "Unknown signal",                                                         16,   3 },
        {  15, "WH_PA",         "Cruise control lever parity (even parity)",                              19,   1 },
        {  16, "BZ240h",        "Message counter",                                                        20,   4 },
        {  17, "UNKNOWN_3",     "Unknown signal",                                                         24,   3 },
        {  18, "ASG_SPORT_BET", "Automated transmission Sport mode on/off operated",                      27,   1 },
        {  19, "UNKNOWN_4",     "Unknown signal",                                                         28,   2 },
        {  20, "CRASH_CNF",     "Crash confirmation bit",                                                 30,   1 },
        {  21, "CRASH",         "Crash signal from airbag control unit",                                  31,   1 },
        {  22, "BN_NTLF",       "On-board electrical emergency: Prio 1/2 off, secondary battery backup",  32,   1 },
        {  23, "ESP_BET",       "ESP on/off button operated",                                             33,   2 },
        {  24, "HAS_KL",        "Parking brake applied (warning lamp)",                                   35,   1 },
        {  25, "KL_31B",        "Wiper outside park position",                                            36,   1 },
        {  26, "UNKNOWN_5",     "Unknown signal",                                                         37,   1 },
        {  27, "BLI_RE",        "Turn signal right",                                                      38,   1 },
        {  28, "BLI_LI",        "Turn signal left",                                                       39,   1 },
        {  29, "ST2_BET",       "Air suspension / ABC 2-stage switch operated",                           40,   2 },
        {  30, "ST3_BET",       "Air suspension / ABC 3-stage switch operated",                           42,   2 },
        {  31, "ART_ABW_BET",   "Distronic (ART) distance warning on/off operated",                       44,   2 },
        {  32, "ABL_EIN",       "Low beam switch on",                                                     46,   1 },
        {  33, "KL54_RM",       "Terminal 54 hardware active",                                            47,   1 },
        {  34, "ART_ABSTAND",   "Distronic (ART) distance factor",                                        48,   8 },
        {  35, "ART_VH",        "Distronic (ART) present",                                                56,   1 },
        {  36, "GBL_AUS",       "Electric suction fan: Base ventilation off",                             57,   1 },
        {  37, "UNKNOWN_6",     "Unknown signal",                                                         58,   1 },
        {  38, "FZGVERSN",      "Model series vehicle version (220/215/230 only)",                        59,   3 },
        {  39, "LDC",           "Country code",                                                           62,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ZGW_248h (ID: 0x0248) - Central Gateway Lighting, Pump & Trailer Status (8 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto zgw_248h_0248_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DIAG_X4_B",   "Start Xenon 4 diagnosis procedure passenger side",   0,   1 },
        {   1, "DIAG_X4_F",   "Start Xenon 4 diagnosis procedure driver side",      1,   1 },
        {   2, "UNKNOWN_1",   "Unknown signal",                                     2,   1 },
        {   3, "ABL_EIN",     "Low beam switch on",                                 3,   1 },
        {   4, "UNKNOWN_2",   "Unknown signal",                                     4,   8 },
        {   5, "AFL_ABL_EIN", "Headlamp assist request: Low beam switch on",       12,   1 },
        {   6, "ZWP_LFT",     "Auxiliary water pump running",                      13,   1 },
        {   7, "ANH_ERK2",    "Trailer detected",                                  14,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ZGW_24Ch (ID: 0x024C) - Central Gateway Low Beam Faults (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto zgw_24ch_024c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1",    "Unknown signal",                            0,  38 },
        {   1, "ABL_DEF_BF_R", "Low beam defective passenger/right side",  38,   1 },
        {   2, "ABL_DEF_F_L",  "Low beam defective driver/left side",      39,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_40Eh (ID: 0x040E) - Heating Demand (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_40eh_040e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HZL_ANF",           "Heating output demand (%)",                                                     0,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_410h (ID: 0x0410) - Climate Control Compressor & Fan Demands (14 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_410h_0410_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZH_EIN_OK",   "Auxiliary heater enable permitted",                 0,   1 },
        {   1, "LL_DZA",      "Idle speed increase for cooling capacity",          1,   1 },
        {   2, "UNKNOWN_1",   "Unknown signal",                                    2,   1 },
        {   3, "SENDE_NEU",   "Signal version compressor torque",                  3,   1 },
        {   4, "UNKNOWN_2",   "Unknown signal",                                    4,   1 },
        {   5, "M_KOMPPAR",   "A/C compressor torque parity (even parity)",        5,   1 },
        {   6, "M_KOMPTGL",   "A/C compressor torque toggle",                      6,   1 },
        {   7, "KOMP_EIN",    "A/C compressor switched on",                        7,   1 },
        {   8, "P_KAELTE8",   "Refrigerant pressure (bar)",                        8,   8 },
        {   9, "M_KOMP",      "A/C compressor torque absorption (Nm)",            16,   8 },
        {  10, "M_KOMP_NEU",  "A/C compressor torque new (Nm)",                   16,   8 },
        {  11, "NLFTS",       "Engine cooling fan target speed (%)",              24,   8 },
        {  12, "UNKNOWN_3",   "Unknown signal",                                   32,   8 },
        {  13, "T_AUSSEN_WM", "Outside temperature for thermal management (°C)",  40,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_MS (ID: 0x074C) - Application Interface to Engine Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_ms_074c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG_MS",        "External application to control unit",                                          0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ART (ID: 0x078E) - Diagnostic Request Distronic (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_art_078e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_BS (ID: 0x0784) - Diagnostic Request Brake System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_bs_0784_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_CAS (ID: 0x077A) - Diagnostic Request Collision Avoidance System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_cas_077a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_CAS",          "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_DTR (ID: 0x0702) - Diagnostic Request Distronic Radar (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_dtr_0702_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_DTR",          "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EHB (ID: 0x079A) - Diagnostic Request Electrohydraulic Brake (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ehb_079a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EHB2 (ID: 0x07B0) - Diagnostic Request Electrohydraulic Brake 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ehb2_07b0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EWM (ID: 0x0788) - Diagnostic Request Electronic Selector Lever Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ewm_0788_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FS (ID: 0x078C) - Diagnostic Request Active Body Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fs_078c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FSCM (ID: 0x0778) - Diagnostic Request Fuel System Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fscm_0778_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_FSCM",         "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_GS (ID: 0x07E1) - Diagnostic Request Transmission Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_gs_07e1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ISM (ID: 0x06EA) - Diagnostic Request Intelligent Servo Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ism_06ea_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_KOMBI_C (ID: 0x0796) - Diagnostic Request Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_kombi_c_0796_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_LWR (ID: 0x0794) - Diagnostic Request Headlight Range Adjustment (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_lwr_0794_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MS (ID: 0x07E0) - Diagnostic Request Engine Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ms_07e0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_RGS_L (ID: 0x07B2) - Diagnostic Request Seatbelt Tensioner Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_rgs_l_07b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_RGS_R (ID: 0x07B4) - Diagnostic Request Seatbelt Tensioner Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_rgs_r_07b4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_UP28 (ID: 0x07A2) - Diagnostic Request Microprocessor Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_up28_07a2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_APPL2 (ID: 0x06E4) - Application Interface Transmission Control 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_appl2_06e4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL2",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL1 (ID: 0x074A) - Application Interface Engine Control 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl1_074a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL3 (ID: 0x06E0) - Application Interface Engine Control 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl3_06e0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL3",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_MESS1 (ID: 0x060E) - Ignition Switch Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_mess1_060e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DG_RQ_OBD (ID: 0x07DF) - Global OBD-II Diagnostic Request (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dg_rq_obd_07df_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",              "KWP2000 diagnostic request",                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VIN (ID: 0x06FA) - Vehicle Identification Number (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vin_06fa_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1", "Unknown signal",    0,   6 },
        {   1, "VIN_MSG",   "VIN signal part",   6,   2 },
        {   2, "VIN_DATA",  "VIN data",          8,  56 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_408h (ID: 0x0408) - Fuel Level, Door Status, Speedometer & Odometer (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_408h_0408_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TANK_FS",           "Fuel tank level (%)",                                                           0,   8 },
        {   1, "TF_AUF",            "Driver door open",                                                              8,   1 },
        {   2, "V_DSPL_AUS",        "Speed limiter / cruise control display not possible",                           9,   1 },
        {   3, "TACHO_SYM",         "Speedometer calibration",                                                      10,   1 },
        {   4, "V_MPH",             "mph instead of km/h (variable speed limiter)",                                 11,   1 },
        {   5, "KLA_VH",            "Air conditioning system present",                                              12,   1 },
        {   6, "VGL_KL_DEF",        "Preheating indicator lamp defective",                                          13,   1 },
        {   7, "TFSM",              "Fuel level minimum",                                                           14,   1 },
        {   8, "KL_61E",            "Terminal 61 decoupled",                                                        15,   1 },
        {   9, "T_AUSSEN",          "Outside ambient air temperature raw value (°C)",                               16,   8 },
        {  10, "KL_58D",            "Terminal 58 dimmed (%)",                                                       24,   8 },
        {  11, "MAZ",               "Engine off time (sent from Kl.15) (min)",                                      32,   8 },
        {  12, "KM16",              "Odometer reading (km)",                                                        40,  16 },
        {  13, "WRC3",              "Winter tyre top speed limit (bit 3)",                                          56,   1 },
        {  14, "V_DSPL_AKT",        "Speed limiter / cruise control display active",                                57,   1 },
        {  15, "SGT_VH",            "Segment speedometer present",                                                  58,   1 },
        {  16, "ZH_FREIG",          "Auxiliary heater enable",                                                      59,   1 },
        {  17, "RT_EIN",            "Roller test mode ESP switch on",                                               60,   1 },
        {  18, "WRC",               "Winter tyre top speed limit (4-bit)",                                          61,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_412h (ID: 0x0412) - Cluster Speed, Warnings & Distance Setting (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_412h_0412_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AKU_WARN_AUS", "Acoustic warning off",                        0,   1 },
        {   1, "OPT_WARN_AUS", "Visual warning off",                          1,   1 },
        {   2, "UNKNOWN_1",    "Unknown signal",                              2,   2 },
        {   3, "ECO_WARN_ST",  "ECO warning status",                          4,   1 },
        {   4, "UNKNOWN_2",    "Unknown signal",                              5,   3 },
        {   5, "ABST_S",       "Distance unit",                               8,   1 },
        {   6, "IST_ABST",     "Set distance",                                9,   3 },
        {   7, "V_ANZ",        "Displayed vehicle speed (km/h)",             12,  12 },
        {   8, "DRTGANZ",      "Wheel rotation direction for V_ANZ",         24,   2 },
        {   9, "DANZ",         "Wheel speed calculated from V_ANZ (1/min)",  26,  14 },
        {  10, "UNKNOWN_3",    "Unknown signal",                             40,   4 },
        {  11, "ECO_AKT",      "ECO activation in instrument cluster menu",  44,   1 },
        {  12, "UNKNOWN_4",    "Unknown signal",                             45,   1 },
        {  13, "PRW_ANF",      "Flat tyre warning request",                  46,   2 },
        {  14, "UNKNOWN_5",    "Unknown signal",                             48,   4 },
        {  15, "MAZ_NEU",      "Engine off time (new) (min)",                52,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_KOMBI_C (ID: 0x0797) - Diagnostic Response Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_kombi_c_0797_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_MESS1 (ID: 0x0680) - Instrument Cluster Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_mess1_0680_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_MESS2 (ID: 0x0681) - Instrument Cluster Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_mess2_0681_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: LRW_236h (ID: 0x0236) - Steering Wheel Angle & Angular Speed (9 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto lrw_236h_0236_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1", "Unknown signal",                               0,   2 },
        {   1, "LRW",       "Steering wheel angle (°)",                     2,  14 },
        {   2, "UNKNOWN_2", "Unknown signal",                              16,   2 },
        {   3, "VLRW",      "Steering wheel angular velocity (°/s)",       18,  14 },
        {   4, "BZ236h",    "Message counter",                             32,   4 },
        {   5, "LRWS_ID",   "Steering wheel angle sensor identification",  36,   2 },
        {   6, "LRWS_ST",   "Steering wheel angle sensor status",          38,   2 },
        {   7, "UNKNOWN_3", "Unknown signal",                              40,  16 },
        {   8, "CRC236h",   "CRC checksum byte 1-7 per SAE J1850",         56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_238h (ID: 0x0238) - Cruise Control Lever & Steering Angle (18 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_238h_0238_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1", "Unknown signal",                                         0,   2 },
        {   1, "WH_UP",     "Cruise control lever implausible",                       2,   1 },
        {   2, "VMAX_AKT",  "Variable speed limiter operation",                       3,   1 },
        {   3, "S_MINUS_B", "Cruise control lever: \"Set and decelerate stage 0\"",   4,   1 },
        {   4, "S_PLUS_B",  "Cruise control lever: \"Set and accelerate stage 0\"",   5,   1 },
        {   5, "WA",        "Cruise control lever: \"Resume\"",                       6,   1 },
        {   6, "AUS",       "Cruise control lever: \"Off\"",                          7,   1 },
        {   7, "BLI_RE",    "Turn signal right",                                      8,   1 },
        {   8, "BLI_LI",    "Turn signal left",                                       9,   1 },
        {   9, "UNKNOWN_2", "Unknown signal",                                        10,   1 },
        {  10, "WH_PA",     "Cruise control lever parity (even parity)",             11,   1 },
        {  11, "BZ238h",    "Message counter",                                       12,   4 },
        {  12, "LW_PA",     "Steering angle parity (even parity)",                   16,   1 },
        {  13, "LW_OV",     "Steering angle sensor: Overflow",                       17,   1 },
        {  14, "LW_CF",     "Steering angle sensor: Code error",                     18,   1 },
        {  15, "LW_INI",    "Steering angle sensor: Not initialized",                19,   1 },
        {  16, "LW_VZ",     "Steering angle sign",                                   20,   1 },
        {  17, "LW",        "Steering angle (°)",                                    21,  11 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ARCADE_A2 (ID: 0x0035) - Crash Sensor Confirmation & Frontal Event (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto arcade_a2_0035_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "CONF_CRASH", "Confirmation bit for all crash events (toggling)",   0,   1 },
        {   1, "UNKNOWN_1",  "Unknown signal",                                     1,   1 },
        {   2, "CRASH_F",    "Frontal impact event 2",                             2,   1 },
        {   3, "UNKNOWN_2",  "Unknown signal",                                     3,   2 },
        {   4, "CRASH_C",    "Frontal impact event 5",                             5,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_ANZ (ID: 0x033D) - Engine Start/Stop Display Status (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_anz_033d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1",     "Unknown signal",                               0,  16 },
        {   1, "ASS_WARN",      "Start/Stop warning message number",           16,   4 },
        {   2, "ASS_DSPL",      "Start/Stop status message number",            20,   4 },
        {   3, "ASS_LTEST_AUS", "Suppression of lamp test during stop phase",  24,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_102h (ID: 0x0102) - Drive Authorization FBS Message from GS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_102h_0102_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GS_FBS",            "FBS message to EZS (8 bytes)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_104h (ID: 0x0104) - Drive Authorization FBS Message from EWM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_104h_0104_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "EWM_FBS",           "FBS message to EZS (8 bytes)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SBW_232h (ID: 0x0232) - Shift-by-Wire Control Buttons & Lever Status (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sbw_232h_0232_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SID_SBW",     "Transmitter identification",                     0,   2 },
        {   1, "UNKNOWN_1",   "Unknown signal",                                 2,   3 },
        {   2, "LRT_PM3",     "Steering wheel buttons \"+\", \"-\" operated",   5,   3 },
        {   3, "SBWB_ID",     "Shift-by-wire control element identification",   8,   2 },
        {   4, "SBWB_ST_P",   "Shift-by-wire control element P button",        10,   2 },
        {   5, "SBWB_ST_RND", "Shift-by-wire control element status RND",      12,   4 },
        {   6, "BZ232h",      "Message counter",                               16,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ART_250h (ID: 0x0250) - Distronic Torque Request & Target Gears (18 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto art_250h_0250_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SLV_ART",   "Shift line offset Distronic (ART)",               0,   4 },
        {   1, "ART_OK",    "Distronic (ART) OK",                              4,   1 },
        {   2, "ART_BRE",   "Distronic (ART) braking",                         5,   1 },
        {   3, "BL_UNT",    "Brake light suppression",                         6,   1 },
        {   4, "DYN_UNT",   "Suppression of dynamic full-load downshift",      7,   1 },
        {   5, "MPAR_ART",  "Engine torque request parity (even parity)",      8,   1 },
        {   6, "MDYN_ART",  "Engine torque request dynamic",                   9,   1 },
        {   7, "CAS_REG",   "City assistant controlling",                     10,   1 },
        {   8, "UNKNOWN_1", "Unknown signal",                                 11,   6 },
        {   9, "LIM_REG",   "Limiter controlling",                            17,   1 },
        {  10, "ART_REG",   "Distronic (ART) controlling",                    18,   1 },
        {  11, "M_ART",     "Requested engine torque (Nm)",                   19,  13 },
        {  12, "BZ250h",    "Message counter",                                32,   4 },
        {  13, "MBRE_ART",  "Brake torque (0000h: passive value) (Nm)",       36,  12 },
        {  14, "AKT_R_ART", "Distronic (ART) request: \"Active downshift\"",  48,   1 },
        {  15, "UNKNOWN_2", "Unknown signal",                                 49,   1 },
        {  16, "GMAX_ART",  "Target gear, upper limit",                       50,   3 },
        {  17, "GMIN_ART",  "Target gear, lower limit",                       53,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ART_258h (ID: 0x0258) - Distronic Target Distance, Speed & Object Detection (26 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto art_258h_0258_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ART_DSPL_EIN",      "Switch display to Distronic (ART) view",                                        0,   1 },
        {   1, "S_OBJ",             "Stationary object detected",                                                    1,   1 },
        {   2, "ART_WT",            "Distronic (ART) warning tone",                                                  2,   1 },
        {   3, "ART_INFO",          "Distronic (ART) info lamp",                                                     3,   1 },
        {   4, "ART_ERR",           "Distronic (ART) error code",                                                    4,   4 },
        {   5, "V_ART",             "Set Distronic (ART) speed (km/h)",                                              8,   8 },
        {   6, "ABST_R_OBJ",        "Distance to relevant object (m)",                                              16,   8 },
        {   7, "SOLL_ABST",         "Driver target distance (m)",                                                   24,   8 },
        {   8, "ART_DSPL_PGB",      "Display \"Winter tyre limit reached\"",                                        32,   1 },
        {   9, "ART_VFBR",          "Display \"DTR OFF [0]\"",                                                      33,   1 },
        {  10, "ART_DSPL_LIM",      "Display \"---\"",                                                              34,   1 },
        {  11, "ART_EIN",           "Adaptive cruise control switched on",                                          35,   1 },
        {  12, "OBJ_ERK",           "Relevant object detected",                                                     36,   1 },
        {  13, "ART_SEG_EIN",       "Distronic (ART) segment display on",                                           37,   1 },
        {  14, "ART_DSPL_BL",       "Speed display flashing",                                                       38,   1 },
        {  15, "TM_EIN_ART",        "Distronic (ART) cruise control on",                                            39,   1 },
        {  16, "V_ZIEL",            "Target vehicle speed (km/h)",                                                  40,   8 },
        {  17, "ART_DSPL_NEU",      "Retrigger minimum display time",                                               48,   1 },
        {  18, "ART_UEBERSP",       "Distronic (ART) overridden by driver",                                         49,   1 },
        {  19, "ART_REAKT",         "Display system availability after system error",                               50,   1 },
        {  20, "ART_ABW_AKT",       "Distronic (ART) distance warning switched on",                                 51,   1 },
        {  21, "OBJ_AGB",           "Object offer distance assistant",                                              52,   1 },
        {  22, "AAS_LED_BL",        "Distance assistant LED flashing",                                              53,   1 },
        {  23, "ASSIST_FKT_AKT",    "Active assistance function",                                                   54,   2 },
        {  24, "CAS_ERR_ANZ_V2",    "CAS display request",                                                          56,   3 },
        {  25, "ASSIST_ANZ_V2",     "Assistance system display request",                                            59,   5 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FS_340h (ID: 0x0340) - ABC Suspension Pump Load Torque (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fs_340h_0340_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1", "Unknown signal",              0,  60 },
        {   1, "M_LAST",    "ABC pump load torque (Nm)",  60,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: PSM_3B4h (ID: 0x03B4) - Parametric Module Engine Speed & Torque Limits (14 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto psm_3b4h_03b4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PSM_ADR_PAR",  "Working speed control parity bit",                    0,   1 },
        {   1, "PSM_ADR_TGL",  "Working speed control toggle bit",                    1,   1 },
        {   2, "PSM_ADR_AKT",  "Working speed control active",                        2,   1 },
        {   3, "UNKNOWN_1",    "Unknown signal",                                      3,   5 },
        {   4, "PSM_N_SOLL",   "Engine target speed working speed control (1/min)",   8,  16 },
        {   5, "PSM_MOM_PAR",  "Torque limit parity bit",                            24,   1 },
        {   6, "PSM_MOM_TGL",  "Torque limit toggle bit",                            25,   1 },
        {   7, "PSM_MOM_AKT",  "Torque limit active",                                26,   1 },
        {   8, "PSM_MOM_SOLL", "Maximum engine torque (Nm)",                         27,  13 },
        {   9, "PSM_DZ_PAR",   "Speed limit parity bit",                             40,   1 },
        {  10, "PSM_DZ_TGL",   "Speed limit toggle bit",                             41,   1 },
        {  11, "PSM_DZ_AKT",   "Speed limit active",                                 42,   1 },
        {  12, "UNKNOWN_2",    "Unknown signal",                                     43,   5 },
        {  13, "PSM_DZ_MAX",   "Maximum engine speed (1/min)",                       48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: PSM_3B8h (ID: 0x03B8) - Parametric Module Speed Limit & Remote Start (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto psm_3b8h_03b8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PSM_V_PAR",      "Speed limit parity bit",          0,   1 },
        {   1, "PSM_V_TGL",      "Speed limit toggle bit",          1,   1 },
        {   2, "PSM_V_AKT",      "Speed limit active",              2,   1 },
        {   3, "UNKNOWN_1",      "Unknown signal",                  3,   5 },
        {   4, "PSM_V_SOLL",     "Vehicle speed limit (km/h)",      8,   8 },
        {   5, "PSM_DZ_PAR",     "Speed limit parity bit",         16,   1 },
        {   6, "PSM_DZ_TGL",     "Speed limit toggle bit",         17,   1 },
        {   7, "PSM_FERN_START", "Engine remote start active",     18,   1 },
        {   8, "PSM_FERN_STOP",  "Engine remote stop active",      19,   1 },
        {   9, "PSM_FPM_SP",     "Lock accelerator pedal module",  20,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_414h (ID: 0x0414) - Filtered Outside Temperature (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_414h_0414_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "T_AUSSEN_K",        "Filtered outside ambient air temperature (°C)",                                 0,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VG_428h (ID: 0x0428) - Transfer Case Gear & Neutral Request (8 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vg_428h_0428_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1",  "Unknown signal",                                            0,   2 },
        {   1, "VG_ERR",     "Transfer case fault detected",                              2,   1 },
        {   2, "UNKNOWN_2",  "Unknown signal",                                            3,   2 },
        {   3, "VG_GANG",    "Current transfer case gear",                                5,   3 },
        {   4, "UNKNOWN_3",  "Unknown signal",                                            8,   4 },
        {   5, "ANFNPAR_VG", "Transfer case request engage \"N\" parity (even parity)",  12,   1 },
        {   6, "ANFNTGL_VG", "Transfer case request engage \"N\" toggle 20ms",           13,   1 },
        {   7, "ANFN_VG",    "Transfer case request engage \"N\"",                       14,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: LWR_530h (ID: 0x0530) - Headlight Range & Cornering Light Messages (10 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto lwr_530h_0530_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "UNKNOWN_1", "Unknown signal",                                                                   0,   1 },
        {   1, "LWR_M7",    "Display Msg 7: \"Cornering light currently not available\"",                       1,   1 },
        {   2, "LWR_M6",    "Display Msg 6: \"Cornering light right\"",                                         2,   1 },
        {   3, "LWR_M5",    "Display Msg 5: \"Cornering light left\"",                                          3,   1 },
        {   4, "LWR_M4",    "Display Msg 4: \"Active curve light currently not available\" (white/flashing)",   4,   1 },
        {   5, "LWR_M3",    "Display Msg 3: \"Active curve light currently not available\" (white)",            5,   1 },
        {   6, "LWR_M2",    "Display Msg 2: \"Active curve light substitute light activated\" (white)",         6,   1 },
        {   7, "LWR_M1",    "Display Msg 1: \"Active curve light defective, visit workshop\"",                  7,   1 },
        {   8, "SUB_ABL_L", "Low beam substitution left",                                                       8,   1 },
        {   9, "SUB_ABL_R", "Low beam substitution right",                                                      9,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: CONFIG_6FFh (ID: 0x06FF) - Drivetrain Equipment & Differential Lock Coding (8 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto config_6ffh_06ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GBL_AUS",   "Electric suction fan: Base ventilation off",   0,   1 },
        {   1, "UNKNOWN_1", "Unknown signal",                               1,  49 },
        {   2, "KLA_VH",    "Air conditioning system present",             50,   1 },
        {   3, "UNKNOWN_2", "Unknown signal",                              51,   9 },
        {   4, "DSH_VH",    "Rear differential lock present",              60,   1 },
        {   5, "DSM_VH",    "Center differential lock present",            61,   1 },
        {   6, "DSV_VH",    "Front differential lock present",             62,   1 },
        {   7, "VG_VH",     "Transfer case control present",               63,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ART (ID: 0x078F) - Diagnostic Response Distronic (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_art_078f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_CAS (ID: 0x077B) - Diagnostic Response Collision Avoidance System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_cas_077b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_CAS",          "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_DTR (ID: 0x04A0) - Diagnostic Response Distronic Radar (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_dtr_04a0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_DTR",          "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EHB (ID: 0x079B) - Diagnostic Response Electrohydraulic Brake (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ehb_079b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EHB2 (ID: 0x07B1) - Diagnostic Response Electrohydraulic Brake 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ehb2_07b1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FS (ID: 0x078D) - Diagnostic Response Active Body Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fs_078d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ISM (ID: 0x049D) - Diagnostic Response Intelligent Servo Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ism_049d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_LWR (ID: 0x0795) - Diagnostic Response Headlight Range Adjustment (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_lwr_0795_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_RGS_L (ID: 0x07B3) - Diagnostic Response Seatbelt Tensioner Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_rgs_l_07b3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_RGS_R (ID: 0x07B5) - Diagnostic Response Seatbelt Tensioner Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_rgs_r_07b5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_UP28 (ID: 0x07A3) - Diagnostic Response Microprocessor Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_up28_07a3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",              "KWP2000 diagnostic response",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_APPL1 (ID: 0x0634) - Application Interface Brake System 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_appl1_0634_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL1 (ID: 0x0630) - Application Interface Engine Control (Alternate) (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl1_0630_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL3 (ID: 0x0632) - Application Interface Engine Control 3 (Alternate) (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl3_0632_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL3",             "Application",                                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_MESS2 (ID: 0x060F) - Ignition Switch Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_mess2_060f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",             "Measurement values",                                                            0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS1 (ID: 0x0501) - Test Bench Manual Control 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs1_0501_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS1",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS2 (ID: 0x0502) - Test Bench Manual Control 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs2_0502_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS2",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS3 (ID: 0x0503) - Test Bench Manual Control 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs3_0503_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS3",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS4 (ID: 0x0504) - Test Bench Manual Control 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs4_0504_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS4",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS5 (ID: 0x0505) - Test Bench Manual Control 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs5_0505_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS5",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS6 (ID: 0x0506) - Test Bench Manual Control 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs6_0506_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS6",               "Manual control on test bench",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // CAN-C Message Registry (120 Message(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto w209_c_messages = std::to_array<const can_message_spec>
    ({
        { 0x0000, false, 8, "UNKNOWN",      "Brake Status, ESP & Front Wheel Speeds",                        unknown_0000_signals.data(),      unknown_0000_signals.size()      },
        { 0x0208, false, 8, "BS_208h",      "ESP Brake Intervention & Rear Wheel Speeds",                    bs_208h_0208_signals.data(),      bs_208h_0208_signals.size()      },
        { 0x0270, false, 8, "BS_270h",      "Rear Wheel Pulse Rings & Flat Tyre Warning",                    bs_270h_0270_signals.data(),      bs_270h_0270_signals.size()      },
        { 0x0300, false, 8, "BS_300h",      "ESP & ART Torque Request & Yaw Rate",                           bs_300h_0300_signals.data(),      bs_300h_0300_signals.size()      },
        { 0x0328, false, 8, "BS_328h",      "Roll Moment, Lateral Acceleration & Front Pulse Rings",         bs_328h_0328_signals.data(),      bs_328h_0328_signals.size()      },
        { 0x0785, false, 8, "D_RS_BS",      "Diagnostic Response Brake System",                              d_rs_bs_0785_signals.data(),      d_rs_bs_0785_signals.size()      },
        { 0x0722, false, 8, "SD_RS_BS",     "System Diagnostic Response Brake System",                       sd_rs_bs_0722_signals.data(),     sd_rs_bs_0722_signals.size()     },
        { 0x0635, false, 8, "BS_APPL2",     "Application Interface Brake System 2",                          bs_appl2_0635_signals.data(),     bs_appl2_0635_signals.size()     },
        { 0x0100, false, 8, "MS_100h",      "Drive Authorization FBS Message to EZS",                        ms_100h_0100_signals.data(),      ms_100h_0100_signals.size()      },
        { 0x0101, false, 8, "MS_101h",      "Drive Authorization FBS Message to EZS (Alternate)",            ms_101h_0101_signals.data(),      ms_101h_0101_signals.size()      },
        { 0x0210, false, 8, "MS_210h",      "Engine Status, Accelerator Pedal & Cruise Control",             ms_210h_0210_signals.data(),      ms_210h_0210_signals.size()      },
        { 0x0212, false, 8, "MS_212h",      "Engine Speed & Torque Requests",                                ms_212h_0212_signals.data(),      ms_212h_0212_signals.size()      },
        { 0x0268, false, 8, "MS_268h",      "Gear Ratio Limits, Alternator Load & A/C Torque",               ms_268h_0268_signals.data(),      ms_268h_0268_signals.size()      },
        { 0x02F3, false, 8, "MS_2F3h",      "Shift Recommendation",                                          ms_2f3h_02f3_signals.data(),      ms_2f3h_02f3_signals.size()      },
        { 0x0308, false, 8, "MS_308h",      "Engine RPM, Temperatures & Warning Lamps",                      ms_308h_0308_signals.data(),      ms_308h_0308_signals.size()      },
        { 0x0312, false, 8, "MS_312h",      "Engine Static, Max & Min Torque Limits",                        ms_312h_0312_signals.data(),      ms_312h_0312_signals.size()      },
        { 0x0580, false, 8, "AAD_580h",     "Adaptive Accelerator Pedal & Driver Behavior",                  aad_580h_0580_signals.data(),     aad_580h_0580_signals.size()     },
        { 0x0608, false, 8, "MS_608h",      "Coolant & Intake Air Temp, Consumption & Particulate Filter",   ms_608h_0608_signals.data(),      ms_608h_0608_signals.size()      },
        { 0x0779, false, 8, "D_RS_FSCM",    "Diagnostic Response Fuel System Control Module",                d_rs_fscm_0779_signals.data(),    d_rs_fscm_0779_signals.size()    },
        { 0x07E8, false, 8, "D_RS_MS",      "Diagnostic Response Engine Control Module",                     d_rs_ms_07e8_signals.data(),      d_rs_ms_07e8_signals.size()      },
        { 0x0720, false, 8, "SD_RS_MS",     "System Diagnostic Response Engine Control Module",              sd_rs_ms_0720_signals.data(),     sd_rs_ms_0720_signals.size()     },
        { 0x0529, false, 8, "SG_APPL_MS",   "Application Interface Engine Control",                          sg_appl_ms_0529_signals.data(),   sg_appl_ms_0529_signals.size()   },
        { 0x04A9, false, 8, "MS_APPL2",     "Application Interface Engine Control 2",                        ms_appl2_04a9_signals.data(),     ms_appl2_04a9_signals.size()     },
        { 0x0633, false, 8, "MS_APPL4",     "Application Interface Engine Control 4",                        ms_appl4_0633_signals.data(),     ms_appl4_0633_signals.size()     },
        { 0x06A8, false, 8, "MS_APPL5",     "Application Interface Engine Control 5",                        ms_appl5_06a8_signals.data(),     ms_appl5_06a8_signals.size()     },
        { 0x0610, false, 8, "MS_APPL6",     "Application Interface Engine Control 6",                        ms_appl6_0610_signals.data(),     ms_appl6_0610_signals.size()     },
        { 0x0618, false, 8, "MS_APPL7",     "Application Interface Engine Control 7",                        ms_appl7_0618_signals.data(),     ms_appl7_0618_signals.size()     },
        { 0x0670, false, 8, "EDC_MESS1",    "Electronic Diesel Control Measurement Data 1",                  edc_mess1_0670_signals.data(),    edc_mess1_0670_signals.size()    },
        { 0x0671, false, 8, "EDC_MESS2",    "Electronic Diesel Control Measurement Data 2",                  edc_mess2_0671_signals.data(),    edc_mess2_0671_signals.size()    },
        { 0x0218, false, 8, "GS_218h",      "Transmission Shift Status, Current Gear & Torque Intervention", gs_218h_0218_signals.data(),      gs_218h_0218_signals.size()      },
        { 0x0338, false, 8, "GS_338h",      "Transmission Output & Turbine RPM",                             gs_338h_0338_signals.data(),      gs_338h_0338_signals.size()      },
        { 0x0418, false, 8, "GS_418h",      "Drive Program, Transmission Temp & Wheel Torque Factor",        gs_418h_0418_signals.data(),      gs_418h_0418_signals.size()      },
        { 0x07E9, false, 8, "D_RS_GS",      "Diagnostic Response Transmission Control",                      d_rs_gs_07e9_signals.data(),      d_rs_gs_07e9_signals.size()      },
        { 0x0723, false, 8, "SD_RS_GS",     "System Diagnostic Response Transmission Control",               sd_rs_gs_0723_signals.data(),     sd_rs_gs_0723_signals.size()     },
        { 0x051C, false, 8, "GS_APPL1",     "Application Interface Transmission Control 1",                  gs_appl1_051c_signals.data(),     gs_appl1_051c_signals.size()     },
        { 0x050A, false, 8, "GS_HSA",       "Test Bench Manual Control A",                                   gs_hsa_050a_signals.data(),       gs_hsa_050a_signals.size()       },
        { 0x050B, false, 8, "GS_HSB",       "Test Bench Manual Control B",                                   gs_hsb_050b_signals.data(),       gs_hsb_050b_signals.size()       },
        { 0x050C, false, 8, "GS_HSC",       "Test Bench Manual Control C",                                   gs_hsc_050c_signals.data(),       gs_hsc_050c_signals.size()       },
        { 0x050D, false, 8, "GS_HSD",       "Test Bench Manual Control D",                                   gs_hsd_050d_signals.data(),       gs_hsd_050d_signals.size()       },
        { 0x050E, false, 8, "GS_HSE",       "Test Bench Manual Control E",                                   gs_hse_050e_signals.data(),       gs_hse_050e_signals.size()       },
        { 0x0230, false, 8, "EWM_230h",     "Electronic Selector Lever Module Status",                       ewm_230h_0230_signals.data(),     ewm_230h_0230_signals.size()     },
        { 0x0789, false, 8, "D_RS_EWM",     "Diagnostic Response Electronic Selector Lever Module",          d_rs_ewm_0789_signals.data(),     d_rs_ewm_0789_signals.size()     },
        { 0x0724, false, 8, "SD_RS_EWM",    "System Diagnostic Response Electronic Selector Lever Module",   sd_rs_ewm_0724_signals.data(),    sd_rs_ewm_0724_signals.size()    },
        { 0x06F0, false, 8, "EWM_MESS1",    "Selector Lever Measurement Data 1",                             ewm_mess1_06f0_signals.data(),    ewm_mess1_06f0_signals.size()    },
        { 0x06F1, false, 8, "EWM_MESS2",    "Selector Lever Measurement Data 2",                             ewm_mess2_06f1_signals.data(),    ewm_mess2_06f1_signals.size()    },
        { 0x0110, false, 8, "FBS_110h",     "Drive Authorization FBS Message to MS",                         fbs_110h_0110_signals.data(),     fbs_110h_0110_signals.size()     },
        { 0x0111, false, 8, "FBS_111h",     "Drive Authorization FBS Message to MS (Alternate)",             fbs_111h_0111_signals.data(),     fbs_111h_0111_signals.size()     },
        { 0x0112, false, 8, "FBS_112h",     "Drive Authorization FBS Message to GS",                         fbs_112h_0112_signals.data(),     fbs_112h_0112_signals.size()     },
        { 0x0114, false, 8, "FBS_114h",     "Drive Authorization FBS Message to EWM",                        fbs_114h_0114_signals.data(),     fbs_114h_0114_signals.size()     },
        { 0x0240, false, 8, "EZS_240h",     "Ignition Switch, Cruise Control Lever & Terminal Status",       ezs_240h_0240_signals.data(),     ezs_240h_0240_signals.size()     },
        { 0x0248, false, 8, "ZGW_248h",     "Central Gateway Lighting, Pump & Trailer Status",               zgw_248h_0248_signals.data(),     zgw_248h_0248_signals.size()     },
        { 0x024C, false, 8, "ZGW_24Ch",     "Central Gateway Low Beam Faults",                               zgw_24ch_024c_signals.data(),     zgw_24ch_024c_signals.size()     },
        { 0x040E, false, 8, "KLA_40Eh",     "Heating Demand",                                                kla_40eh_040e_signals.data(),     kla_40eh_040e_signals.size()     },
        { 0x0410, false, 8, "KLA_410h",     "Climate Control Compressor & Fan Demands",                      kla_410h_0410_signals.data(),     kla_410h_0410_signals.size()     },
        { 0x074C, false, 8, "APPL_SG_MS",   "Application Interface to Engine Control",                       appl_sg_ms_074c_signals.data(),   appl_sg_ms_074c_signals.size()   },
        { 0x078E, false, 8, "D_RQ_ART",     "Diagnostic Request Distronic",                                  d_rq_art_078e_signals.data(),     d_rq_art_078e_signals.size()     },
        { 0x0784, false, 8, "D_RQ_BS",      "Diagnostic Request Brake System",                               d_rq_bs_0784_signals.data(),      d_rq_bs_0784_signals.size()      },
        { 0x077A, false, 8, "D_RQ_CAS",     "Diagnostic Request Collision Avoidance System",                 d_rq_cas_077a_signals.data(),     d_rq_cas_077a_signals.size()     },
        { 0x0702, false, 8, "D_RQ_DTR",     "Diagnostic Request Distronic Radar",                            d_rq_dtr_0702_signals.data(),     d_rq_dtr_0702_signals.size()     },
        { 0x079A, false, 8, "D_RQ_EHB",     "Diagnostic Request Electrohydraulic Brake",                     d_rq_ehb_079a_signals.data(),     d_rq_ehb_079a_signals.size()     },
        { 0x07B0, false, 8, "D_RQ_EHB2",    "Diagnostic Request Electrohydraulic Brake 2",                   d_rq_ehb2_07b0_signals.data(),    d_rq_ehb2_07b0_signals.size()    },
        { 0x0788, false, 8, "D_RQ_EWM",     "Diagnostic Request Electronic Selector Lever Module",           d_rq_ewm_0788_signals.data(),     d_rq_ewm_0788_signals.size()     },
        { 0x078C, false, 8, "D_RQ_FS",      "Diagnostic Request Active Body Control",                        d_rq_fs_078c_signals.data(),      d_rq_fs_078c_signals.size()      },
        { 0x0778, false, 8, "D_RQ_FSCM",    "Diagnostic Request Fuel System Control Module",                 d_rq_fscm_0778_signals.data(),    d_rq_fscm_0778_signals.size()    },
        { 0x07E1, false, 8, "D_RQ_GS",      "Diagnostic Request Transmission Control",                       d_rq_gs_07e1_signals.data(),      d_rq_gs_07e1_signals.size()      },
        { 0x06EA, false, 8, "D_RQ_ISM",     "Diagnostic Request Intelligent Servo Module",                   d_rq_ism_06ea_signals.data(),     d_rq_ism_06ea_signals.size()     },
        { 0x0796, false, 8, "D_RQ_KOMBI_C", "Diagnostic Request Instrument Cluster",                         d_rq_kombi_c_0796_signals.data(), d_rq_kombi_c_0796_signals.size() },
        { 0x0794, false, 8, "D_RQ_LWR",     "Diagnostic Request Headlight Range Adjustment",                 d_rq_lwr_0794_signals.data(),     d_rq_lwr_0794_signals.size()     },
        { 0x07E0, false, 8, "D_RQ_MS",      "Diagnostic Request Engine Control Module",                      d_rq_ms_07e0_signals.data(),      d_rq_ms_07e0_signals.size()      },
        { 0x07B2, false, 8, "D_RQ_RGS_L",   "Diagnostic Request Seatbelt Tensioner Left",                    d_rq_rgs_l_07b2_signals.data(),   d_rq_rgs_l_07b2_signals.size()   },
        { 0x07B4, false, 8, "D_RQ_RGS_R",   "Diagnostic Request Seatbelt Tensioner Right",                   d_rq_rgs_r_07b4_signals.data(),   d_rq_rgs_r_07b4_signals.size()   },
        { 0x07A2, false, 8, "D_RQ_UP28",    "Diagnostic Request Microprocessor Control",                     d_rq_up28_07a2_signals.data(),    d_rq_up28_07a2_signals.size()    },
        { 0x06E4, false, 8, "GS_APPL2",     "Application Interface Transmission Control 2",                  gs_appl2_06e4_signals.data(),     gs_appl2_06e4_signals.size()     },
        { 0x074A, false, 8, "MS_APPL1",     "Application Interface Engine Control 1",                        ms_appl1_074a_signals.data(),     ms_appl1_074a_signals.size()     },
        { 0x06E0, false, 8, "MS_APPL3",     "Application Interface Engine Control 3",                        ms_appl3_06e0_signals.data(),     ms_appl3_06e0_signals.size()     },
        { 0x060E, false, 8, "EZS_MESS1",    "Ignition Switch Measurement Data 1",                            ezs_mess1_060e_signals.data(),    ezs_mess1_060e_signals.size()    },
        { 0x07DF, false, 8, "DG_RQ_OBD",    "Global OBD-II Diagnostic Request",                              dg_rq_obd_07df_signals.data(),    dg_rq_obd_07df_signals.size()    },
        { 0x06FA, false, 8, "VIN",          "Vehicle Identification Number",                                 vin_06fa_signals.data(),          vin_06fa_signals.size()          },
        { 0x0408, false, 8, "KOMBI_408h",   "Fuel Level, Door Status, Speedometer & Odometer",               kombi_408h_0408_signals.data(),   kombi_408h_0408_signals.size()   },
        { 0x0412, false, 8, "KOMBI_412h",   "Cluster Speed, Warnings & Distance Setting",                    kombi_412h_0412_signals.data(),   kombi_412h_0412_signals.size()   },
        { 0x0797, false, 8, "D_RS_KOMBI_C", "Diagnostic Response Instrument Cluster",                        d_rs_kombi_c_0797_signals.data(), d_rs_kombi_c_0797_signals.size() },
        { 0x0680, false, 8, "KOMBI_MESS1",  "Instrument Cluster Measurement Data 1",                         kombi_mess1_0680_signals.data(),  kombi_mess1_0680_signals.size()  },
        { 0x0681, false, 8, "KOMBI_MESS2",  "Instrument Cluster Measurement Data 2",                         kombi_mess2_0681_signals.data(),  kombi_mess2_0681_signals.size()  },
        { 0x0236, false, 8, "LRW_236h",     "Steering Wheel Angle & Angular Speed",                          lrw_236h_0236_signals.data(),     lrw_236h_0236_signals.size()     },
        { 0x0238, false, 8, "MRM_238h",     "Cruise Control Lever & Steering Angle",                         mrm_238h_0238_signals.data(),     mrm_238h_0238_signals.size()     },
        { 0x0035, false, 8, "ARCADE_A2",    "Crash Sensor Confirmation & Frontal Event",                     arcade_a2_0035_signals.data(),    arcade_a2_0035_signals.size()    },
        { 0x033D, false, 8, "MS_ANZ",       "Engine Start/Stop Display Status",                              ms_anz_033d_signals.data(),       ms_anz_033d_signals.size()       },
        { 0x0102, false, 8, "GS_102h",      "Drive Authorization FBS Message from GS",                       gs_102h_0102_signals.data(),      gs_102h_0102_signals.size()      },
        { 0x0104, false, 8, "EWM_104h",     "Drive Authorization FBS Message from EWM",                      ewm_104h_0104_signals.data(),     ewm_104h_0104_signals.size()     },
        { 0x0232, false, 8, "SBW_232h",     "Shift-by-Wire Control Buttons & Lever Status",                  sbw_232h_0232_signals.data(),     sbw_232h_0232_signals.size()     },
        { 0x0250, false, 8, "ART_250h",     "Distronic Torque Request & Target Gears",                       art_250h_0250_signals.data(),     art_250h_0250_signals.size()     },
        { 0x0258, false, 8, "ART_258h",     "Distronic Target Distance, Speed & Object Detection",           art_258h_0258_signals.data(),     art_258h_0258_signals.size()     },
        { 0x0340, false, 8, "FS_340h",      "ABC Suspension Pump Load Torque",                               fs_340h_0340_signals.data(),      fs_340h_0340_signals.size()      },
        { 0x03B4, false, 8, "PSM_3B4h",     "Parametric Module Engine Speed & Torque Limits",                psm_3b4h_03b4_signals.data(),     psm_3b4h_03b4_signals.size()     },
        { 0x03B8, false, 8, "PSM_3B8h",     "Parametric Module Speed Limit & Remote Start",                  psm_3b8h_03b8_signals.data(),     psm_3b8h_03b8_signals.size()     },
        { 0x0414, false, 8, "KOMBI_414h",   "Filtered Outside Temperature",                                  kombi_414h_0414_signals.data(),   kombi_414h_0414_signals.size()   },
        { 0x0428, false, 8, "VG_428h",      "Transfer Case Gear & Neutral Request",                          vg_428h_0428_signals.data(),      vg_428h_0428_signals.size()      },
        { 0x0530, false, 8, "LWR_530h",     "Headlight Range & Cornering Light Messages",                    lwr_530h_0530_signals.data(),     lwr_530h_0530_signals.size()     },
        { 0x06FF, false, 8, "CONFIG_6FFh",  "Drivetrain Equipment & Differential Lock Coding",               config_6ffh_06ff_signals.data(),  config_6ffh_06ff_signals.size()  },
        { 0x078F, false, 8, "D_RS_ART",     "Diagnostic Response Distronic",                                 d_rs_art_078f_signals.data(),     d_rs_art_078f_signals.size()     },
        { 0x077B, false, 8, "D_RS_CAS",     "Diagnostic Response Collision Avoidance System",                d_rs_cas_077b_signals.data(),     d_rs_cas_077b_signals.size()     },
        { 0x04A0, false, 8, "D_RS_DTR",     "Diagnostic Response Distronic Radar",                           d_rs_dtr_04a0_signals.data(),     d_rs_dtr_04a0_signals.size()     },
        { 0x079B, false, 8, "D_RS_EHB",     "Diagnostic Response Electrohydraulic Brake",                    d_rs_ehb_079b_signals.data(),     d_rs_ehb_079b_signals.size()     },
        { 0x07B1, false, 8, "D_RS_EHB2",    "Diagnostic Response Electrohydraulic Brake 2",                  d_rs_ehb2_07b1_signals.data(),    d_rs_ehb2_07b1_signals.size()    },
        { 0x078D, false, 8, "D_RS_FS",      "Diagnostic Response Active Body Control",                       d_rs_fs_078d_signals.data(),      d_rs_fs_078d_signals.size()      },
        { 0x049D, false, 8, "D_RS_ISM",     "Diagnostic Response Intelligent Servo Module",                  d_rs_ism_049d_signals.data(),     d_rs_ism_049d_signals.size()     },
        { 0x0795, false, 8, "D_RS_LWR",     "Diagnostic Response Headlight Range Adjustment",                d_rs_lwr_0795_signals.data(),     d_rs_lwr_0795_signals.size()     },
        { 0x07B3, false, 8, "D_RS_RGS_L",   "Diagnostic Response Seatbelt Tensioner Left",                   d_rs_rgs_l_07b3_signals.data(),   d_rs_rgs_l_07b3_signals.size()   },
        { 0x07B5, false, 8, "D_RS_RGS_R",   "Diagnostic Response Seatbelt Tensioner Right",                  d_rs_rgs_r_07b5_signals.data(),   d_rs_rgs_r_07b5_signals.size()   },
        { 0x07A3, false, 8, "D_RS_UP28",    "Diagnostic Response Microprocessor Control",                    d_rs_up28_07a3_signals.data(),    d_rs_up28_07a3_signals.size()    },
        { 0x0634, false, 8, "BS_APPL1",     "Application Interface Brake System 1",                          bs_appl1_0634_signals.data(),     bs_appl1_0634_signals.size()     },
        { 0x0630, false, 8, "MS_APPL1",     "Application Interface Engine Control (Alternate)",              ms_appl1_0630_signals.data(),     ms_appl1_0630_signals.size()     },
        { 0x0632, false, 8, "MS_APPL3",     "Application Interface Engine Control 3 (Alternate)",            ms_appl3_0632_signals.data(),     ms_appl3_0632_signals.size()     },
        { 0x060F, false, 8, "EZS_MESS2",    "Ignition Switch Measurement Data 2",                            ezs_mess2_060f_signals.data(),    ezs_mess2_060f_signals.size()    },
        { 0x0501, false, 8, "GS_HS1",       "Test Bench Manual Control 1",                                   gs_hs1_0501_signals.data(),       gs_hs1_0501_signals.size()       },
        { 0x0502, false, 8, "GS_HS2",       "Test Bench Manual Control 2",                                   gs_hs2_0502_signals.data(),       gs_hs2_0502_signals.size()       },
        { 0x0503, false, 8, "GS_HS3",       "Test Bench Manual Control 3",                                   gs_hs3_0503_signals.data(),       gs_hs3_0503_signals.size()       },
        { 0x0504, false, 8, "GS_HS4",       "Test Bench Manual Control 4",                                   gs_hs4_0504_signals.data(),       gs_hs4_0504_signals.size()       },
        { 0x0505, false, 8, "GS_HS5",       "Test Bench Manual Control 5",                                   gs_hs5_0505_signals.data(),       gs_hs5_0505_signals.size()       },
        { 0x0506, false, 8, "GS_HS6",       "Test Bench Manual Control 6",                                   gs_hs6_0506_signals.data(),       gs_hs6_0506_signals.size()       },
    });

    can_profile& get_profile()
    {
        static can_profile profile
        (
            "Mercedes W209 CAN-C"_ct,
            "Mercedes-Benz W209 Drivetrain CAN-C Bus"_ct,
            w209_c_messages.data(),
            w209_c_messages.size(),
            can_profile::big_endian
        );
        return profile;
    }
}
