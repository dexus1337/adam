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
#include <cstring>

#include "data/format.hpp"
#include "version/version.hpp"
#include "resources/language.hpp"
#include "controller/registry.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"

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
        friend class registry_module_manager; /**< The registry_module_manager class is declared as a friend to allow it to access the protected members of the module class for managing module lifecycles and interactions. */
        friend class module_view;              /**< The module_view class is declared as a friend to allow it to access the protected members of the module class for managing the local view of modules in the commander. */

    public:

        typedef adam::module* (*get_adam_module_fn)();                              /**< A function pointer type for the module entry point function that modules must export to provide access to their module instance. */
        static constexpr string_hashed_ct entry_point_name = "get_adam_module";     /**< The name of the entry point function that modules must export to provide access to their module instance. */

        #pragma pack(push, 1) // align to 1 byte
        struct basic_info
        {
            enum incompat_reason : uint8_t
            {
                incompat_reason_unknown     = 0,
                incompat_reason_sdk_too_old = 1,
            } rsn;
            enum status : uint8_t
            {
                available = 0,
                unavailable = 1,
                loaded = 2
            } stat;
            char name[max_name_length];      /**< The name of the module. */
            char path[max_path_length];      /**< The file path to the module's shared library. */
            uint32_t version;                /**< The version of the module. */

            void setup(status s, const char* n, const char* p, uint32_t v, uint8_t r = incompat_reason_unknown)
            {
                stat = s;
                std::strncpy(name, n, sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
                std::strncpy(path, p, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
                version = v;
                rsn = static_cast<incompat_reason>(r);
            }
        };
        #pragma pack(pop)
        static_assert(sizeof(module::basic_info) <= command::get_max_data_length(), "module::basic_info exceeds maximum command data size");

        /** @brief Constructs a new module object. */
        module(const string_hashed& name, uint32_t version = adam::make_version(1, 0, 0), uint32_t req_sdk_ver = adam::sdk_version);

        /** @brief Destroys the module object and cleans up resources. */
        ~module();

        const string_hashed&                    get_name()                     const { return m_str_name; }
        const string_hashed&                    get_filepath()                 const { return m_str_filepath; }
        const std::string&                      get_description(language lang) const { return m_descriptions[static_cast<size_t>(lang)]; }
        uintptr_t                               get_module_handle()            const { return m_mod_handle; }
        uint32_t                                get_required_sdk_version()     const { return m_ui32_req_sdk_version; }
        uint32_t                                get_version()                  const { return m_ui32_version; }
        const registry::data_format_map&        get_data_formats()             const { return m_data_formats; }
        const registry::port_factory_map&       get_port_factories()           const { return m_port_factories; }
        const registry::filter_factory_map&     get_filter_factories()         const { return m_filter_factories; }
        const registry::converter_factory_map&  get_converter_factories()      const { return m_converter_factories; }

    protected:

        string_hashed                   m_str_name;                 /**< The name of the module, used for identification and lookup in the ADAM system. */
        string_hashed                   m_str_filepath;             /**< The file path of the module's shared library, used for loading and unloading the module. */
        uintptr_t                       m_mod_handle;               /**< The handle to the loaded module's shared library, used for managing the module's lifecycle. */
        uint32_t                        m_ui32_req_sdk_version;     /**< The minimum SDK version required for this module to function correctly. */
        uint32_t                        m_ui32_version;             /**< The version of the module. */
        registry::data_format_map       m_data_formats;             /**< A map of data formats supported by this module, indexed by their hashed string names for efficient lookup. */
        registry::port_factory_map      m_port_factories;           /**< A map of port factories provided by this module. */
        registry::filter_factory_map    m_filter_factories;         /**< A map of filter factories provided by this module. */
        registry::converter_factory_map m_converter_factories;      /**< A map of converter factories provided by this module. */

        std::array<std::string, static_cast<size_t>(languages_count)> m_descriptions; /**< The multi-language description of the module. */
    };
}
