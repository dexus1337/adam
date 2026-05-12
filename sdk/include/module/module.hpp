#pragma once

/**
 * @file    module.hpp
 * @author  dexus1337
 * @brief   A base class for any external module that can be loaded into the ADAM system, providing a common interface for initialization and cleanup.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"

#include <array>
#include <string>
#include <unordered_map>

#include "data/format.hpp"
#include "factory/factory.hpp"
#include "version/version.hpp"
#include "resources/language.hpp"

namespace adam 
{
    class port;
    class filter;
    class converter;
    
    /**
     * @class   module
     * @brief   A base class for any external module that can be loaded into the ADAM system, providing a common interface for initialization and cleanup.
     */
    class ADAM_SDK_API module 
    {
        friend class controller_module_manager;    /**< The controller_module_manager class is declared as a friend to allow it to access the protected members of the module class for managing module lifecycles and interactions. */

    public:

        typedef adam::module* (*get_adam_module_fn)();                                              /**< A function pointer type for the module entry point function that modules must export to provide access to their module instance. */
        static constexpr string_hashed_ct entry_point_name = "get_adam_module";                     /**< The name of the entry point function that modules must export to provide access to their module instance. */

        using data_format_map       = std::unordered_map<string_hashed, const data_format*>;        /**< A type alias for a map of data formats supported by a module, indexed by their hashed string names for efficient lookup. */
        using port_factory_map      = std::unordered_map<string_hashed, const factory<port>*>;      /**< A map of factories for creating ports provided by a module. */
        using filter_factory_map    = std::unordered_map<string_hashed, const factory<filter>*>;    /**< A map of factories for creating filters provided by a module. */
        using converter_factory_map = std::unordered_map<string_hashed, const factory<converter>*>; /**< A map of factories for creating converters provided by a module. */

        /** @brief Constructs a new module object. */
        module(const string_hashed& name, uint32_t version = adam::make_version(1, 0, 0), uint32_t req_sdk_ver = adam::sdk_version);

        /** @brief Destroys the module object and cleans up resources. */
        ~module();

        const string_hashed&         get_name()                     const { return m_str_name; }
        const string_hashed&         get_filepath()                 const { return m_str_filepath; }
        const std::string&           get_description(language lang) const { return m_descriptions[static_cast<size_t>(lang)]; }
        uintptr_t                    get_module_handle()            const { return m_mod_handle; }
        uint32_t                     get_required_sdk_version()     const { return m_ui32_req_sdk_version; }
        uint32_t                     get_version()                  const { return m_ui32_version; }
        const data_format_map&       get_data_formats()             const { return m_data_formats; }
        const port_factory_map&      get_port_factories()           const { return m_port_factories; }
        const filter_factory_map&    get_filter_factories()         const { return m_filter_factories; }
        const converter_factory_map& get_converter_factories()      const { return m_converter_factories; }

    protected:

        string_hashed           m_str_name;                 /**< The name of the module, used for identification and lookup in the ADAM system. */
        string_hashed           m_str_filepath;             /**< The file path of the module's shared library, used for loading and unloading the module. */
        uintptr_t               m_mod_handle;               /**< The handle to the loaded module's shared library, used for managing the module's lifecycle. */
        uint32_t                m_ui32_req_sdk_version;     /**< The minimum SDK version required for this module to function correctly. */
        uint32_t                m_ui32_version;             /**< The version of the module. */
        data_format_map         m_data_formats;             /**< A map of data formats supported by this module, indexed by their hashed string names for efficient lookup. */
        port_factory_map        m_port_factories;           /**< A map of port factories provided by this module. */
        filter_factory_map      m_filter_factories;         /**< A map of filter factories provided by this module. */
        converter_factory_map   m_converter_factories;      /**< A map of converter factories provided by this module. */

        std::array<std::string, static_cast<size_t>(languages_count)> m_descriptions; /**< The multi-language description of the module. */
    };
}
