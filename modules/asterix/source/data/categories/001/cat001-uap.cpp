#include "data/categories/001/cat001-uap.hpp"

#include <array>

#include "data/categories/001/cat001-structs.hpp"

namespace adam::modules::asterix::cat001
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for sub-UAPs so the main UAPs can sit at the top
    extern uap                      cat001_track_uap;
    extern uap                      cat001_plot_uap;
    extern uap::selector_function   cat001_selector_fn;

    // Base UAP for CAT001, this will be used to determine Actual UAP to be used
    const auto cat001_base = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I001/010 Data Source Identifier"                                      },
        {  2, item_type_variable,    1,      1, "I001/020 Target Report Descriptor"                                    },
    });

    // Plot UAP for CAT001
    const auto cat001_plot_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I001/010 Data Source Identifier"                                      },
        {  2, item_type_variable,    1,      1, "I001/020 Target Report Descriptor"                                    },
        {  3, item_type_fixed,       0,      4, "I001/040 Measured Position in Polar Coordinates"                      },
        {  4, item_type_fixed,       0,      2, "I001/070 Mode-3/A Code in Octal Representation"                       },
        {  5, item_type_fixed,       0,      2, "I001/090 Mode-C Code in Binary Representation"                        },
        {  6, item_type_variable,    1,      1, "I001/130 Radar Plot Characteristics"                                  },
        {  7, item_type_fixed,       0,      2, "I001/141 Truncated Time of Day"                                       },
        
        // FSPEC Byte 2
        {  8, item_type_fixed,       0,      2, "I001/050 Mode-2 Code in Octal Representation"                         },
        {  9, item_type_fixed,       0,      1, "I001/120 Measured Radial Doppler Speed"                               },
        { 10, item_type_fixed,       0,      1, "I001/131 Received Power"                                              },
        { 11, item_type_fixed,       0,      2, "I001/080 Mode-3/A Code Confidence Indicator"                          },
        { 12, item_type_fixed,       0,      3, "I001/100 Mode-C Code and Code Confidence Indicator"                   },
        { 13, item_type_fixed,       0,      2, "I001/060 Mode-2 Code Confidence Indicator"                            },
        { 14, item_type_variable,    1,      1, "I001/030 Warning/Error Conditions"                                    },

        // FSPEC Byte 3
        { 15, item_type_fixed,       0,      1, "I001/150 Presence of X-Pulse"                                         },
    });

    uap cat001_plot_uap(1, "CAT001 1.4 - PLOT"_ct, cat001_plot_items.data(), cat001_plot_items.size());

    
    // Track UAP for CAT001
    const auto cat001_track_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I001/010 Data Source Identifier"                                      },
        {  2, item_type_variable,    1,      1, "I001/020 Target Report Descriptor"                                    },
        {  3, item_type_fixed,       0,      2, "I001/161 Track/Plot Number"                                           },
        {  4, item_type_fixed,       0,      4, "I001/040 Measured Position in Polar Coordinates"                      },
        {  5, item_type_fixed,       0,      4, "I001/042 Calculated Position in Cartesian Coordinates"                },
        {  6, item_type_fixed,       0,      4, "I001/200 Calculated Track Velocity in polar Coordinates"              },
        {  7, item_type_fixed,       0,      2, "I001/070 Mode-3/A Code in Octal Representation"                       },

        // FSPEC Byte 2
        {  8, item_type_fixed,       0,      2, "I001/090 Mode-C Code in Binary Representation"                        },
        {  9, item_type_variable,    1,      1, "I001/130 Radar Plot Characteristics"                                  },
        { 10, item_type_fixed,       0,      2, "I001/141 Truncated Time of Day"                                       },
        { 11, item_type_fixed,       0,      1, "I001/131 Received Power"                                              },
        { 12, item_type_fixed,       0,      1, "I001/120 Measured Radial Doppler Speed"                               },
        { 13, item_type_variable,    1,      1, "I001/170 Track Status"                                                },
        { 14, item_type_variable,    1,      1, "I001/210 Track Quality"                                               }
    });

    uap cat001_track_uap(1, "CAT001 1.4 - TRACK"_ct, cat001_track_items.data(), cat001_track_items.size());

    const auto cat001_alt = std::to_array<const uap*>({&cat001_track_uap, &cat001_plot_uap});

    // The CAT001 Selector function, Checks the Msg wether its Plot or Track and then returns the correct UAP for it.
    uap::selector_function cat001_selector_fn = [](const raw_record_header* raw_head, uint8_t fspec_size, uint32_t raw_len) -> const uap*
    {
        const auto* data = reinterpret_cast<const uint8_t* >(raw_head);

        // FRN 2 = Target Report Descriptor, MUST be here
        if (!raw_head->fspec.is_frn_active_here(2)) return nullptr;

        // Move pointer to items start
        data += fspec_size;
        
        // If FRN 1 is here, move ptr
        if (raw_head->fspec.is_frn_active_here(1))
        {
            auto advance_size = cat001::get_uap().get_spec(1)->data_size;
            if (raw_len < advance_size) return nullptr;
            data += advance_size;
        }

        // Determine type using the TRD
        const auto* trd = reinterpret_cast<const cat001::target_report_descriptor*>(data);
        switch (trd->msg_type)
        {
            case cat001::message_type::message_type_plot:   return &cat001_plot_uap;
            case cat001::message_type::message_type_track:  return &cat001_track_uap;
        }

        return nullptr;
    };

    uap& get_uap()
    {
        // We need to init this here, after the static initialization of the sub-UAPs and selector function, but before any potential usage.
        static uap cat001_uap(1, "CAT001 1.4 - BASE"_ct, cat001_base.data(), cat001_base.size(), cat001_alt.data(), cat001_alt.size(), cat001_selector_fn );
        return cat001_uap;
    }
}
