#pragma once

/**
 * @file    port-file.hpp
 * @author  dexus1337
 * @brief   Defines a template base class for file-based ports in the recrep module.
 * @version 1.0
 * @date    01.09.2026
 */


#include "api/api-recrep.hpp"

#include <adam-core.hpp>

#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>


namespace adam::modules::recrep
{
    /**
     * @class port_file_base
     * @brief Non-template helper for shared file port logic, default storage path, and format maps.
     */
    class ADAM_RECREP_API port_file_base
    {
    public:

        static const std::unordered_map<string_hash, string_hashed_ct>& get_format_map();
        static std::string_view                                         get_extension_for_format(const string_hashed& format);
        static string_hash                                              resolve_file_format(const std::string& path, const string_hashed& requested_format);
        static configuration_parameter_string::presets_container        create_data_format_presets(bool include_any = false);

        static string_hashed                                            get_default_storage_path();
        static std::string                                              resolve_path(const string_hashed& base_path, const std::string& file_name);
    };

    /**
     * @class port_file
     * @brief Template base class for file-based input and output ports.
     * @tparam port_base Either port_input or port_output.
     */
    template<typename port_base>
    class port_file : public port_base
    {
    public:

        static inline const std::unordered_map<string_hash, string_hashed_ct>& get_format_map() { return port_file_base::get_format_map(); }
        static inline string_hashed get_default_storage_path()                                  { return port_file_base::get_default_storage_path(); }
        static inline std::string_view get_extension_for_format(const string_hashed& format)    { return port_file_base::get_extension_for_format(format); }

        port_file(const string_hashed& item_name, uint32_t state_buffer_size = (sizeof(port::state_buffer_data) / sizeof(uintptr_t) + 1) * sizeof(uintptr_t))
         :  port_base(item_name, state_buffer_size)
        {
        }

        virtual ~port_file() = default;

        inline const string_hashed& get_path_value()                                const { return m_path_param->get_value(); }
        inline const string_hashed& get_data_format_value()                         const { return m_data_format_param->get_value(); }
        inline std::string_view     get_current_extension()                         const { return port_file_base::get_extension_for_format(get_data_format_value()); }
        inline std::string          resolve_file_path(const std::string& file_name) const { return port_file_base::resolve_path(get_path_value(), file_name); }

        inline void                 bind_file_parameters(adam::configuration_parameter_list* user_params)
        {
            if (!user_params)
                return;

            m_path_param        = user_params->get<adam::configuration_parameter_string>("path"_ct);
            m_data_format_param = user_params->get<adam::configuration_parameter_string>("data_format"_ct);
        }

    protected:

        adam::configuration_parameter_string* m_path_param        = nullptr;
        adam::configuration_parameter_string* m_data_format_param = nullptr;
    };
}

