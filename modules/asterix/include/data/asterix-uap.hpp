#pragma once

/**
 * @file    asterix-uap.hpp
 * @author  dexus1337
 * @brief   Defines the User Application Profiles (UAP) structures for Asterix category definitions.
 * @version 1.0
 * @date    15.06.2026
 */

#include "data/asterix-types.hpp"
#include <adam-sdk.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <functional>

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

        using selector_function = std::function<const uap*(const raw_record_header* raw_head, uint8_t fspec_size, uint32_t raw_len)>;
    
        /**
         * @brief Constructs a UAP without alternatives and initializes the O(1) lookup arrays.
         * @param cat           The category ID.
         * @param name          The name of the UAP.
         * @param spec_array    Array of field specs.
         * @param spec_count    Number of elements in spec_array.
         * @param sacsic_frn    The FRN for the SAC/SIC field. Defaults to 0 if not specified.
         */
        uap(uint8_t cat, const string_hashed_ct& name, const field_spec* spec_array, size_t spec_count, uint8_t sacsic_frn = 0);

        /**
         * @brief Constructs a UAP with alternatives and initializes the O(1) lookup arrays.
         * @param cat           The category ID.
         * @param name          The name of the UAP.
         * @param spec_array    Array of field specs.
         * @param spec_count    Number of elements in spec_array.
         * @param alt_array     Array of alternative uaps.
         * @param alt_cout      Number of elements in alternatives.
         * @param sel           The selector function.
         * @param sacsic_frn    The FRN for the SAC/SIC field. Defaults to 0 if not specified.
         */
        uap(uint8_t cat, const string_hashed_ct& name, const field_spec* spec_array, size_t spec_count, const uap*const* alt_array, size_t alt_cout, selector_function sel, uint8_t sacsic_frn = 0);

        
        // Prevent copying
        uap(const uap&) = delete;
        uap& operator=(const uap&) = delete;

        inline const string_hashed_ct&  get_name()                  const { return m_name; }
        inline uint8_t                  get_cat_number()            const { return m_cat_id; }
        inline uint8_t                  get_sacsic_frn()            const { return m_sacsic_frn; }
        inline uint8_t                  get_highest_frn()           const { return m_last_frn; }
        inline const field_spec*        get_spec(uint8_t frn)       const { return m_items_by_frn[frn]; }
        inline uap_expansion*           get_expansion(uint8_t frn)  const { return m_custom_expansions[frn]; }
        inline bool                     has_alternatives()          const { return !m_alternatives.empty(); }

        inline const uap*               select_alternative(const raw_record_header* raw_head, uint8_t fspec_size, uint32_t raw_len) const 
        { 
            return m_selector_fn(raw_head, fspec_size, raw_len); 
        }

        inline const uap*               find_alternative(string_hash alt_name) const
        {
            auto it = m_alternatives.find(alt_name);
            if (it != m_alternatives.end())
                return it->second;
            return nullptr;
        }
        
        /** @brief Initializer. MUST be called before inserting this into the pool */
        void setup();

        /** @brief Register custom third-party metadata. */
        void expand_parameter(uint8_t frn, uap_expansion* data);

    protected:
    
        uint8_t                 m_cat_id;                                     /**< The category ID (e.g., 48, 62). */
        uint8_t                 m_sacsic_frn;                                 /**< The FRN for the SAC/SIC field. */
        string_hashed_ct        m_name;                                       /**< Name for printing or debugging */
        uint8_t                 m_last_frn;                                   /**< Highest registered FRN; Also determines fixed FSPEC octet count for explicit items. */
        uint16_t                m_subitem_count;                              /**< Summended last_frn of all children. */
        const field_spec*       m_items_by_frn[asterix::highest_frn];         /**< O(1) array mapping FRN to field spec. */
        uap_expansion*          m_custom_expansions[asterix::highest_frn];    /**< O(1) array mapping FRN to custom expansion metadata. */

        /** 
         * Some Categories (eg CAT001), may use different UAPs based on certain fields. 
         * To handle these automatically, the uap will have "alternatives" and
         * a "selector" function that will retrieve the correct alternative uap
         */
        std::unordered_map<string_hash, const uap*> m_alternatives;           /**< All available alternative uaps  */
        selector_function                           m_selector_fn;            /**< The selector function to choose from the alternatives, based on input data */

        /** @brief Compute the number of sub-items. */
        void compute_subitem_count();
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

        /** @brief Register a UAP into the global pool. Will call setup() on the given uap */
        void register_uap(uap* uap);

        /** @brief Get a UAP by its Category ID in O(1) time. */
        inline const uap* get_uap(uint8_t cat_id) const { return m_registered_uaps[cat_id]; }

        /** @brief Retrieves the correct (sub) UAP for a Record at data with a max size of data_size  */
        inline const uap* retrieve_uap(const raw_block_header* raw_block_head, const raw_record_header* raw_rec_head, uint8_t fspec_size, uint32_t raw_len)
        {
            const auto* cur_cap = get_uap(raw_block_head->category);

            if (!cur_cap) return nullptr;

            if (!cur_cap->has_alternatives()) return cur_cap;

            return cur_cap->select_alternative(raw_rec_head, fspec_size, raw_len);
        }

    private:
        uap_pool(); // Private constructor for self-initialization
        ~uap_pool() = default;

        // Prevent copying
        uap_pool(const uap_pool&) = delete;
        uap_pool& operator=(const uap_pool&) = delete;

        const uap* m_registered_uaps[256];
    };

    inline const uap* raw_block_header::get_uap() const
    {
        return uap_pool::get().get_uap(category);
    }
    
    inline const uap* raw_record_header::retrieve_uap(const raw_block_header* raw_block_head, uint8_t fspec_size, uint32_t raw_len) const
    {
        return uap_pool::get().retrieve_uap(raw_block_head, this, fspec_size, raw_len);
    }

    inline const uap* record::find_used_uap() const
    {
        auto base = uap_pool::get().get_uap(category);

        if (!base) return nullptr;

        if (base->get_name() == this->used_uap)
            return base;

        auto alt = base->find_alternative(used_uap);

        return alt;
    }
    
    inline const uap* block::get_uap() const
    {
        return uap_pool::get().get_uap(category);
    }
}
