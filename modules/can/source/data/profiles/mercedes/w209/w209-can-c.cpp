#include "data/profiles/mercedes/w209/w209-can-c.hpp"

#include <array>

/**
 * @file    w209-can-c.cpp
 * @author  dexus1337
 * @brief   Implements the complete Mercedes-Benz W209 Drivetrain CAN-C profile definitions.
 * @version 1.0
 * @date    24.08.2026
 */

namespace adam::modules::can::profiles::mercedes::w209
{
    using namespace adam::string_hashed_ct_literals;

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: UNKNOWN (ID: 0x0000) - Brake Status, ESP & Front Wheel Speeds (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto unknown_0000_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "BRE_KL",         "Brake defective lamp (EBV_KL at 463/461 / NCV2)",                                0,   1 },
        {   1, "BAS_KL",         "BAS defect indicator",                                                           1,   1 },
        {   2, "ESP_INFO_BL",    "ESP Info Lamp flashing light",                                                   2,   1 },
        {   3, "ESP_INFO_DL",    "ESP Info Lamp steady light",                                                     3,   1 },
        {   4, "ESP_KL",         "ESP faulty warning light",                                                       4,   1 },
        {   5, "ABS_KL",         "ABS warning light defective",                                                    5,   1 },
        {   6, "BLS_UNT",        "Brake light suppression (EBV_KL at 163 / T0 / T1N)",                             8,   1 },
        {   7, "BLS_PA",         "BLS parity (even parity)",                                                       9,   1 },
        {   8, "BZ200h",         "message count",                                                                 10,   4 },
        {   9, "BLS",            "Brake Light Switch",                                                            14,   2 },
        {  10, "DRTGVL",         "Rotation direction left front wheel",                                           16,   2 },
        {  11, "DVL",            "Front left wheel speed",                                                        18,  14 },
        {  12, "DRTGVR",         "Direction of rotation right front wheel",                                       32,   2 },
        {  13, "DVR",            "Front right wheel speed",                                                       34,  14 },
        {  14, "DRTGTM",         "Rotation direction left wheel for cruise control",                              48,   2 },
        {  15, "TM_DL",          "Wheel left for cruise control",                                                 50,  14 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_208h (ID: 0x0208) - ESP Brake Intervention & Rear Wheel Speeds (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_208h_0208_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AKT_R_ESP",      "ESP / ART-wish: \"Active downshift\"",                                           0,   1 },
        {   1, "MINMAX_ART",     "Should speed requirement of ART",                                                1,   1 },
        {   2, "GMAX_ESP",       "Target gear, upper limit",                                                       2,   3 },
        {   3, "GMIN_ESP",       "Target gear, lower limit",                                                       5,   3 },
        {   4, "DDYN_UNT",       "Suppression dynamic Vollastrückschaltung",                                       8,   1 },
        {   5, "SZS",            "system state",                                                                   9,   2 },
        {   6, "TM_AUS",         "Cruise control operation from",                                                 11,   1 },
        {   7, "SLV_ESP",        "Switching line shift ESP",                                                      12,   4 },
        {   8, "BRE_AKT_ESP",    "ESP brake intervention active",                                                 16,   1 },
        {   9, "ANFN",           "Insert \"N\": ESP request",                                                     17,   2 },
        {  10, "BRE_AKT_ART",    "ART-active brake intervention",                                                 19,   1 },
        {  11, "MBRE_ESP",       "Adjusted brake torque (BR240 factor of 1.8 larger)",                            20,  12 },
        {  12, "DRTGHR",         "Direction of rotation right rear wheel",                                        32,   2 },
        {  13, "DHR",            "Rear right wheel speed",                                                        34,  14 },
        {  14, "DRTGHL",         "Rotation direction left rear wheel",                                            48,   2 },
        {  15, "DHL",            "Rear left wheel speed",                                                         50,  14 },
        {  16, "SZS_NEU",        "Systemzustand",                                                                  9,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_270h (ID: 0x0270) - Rear Wheel Pulse Rings & Flat Run Warner (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_270h_0270_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "RIZ_HL",         "Pulse ring counter left rear wheel (48 per revolution)",                         0,   8 },
        {   1, "RIZ_HR",         "Pulse ring counter right rear wheel (48 per revolution)",                        8,   8 },
        {   2, "PRW_WARN",       "Alerts Platt Roll Warner",                                                      16,   4 },
        {   3, "PRW_ST",         "Status Platt Roll Warner",                                                      21,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_300h (ID: 0x0300) - ESP & ART Torque Request & Yaw Rate (23 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_300h_0300_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DMPAR_ART",      "Engine torque request parity (even parity)",                                     0,   1 },
        {   1, "DMDYN_ART",      "Engine torque request Dynamic",                                                  1,   1 },
        {   2, "BAS_AKT",        "BAS control active",                                                             2,   1 },
        {   3, "VOLLBRE",        "Emergency braking (ABS controls all 4 wheels)",                                  3,   1 },
        {   4, "ART_E",          "Enable ART",                                                                     4,   1 },
        {   5, "ESP_GIER_AKT",   "ESP yaw moment control is active",                                               5,   1 },
        {   6, "LWS_INI_OK",     "Initialization steering angle sensor O.K.",                                      6,   1 },
        {   7, "LWS_INI_EIN",    "Initialization steering angle sensor possible",                                  7,   1 },
        {   8, "MPAR_ESP",       "Engine torque request parity (even parity)",                                     8,   1 },
        {   9, "MDYN_ESP",       "Engine torque request Dynamic",                                                  9,   1 },
        {  10, "AMR_AKT_ESP",    "Drive torque control active",                                                   10,   1 },
        {  11, "T_Z",            "Cycle time",                                                                    11,   2 },
        {  12, "SFB_PA",         "Driver brakes parity (even parity)",                                            13,   1 },
        {  13, "SFB",            "driver brakes",                                                                 14,   2 },
        {  14, "DMTGL_ART",      "Motormomentenanf. Toggle 40ms + -10",                                           16,   1 },
        {  15, "DMMIN_ART",      "Engine torque requirement Min",                                                 17,   1 },
        {  16, "DMMAX_ART",      "Engine torque requirement Max",                                                 18,   1 },
        {  17, "DM_ART",         "Geford. engine torque",                                                         19,  13 },
        {  18, "MTGL_ESP",       "Motormomentenanf. Toggle 40ms + -10",                                           32,   1 },
        {  19, "MMIN_ESP",       "Engine torque requirement Min",                                                 33,   1 },
        {  20, "MMAX_ESP",       "Engine torque requirement Max",                                                 34,   1 },
        {  21, "M_ESP",          "Geford. engine torque",                                                         35,  13 },
        {  22, "GIER_ROH",       "Raw signal yaw rate without adjustment / filtration (+ = left)",                48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_328h (ID: 0x0328) - Lateral Acceleration & Front Pulse Rings (9 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_328h_0328_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "WMS_PA",         "WMS parity (even parity)",                                                       0,   1 },
        {   1, "WMS",            "Target Wankmomentenänderung",                                                    1,  15 },
        {   2, "AY_S",           "Fahrzeugquerbeschleunig. the center of gravity (+ = left)",                     16,   8 },
        {   3, "ESP_DSPL",       "ESP display messages",                                                          35,   5 },
        {   4, "NOTBRE",         "Emergency braking (brake light flashing)",                                      41,   1 },
        {   5, "KPL_OEF",        "open clutch",                                                                   44,   1 },
        {   6, "BZ328h",         "message count",                                                                 45,   3 },
        {   7, "RIZ_VL",         "Pulse ring counter left front wheel (48 per revolution)",                       48,   8 },
        {   8, "RIZ_VR",         "Pulse ring counter right front wheel (48 per revolution)",                      56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_BS (ID: 0x0785) - Diagnostic Response Brake System ESP (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_bs_0785_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_BS (ID: 0x0722) - System Diagnostic Response Brake System ESP (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_bs_0722_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",          "Systemdiagnose-Response",                                                        0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_APPL2 (ID: 0x0635) - Application Interface Brake System 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_appl2_0635_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL2",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_100h (ID: 0x0100) - Drive Authorization FBS Message to EZS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_100h_0100_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS",            "FBS-Botschaft an EZS (8 Byte)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_101h (ID: 0x0101) - Drive Authorization FBS Redundant Message (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_101h_0101_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MS_FBS",         "FBS-Botschaft an EZS (8 Byte)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_210h (ID: 0x0210) - Pedal Position, Idle & Cruise Control (35 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_210h_0210_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KOMP_NOTAUS",    "Air compressor Emergency Shutdown",                                              0,   1 },
        {   1, "SLV_MS",         "Switching line shift MS",                                                        1,   4 },
        {   2, "KRIECH_AUS",     "off KSG-creep",                                                                  6,   1 },
        {   3, "ANF1",           "MS-wish: \"Approach 1.Gang\"",                                                   7,   1 },
        {   4, "AKT_R_MS",       "MS-wish: \"Active downshift\"",                                                  8,   1 },
        {   5, "ZH_AUS_MS",      "off heater",                                                                     9,   1 },
        {   6, "GMAX_MS",        "Target gear, upper limit",                                                      10,   3 },
        {   7, "GMIN_MS",        "Target gear, lower limit",                                                      13,   3 },
        {   8, "PW",             "pedal",                                                                         16,   8 },
        {   9, "V_DSPL_NEU",     "retrigger minimum display time on the display",                                 24,   1 },
        {  10, "LL_STBL",        "Idle is stable",                                                                25,   1 },
        {  11, "VGL_ST",         "Vorglühstatus",                                                                 26,   1 },
        {  12, "MSS_DEF",        "Engine start / stop system defective",                                          27,   1 },
        {  13, "MSS_KL",         "Engine start / stop system warning",                                            28,   1 },
        {  14, "MSS_AKT",        "Engine start / stop system active",                                             29,   1 },
        {  15, "KOMP_BAUS",      "turn air compressor: Acceleration",                                             30,   1 },
        {  16, "CRASH_MS",       "Crash signal from motor control",                                               31,   1 },
        {  17, "PWG_ERR",        "Error pedal sensor",                                                            32,   1 },
        {  18, "LL",             "Neutral",                                                                       33,   1 },
        {  19, "KUEB_S_A",       "Beg. \"Slip\" lock-up clutch",                                                  34,   1 },
        {  20, "TM_REG",         "cruise control regulates",                                                      35,   1 },
        {  21, "V_MAX_EIN",      "Speed limit switched",                                                          36,   1 },
        {  22, "KD_MS",          "Kick Down (changeover scenario open!)",                                         37,   1 },
        {  23, "NOTL",           "emergency operation",                                                           38,   1 },
        {  24, "V_MAX_SUM",      "Warning buzzer",                                                                39,   1 },
        {  25, "FBS_SE",         "FBS: Start Error",                                                              40,   1 },
        {  26, "V_DSPL_PGB",     "Display \"winter tires limit reached\" on the display",                         41,   1 },
        {  27, "TM_EIN",         "Cruise control switched on",                                                    42,   1 },
        {  28, "V_MAX_REG",      "Speed controls",                                                                43,   1 },
        {  29, "V_DSPL_LIM",     "Display \"limit?\" on display",                                                 44,   1 },
        {  30, "V_DSPL_ERR",     "\"Error\" indicator on the display",                                            45,   1 },
        {  31, "V_DSPL_BL",      "display flashes",                                                               46,   1 },
        {  32, "V_DSPL_EIN",     "Geschw.begrenzer- / cruise control display a",                                  47,   1 },
        {  33, "FMMOTMAX",       "Factor for fill value. d. Max. Mom with remo.. A.druck",                        48,   8 },
        {  34, "V_MAX_TM",       "Set maximum or cruise control speed",                                           56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_212h (ID: 0x0212) - Target Idle Speed & Driver Requested Torque (11 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_212h_0212_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NMOTS",          "Engine idling target speed",                                                     0,  16 },
        {   1, "TM_MS",          "Series Cruise control is variant encodes",                                      17,   1 },
        {   2, "M_ART_E",        "Enable torque requirement ART",                                                 18,   1 },
        {   3, "M_FV",           "Default moment driver",                                                         19,  13 },
        {   4, "SME_E",          "Enable Quick torque adjustment",                                                33,   1 },
        {   5, "M_ESP_E",        "Enable torque request ESP",                                                     34,   1 },
        {   6, "M_FEV",          "Spare default torque driver",                                                   35,  13 },
        {   7, "CALID_CVN_E",    "Transmission CALID / CVN enable",                                               48,   1 },
        {   8, "M_EGS_Q",        "Acknowledgment torque request EGS",                                             49,   1 },
        {   9, "M_EGS_E",        "Enable torque request EGS",                                                     50,   1 },
        {  10, "M_ESPV",         "Default moment ESP",                                                            51,  13 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_268h (ID: 0x0268) - Generator Load & Target Speed Range (11 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_268h_0268_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "IMIN_MS",        "Target speed, lower limit (fcvt)",                                               0,   8 },
        {   1, "IMAX_MS",        "Target speed, upper limit (fcvt)",                                               8,   8 },
        {   2, "KL_61_EIN",      "terminal 61",                                                                   16,   1 },
        {   3, "OEL_INFO_169",   "Oil Info, Reserved M266",                                                       17,   1 },
        {   4, "ASV_KKL_169",    "Shut-off valve cooling circuit M266 ATL",                                       18,   1 },
        {   5, "HZL_ST",         "status heating",                                                                22,   2 },
        {   6, "KID_MS",         "Request force in free \"D\" (fcvt)",                                            24,   1 },
        {   7, "LRS_MODE",       "Mode air control system",                                                       25,   1 },
        {   8, "LAST_GEN",       "Generator capacity utilization (for LIN generators!)",                          26,   6 },
        {   9, "M_KOMP_MAX",     "Max. Air Compressor moment",                                                    32,   8 },
        {  10, "PW_F",           "Pedal value drivers (only 169)",                                                40,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_2F3h (ID: 0x02F3) - Gear Shift Recommendation (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_2f3h_02f3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FSC_IST",        "Gear shift recommendation \"Is\"",                                               0,   8 },
        {   1, "FSC_SOLL",       "Gear shift recommendation \"shall\"",                                           40,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_308h (ID: 0x0308) - Engine Speed, Oil Levels & Diagnostics (27 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_308h_0308_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "KPL",            "clutch is pressed",                                                              0,   1 },
        {   1, "KUEB_O_A",       "Beg. \"Open\" lock-up clutch",                                                   1,   1 },
        {   2, "N_MAX_BG",       "Speed limiting function active",                                                 2,   1 },
        {   3, "SAST",           "Part fuel cut",                                                                  3,   1 },
        {   4, "SASV",           "Fuel cut full",                                                                  4,   1 },
        {   5, "KSF_KL",         "Fuel filter clogged warning light (only CR2 USA)",                               5,   1 },
        {   6, "WKS_KL",         "Water in fuel warning light (only CR2 USA)",                                     6,   1 },
        {   7, "ZASBED",         "Zylinderabschaltbedingungen met",                                                7,   1 },
        {   8, "NMOT",           "Engine speed",                                                                   8,  16 },
        {   9, "ELHP_WARN",      "Alert Eco-steering pump",                                                       25,   1 },
        {  10, "EOH",            "Ethanol operation detected",                                                    26,   1 },
        {  11, "LUFI_KL",        "Air filter clogged pilot (diesel)",                                             27,   1 },
        {  12, "VGL_KL",         "preglow warning",                                                               28,   1 },
        {  13, "OEL_KL",         "Oil level / oil pressure warning light",                                        29,   1 },
        {  14, "DIAG_KL",        "Diagnostic indicator (OBD II)",                                                 30,   1 },
        {  15, "TANK_KL",        "Tank lid open warning",                                                         31,   1 },
        {  16, "UEHITZ",         "Engine oil temperature too high (overheating)",                                 32,   1 },
        {  17, "ZAS",            "cylinder deactivation",                                                         33,   1 },
        {  18, "ADR_KL",         "ADR pilot light (only commercial vehicles)",                                    34,   1 },
        {  19, "ADR_DEF_KL",     "ADR faulty indicator light (only commercial vehicles)",                         35,   1 },
        {  20, "ANL_LFT",        "Cranking",                                                                      36,   1 },
        {  21, "LUEFT_MOT_KL",   "Motor fan defective indicator light",                                           37,   1 },
        {  22, "DBAA",           "Speed limit display is active (0 for CR)",                                      38,   1 },
        {  23, "TEMP_KL",        "Cooling water temperature too high",                                            39,   1 },
        {  24, "T_OEL",          "oil temperature",                                                               40,   8 },
        {  25, "OEL_FS",         "oil level",                                                                     48,   8 },
        {  26, "OEL_QUAL",       "oil quality",                                                                   56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_312h (ID: 0x0312) - Engine Torques (Static, Max, Min, Turbo) (4 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_312h_0312_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "M_STA",          "Engine torque statically",                                                       3,  13 },
        {   1, "M_MAX_ATL",      "Engine torque maximum incl. Dyn. turbocharger share",                           19,  13 },
        {   2, "M_MAX",          "Engine torque maximum",                                                         35,  13 },
        {   3, "M_MIN",          "Motor torque minimum",                                                          51,  13 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: AAD_580h (ID: 0x0580) - Adaptive Acceleration & Driving Style (7 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto aad_580h_0580_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FTK_BMI",        "Code acceleration type (> 100: dynamic)",                                        0,   8 },
        {   1, "FTK_LMI",        "Code type lateral acceleration (> 100: dynamic)",                                8,   8 },
        {   2, "FTK_VMI",        "Code brake type (> 100: dynamic)",                                              16,   8 },
        {   3, "FTK_DPW",        "Max. Diff. Pedal angle value per maneuvers",                                    32,   8 },
        {   4, "AADKB",          "Continuous driver observation",                                                 40,   8 },
        {   5, "AADKBDYN",       "Spontaneous dynamic request",                                                   48,   1 },
        {   6, "AADNT",          "nervousness",                                                                   56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_608h (ID: 0x0608) - Engine Temperature & Fuel Consumption (13 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_608h_0608_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "T_MOT",          "Engine coolant temperature",                                                     0,   8 },
        {   1, "T_LUFT",         "intake",                                                                         8,   8 },
        {   2, "FCOD_KAR",       "Vehicle code body",                                                             16,   3 },
        {   3, "FCOD_BR",        "Vehicle Code series",                                                           19,   5 },
        {   4, "FCOD_MOT6",      "Motor vehicle code with 7-bit, bit 6",                                          24,   1 },
        {   5, "GS_NVH",         "Transmission control No",                                                       25,   1 },
        {   6, "FCOD_MOT",       "Fzgcod.Motor 7Bit, Bit0-5 (Bit6 -> Signal FCOD_MOT6)",                          26,   6 },
        {   7, "V_MAX_FIX",      "fixed speed",                                                                   32,   8 },
        {   8, "VB",             "consumption",                                                                   40,  16 },
        {   9, "ZWP_EIN_MS",     "Switch on auxiliary water pump",                                                56,   1 },
        {  10, "PFW",            "particulate filter warning",                                                    57,   2 },
        {  11, "ZVB_EIN_MS",     "Turn on additional consumer",                                                   59,   1 },
        {  12, "PFKO",           "Particulate filter correction offset FMMOTMAX",                                 60,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FSCM (ID: 0x0779) - Diagnostic Response Fuel System Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fscm_0779_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_FSCM",      "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_MS (ID: 0x07E8) - Diagnostic Response Engine Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ms_07e8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_MS (ID: 0x0720) - System Diagnostic Response Engine Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_ms_0720_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",          "Systemdiagnose-Response",                                                        0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SG_APPL_MS (ID: 0x0529) - Application Interface Engine Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sg_appl_ms_0529_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SG_APPL_MS",     "Steuergerät an externe Applikation",                                             0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL2 (ID: 0x04A9) - Application Interface Engine Control 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl2_04a9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL2",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL4 (ID: 0x0633) - Application Interface Engine Control 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl4_0633_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL4",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL5 (ID: 0x06A8) - Application Interface Engine Control 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl5_06a8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL5",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL6 (ID: 0x0610) - Application Interface Engine Control 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl6_0610_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL6",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL7 (ID: 0x0618) - Application Interface Engine Control 7 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl7_0618_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL7",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EDC_MESS1 (ID: 0x0670) - Diesel Control Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto edc_mess1_0670_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EDC_MESS2 (ID: 0x0671) - Diesel Control Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto edc_mess2_0671_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_218h (ID: 0x0218) - Transmission Gears & Converter Clutch Status (32 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_218h_0218_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MTGL_EGS",       "Motormomentenanf. Toggle 40ms + -10",                                            0,   1 },
        {   1, "MMIN_EGS",       "Engine torque requirement Min",                                                  1,   1 },
        {   2, "MMAX_EGS",       "Engine torque requirement Max",                                                  2,   1 },
        {   3, "M_EGS",          "Geford. engine torque",                                                          3,  13 },
        {   4, "GZC",            "Target gear",                                                                   16,   4 },
        {   5, "GIC",            "Actual gear",                                                                   20,   4 },
        {   6, "K_S_B",          "Best. (Wandlerüberbrück.-) clutch \"slipping\"",                                24,   1 },
        {   7, "K_O_B",          "Best. (Wandlerüberbrück.-) Clutch \"open\"",                                    25,   1 },
        {   8, "K_G_B",          "Best. (Wandlerüberbrück.-) clutch \"closed\"",                                  26,   1 },
        {   9, "G_G",            "off-road gear",                                                                 27,   1 },
        {  10, "GSP_OK",         "Basic shift program O.K.",                                                      28,   1 },
        {  11, "FW_HOCH",        "Driving resistance is high",                                                    29,   1 },
        {  12, "SCHALT",         "circuit",                                                                       30,   1 },
        {  13, "HSM",            "Manual shift mode",                                                             31,   1 },
        {  14, "GET_OK",         "transmission ok",                                                               32,   1 },
        {  15, "KS",             "start bang",                                                                    33,   1 },
        {  16, "ALF",            "start enabling",                                                                34,   1 },
        {  17, "GS_NOTL",        "GS in emergency operation",                                                     35,   1 },
        {  18, "UEHITZ_GET",     "Overtemperature gear",                                                          36,   1 },
        {  19, "KD",             "Kick down",                                                                     37,   1 },
        {  20, "FPC_AAD",        "Driving program for AAD",                                                       38,   2 },
        {  21, "MPAR_EGS",       "Engine torque request parity (even parity)",                                    40,   1 },
        {  22, "DYN1_EGS",       "Engagement mode / drive torque control",                                        41,   1 },
        {  23, "DYN0_AMR_EGS",   "Engagement mode / drive torque control",                                        42,   1 },
        {  24, "K_LSTFR",        "Converter lockup clutch free of load",                                          45,   1 },
        {  25, "MOT_NAUS_CNF",   "MOT_NAUS-Confirmbit",                                                           46,   1 },
        {  26, "MOT_NAUS",       "Motor Emergency Shutdown",                                                      47,   1 },
        {  27, "MKRIECH",        "Creep (FFh at EGS, CVT) or CALID / CVN",                                        48,   8 },
        {  28, "FEHLPRF_ST",     "Status error checking",                                                         56,   2 },
        {  29, "CALID_CVN_AKT",  "CALID / CVN-transmission active",                                               58,   1 },
        {  30, "FEHLER",         "Error number or counter for CALID / CVN transmission",                          59,   5 },
        {  31, "I_IST_GET",      "Ist-Übersetzung (nur bei FCVT, sonst Ist-/Zielgang)",                           16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_338h (ID: 0x0338) - Transmission Output & Turbine Speeds (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_338h_0338_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "NAB",            "Transmission output speed (only 463/461, otherwise FFFFh)",                      0,  16 },
        {   1, "NTURBINE",       "Turbine speed (EGS52-NAG, VGS NAG2)",                                           48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_418h (ID: 0x0418) - Driving Position, Oil Temp & Wheel Torque (17 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_418h_0418_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FSC",            "driving position",                                                               0,   8 },
        {   1, "FPC",            "driving program",                                                                8,   8 },
        {   2, "T_GET",          "Transmission oil temperature",                                                  16,   8 },
        {   3, "ALLRAD",         "all wheel drive",                                                               24,   1 },
        {   4, "FRONT",          "Front-wheel drive [1], rear-wheel drive [0]",                                   25,   1 },
        {   5, "SCHALT",         "circuit",                                                                       26,   1 },
        {   6, "CVT",            "Continuously variable transmission [1], step transmission [0]",                 27,   1 },
        {   7, "MECH",           "Gear-mechanism variant",                                                        28,   2 },
        {   8, "ESV_BRE",        "Brake invest in power-",                                                        30,   1 },
        {   9, "KD",             "Kick down",                                                                     31,   1 },
        {  10, "GZC",            "Target gear",                                                                   32,   4 },
        {  11, "GIC",            "Actual gear",                                                                   36,   4 },
        {  12, "M_VERL",         "Loss torque (FFh at KSG)",                                                      40,   8 },
        {  13, "FMRADPAR",       "Factor wheel torque parity (even parity)",                                      48,   1 },
        {  14, "FMRADTGL",       "Factor wheel torque Toggle 40ms + -10",                                         49,   1 },
        {  15, "WHST",           "Gear selector lever position (NAG, KSG, CVT)",                                  50,   3 },
        {  16, "FMRAD",          "Factor wheel torque (7FFh KSG)",                                                53,  11 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_GS (ID: 0x07E9) - Diagnostic Response Transmission Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_gs_07e9_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_GS (ID: 0x0723) - System Diagnostic Response Transmission Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_gs_0723_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS_GS",       "Systemdiagnose-Response",                                                        0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_APPL1 (ID: 0x051C) - Application Interface Transmission Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_appl1_051c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSA (ID: 0x050A) - Test Bench Manual Control A (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsa_050a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSA",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSB (ID: 0x050B) - Test Bench Manual Control B (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsb_050b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSB",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSC (ID: 0x050C) - Test Bench Manual Control C (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsc_050c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSC",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSD (ID: 0x050D) - Test Bench Manual Control D (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hsd_050d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSD",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HSE (ID: 0x050E) - Test Bench Manual Control E (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hse_050e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HSE",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_230h (ID: 0x0230) - Electronic Selector Lever Position & Program (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_230h_0230_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "W_S",            "driving program",                                                                0,   1 },
        {   1, "FPT",            "Driving program button is pressed",                                              1,   1 },
        {   2, "KD",             "Kick down",                                                                      2,   1 },
        {   3, "SPERR",          "Blocking coil energized",                                                        3,   1 },
        {   4, "WHC",            "Gear selector lever position (NOS only)",                                        4,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EWM (ID: 0x0789) - Diagnostic Response Electronic Selector Lever (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ewm_0789_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SD_RS_EWM (ID: 0x0724) - System Diagnostic Response Selector Lever (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sd_rs_ewm_0724_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SD_RS",          "Systemdiagnose-Response",                                                        0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_MESS1 (ID: 0x06F0) - Selector Lever Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_mess1_06f0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_MESS2 (ID: 0x06F1) - Selector Lever Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_mess2_06f1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_110h (ID: 0x0110) - Drive Authorization FBS Message to Engine (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_110h_0110_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "IFZ_ST",         "FBS-Botschaft an MS (8 Byte)",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_111h (ID: 0x0111) - Drive Authorization FBS Message to Engine 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_111h_0111_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS_MS",         "FBS-Botschaft an MS (8 Byte)",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_112h (ID: 0x0112) - Drive Authorization FBS Message to Transmission (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_112h_0112_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS_GS",         "FBS-Botschaft an GS (8 Byte)",                                                   0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FBS_114h (ID: 0x0114) - Drive Authorization FBS Message to Selector Lever (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fbs_114h_0114_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "FBS_EWM",        "FBS-Botschaft an EWM (8 Byte)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_240h (ID: 0x0240) - Cruise Control Lever, Terminals & Lights (34 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_240h_0240_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "WH_UP",          "Cruise control lever implausible",                                               2,   1 },
        {   1, "VMAX_AKT",       "Operation variable speed limit",                                                 3,   1 },
        {   2, "S_MINUS_B",      "Cruise control lever: \"Sit and delay Stufe0\"",                                 4,   1 },
        {   3, "S_PLUS_B",       "Cruise control lever: \"Sit and accelerating Stufe0\"",                          5,   1 },
        {   4, "WA",             "Cruise control lever: \"resume\"",                                               6,   1 },
        {   5, "AUS",            "Cruise control lever \"off\"",                                                   7,   1 },
        {   6, "KG_KL_AKT",      "Keyless Go terminal control active",                                             8,   1 },
        {   7, "KG_ALB_OK",      "Keyles Go annealing conditions met",                                             9,   1 },
        {   8, "LL_RLC",         "LHD / RHD",                                                                     10,   2 },
        {   9, "RG_SCHALT",      "engaged reverse gear (manual transmission only)",                               12,   1 },
        {  10, "BS_SL",          "Brake switch for Shift Lock",                                                   13,   1 },
        {  11, "KL_15",          "terminal 15",                                                                   14,   1 },
        {  12, "KL_50",          "terminal 50",                                                                   15,   1 },
        {  13, "WH_PA",          "Cruise control lever parity (even parity)",                                     19,   1 },
        {  14, "BZ240h",         "message count",                                                                 20,   4 },
        {  15, "ASG_SPORT_BET",  "ASG Sport mode on / off operated (ST2_LED_DL when ABC available)",              27,   1 },
        {  16, "CRASH_CNF",      "CRASH Confirmbit",                                                              30,   1 },
        {  17, "CRASH",          "Crash signal from airbag SG",                                                   31,   1 },
        {  18, "BN_NTLF",        "Wiring emergency: Prio1- and Prio2-consumers, Second battery supports",         32,   1 },
        {  19, "ESP_BET",        "ESP operated on / off",                                                         33,   2 },
        {  20, "HAS_KL",         "Hand brake applied (control light)",                                            35,   1 },
        {  21, "KL_31B",         "Wiper outside parking position",                                                36,   1 },
        {  22, "BLI_RE",         "Directional blinking right",                                                    38,   1 },
        {  23, "BLI_LI",         "Directional blinking left",                                                     39,   1 },
        {  24, "ST2_BET",        "LF / ABC 2-stage switch actuated",                                              40,   2 },
        {  25, "ST3_BET",        "LF / ABC 3-position switch is actuated",                                        42,   2 },
        {  26, "ART_ABW_BET",    "ART-distance warning actuated on / off",                                        44,   2 },
        {  27, "ABL_EIN",        "switch on low beam",                                                            46,   1 },
        {  28, "KL54_RM",        "Terminal 54 Hardware enabled",                                                  47,   1 },
        {  29, "ART_ABSTAND",    "distance factor",                                                               48,   8 },
        {  30, "ART_VH",         "ART available",                                                                 56,   1 },
        {  31, "GBL_AUS",        "E-extractor: basic ventilation from",                                           57,   1 },
        {  32, "FZGVERSN",       "Series addicts vehicle version (only 220/215/230)",                             59,   3 },
        {  33, "LDC",            "country code",                                                                  62,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ZGW_248h (ID: 0x0248) - Gateway Headlight & Towing Status (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto zgw_248h_0248_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "DIAG_X4_B",      "Start Xenon4 diagnostic procedure passenger side",                               0,   1 },
        {   1, "DIAG_X4_F",      "Start Xenon4 diagnostic procedure driver's side",                                1,   1 },
        {   2, "ABL_EIN",        "switch on low beam",                                                             3,   1 },
        {   3, "AFL_ABL_EIN",    "AFL request: Switch low beam",                                                  12,   1 },
        {   4, "ZWP_LFT",        "Auxiliary water pump runs",                                                     13,   1 },
        {   5, "ANH_ERK2",       "Towing recognized",                                                             14,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ZGW_24Ch (ID: 0x024C) - Gateway Low Beam Failure Status (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto zgw_24ch_024c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ABL_DEF_BF_R",   "Low beam defective passenger / right (depending on BR)",                        38,   1 },
        {   1, "ABL_DEF_F_L",    "Low beam faulty driver / left (depending BR)",                                  39,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_40Eh (ID: 0x040E) - Heating Power Demand (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_40eh_040e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HZL_ANF",        "Anforderung Heizleistung",                                                       0,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KLA_410h (ID: 0x0410) - Climate Control Compressor & Fan Demand (11 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kla_410h_0410_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ZH_EIN_OK",      "Heater switch allows",                                                           0,   1 },
        {   1, "LL_DZA",         "Idle speed increase for the cooling capacity increase",                          1,   1 },
        {   2, "KOMP_EIN",       "Air compressor switched",                                                        7,   1 },
        {   3, "P_KAELTE8",      "Refrigerant pressure",                                                           8,   8 },
        {   4, "M_KOMP",         "Torque absorption chiller",                                                     16,   8 },
        {   5, "NLFTS",          "Motor fan set speed",                                                           24,   8 },
        {   6, "T_AUSSEN_WM",    "Outside air temperature for heat management",                                   40,   8 },
        {   7, "SENDE_NEU",      "Signal version Compressor moment",                                               3,   1 },
        {   8, "M_KOMPPAR",      "Air compressor moment parity (even parity)",                                     5,   1 },
        {   9, "M_KOMPTGL",      "Air compressor moment Toggle",                                                   6,   1 },
        {  10, "M_KOMP_NEU",     "Air compressor instant anew",                                                   16,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: APPL_SG_MS (ID: 0x074C) - Application Interface to Engine Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto appl_sg_ms_074c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL_SG_MS",     "Externe Applikation zu Steuergerät",                                             0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ART (ID: 0x078E) - Diagnostic Request Adaptive Cruise Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_art_078e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_BS (ID: 0x0784) - Diagnostic Request Brake System ESP (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_bs_0784_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_CAS (ID: 0x077A) - Diagnostic Request Collision Avoidance System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_cas_077a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_CAS",       "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_DTR (ID: 0x0702) - Diagnostic Request Distronic System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_dtr_0702_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_DTR",       "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EHB (ID: 0x079A) - Diagnostic Request Electrohydraulic Brake (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ehb_079a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EHB2 (ID: 0x07B0) - Diagnostic Request Electrohydraulic Brake 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ehb2_07b0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_EWM (ID: 0x0788) - Diagnostic Request Electronic Selector Lever (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ewm_0788_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FS (ID: 0x078C) - Diagnostic Request Active Body Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fs_078c_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_FSCM (ID: 0x0778) - Diagnostic Request Fuel System Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_fscm_0778_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ_FSCM",      "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_GS (ID: 0x07E1) - Diagnostic Request Transmission Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_gs_07e1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_ISM (ID: 0x06EA) - Diagnostic Request Intelligent Servo Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ism_06ea_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_KOMBI_C (ID: 0x0796) - Diagnostic Request Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_kombi_c_0796_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_LWR (ID: 0x0794) - Diagnostic Request Headlight Range Adjustment (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_lwr_0794_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_MS (ID: 0x07E0) - Diagnostic Request Engine Control Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_ms_07e0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_RGS_L (ID: 0x07B2) - Diagnostic Request Seatbelt Tensioner Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_rgs_l_07b2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_RGS_R (ID: 0x07B4) - Diagnostic Request Seatbelt Tensioner Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_rgs_r_07b4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RQ_UP28 (ID: 0x07A2) - Diagnostic Request Microprocessor Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rq_up28_07a2_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_APPL2 (ID: 0x06E4) - Application Interface Transmission Control 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_appl2_06e4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL2",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL1 (ID: 0x074A) - Application Interface Engine Control 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl1_074a_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL3 (ID: 0x06E0) - Application Interface Engine Control 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl3_06e0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL3",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_MESS1 (ID: 0x060E) - Ignition Switch Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_mess1_060e_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: DG_RQ_OBD (ID: 0x07DF) - Global OBD-II Diagnostic Request (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto dg_rq_obd_07df_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RQ",           "KWP2000 Diagnose-Request",                                                       0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VIN (ID: 0x06FA) - Vehicle Identification Number (2 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vin_06fa_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VIN_MSG",        "VIN Control unit",                                                               6,   2 },
        {   1, "VIN_DATA",       "VIN data",                                                                       8,  56 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_408h (ID: 0x0408) - Fuel Level, Door Status, Speedometer & Odometer (19 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_408h_0408_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "TANK_FS",        "tank level",                                                                     0,   8 },
        {   1, "TF_AUF",         "Driver's door",                                                                  8,   1 },
        {   2, "V_DSPL_AUS",     "Geschw.begrenzer- / cruise control display not possible",                        9,   1 },
        {   3, "TACHO_SYM",      "Tachoeichung",                                                                  10,   1 },
        {   4, "V_MPH",          "mph instead km / h (variable Geschwindigkeitsbegr.)",                           11,   1 },
        {   5, "KLA_VH",         "Air conditioning available",                                                    12,   1 },
        {   6, "VGL_KL_DEF",     "Preglow pilot defective",                                                       13,   1 },
        {   7, "TFSM",           "Tank level minimum",                                                            14,   1 },
        {   8, "KL_61E",         "Terminal 61 decoupled",                                                         15,   1 },
        {   9, "T_AUSSEN",       "Outside air temperature raw value",                                             16,   8 },
        {  10, "KL_58D",         "Terminal 58 dimmed",                                                            24,   8 },
        {  11, "MAZ",            "Motorabstellzeit (is sent from terminal 15)",                                   32,   8 },
        {  12, "KM16",           "mileage",                                                                       40,  16 },
        {  13, "WRC3",           "Winter tire speed Bit 3",                                                       56,   1 },
        {  14, "V_DSPL_AKT",     "Geschw.begrenzer- / cruise control display active",                             57,   1 },
        {  15, "SGT_VH",         "Segment tachometer available",                                                  58,   1 },
        {  16, "ZH_FREIG",       "enable auxiliary heaters",                                                      59,   1 },
        {  17, "RT_EIN",         "Roll test mode ESP switch",                                                     60,   1 },
        {  18, "WRC",            "Winter tire speed with 4 bits",                                                 61,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_412h (ID: 0x0412) - Cluster Speed, Warnings & Distance Setting (11 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_412h_0412_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "AKU_WARN_AUS",   "Acoustic warning",                                                               0,   1 },
        {   1, "OPT_WARN_AUS",   "Optical warning",                                                                1,   1 },
        {   2, "ECO_WARN_ST",    "Status ECO Warning",                                                             4,   1 },
        {   3, "ABST_S",         "distance unit",                                                                  8,   1 },
        {   4, "IST_ABST",       "set distance",                                                                   9,   3 },
        {   5, "V_ANZ",          "Displaying speed",                                                              12,  12 },
        {   6, "DRTGANZ",        "Wheel rotation to V_ANZ",                                                       24,   2 },
        {   7, "DANZ",           "Wheel speed calculated from V_ANZ",                                             26,  14 },
        {   8, "ECO_AKT",        "ECO activation in combination menu",                                            44,   1 },
        {   9, "PRW_ANF",        "Requirement Platt Roll Warner",                                                 46,   2 },
        {  10, "MAZ_NEU",        "Motorabstellzeit",                                                              52,  12 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_KOMBI_C (ID: 0x0797) - Diagnostic Response Instrument Cluster (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_kombi_c_0797_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_MESS1 (ID: 0x0680) - Instrument Cluster Measurement Data 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_mess1_0680_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS1",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_MESS2 (ID: 0x0681) - Instrument Cluster Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_mess2_0681_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: LRW_236h (ID: 0x0236) - Steering Wheel Angle & Angular Speed (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto lrw_236h_0236_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LRW",            "steering wheel angle",                                                           2,  14 },
        {   1, "VLRW",           "Steering wheel angle speed",                                                    18,  14 },
        {   2, "BZ236h",         "message count",                                                                 32,   4 },
        {   3, "LRWS_ID",        "Identification steering wheel angle sensor",                                    36,   2 },
        {   4, "LRWS_ST",        "Status steering wheel angle sensor",                                            38,   2 },
        {   5, "CRC236h",        "CRC checksum byte 1-7 to SAE J1850",                                            56,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MRM_238h (ID: 0x0238) - Cruise Control Lever & Steering Angle (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto mrm_238h_0238_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "WH_UP",          "Cruise control lever implausible",                                               2,   1 },
        {   1, "VMAX_AKT",       "Operation variable speed limit",                                                 3,   1 },
        {   2, "S_MINUS_B",      "Cruise control lever: \"Sit and delay Stufe0\"",                                 4,   1 },
        {   3, "S_PLUS_B",       "Cruise control lever: \"Sit and accelerating Stufe0\"",                          5,   1 },
        {   4, "WA",             "Cruise control lever: \"resume\"",                                               6,   1 },
        {   5, "AUS",            "Cruise control lever \"off\"",                                                   7,   1 },
        {   6, "BLI_RE",         "Directional blinking right",                                                     8,   1 },
        {   7, "BLI_LI",         "Directional blinking left",                                                      9,   1 },
        {   8, "WH_PA",          "Cruise control lever parity (even parity)",                                     11,   1 },
        {   9, "BZ238h",         "message count",                                                                 12,   4 },
        {  10, "LW_PA",          "Steering angle parity (even parity)",                                           16,   1 },
        {  11, "LW_OV",          "Steering angle sensor: Overflow",                                               17,   1 },
        {  12, "LW_CF",          "Steering angle sensor: Code Error",                                             18,   1 },
        {  13, "LW_INI",         "Steering angle sensor: not initialized",                                        19,   1 },
        {  14, "LW_VZ",          "Steering angle sign",                                                           20,   1 },
        {  15, "LW",             "steering angle",                                                                21,  11 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ARCADE_A2 (ID: 0x0035) - Crash Sensor Confirmation & Frontal Event (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto arcade_a2_0035_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "CONF_CRASH",     "Confirm bit for all crash events, toggling",                                     0,   1 },
        {   1, "CRASH_F",        "Frontal Event 2",                                                                2,   1 },
        {   2, "CRASH_C",        "Frontal Event 5",                                                                5,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_ANZ (ID: 0x033D) - Engine Start/Stop Display Status (3 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_anz_033d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ASS_WARN",       "Number of ASA alert",                                                           16,   4 },
        {   1, "ASS_DSPL",       "Number of ASA status message",                                                  20,   4 },
        {   2, "ASS_LTEST_AUS",  "Suppression of lamp test during stop phase",                                    24,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_102h (ID: 0x0102) - Drive Authorization FBS Message from GS (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_102h_0102_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GS_FBS",         "FBS-Botschaft an EZS (8 Byte)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EWM_104h (ID: 0x0104) - Drive Authorization FBS Message from EWM (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ewm_104h_0104_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "EWM_FBS",        "FBS-Botschaft an EZS (8 Byte)",                                                  0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: SBW_232h (ID: 0x0232) - Shift-by-Wire Control Buttons & Lever Status (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto sbw_232h_0232_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SID_SBW",        "sender ID",                                                                      0,   2 },
        {   1, "LRT_PM3",        "Steering wheel buttons \"+\" - operated \"\"",                                   5,   3 },
        {   2, "SBWB_ID",        "Shift-by-wire control element identification",                                   8,   2 },
        {   3, "SBWB_ST_P",      "Shift-by-wire control element P button",                                        10,   2 },
        {   4, "SBWB_ST_RND",    "Shift-by-wire control element status RND",                                      12,   4 },
        {   5, "BZ232h",         "message count",                                                                 16,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ART_250h (ID: 0x0250) - Distronic Torque Request & Target Gears (16 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto art_250h_0250_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "SLV_ART",        "Switching line shift ART",                                                       0,   4 },
        {   1, "ART_OK",         "ART fine",                                                                       4,   1 },
        {   2, "ART_BRE",        "ART brakes",                                                                     5,   1 },
        {   3, "BL_UNT",         "Brake light suppression",                                                        6,   1 },
        {   4, "DYN_UNT",        "Suppression dynamic Vollastrückschaltung",                                       7,   1 },
        {   5, "MPAR_ART",       "Engine torque request parity (even parity)",                                     8,   1 },
        {   6, "MDYN_ART",       "Engine torque request Dynamic",                                                  9,   1 },
        {   7, "CAS_REG",        "City assistant controls",                                                       10,   1 },
        {   8, "LIM_REG",        "limiter regulates",                                                             17,   1 },
        {   9, "ART_REG",        "ART regulates",                                                                 18,   1 },
        {  10, "M_ART",          "Geford. engine torque",                                                         19,  13 },
        {  11, "BZ250h",         "message count",                                                                 32,   4 },
        {  12, "MBRE_ART",       "Braking torque (0000h: Passive value)",                                         36,  12 },
        {  13, "AKT_R_ART",      "ART-wish: \"Active downshift\"",                                                48,   1 },
        {  14, "GMAX_ART",       "Target gear, upper limit",                                                      50,   3 },
        {  15, "GMIN_ART",       "Target gear, lower limit",                                                      53,   3 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: ART_258h (ID: 0x0258) - Distronic Target Distance, Speed & Object Detection (26 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto art_258h_0258_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "ART_DSPL_EIN",   "switch display on ART display",                                                  0,   1 },
        {   1, "S_OBJ",          "Detecting stationary object",                                                    1,   1 },
        {   2, "ART_WT",         "ART warning",                                                                    2,   1 },
        {   3, "ART_INFO",       "ART Info Lamp",                                                                  3,   1 },
        {   4, "ART_ERR",        "ART error code",                                                                 4,   4 },
        {   5, "V_ART",          "set ART-speed",                                                                  8,   8 },
        {   6, "ABST_R_OBJ",     "Distance relevant object",                                                      16,   8 },
        {   7, "SOLL_ABST",      "Driver's desired distance",                                                     24,   8 },
        {   8, "ART_DSPL_PGB",   "Display \"winter tires limit reached\" on the display",                         32,   1 },
        {   9, "ART_VFBR",       "Display \"DTR OFF [0]\" on the display",                                        33,   1 },
        {  10, "ART_DSPL_LIM",   "Display \"---\" on the display",                                                34,   1 },
        {  11, "ART_EIN",        "Adaptive cruise control switched on",                                           35,   1 },
        {  12, "OBJ_ERK",        "Relevant object detected",                                                      36,   1 },
        {  13, "ART_SEG_EIN",    "Switch ART-segment display",                                                    37,   1 },
        {  14, "ART_DSPL_BL",    "flashing speed indicator",                                                      38,   1 },
        {  15, "TM_EIN_ART",     "ART-cruise control activated",                                                  39,   1 },
        {  16, "V_ZIEL",         "Speed unrecognized target vehicle",                                             40,   8 },
        {  17, "ART_DSPL_NEU",   "retrigger minimum display time on the display",                                 48,   1 },
        {  18, "ART_UEBERSP",    "ART is dubbed by the driver",                                                   49,   1 },
        {  19, "ART_REAKT",      "Display of system availability to system error",                                50,   1 },
        {  20, "ART_ABW_AKT",    "ART-distance warning is switched on",                                           51,   1 },
        {  21, "OBJ_AGB",        "Property Offer distance Assistant",                                             52,   1 },
        {  22, "AAS_LED_BL",     "LED flash distance Assistant",                                                  53,   1 },
        {  23, "ASSIST_FKT_AKT", "active function",                                                               54,   2 },
        {  24, "CAS_ERR_ANZ_V2", "CAS ad request",                                                                56,   3 },
        {  25, "ASSIST_ANZ_V2",  "Assistance system display request",                                             59,   5 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: FS_340h (ID: 0x0340) - ABC Suspension Pump Load Torque (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto fs_340h_0340_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "M_LAST",         "Lastmoment ABC-Pumpe",                                                          60,   4 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: PSM_3B4h (ID: 0x03B4) - Parametric Module Engine Speed & Torque Limits (12 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto psm_3b4h_03b4_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PSM_ADR_PAR",    "Working speed control - parity",                                                 0,   1 },
        {   1, "PSM_ADR_TGL",    "Working speed control - toggle",                                                 1,   1 },
        {   2, "PSM_ADR_AKT",    "Working speed control active",                                                   2,   1 },
        {   3, "PSM_N_SOLL",     "Target engine speed ADR",                                                        8,  16 },
        {   4, "PSM_MOM_PAR",    "Torque limit - parity",                                                         24,   1 },
        {   5, "PSM_MOM_TGL",    "Torque limit - toggle",                                                         25,   1 },
        {   6, "PSM_MOM_AKT",    "Torque limit active",                                                           26,   1 },
        {   7, "PSM_MOM_SOLL",   "Maximum engine torque",                                                         27,  13 },
        {   8, "PSM_DZ_PAR",     "Speed limit - parity",                                                          40,   1 },
        {   9, "PSM_DZ_TGL",     "Speed limit - toggle",                                                          41,   1 },
        {  10, "PSM_DZ_AKT",     "Speed limit active",                                                            42,   1 },
        {  11, "PSM_DZ_MAX",     "maximum speed",                                                                 48,  16 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: PSM_3B8h (ID: 0x03B8) - Parametric Module Speed Limit & Remote Start (9 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto psm_3b8h_03b8_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "PSM_V_PAR",      "Speed limit - parity",                                                           0,   1 },
        {   1, "PSM_V_TGL",      "Speed limit - toggle",                                                           1,   1 },
        {   2, "PSM_V_AKT",      "Speed limit active",                                                             2,   1 },
        {   3, "PSM_V_SOLL",     "speed limit",                                                                    8,   8 },
        {   4, "PSM_DZ_PAR",     "Speed limit - parity",                                                          16,   1 },
        {   5, "PSM_DZ_TGL",     "Speed limit - toggle",                                                          17,   1 },
        {   6, "PSM_FERN_START", "Remote engine start active",                                                    18,   1 },
        {   7, "PSM_FERN_STOP",  "Remote engine stop active",                                                     19,   1 },
        {   8, "PSM_FPM_SP",     "Lock accelerator pedal module",                                                 20,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: KOMBI_414h (ID: 0x0414) - Filtered Outside Temperature (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto kombi_414h_0414_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "T_AUSSEN_K",     "gefilterte Außentemperatur Kombi",                                               0,   8 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: VG_428h (ID: 0x0428) - Transfer Case Gear & Neutral Request (5 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto vg_428h_0428_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "VG_ERR",         "Error VG (ECU failure detected)",                                                2,   1 },
        {   1, "VG_GANG",        "Current speed transfer case",                                                    5,   3 },
        {   2, "ANFNPAR_VG",     "VG - request \"N\" Loading parity (even parity)",                               12,   1 },
        {   3, "ANFNTGL_VG",     "VG - beg. \"N\" Insert Toggle 20ms (1 / embassy)",                              13,   1 },
        {   4, "ANFN_VG",        "Insert request \"N\" - VG",                                                     14,   2 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: LWR_530h (ID: 0x0530) - Headlight Range & Cornering Light Messages (9 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto lwr_530h_0530_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "LWR_M7",         "Display message 7: \"cornering currently unavailable\"",                         1,   1 },
        {   1, "LWR_M6",         "Display message 6: \"cornering right\"",                                         2,   1 },
        {   2, "LWR_M5",         "Display message 5: \"cornering left\"",                                          3,   1 },
        {   3, "LWR_M4",         "Display message 4: \"Headlights not available\" (white / 5x flashing at 1Hz)",   4,   1 },
        {   4, "LWR_M3",         "Display message 3: \"Headlights not available\" (white).",                       5,   1 },
        {   5, "LWR_M2",         "Display Message 2: \"Headlights, spare light activated!\" (White)",              6,   1 },
        {   6, "LWR_M1",         "Display Message 1: \"Headlights defect Drive to workshop\"",                     7,   1 },
        {   7, "SUB_ABL_L",      "Substitution low beam left",                                                     8,   1 },
        {   8, "SUB_ABL_R",      "Substitution low beam right",                                                    9,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: CONFIG_6FFh (ID: 0x06FF) - Drivetrain Equipment & Differential Lock Coding (6 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto config_6ffh_06ff_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "GBL_AUS",        "E-extractor: basic ventilation from",                                            0,   1 },
        {   1, "KLA_VH",         "Air conditioning available",                                                    50,   1 },
        {   2, "DSH_VH",         "Rear differential lock available",                                              60,   1 },
        {   3, "DSM_VH",         "Differential lock center available",                                            61,   1 },
        {   4, "DSV_VH",         "Front differential lock available",                                             62,   1 },
        {   5, "VG_VH",          "Transfer case control available",                                               63,   1 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ART (ID: 0x078F) - Diagnostic Response Distronic (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_art_078f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_CAS (ID: 0x077B) - Diagnostic Response Collision Avoidance System (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_cas_077b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_CAS",       "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_DTR (ID: 0x04A0) - Diagnostic Response Distronic Radar (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_dtr_04a0_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS_DTR",       "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EHB (ID: 0x079B) - Diagnostic Response Electrohydraulic Brake (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ehb_079b_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_EHB2 (ID: 0x07B1) - Diagnostic Response Electrohydraulic Brake 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ehb2_07b1_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_FS (ID: 0x078D) - Diagnostic Response Active Body Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_fs_078d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_ISM (ID: 0x049D) - Diagnostic Response Intelligent Servo Module (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_ism_049d_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_LWR (ID: 0x0795) - Diagnostic Response Headlight Range Adjustment (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_lwr_0795_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_RGS_L (ID: 0x07B3) - Diagnostic Response Seatbelt Tensioner Left (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_rgs_l_07b3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_RGS_R (ID: 0x07B5) - Diagnostic Response Seatbelt Tensioner Right (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_rgs_r_07b5_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: D_RS_UP28 (ID: 0x07A3) - Diagnostic Response Microprocessor Control (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto d_rs_up28_07a3_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "D_RS",           "KWP2000 Diagnose-Response",                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: BS_APPL1 (ID: 0x0634) - Application Interface Brake System 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto bs_appl1_0634_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL1 (ID: 0x0630) - Application Interface Engine Control (Alternate) (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl1_0630_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL1",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: MS_APPL3 (ID: 0x0632) - Application Interface Engine Control 3 (Alternate) (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ms_appl3_0632_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "APPL3",          "Applikation",                                                                    0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: EZS_MESS2 (ID: 0x060F) - Ignition Switch Measurement Data 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto ezs_mess2_060f_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "MESS2",          "Messwerte",                                                                      0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS1 (ID: 0x0501) - Test Bench Manual Control 1 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs1_0501_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS1",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS2 (ID: 0x0502) - Test Bench Manual Control 2 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs2_0502_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS2",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS3 (ID: 0x0503) - Test Bench Manual Control 3 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs3_0503_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS3",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS4 (ID: 0x0504) - Test Bench Manual Control 4 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs4_0504_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS4",            "Handsteuerung am Prüfstand",                                                     0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS5 (ID: 0x0505) - Test Bench Manual Control 5 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs5_0505_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS5",            "",                                                                               0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // ECU: GS_HS6 (ID: 0x0506) - Test Bench Manual Control 6 (1 Signal(s))
    // -------------------------------------------------------------------------------------------------------------- //
    const auto gs_hs6_0506_signals = std::to_array<const can_signal_spec>
    ({
        {   0, "HS6",            "",                                                                               0,  64 },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // W209 CAN-C Message Definitions Table
    // -------------------------------------------------------------------------------------------------------------- //
    const auto w209_c_messages = std::to_array<const can_message_spec>
    ({
        { 0x0000, false, 8, "UNKNOWN",      "Brake Status, ESP & Front Wheel Speeds",              unknown_0000_signals.data(),      unknown_0000_signals.size()      },
        { 0x0208, false, 8, "BS_208h",      "ESP Brake Intervention & Rear Wheel Speeds",          bs_208h_0208_signals.data(),      bs_208h_0208_signals.size()      },
        { 0x0270, false, 8, "BS_270h",      "Rear Wheel Pulse Rings & Flat Run Warner",            bs_270h_0270_signals.data(),      bs_270h_0270_signals.size()      },
        { 0x0300, false, 8, "BS_300h",      "ESP & ART Torque Request & Yaw Rate",                 bs_300h_0300_signals.data(),      bs_300h_0300_signals.size()      },
        { 0x0328, false, 8, "BS_328h",      "Lateral Acceleration & Front Pulse Rings",            bs_328h_0328_signals.data(),      bs_328h_0328_signals.size()      },
        { 0x0785, false, 8, "D_RS_BS",      "Diagnostic Response Brake System ESP",                d_rs_bs_0785_signals.data(),      d_rs_bs_0785_signals.size()      },
        { 0x0722, false, 8, "SD_RS_BS",     "System Diagnostic Response Brake System ESP",         sd_rs_bs_0722_signals.data(),     sd_rs_bs_0722_signals.size()     },
        { 0x0635, false, 8, "BS_APPL2",     "Application Interface Brake System 2",                bs_appl2_0635_signals.data(),     bs_appl2_0635_signals.size()     },
        { 0x0100, false, 8, "MS_100h",      "Drive Authorization FBS Message to EZS",              ms_100h_0100_signals.data(),      ms_100h_0100_signals.size()      },
        { 0x0101, false, 8, "MS_101h",      "Drive Authorization FBS Redundant Message",           ms_101h_0101_signals.data(),      ms_101h_0101_signals.size()      },
        { 0x0210, false, 8, "MS_210h",      "Pedal Position, Idle & Cruise Control",               ms_210h_0210_signals.data(),      ms_210h_0210_signals.size()      },
        { 0x0212, false, 8, "MS_212h",      "Target Idle Speed & Driver Requested Torque",         ms_212h_0212_signals.data(),      ms_212h_0212_signals.size()      },
        { 0x0268, false, 8, "MS_268h",      "Generator Load & Target Speed Range",                 ms_268h_0268_signals.data(),      ms_268h_0268_signals.size()      },
        { 0x02F3, false, 8, "MS_2F3h",      "Gear Shift Recommendation",                           ms_2f3h_02f3_signals.data(),      ms_2f3h_02f3_signals.size()      },
        { 0x0308, false, 8, "MS_308h",      "Engine Speed, Oil Levels & Diagnostics",              ms_308h_0308_signals.data(),      ms_308h_0308_signals.size()      },
        { 0x0312, false, 8, "MS_312h",      "Engine Torques (Static, Max, Min, Turbo)",            ms_312h_0312_signals.data(),      ms_312h_0312_signals.size()      },
        { 0x0580, false, 8, "AAD_580h",     "Adaptive Acceleration & Driving Style",               aad_580h_0580_signals.data(),     aad_580h_0580_signals.size()     },
        { 0x0608, false, 8, "MS_608h",      "Engine Temperature & Fuel Consumption",               ms_608h_0608_signals.data(),      ms_608h_0608_signals.size()      },
        { 0x0779, false, 8, "D_RS_FSCM",    "Diagnostic Response Fuel System Control",             d_rs_fscm_0779_signals.data(),    d_rs_fscm_0779_signals.size()    },
        { 0x07E8, false, 8, "D_RS_MS",      "Diagnostic Response Engine Control Module",           d_rs_ms_07e8_signals.data(),      d_rs_ms_07e8_signals.size()      },
        { 0x0720, false, 8, "SD_RS_MS",     "System Diagnostic Response Engine Control",           sd_rs_ms_0720_signals.data(),     sd_rs_ms_0720_signals.size()     },
        { 0x0529, false, 8, "SG_APPL_MS",   "Application Interface Engine Control",                sg_appl_ms_0529_signals.data(),   sg_appl_ms_0529_signals.size()   },
        { 0x04A9, false, 8, "MS_APPL2",     "Application Interface Engine Control 2",              ms_appl2_04a9_signals.data(),     ms_appl2_04a9_signals.size()     },
        { 0x0633, false, 8, "MS_APPL4",     "Application Interface Engine Control 4",              ms_appl4_0633_signals.data(),     ms_appl4_0633_signals.size()     },
        { 0x06A8, false, 8, "MS_APPL5",     "Application Interface Engine Control 5",              ms_appl5_06a8_signals.data(),     ms_appl5_06a8_signals.size()     },
        { 0x0610, false, 8, "MS_APPL6",     "Application Interface Engine Control 6",              ms_appl6_0610_signals.data(),     ms_appl6_0610_signals.size()     },
        { 0x0618, false, 8, "MS_APPL7",     "Application Interface Engine Control 7",              ms_appl7_0618_signals.data(),     ms_appl7_0618_signals.size()     },
        { 0x0670, false, 8, "EDC_MESS1",    "Diesel Control Measurement Data 1",                   edc_mess1_0670_signals.data(),    edc_mess1_0670_signals.size()    },
        { 0x0671, false, 8, "EDC_MESS2",    "Diesel Control Measurement Data 2",                   edc_mess2_0671_signals.data(),    edc_mess2_0671_signals.size()    },
        { 0x0218, false, 8, "GS_218h",      "Transmission Gears & Converter Clutch Status",        gs_218h_0218_signals.data(),      gs_218h_0218_signals.size()      },
        { 0x0338, false, 8, "GS_338h",      "Transmission Output & Turbine Speeds",                gs_338h_0338_signals.data(),      gs_338h_0338_signals.size()      },
        { 0x0418, false, 8, "GS_418h",      "Driving Position, Oil Temp & Wheel Torque",           gs_418h_0418_signals.data(),      gs_418h_0418_signals.size()      },
        { 0x07E9, false, 8, "D_RS_GS",      "Diagnostic Response Transmission Control",            d_rs_gs_07e9_signals.data(),      d_rs_gs_07e9_signals.size()      },
        { 0x0723, false, 8, "SD_RS_GS",     "System Diagnostic Response Transmission Control",     sd_rs_gs_0723_signals.data(),     sd_rs_gs_0723_signals.size()     },
        { 0x051C, false, 8, "GS_APPL1",     "Application Interface Transmission Control",          gs_appl1_051c_signals.data(),     gs_appl1_051c_signals.size()     },
        { 0x050A, false, 8, "GS_HSA",       "Test Bench Manual Control A",                         gs_hsa_050a_signals.data(),       gs_hsa_050a_signals.size()       },
        { 0x050B, false, 8, "GS_HSB",       "Test Bench Manual Control B",                         gs_hsb_050b_signals.data(),       gs_hsb_050b_signals.size()       },
        { 0x050C, false, 8, "GS_HSC",       "Test Bench Manual Control C",                         gs_hsc_050c_signals.data(),       gs_hsc_050c_signals.size()       },
        { 0x050D, false, 8, "GS_HSD",       "Test Bench Manual Control D",                         gs_hsd_050d_signals.data(),       gs_hsd_050d_signals.size()       },
        { 0x050E, false, 8, "GS_HSE",       "Test Bench Manual Control E",                         gs_hse_050e_signals.data(),       gs_hse_050e_signals.size()       },
        { 0x0230, false, 8, "EWM_230h",     "Electronic Selector Lever Position & Program",        ewm_230h_0230_signals.data(),     ewm_230h_0230_signals.size()     },
        { 0x0789, false, 8, "D_RS_EWM",     "Diagnostic Response Electronic Selector Lever",       d_rs_ewm_0789_signals.data(),     d_rs_ewm_0789_signals.size()     },
        { 0x0724, false, 8, "SD_RS_EWM",    "System Diagnostic Response Selector Lever",           sd_rs_ewm_0724_signals.data(),    sd_rs_ewm_0724_signals.size()    },
        { 0x06F0, false, 8, "EWM_MESS1",    "Selector Lever Measurement Data 1",                   ewm_mess1_06f0_signals.data(),    ewm_mess1_06f0_signals.size()    },
        { 0x06F1, false, 8, "EWM_MESS2",    "Selector Lever Measurement Data 2",                   ewm_mess2_06f1_signals.data(),    ewm_mess2_06f1_signals.size()    },
        { 0x0110, false, 8, "FBS_110h",     "Drive Authorization FBS Message to Engine",           fbs_110h_0110_signals.data(),     fbs_110h_0110_signals.size()     },
        { 0x0111, false, 8, "FBS_111h",     "Drive Authorization FBS Message to Engine 2",         fbs_111h_0111_signals.data(),     fbs_111h_0111_signals.size()     },
        { 0x0112, false, 8, "FBS_112h",     "Drive Authorization FBS Message to Transmission",     fbs_112h_0112_signals.data(),     fbs_112h_0112_signals.size()     },
        { 0x0114, false, 8, "FBS_114h",     "Drive Authorization FBS Message to Selector Lever",   fbs_114h_0114_signals.data(),     fbs_114h_0114_signals.size()     },
        { 0x0240, false, 8, "EZS_240h",     "Cruise Control Lever, Terminals & Lights",            ezs_240h_0240_signals.data(),     ezs_240h_0240_signals.size()     },
        { 0x0248, false, 8, "ZGW_248h",     "Gateway Headlight & Towing Status",                   zgw_248h_0248_signals.data(),     zgw_248h_0248_signals.size()     },
        { 0x024C, false, 8, "ZGW_24Ch",     "Gateway Low Beam Failure Status",                     zgw_24ch_024c_signals.data(),     zgw_24ch_024c_signals.size()     },
        { 0x040E, false, 8, "KLA_40Eh",     "Heating Power Demand",                                kla_40eh_040e_signals.data(),     kla_40eh_040e_signals.size()     },
        { 0x0410, false, 8, "KLA_410h",     "Climate Control Compressor & Fan Demand",             kla_410h_0410_signals.data(),     kla_410h_0410_signals.size()     },
        { 0x074C, false, 8, "APPL_SG_MS",   "Application Interface to Engine Control",             appl_sg_ms_074c_signals.data(),   appl_sg_ms_074c_signals.size()   },
        { 0x078E, false, 8, "D_RQ_ART",     "Diagnostic Request Adaptive Cruise Control",          d_rq_art_078e_signals.data(),     d_rq_art_078e_signals.size()     },
        { 0x0784, false, 8, "D_RQ_BS",      "Diagnostic Request Brake System ESP",                 d_rq_bs_0784_signals.data(),      d_rq_bs_0784_signals.size()      },
        { 0x077A, false, 8, "D_RQ_CAS",     "Diagnostic Request Collision Avoidance System",       d_rq_cas_077a_signals.data(),     d_rq_cas_077a_signals.size()     },
        { 0x0702, false, 8, "D_RQ_DTR",     "Diagnostic Request Distronic System",                 d_rq_dtr_0702_signals.data(),     d_rq_dtr_0702_signals.size()     },
        { 0x079A, false, 8, "D_RQ_EHB",     "Diagnostic Request Electrohydraulic Brake",           d_rq_ehb_079a_signals.data(),     d_rq_ehb_079a_signals.size()     },
        { 0x07B0, false, 8, "D_RQ_EHB2",    "Diagnostic Request Electrohydraulic Brake 2",         d_rq_ehb2_07b0_signals.data(),    d_rq_ehb2_07b0_signals.size()    },
        { 0x0788, false, 8, "D_RQ_EWM",     "Diagnostic Request Electronic Selector Lever",        d_rq_ewm_0788_signals.data(),     d_rq_ewm_0788_signals.size()     },
        { 0x078C, false, 8, "D_RQ_FS",      "Diagnostic Request Active Body Control",              d_rq_fs_078c_signals.data(),      d_rq_fs_078c_signals.size()      },
        { 0x0778, false, 8, "D_RQ_FSCM",    "Diagnostic Request Fuel System Control Module",       d_rq_fscm_0778_signals.data(),    d_rq_fscm_0778_signals.size()    },
        { 0x07E1, false, 8, "D_RQ_GS",      "Diagnostic Request Transmission Control",             d_rq_gs_07e1_signals.data(),      d_rq_gs_07e1_signals.size()      },
        { 0x06EA, false, 8, "D_RQ_ISM",     "Diagnostic Request Intelligent Servo Module",         d_rq_ism_06ea_signals.data(),     d_rq_ism_06ea_signals.size()     },
        { 0x0796, false, 8, "D_RQ_KOMBI_C", "Diagnostic Request Instrument Cluster",               d_rq_kombi_c_0796_signals.data(), d_rq_kombi_c_0796_signals.size() },
        { 0x0794, false, 8, "D_RQ_LWR",     "Diagnostic Request Headlight Range Adjustment",       d_rq_lwr_0794_signals.data(),     d_rq_lwr_0794_signals.size()     },
        { 0x07E0, false, 8, "D_RQ_MS",      "Diagnostic Request Engine Control Module",            d_rq_ms_07e0_signals.data(),      d_rq_ms_07e0_signals.size()      },
        { 0x07B2, false, 8, "D_RQ_RGS_L",   "Diagnostic Request Seatbelt Tensioner Left",          d_rq_rgs_l_07b2_signals.data(),   d_rq_rgs_l_07b2_signals.size()   },
        { 0x07B4, false, 8, "D_RQ_RGS_R",   "Diagnostic Request Seatbelt Tensioner Right",         d_rq_rgs_r_07b4_signals.data(),   d_rq_rgs_r_07b4_signals.size()   },
        { 0x07A2, false, 8, "D_RQ_UP28",    "Diagnostic Request Microprocessor Control",           d_rq_up28_07a2_signals.data(),    d_rq_up28_07a2_signals.size()    },
        { 0x06E4, false, 8, "GS_APPL2",     "Application Interface Transmission Control 2",        gs_appl2_06e4_signals.data(),     gs_appl2_06e4_signals.size()     },
        { 0x074A, false, 8, "MS_APPL1",     "Application Interface Engine Control 1",              ms_appl1_074a_signals.data(),     ms_appl1_074a_signals.size()     },
        { 0x06E0, false, 8, "MS_APPL3",     "Application Interface Engine Control 3",              ms_appl3_06e0_signals.data(),     ms_appl3_06e0_signals.size()     },
        { 0x060E, false, 8, "EZS_MESS1",    "Ignition Switch Measurement Data 1",                  ezs_mess1_060e_signals.data(),    ezs_mess1_060e_signals.size()    },
        { 0x07DF, false, 8, "DG_RQ_OBD",    "Global OBD-II Diagnostic Request",                    dg_rq_obd_07df_signals.data(),    dg_rq_obd_07df_signals.size()    },
        { 0x06FA, false, 8, "VIN",          "Vehicle Identification Number",                       vin_06fa_signals.data(),          vin_06fa_signals.size()          },
        { 0x0408, false, 8, "KOMBI_408h",   "Fuel Level, Door Status, Speedometer & Odometer",     kombi_408h_0408_signals.data(),   kombi_408h_0408_signals.size()   },
        { 0x0412, false, 8, "KOMBI_412h",   "Cluster Speed, Warnings & Distance Setting",          kombi_412h_0412_signals.data(),   kombi_412h_0412_signals.size()   },
        { 0x0797, false, 8, "D_RS_KOMBI_C", "Diagnostic Response Instrument Cluster",              d_rs_kombi_c_0797_signals.data(), d_rs_kombi_c_0797_signals.size() },
        { 0x0680, false, 8, "KOMBI_MESS1",  "Instrument Cluster Measurement Data 1",               kombi_mess1_0680_signals.data(),  kombi_mess1_0680_signals.size()  },
        { 0x0681, false, 8, "KOMBI_MESS2",  "Instrument Cluster Measurement Data 2",               kombi_mess2_0681_signals.data(),  kombi_mess2_0681_signals.size()  },
        { 0x0236, false, 8, "LRW_236h",     "Steering Wheel Angle & Angular Speed",                lrw_236h_0236_signals.data(),     lrw_236h_0236_signals.size()     },
        { 0x0238, false, 8, "MRM_238h",     "Cruise Control Lever & Steering Angle",               mrm_238h_0238_signals.data(),     mrm_238h_0238_signals.size()     },
        { 0x0035, false, 8, "ARCADE_A2",    "Crash Sensor Confirmation & Frontal Event",           arcade_a2_0035_signals.data(),    arcade_a2_0035_signals.size()    },
        { 0x033D, false, 8, "MS_ANZ",       "Engine Start/Stop Display Status",                    ms_anz_033d_signals.data(),       ms_anz_033d_signals.size()       },
        { 0x0102, false, 8, "GS_102h",      "Drive Authorization FBS Message from GS",             gs_102h_0102_signals.data(),      gs_102h_0102_signals.size()      },
        { 0x0104, false, 8, "EWM_104h",     "Drive Authorization FBS Message from EWM",            ewm_104h_0104_signals.data(),     ewm_104h_0104_signals.size()     },
        { 0x0232, false, 8, "SBW_232h",     "Shift-by-Wire Control Buttons & Lever Status",        sbw_232h_0232_signals.data(),     sbw_232h_0232_signals.size()     },
        { 0x0250, false, 8, "ART_250h",     "Distronic Torque Request & Target Gears",             art_250h_0250_signals.data(),     art_250h_0250_signals.size()     },
        { 0x0258, false, 8, "ART_258h",     "Distronic Target Distance, Speed & Object Detection", art_258h_0258_signals.data(),     art_258h_0258_signals.size()     },
        { 0x0340, false, 8, "FS_340h",      "ABC Suspension Pump Load Torque",                     fs_340h_0340_signals.data(),      fs_340h_0340_signals.size()      },
        { 0x03B4, false, 8, "PSM_3B4h",     "Parametric Module Engine Speed & Torque Limits",      psm_3b4h_03b4_signals.data(),     psm_3b4h_03b4_signals.size()     },
        { 0x03B8, false, 8, "PSM_3B8h",     "Parametric Module Speed Limit & Remote Start",        psm_3b8h_03b8_signals.data(),     psm_3b8h_03b8_signals.size()     },
        { 0x0414, false, 8, "KOMBI_414h",   "Filtered Outside Temperature",                        kombi_414h_0414_signals.data(),   kombi_414h_0414_signals.size()   },
        { 0x0428, false, 8, "VG_428h",      "Transfer Case Gear & Neutral Request",                vg_428h_0428_signals.data(),      vg_428h_0428_signals.size()      },
        { 0x0530, false, 8, "LWR_530h",     "Headlight Range & Cornering Light Messages",          lwr_530h_0530_signals.data(),     lwr_530h_0530_signals.size()     },
        { 0x06FF, false, 8, "CONFIG_6FFh",  "Drivetrain Equipment & Differential Lock Coding",     config_6ffh_06ff_signals.data(),  config_6ffh_06ff_signals.size()  },
        { 0x078F, false, 8, "D_RS_ART",     "Diagnostic Response Distronic",                       d_rs_art_078f_signals.data(),     d_rs_art_078f_signals.size()     },
        { 0x077B, false, 8, "D_RS_CAS",     "Diagnostic Response Collision Avoidance System",      d_rs_cas_077b_signals.data(),     d_rs_cas_077b_signals.size()     },
        { 0x04A0, false, 8, "D_RS_DTR",     "Diagnostic Response Distronic Radar",                 d_rs_dtr_04a0_signals.data(),     d_rs_dtr_04a0_signals.size()     },
        { 0x079B, false, 8, "D_RS_EHB",     "Diagnostic Response Electrohydraulic Brake",          d_rs_ehb_079b_signals.data(),     d_rs_ehb_079b_signals.size()     },
        { 0x07B1, false, 8, "D_RS_EHB2",    "Diagnostic Response Electrohydraulic Brake 2",        d_rs_ehb2_07b1_signals.data(),    d_rs_ehb2_07b1_signals.size()    },
        { 0x078D, false, 8, "D_RS_FS",      "Diagnostic Response Active Body Control",             d_rs_fs_078d_signals.data(),      d_rs_fs_078d_signals.size()      },
        { 0x049D, false, 8, "D_RS_ISM",     "Diagnostic Response Intelligent Servo Module",        d_rs_ism_049d_signals.data(),     d_rs_ism_049d_signals.size()     },
        { 0x0795, false, 8, "D_RS_LWR",     "Diagnostic Response Headlight Range Adjustment",      d_rs_lwr_0795_signals.data(),     d_rs_lwr_0795_signals.size()     },
        { 0x07B3, false, 8, "D_RS_RGS_L",   "Diagnostic Response Seatbelt Tensioner Left",         d_rs_rgs_l_07b3_signals.data(),   d_rs_rgs_l_07b3_signals.size()   },
        { 0x07B5, false, 8, "D_RS_RGS_R",   "Diagnostic Response Seatbelt Tensioner Right",        d_rs_rgs_r_07b5_signals.data(),   d_rs_rgs_r_07b5_signals.size()   },
        { 0x07A3, false, 8, "D_RS_UP28",    "Diagnostic Response Microprocessor Control",          d_rs_up28_07a3_signals.data(),    d_rs_up28_07a3_signals.size()    },
        { 0x0634, false, 8, "BS_APPL1",     "Application Interface Brake System 1",                bs_appl1_0634_signals.data(),     bs_appl1_0634_signals.size()     },
        { 0x0630, false, 8, "MS_APPL1",     "Application Interface Engine Control (Alternate)",    ms_appl1_0630_signals.data(),     ms_appl1_0630_signals.size()     },
        { 0x0632, false, 8, "MS_APPL3",     "Application Interface Engine Control 3 (Alternate)",  ms_appl3_0632_signals.data(),     ms_appl3_0632_signals.size()     },
        { 0x060F, false, 8, "EZS_MESS2",    "Ignition Switch Measurement Data 2",                  ezs_mess2_060f_signals.data(),    ezs_mess2_060f_signals.size()    },
        { 0x0501, false, 8, "GS_HS1",       "Test Bench Manual Control 1",                         gs_hs1_0501_signals.data(),       gs_hs1_0501_signals.size()       },
        { 0x0502, false, 8, "GS_HS2",       "Test Bench Manual Control 2",                         gs_hs2_0502_signals.data(),       gs_hs2_0502_signals.size()       },
        { 0x0503, false, 8, "GS_HS3",       "Test Bench Manual Control 3",                         gs_hs3_0503_signals.data(),       gs_hs3_0503_signals.size()       },
        { 0x0504, false, 8, "GS_HS4",       "Test Bench Manual Control 4",                         gs_hs4_0504_signals.data(),       gs_hs4_0504_signals.size()       },
        { 0x0505, false, 8, "GS_HS5",       "Test Bench Manual Control 5",                         gs_hs5_0505_signals.data(),       gs_hs5_0505_signals.size()       },
        { 0x0506, false, 8, "GS_HS6",       "Test Bench Manual Control 6",                         gs_hs6_0506_signals.data(),       gs_hs6_0506_signals.size()       },
    });

    namespace can_c
    {
        can_profile& get_profile()
        {
            static can_profile profile
            (
                "Mercedes W209 CAN-C"_ct,
                "Mercedes-Benz W209 Drivetrain CAN-C Bus"_ct,
                w209_c_messages.data(),
                w209_c_messages.size()
            );
            return profile;
        }
    }

    can_profile& get_can_c_profile()
    {
        return can_c::get_profile();
    }
}
