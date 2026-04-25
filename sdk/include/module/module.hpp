#pragma once

/**
 * @file        module.hpp
 * @author      dexus1337
 * @brief       A base class for any external module that can be loaded into the ADAM system, providing a common interface for initialization and cleanup.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include <unordered_map>

#include "data/format/data-format.hpp"

namespace adam 
{
    class input_port;
    class output_port;
    class filter;
    class converter;
    
    /**
     * @class   module
     * @brief   A base class for any external module that can be loaded into the ADAM system, providing a common interface for initialization and cleanup.
     */
    class ADAM_SDK_API module 
    {
    public:

        typedef const adam::module* (*get_module_fn)();                 /**< A function pointer type for the module entry point function that modules must export to provide access to their module instance. */
        static constexpr const char* entry_point_name = "get_module";   /**< The name of the entry point function that modules must export to provide access to their module instance. */

        using data_format_map = std::unordered_map<string_hashed, const data_format&>; /**< A type alias for a map of data formats supported by a module, indexed by their hashed string names for efficient lookup. */

        /**
         * @brief Constructs a new module object.
         */
        module(std::string_view name);

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module();

        const string_hashed& get_name()             const { return m_str_name; }
        int get_required_sdk_version()              const { return m_i_required_sdk_version; }
        const data_format_map& get_data_formats()   const { return m_data_formats; }

    protected:

        string_hashed   m_str_name;                 /**< The name of the module, used for identification and lookup in the ADAM system. */
        int             m_i_required_sdk_version;    /**< The minimum SDK version required for this module to function correctly. */
        data_format_map m_data_formats;             /**< A map of data formats supported by this module, indexed by their hashed string names for efficient lookup. */
    };
}