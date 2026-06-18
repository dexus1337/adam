#pragma once

/**
 * @file    asterix-uap.hpp
 * @author  dexus1337
 * @brief   Defines the User Application Profiles (UAP) structures for Asterix category definitions.
 * @version 1.0
 * @date    15.06.2026
 */

#include "data/asterix-types.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace adam { class buffer; }

namespace adam::modules::asterix
{
    /**
     * @struct field_spec
     * @brief Defines how a specific data item in a UAP should be parsed.
     */
    struct field_spec
    {
        uint8_t           frn;                  /**< Field Reference Number (e.g. 10 for I048/010) */
        item_type         type;                 /**< The item type (fixed, variable, etc.) */
        uint8_t           header_size;          /**< Length of the metadata/header indicator. */
        uint16_t          data_size;            /**< Length of the Actual (sometimes reoccuring) data */
        const char*       name;                 /**< The string name of the item. */
        const class uap*  sub_uap = nullptr;    /**< Optional sub-UAP for nested compound items. */
    };

    /**
     * @struct uap_expansion
     * @brief Virtual base class for users to attach custom third-party metadata (e.g. classifications) to UAP items.
     */
    struct uap_expansion
    {
        virtual ~uap_expansion() = default;
    };

    /**
     * @class uap
     * @brief Represents the User Application Profile for an Asterix Category.
     */
    class ADAM_ASTERIX_API uap
    {
    public:
        uint8_t             cat_id;                                     /**< The category ID (e.g., 48, 62). */
        uint8_t             last_frn;                                   /**< Highest registered FRN; Also determines fixed FSPEC octet count for explicit items. */
        uint16_t            subitem_count;                              /**< Summended last_frn of all children. */
        const field_spec*   items_by_frn[asterix::highest_frn];         /**< O(1) array mapping FRN to field spec. */
        uap_expansion*      custom_expansions[asterix::highest_frn];    /**< O(1) array mapping FRN to custom expansion metadata. */

        /**
         * @brief Constructs the UAP and initializes the O(1) lookup arrays.
         * @param cat The category ID.
         * @param spec_array Array of field specs.
         * @param spec_count Number of elements in spec_array.
         */
        uap(uint8_t cat, const field_spec* spec_array, size_t spec_count)
            : cat_id(cat), last_frn(0)
        {
            std::memset(items_by_frn, 0, sizeof(items_by_frn));
            std::memset(custom_expansions, 0, sizeof(custom_expansions));

            for (size_t i = 0; i < spec_count; ++i)
            {
                items_by_frn[spec_array[i].frn] = &spec_array[i];
                if (spec_array[i].frn > last_frn)
                    last_frn = spec_array[i].frn; // Track highest FRN for fixed-FSPEC sizing
            }
        }

        /**
         * @brief   Retrieve the specification for a given FRN in O(1) time.
         * @param   frn The Field Reference Number.
         * @return  Pointer to the spec if found, nullptr otherwise.
         */
        inline const field_spec* get_spec(uint8_t frn) const
        {
            return items_by_frn[frn];
        }

        /**
         * @brief   Retrieve the custom expansion metadata for a given FRN in O(1) time.
         * @param   frn The Field Reference Number.
         * @return  Pointer to the metadata if found, nullptr otherwise.
         */
        inline uap_expansion* get_expansion(uint8_t frn) const
        {
            return custom_expansions[frn];
        }
        
        /**
         * @brief   Compute the number of sub-items.
         * @return  The computed amount.
         */
        void compute_subitem_count()
        {
            subitem_count = 0;
            
            for (size_t i = 0; i <= last_frn; ++i)
            {
                if (items_by_frn[i] && items_by_frn[i]->sub_uap)
                {
                    auto* const_casted_uap = const_cast<class uap*>(items_by_frn[i]->sub_uap);

                    const_casted_uap->compute_subitem_count();

                    subitem_count += const_casted_uap->subitem_count;
                }
            }
        }

        /**
         * @brief Initializer. MUST be called before inserting this into the pool
         */
        void setup()
        {
            compute_subitem_count();
        }

        /**
         * @brief Register custom third-party metadata.
         * @param frn The Field Reference Number to expand.
         * @param data Pointer to the virtual class metadata.
         */
        void expand_parameter(uint8_t frn, uap_expansion* data)
        {
            custom_expansions[frn] = data;
        }

    };

    /**
     * @class uap_pool
     * @brief Global singleton registry for mapping Asterix Category IDs to their UAP definitions.
     */
    class ADAM_ASTERIX_API uap_pool
    {
    public:
        /**
         * @brief Get the singleton instance of the UAP pool.
         */
        static uap_pool& get();

        /**
         * @brief Register a UAP into the global pool.
         * @param uap Pointer to the UAP definition.
         */
        void register_uap(uap* uap);

        /**
         * @brief Retrieve a UAP by its Category ID in O(1) time.
         * @param cat_id The Category ID.
         * @return Pointer to the UAP if found, nullptr otherwise.
         */
        const uap* get_uap(uint8_t cat_id) const;

    private:
        uap_pool(); // Private constructor for self-initialization
        ~uap_pool() = default;

        // Prevent copying
        uap_pool(const uap_pool&) = delete;
        uap_pool& operator=(const uap_pool&) = delete;

        const uap* registered_uaps[256];
    };

    inline const uap* raw_block_header::get_uap() const
    {
        return uap_pool::get().get_uap(category);
    }
    
    inline const uap* record::get_uap() const
    {
        return uap_pool::get().get_uap(category);
    }
    
    inline const uap* block::get_uap() const
    {
        return uap_pool::get().get_uap(category);
    }
}
