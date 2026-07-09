#include "controller/registry-configuration-manager.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "version/version.hpp"

#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace adam 
{
    registry_configuration_manager::registry_configuration_manager(controller& ctrl)
        : m_controller(ctrl),
          m_config_name("Default Configuration"),
          m_config_description("Default system configuration"),
          m_config_created(static_cast<uint64_t>(std::time(nullptr))),
          m_config_modified(static_cast<uint64_t>(std::time(nullptr)))
    {
    }

    registry_configuration_manager::~registry_configuration_manager()
    {
    }

    const configuration_parameter_list* registry_configuration_manager::get_config_paths() const
    {
        return m_controller.get_registry().get_parameter<configuration_parameter_list>("config_paths"_ct);
    }

    bool registry_configuration_manager::add_config_path(const string_hashed& path, uint32_t* index)
    {
        auto* list = const_cast<configuration_parameter_list*>(get_config_paths());
        if (!list) return false;
        
        uint32_t max_idx = 0;
        for (const auto& [name, param] : list->get_children())
        {
            if (auto* str_param = dynamic_cast<configuration_parameter_string*>(param.get()))
                if (str_param->get_value() == path) return false;
            
            uint32_t idx = std::strtoul(name.c_str(), nullptr, 10);
            if (idx >= max_idx) max_idx = idx + 1;
        }
            
        uint32_t target_idx = max_idx;
        if (index && *index > 0 && *index < max_idx)
            target_idx = *index;

        if (index)
            *index = target_idx;

        for (uint32_t i = max_idx; i > target_idx; --i)
            list->rename_child(string_hashed(std::to_string(i - 1)), string_hashed(std::to_string(i)));
        
        auto new_param = std::make_unique<configuration_parameter_string>(string_hashed(std::to_string(target_idx)));
        new_param->set_value(path);
        list->add(std::move(new_param));
        return true;
    }

    bool registry_configuration_manager::remove_config_path(uint32_t index)
    {
        auto* list = const_cast<configuration_parameter_list*>(get_config_paths());
        if (!list) return false;

        uint32_t max_idx = 0;
        for (const auto& [name, param] : list->get_children())
        {
            uint32_t idx = std::strtoul(name.c_str(), nullptr, 10);
            if (idx >= max_idx) max_idx = idx + 1;
        }
        
        if (index == 0 || index >= max_idx) return false;
        
        list->remove(string_hashed(std::to_string(index)));
        for (uint32_t i = index + 1; i < max_idx; ++i)
            list->rename_child(string_hashed(std::to_string(i)), string_hashed(std::to_string(i - 1)));
        
        return true;
    }

    std::vector<registry_configuration_manager::config_file_info> registry_configuration_manager::scan_for_configs() const
    {
        std::vector<config_file_info> results;
        auto* config_paths = get_config_paths();
        if (!config_paths) return results;

        for (const auto& [name, param] : config_paths->get_children())
        {
            auto* str_param = dynamic_cast<configuration_parameter_string*>(param.get());
            if (!str_param) continue;

            const auto& directory = str_param->get_value();
            if (directory.empty()) continue;

            uint32_t path_idx = std::strtoul(name.c_str(), nullptr, 10);

            try 
            {
                if (std::filesystem::exists(directory.c_str()) && std::filesystem::is_directory(directory.c_str())) 
                {
                    for (const auto& entry : std::filesystem::directory_iterator(directory.c_str())) 
                    {
                        if (!entry.is_regular_file()) continue;
                        if (entry.path().extension() != ".adamcfg") continue;

                        std::ifstream ifs(entry.path(), std::ios::binary);
                        if (!ifs) continue;

                        uint32_t magic = 0;
                        configuration_parameter::read_binary(ifs, magic);
                        if (magic != 0xadacf116) continue;

                        version_info ver;
                        configuration_parameter::read_binary(ifs, ver);

                        config_file_info info;
                        info.path_idx = path_idx;
                        info.filename = entry.path().filename().string();

                        info.name = configuration_parameter::read_string(ifs).c_str();
                        info.description = configuration_parameter::read_string(ifs).c_str();
                        configuration_parameter::read_binary(ifs, info.created);
                        configuration_parameter::read_binary(ifs, info.modified);
                        configuration_parameter::read_binary(ifs, info.port_count);
                        configuration_parameter::read_binary(ifs, info.processor_count);
                        configuration_parameter::read_binary(ifs, info.connection_count);

                        results.push_back(std::move(info));
                    }
                }
            }
            catch (const std::filesystem::filesystem_error&)
            {
                continue;
            }
        }
        return results;
    }

    void registry_configuration_manager::write_header(std::ostream& os) const
    {
        configuration_parameter::write_string(os, m_config_name);
        configuration_parameter::write_string(os, m_config_description);
        configuration_parameter::write_binary(os, m_config_created);
        configuration_parameter::write_binary(os, m_config_modified);

        const auto& reg = m_controller.get_registry();
        uint32_t port_count = static_cast<uint32_t>(reg.get_ports().size() + reg.get_unavailable_ports().size());
        uint32_t processor_count = static_cast<uint32_t>(reg.get_processors().size() + reg.get_unavailable_processors().size());
        uint32_t connection_count = static_cast<uint32_t>(reg.get_connections().size() + reg.get_unavailable_connections().size());

        configuration_parameter::write_binary(os, port_count);
        configuration_parameter::write_binary(os, processor_count);
        configuration_parameter::write_binary(os, connection_count);
    }

    bool registry_configuration_manager::read_header(std::istream& is)
    {
        m_config_name = configuration_parameter::read_string(is);
        m_config_description = configuration_parameter::read_string(is);
        configuration_parameter::read_binary(is, m_config_created);
        configuration_parameter::read_binary(is, m_config_modified);

        uint32_t dummy_port_count;
        uint32_t dummy_processor_count;
        uint32_t dummy_connection_count;
        configuration_parameter::read_binary(is, dummy_port_count);
        configuration_parameter::read_binary(is, dummy_processor_count);
        configuration_parameter::read_binary(is, dummy_connection_count);
        return true;
    }

    bool registry_configuration_manager::load_config(uint32_t path_idx, const std::string& filename)
    {
        auto* str_param = dynamic_cast<configuration_parameter_string*>(get_config_paths()->get(string_hashed(std::to_string(path_idx))));
        if (!str_param) return false;

        std::filesystem::path full_path = std::filesystem::path(str_param->get_value().c_str()) / filename;
        return m_controller.get_registry().load(string_hashed(full_path.string()));
    }

    bool registry_configuration_manager::save_config(uint32_t path_idx, const std::string& filename, const std::string& name, const std::string& description)
    {
        m_config_name = name;
        m_config_description = description;
        m_config_modified = static_cast<uint64_t>(std::time(nullptr));

        if (path_idx == 0xFFFFFFFF)
            return m_controller.get_registry().save(string_hashed(filename));

        auto* str_param = dynamic_cast<configuration_parameter_string*>(get_config_paths()->get(string_hashed(std::to_string(path_idx))));
        if (!str_param) return false;

        std::filesystem::path full_path = std::filesystem::path(str_param->get_value().c_str()) / filename;
        return m_controller.get_registry().save(string_hashed(full_path.string()));
    }

    bool registry_configuration_manager::delete_config(uint32_t path_idx, const std::string& filename)
    {
        auto* str_param = dynamic_cast<configuration_parameter_string*>(get_config_paths()->get(string_hashed(std::to_string(path_idx))));
        if (!str_param) return false;

        std::filesystem::path full_path = std::filesystem::path(str_param->get_value().c_str()) / filename;
        
        std::error_code ec;
        if (!std::filesystem::exists(full_path, ec)) return false;

        return std::filesystem::remove(full_path, ec);
    }
}
