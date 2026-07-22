#include "data/categories/001/cat001-uap.hpp"

#include <array>

#include "data/categories/001/cat001-structs.hpp"

namespace adam::modules::asterix::cat001
{
    using namespace adam::string_hashed_ct_literals;

    // Forward declarations for sub-UAPs so the main UAP can sit at the top
    extern uap                          cat001_track_uap;
    extern uap                          cat001_plot_uap;
    extern uap::selector_function       cat001_selector_fn;
    extern uap::type_getter_function    cat001_type_getter_fn;

    // Fill the type database or this Category
    const uap::type_names_database& get_cat001_types() 
    {
        static const uap::type_names_database table = 
        {
            {cat001::message_type_plot,  "Plot"   },
            {cat001::message_type_track, "Track"  }
        };
        return table;
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // Base UAP for CAT001.
    //
    // Only the items that are guaranteed to be present in EVERY record and are
    // required to resolve the correct sub-UAP are listed here. The parser uses
    // these specs to navigate to I001/020 (FRN 2, Target Report Descriptor) and
    // read the msg_type bit before forwarding to the selected sub-UAP.
    //
    // FRN 1 (I001/010) is included so the selector can correctly skip its bytes
    // when navigating to FRN 2.
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat001_base = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        {  1, item_type_fixed,       0,      2, "I001/010 Data Source Identifier"                                      },
        {  2, item_type_variable,    1,      1, "I001/020 Target Report Descriptor"                                    },
    });

    // -------------------------------------------------------------------------------------------------------------- //
    // Plot UAP
    // Applied when bit msg_type (bit 1) of I001/020 == message_type_plot (0).
    // A Plot is a single, untracked detection from a radar sensor.
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat001_plot_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        cat001_base[0], //                      "I001/010 Data Source Identifier"
        cat001_base[1], //                      "I001/020 Target Report Descriptor"
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

    uap cat001_plot_uap
    (
        1,                                        /**< CAT Number.          */
        "CAT001 " CAT001_VERSION " - PLOT"_ct,    /**< CAT Names.           */
        cat001_plot_items.data(),                 /**< Items Array start.   */
        cat001_plot_items.size(),                 /**< Items Array length.  */
        1,                                        /**< SAC/SIC FRN.         */
        get_cat001_types(),                       /**< Type Database.       */
        cat001_type_getter_fn                     /**< Type Getter Function */
    );

    // -------------------------------------------------------------------------------------------------------------- //
    // Track UAP
    // Applied when bit msg_type (bit 1) of I001/020 == message_type_track (1).
    // A Track is a processed target report that includes locally derived motion
    // data (position, velocity) produced by the radar-site tracker.
    // -------------------------------------------------------------------------------------------------------------- //
    const auto cat001_track_items = std::to_array<const field_spec>
    ({
        // FSPEC Byte 1
        cat001_base[0], //                      "I001/010 Data Source Identifier"
        cat001_base[1], //                      "I001/020 Target Report Descriptor"
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
        { 14, item_type_variable,    1,      1, "I001/210 Track Quality"                                               },
    });

    uap cat001_track_uap
    (
        1,                                        /**< CAT Number.          */
        "CAT001 " CAT001_VERSION " - TRACK"_ct,   /**< CAT Names.           */
        cat001_track_items.data(),                /**< Items Array start.   */
        cat001_track_items.size(),                /**< Items Array length.  */
        1,                                        /**< SAC/SIC FRN.         */
        get_cat001_types(),                       /**< Type Database.       */
        cat001_type_getter_fn                     /**< Type Getter Function */
    );

    const auto cat001_alt = std::to_array<const uap*>({&cat001_track_uap, &cat001_plot_uap});

    // -------------------------------------------------------------------------------------------------------------- //
    // CAT001 Selector Function
    //
    // Reads (unparsed) I001/020 (Target Report Descriptor) to determine whether this record
    // is a Plot or a Track, and returns the corresponding sub-UAP.
    //
    // Navigation logic:
    //   1. FRN 2 (I001/020, TRD) must be active in the FSPEC - it is the only
    //      field that carries the msg_type discriminator bit.
    //   2. Move the data pointer past the FSPEC octets.
    //   3. If FRN 1 (I001/010, 2 bytes) is present, skip its data bytes.
    //   4. Cast the current pointer to target_report_descriptor and read msg_type:
    //        msg_type == message_type_plot  (0) → return the Plot  UAP
    //        msg_type == message_type_track (1) → return the Track UAP
    // -------------------------------------------------------------------------------------------------------------- //
    uap::selector_function cat001_selector_fn = [](const raw_record_header* raw_head, uint8_t fspec_size, uint32_t raw_len) -> const uap*
    {
        const auto* data = reinterpret_cast<const uint8_t*>(raw_head);

        // I001/020 is FRN 2 – it must be present to determine the UAP
        if (!raw_head->fspec.is_frn_active_here(2)) return nullptr;

        // Move pointer past the FSPEC octets
        data += fspec_size;

        // If FRN 1 (I001/010, 2 bytes) is present, skip it
        if (raw_head->fspec.is_frn_active_here(1))
        {
            auto advance_size = cat001::get_uap().get_spec(1)->data_size;
            if (raw_len < advance_size) return nullptr;
            data += advance_size;
        }

        // Read I001/020 – cast to TRD struct and inspect msg_type (bit 1)
        const auto* trd = reinterpret_cast<const cat001::target_report_descriptor*>(data);
        switch (trd->msg_type)
        {
            case cat001::message_type_plot:   return &cat001_plot_uap;
            case cat001::message_type_track:  return &cat001_track_uap;
        }

        return nullptr;
    };

    // -------------------------------------------------------------------------------------------------------------- //
    // CAT001 Type Getter Function
    //
    // Reads (parsed) I001/020 (Target Report Descriptor) to determine whether this record
    // is a Plot or a Track, and returns the corresponding enum entry.
    // -------------------------------------------------------------------------------------------------------------- //
    uap::type_getter_function cat001_type_getter_fn = [](const record* rec, const adam::buffer* buf) -> uint8_t
    {
        auto* trd_itm = rec->get_item(2);

        if (!trd_itm->is_populated())
            return 0xff;

        auto* data = trd_itm->get_data_as<const cat001::target_report_descriptor>(buf);

        if (!data)
            return 0xff;

        return data->msg_type;
    };

    uap& get_uap()
    {   
        static auto cat001_uap = uap
        (
            1,                                      /**< CAT Number.                        */
            "CAT001 " CAT001_VERSION " - BASE"_ct,  /**< CAT Names.                         */
            cat001_base.data(),                     /**< Items Array start.                 */
            cat001_base.size(),                     /**< Items Array length.                */
            cat001_alt.data(),                      /**< Alternative UAP Array start.       */
            cat001_alt.size(),                      /**< Alternative UAP Array length.      */
            cat001_selector_fn,                     /**< Alternative UAP Selector Function  */
            1,                                      /**< SAC/SIC FRN.                       */
            get_cat001_types(),                     /**< Type Database.                     */
            cat001_type_getter_fn                   /**< Type Getter Function               */
        );

        return cat001_uap;
    }
}
