#include "configuration/configuration-item.hpp"
#include "version/version.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include <fstream>
#include <string>

namespace adam 
{
    static constexpr uint32_t configuration_file_magic = 0xadacf116;

    configuration_item::configuration_item(const string_hashed& item_name, const configuration_parameter_list& default_params)
        : m_parameters(item_name)
    {
        add_parameters(default_params);
    }

    configuration_item::~configuration_item() {}

    bool configuration_item::save(string_hashed::view filepath) const 
    {
        std::ofstream ofs(std::string(filepath), std::ios::binary);
        if (!ofs) return false;

        configuration_parameter::write_binary(ofs, configuration_file_magic);
        configuration_parameter::write_binary(ofs, decode_version(sdk_version));

        configuration_parameter::serialize(ofs, &m_parameters);
        return ofs.good();
    }

    bool configuration_item::load(string_hashed::view filepath) 
    {
        std::ifstream ifs(std::string(filepath), std::ios::binary);
        if (!ifs) return false;

        uint32_t magic;
        configuration_parameter::read_binary(ifs, magic);
        if (magic != configuration_file_magic) return false;

        version_info ver;
        configuration_parameter::read_binary(ifs, ver);
        uint32_t loaded_version = make_version(ver.major, ver.minor, ver.patch);
        if (get_major(loaded_version) > get_major(sdk_version)) return false;

        auto loaded_root = configuration_parameter::deserialize(ifs);
        if (!loaded_root || loaded_root->get_type() != configuration_parameter::list) return false;

        auto* root_list = static_cast<configuration_parameter_list*>(loaded_root.get());
        for (auto& [name, param] : root_list->get_children())
        {
            if (auto* existing = m_parameters.get(name))
            {
                if (auto* e_bool = dynamic_cast<configuration_parameter_boolean*>(existing)) {
                    if (auto* p_bool = dynamic_cast<configuration_parameter_boolean*>(param.get())) e_bool->set_value(p_bool->get_value());
                } else if (auto* e_int = dynamic_cast<configuration_parameter_integer*>(existing)) {
                    if (auto* p_int = dynamic_cast<configuration_parameter_integer*>(param.get())) e_int->set_value(p_int->get_value());
                } else if (auto* e_dbl = dynamic_cast<configuration_parameter_double*>(existing)) {
                    if (auto* p_dbl = dynamic_cast<configuration_parameter_double*>(param.get())) e_dbl->set_value(p_dbl->get_value());
                } else if (auto* e_str = dynamic_cast<configuration_parameter_string*>(existing)) {
                    if (auto* p_str = dynamic_cast<configuration_parameter_string*>(param.get())) e_str->set_value(p_str->get_value());
                } else if (auto* e_ref = dynamic_cast<configuration_parameter_reference*>(existing)) {
                    if (auto* p_ref = dynamic_cast<configuration_parameter_reference*>(param.get())) e_ref->set_target(p_ref->get_target());
                }
            }
        }

        return ifs.good();
    }

    void configuration_item::set_name(const string_hashed& new_name)
    {
        m_parameters.set_name(new_name);
    }

    void configuration_item::add_parameters(const configuration_parameter_list& params)
    {
        for (const auto& [name, param] : params.get_children())
        {
            if (param) m_parameters.add(param->clone());
        }
    }
}