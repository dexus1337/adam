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

namespace adam::modules::asterix::data
{
    /**
     * @struct asterix_field_spec
     * @brief Defines how a specific data item in a UAP should be parsed.
     */
    struct asterix_field_spec
    {
        uint8_t           frn;      /**< Field Reference Number (e.g. 10 for I048/010) */
        asterix_item_type type;     /**< The item type (FIXED, VARIABLE, etc.) */
        uint16_t          length;   /**< Fixed length, or primary extent length for variable items */
        const char*       name;     /**< The string name of the item. */
        const class asterix_uap* sub_uap = nullptr; /**< Optional sub-UAP for nested COMPOUND items. */
    };

    /**
     * @struct asterix_uap_expansion
     * @brief Virtual base class for users to attach custom third-party metadata (e.g. classifications) to UAP items.
     */
    struct asterix_uap_expansion
    {
        virtual ~asterix_uap_expansion() = default;
    };

    /**
     * @class asterix_uap
     * @brief Represents the User Application Profile for an Asterix Category.
     */
    class asterix_uap
    {
    public:
        uint8_t                         cat_id;                     /**< The category ID (e.g., 48, 62). */
        const asterix_field_spec*       items_by_frn[256];          /**< O(1) array mapping FRN to field spec. */
        asterix_uap_expansion*          custom_expansions[256];     /**< O(1) array mapping FRN to custom expansion metadata. */

        /**
         * @brief Constructs the UAP and initializes the O(1) lookup arrays.
         * @param cat The category ID.
         * @param spec_array Array of field specs.
         * @param spec_count Number of elements in spec_array.
         */
        asterix_uap(uint8_t cat, const asterix_field_spec* spec_array, size_t spec_count)
            : cat_id(cat)
        {
            std::memset(items_by_frn, 0, sizeof(items_by_frn));
            std::memset(custom_expansions, 0, sizeof(custom_expansions));

            for (size_t i = 0; i < spec_count; ++i)
            {
                items_by_frn[spec_array[i].frn] = &spec_array[i];
            }
        }

        /**
         * @brief Register custom third-party metadata.
         * @param frn The Field Reference Number to expand.
         * @param data Pointer to the virtual class metadata.
         */
        void expand_parameter(uint8_t frn, asterix_uap_expansion* data)
        {
            custom_expansions[frn] = data;
        }

        /**
         * @brief Retrieve the specification for a given FRN in O(1) time.
         * @param frn The Field Reference Number.
         * @return Pointer to the spec if found, nullptr otherwise.
         */
        inline const asterix_field_spec* get_spec(uint8_t frn) const
        {
            return items_by_frn[frn];
        }

        /**
         * @brief Retrieve the custom expansion metadata for a given FRN in O(1) time.
         * @param frn The Field Reference Number.
         * @return Pointer to the metadata if found, nullptr otherwise.
         */
        inline asterix_uap_expansion* get_expansion(uint8_t frn) const
        {
            return custom_expansions[frn];
        }
    };

    /**
     * @class uap_pool
     * @brief Global singleton registry for mapping Asterix Category IDs to their UAP definitions.
     */
    class uap_pool
    {
    public:
        /**
         * @brief Get the singleton instance of the UAP pool.
         */
        static ADAM_ASTERIX_API uap_pool& get_instance();

        /**
         * @brief Register a UAP into the global pool.
         * @param uap Pointer to the UAP definition.
         */
        ADAM_ASTERIX_API void register_uap(const asterix_uap* uap);

        /**
         * @brief Retrieve a UAP by its Category ID in O(1) time.
         * @param cat_id The Category ID.
         * @return Pointer to the UAP if found, nullptr otherwise.
         */
        ADAM_ASTERIX_API const asterix_uap* get_uap(uint8_t cat_id) const;

    private:
        uap_pool(); // Private constructor for self-initialization
        ~uap_pool() = default;

        // Prevent copying
        uap_pool(const uap_pool&) = delete;
        uap_pool& operator=(const uap_pool&) = delete;

        const asterix_uap* registered_uaps[256];
    };
}
