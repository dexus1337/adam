#include "data/categories/007/cat007-uap.hpp"

#include <array>

#include "data/categories/007/cat007-structs.hpp"

namespace adam::modules::asterix::cat007
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for sub-UAPs so the main UAP can sit at the top
    extern uap                      cat007_downlink_uap;
    extern uap                      cat007_uplink_uap;
    extern uap::selector_function   cat007_selector_fn;

    // -------------------------------------------------------------------------------------------------------------- //
    // Base UAP for CAT007.
    //
    // Only the items that are guaranteed to be present in EVERY record and are
    // required to resolve the correct sub-UAP are listed here. The parser uses
    // these specs to navigate to I007/410 (FRN 3 in both Downlink and Uplink
    // UAPs) and read the message type before forwarding to the selected sub-UAP.
    //
    // Structure matches the Downlink UAP up to FRN 3 (I007/410), which is the
    // earliest point at which the message type can be determined.
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat007_base = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I007/010 Data Source Identifier"                                      },
        {  2, item_type_fixed,       0,      2, "I007/025 Data Destination Identifier"                                 },
        {  3, item_type_fixed,       0,      1, "I007/410 Directed Interrogation Message Type"                         },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // Downlink UAP – Table 2 (Ed. 1.12)
    // Applied to message types 0–4:
    //   0 = DI Acknowledge
    //   1 = DI Reject
    //   2 = DI Finished
    //   3 = DI Completed
    //   4 = Target Report
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat007_downlink_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        cat007_base[0], //                      "I007/010 Data Source Identifier"                                      },
        cat007_base[1], //                      "I007/025 Data Destination Identifier"                                 },
        cat007_base[2], //                      "I007/410 Directed Interrogation Message Type"                         },
        {  4, item_type_fixed,       0,      3, "I007/140 Time-of-Day"                                                 },
        {  5, item_type_fixed,       0,      2, "I007/400 Directed Interrogation Request Number"                       },
        {  6, item_type_variable,    1,      1, "I007/020 Target Report Descriptor"},
        {  7, item_type_fixed,       0,      4, "I007/040 Measured Position in Slant Polar Coordinates"                },

        // FSPEC Byte 2
        {  8, item_type_fixed,       0,      2, "I007/070 Mode-3/A Code in Octal Representation"                       },
        {  9, item_type_fixed,       0,      2, "I007/090 Flight Level in Binary Representation"                       },
        { 10, item_type_compound,    0,      0, "I007/130 Radar Plot Characteristics"                                  },
        { 11, item_type_fixed,       0,      3, "I007/220 Aircraft Address"                                            },
        { 12, item_type_fixed,       0,      6, "I007/240 Aircraft Identification"                                     },
        { 13, item_type_repetetive,  1,      8, "I007/250 Mode S MB Data"                                              },
        { 14, item_type_fixed,       0,      2, "I007/161 Track Number"                                                },

        // FSPEC Byte 3
        { 15, item_type_fixed,       0,      4, "I007/042 Calculated Position in Cartesian Coordinates"                },
        { 16, item_type_fixed,       0,      4, "I007/200 Calculated Track Velocity in Polar Representation"           },
        { 17, item_type_variable,    1,      1, "I007/170 Track Status"                                                },
        { 18, item_type_fixed,       0,      4, "I007/210 Track Quality"                                               },
        { 19, item_type_variable,    1,      1, "I007/030 Warning/Error Conditions"                                    },
        { 20, item_type_fixed,       0,      2, "I007/080 Mode-3/A Code Confidence Indicator"                          },
        { 21, item_type_fixed,       0,      4, "I007/100 Mode-C Code and Code Confidence Indicator"                   },

        // FSPEC Byte 4
        { 22, item_type_fixed,       0,      2, "I007/110 Height Measured by 3D Radar"                                 },
        { 23, item_type_compound,    0,      0, "I007/120 Radial Doppler Speed"                                        },
        { 24, item_type_fixed,       0,      2, "I007/230 Communications / ACAS Capability and Flight Status"          },
        { 25, item_type_fixed,       0,      7, "I007/260 ACAS Resolution Advisory Report"                             },
        { 26, item_type_fixed,       0,      1, "I007/055 Mode-1 Code in Octal Representation"                         },
        { 27, item_type_fixed,       0,      2, "I007/050 Mode-2 Code in Octal Representation"                         },
        { 28, item_type_fixed,       0,      1, "I007/065 Mode-1 Code Confidence Indicator"                            },

        // FSPEC Byte 5
        { 29, item_type_fixed,       0,      2, "I007/060 Mode-2 Code Confidence Indicator"                            },
        { 30, item_type_compound,    0,      0, "I007/450 Directed Interrogation Result"                               },
        { 31, item_type_variable,    1,      1, "I007/085 Mode 5, Extended Mode 1, X-Pulse"                            },
        // FRN 32 and 33 are Unused
        { 34, item_type_explicit,    0,      0, "SPF Special Purpose Field"                                            },
        { 35, item_type_explicit,    0,      0, "REF Reserved Expansion Field"                                         },
    });

    uap cat007_downlink_uap(7, "CAT007 " CAT007_VERSION " - DOWNLINK"_ct, cat007_downlink_items.data(), cat007_downlink_items.size());

    // -------------------------------------------------------------------------------------------------------------- //
    // Uplink UAP – Table 3 (Ed. 1.12)
    // Applied to message types 5–8:
    //   5 = DI Request Type A (Position)
    //   6 = DI Request Type B (Window)
    //   7 = DI Request Type C (Track-Number)
    //   8 = Selective BDS-Register Request
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat007_uplink_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        cat007_base[0], //                      "I007/010 Data Source Identifier"                                      },
        cat007_base[1], //                      "I007/025 Data Destination Identifier"                                 },
        cat007_base[2], //                      "I007/410 Directed Interrogation Message Type"                         },
        {  4, item_type_fixed,       0,      3, "I007/140 Time-of-Day"                                                 },
        {  5, item_type_fixed,       0,      2, "I007/400 Directed Interrogation Request Number"                       },
        {  6, item_type_fixed,       0,      4, "I007/040 Measured Position in Slant Polar Coordinates"                },
        {  7, item_type_fixed,       0,      3, "I007/220 Aircraft Address"                                            },

        // FSPEC Byte 2
        {  8, item_type_fixed,       0,      2, "I007/161 Track Number"                                                },
        {  9, item_type_fixed,       0,      4, "I007/042 Calculated Position in Cartesian Coordinates"                },
        { 10, item_type_fixed,       0,      4, "I007/200 Calculated Track Velocity in Polar Representation"           },
        { 11, item_type_variable,    1,      1, "I007/415 Required Interrogation Modes"                                },
        { 12, item_type_fixed,       0,      8, "I007/420 Directed Interrogation Window"                               },
        { 13, item_type_repetetive,  1,      1, "I007/440 Directed Interrogation BDS Register Request"                 },
        // FRN 14 is Unused

        // FSPEC Byte 3 – FRNs 15–19 are Unused
        { 20, item_type_explicit,    0,      0, "SPF Special Purpose Field"                                            },
        { 21, item_type_explicit,    0,      0, "REF Reserved Expansion Field"                                         },
    });

    uap cat007_uplink_uap(7, "CAT007 " CAT007_VERSION " - UPLINK"_ct, cat007_uplink_items.data(), cat007_uplink_items.size());

    const auto cat007_alt = std::to_array<const uap*>({&cat007_downlink_uap, &cat007_uplink_uap});

    // -------------------------------------------------------------------------------------------------------------- //
    // CAT007 Selector Function
    //
    // Reads I007/410 (Directed Interrogation Message Type) to determine which
    // sub-UAP to use. I007/410 is always at FRN 3 in both sub-UAPs, and the base
    // UAP defined above is used by the parser to navigate the FSPEC and locate
    // the relevant data.
    //
    // Navigation logic mirrors the CAT001 selector:
    //   1. FRN 3 (I007/410) must be active in the FSPEC.
    //   2. Skip FRN 1 and FRN 2 data bytes if they are present.
    //   3. Read the one-byte message type from I007/410.
    //   4. Return the Downlink UAP for types 0–4, Uplink UAP for types 5–8.
    // -------------------------------------------------------------------------------------------------------------- //
    uap::selector_function cat007_selector_fn = [](const raw_record_header* raw_head, uint8_t fspec_size, uint32_t raw_len) -> const uap*
    {
        const auto* data = reinterpret_cast<const uint8_t*>(raw_head);

        // I007/410 is FRN 3 – it must be present to determine the UAP
        if (!raw_head->fspec.is_frn_active_here(3)) return nullptr;

        // Move pointer past the FSPEC octets
        data += fspec_size;

        // If FRN 1 (I007/010, 2 bytes) is present, skip it
        if (raw_head->fspec.is_frn_active_here(1))
        {
            auto advance_size = cat007::get_uap().get_spec(1)->data_size;
            if (raw_len < advance_size) return nullptr;
            data += advance_size;
        }

        // If FRN 2 (I007/025, 2 bytes) is present, skip it
        if (raw_head->fspec.is_frn_active_here(2))
        {
            auto advance_size = cat007::get_uap().get_spec(2)->data_size;
            if (raw_len < advance_size) return nullptr;
            data += advance_size;
        }

        // Read I007/410 – one-byte message type
        const auto* msg_type_item = reinterpret_cast<const directed_interrogation_msg_type*>(data);

        switch (msg_type_item->msg_type)
        {
            case message_type_acknowledge:
            case message_type_reject:
            case message_type_finished:
            case message_type_completed:
            case message_type_target_report:
                return &cat007_downlink_uap;

            case message_type_request_a:
            case message_type_request_b:
            case message_type_request_c:
            case message_type_bds_request:
                return &cat007_uplink_uap;
        }

        return nullptr;
    };

    uap& get_uap()
    {
        // We need to init this here, after the static initialization of the sub-UAPs and selector function, but before any potential usage.
        static uap cat007_uap(7, "CAT007 " CAT007_VERSION " - BASE"_ct, cat007_base.data(), cat007_base.size(), cat007_alt.data(), cat007_alt.size(), cat007_selector_fn);
        return cat007_uap;
    }
}
