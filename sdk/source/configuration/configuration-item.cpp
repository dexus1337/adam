#include "configuration/configuration-item.hpp"
#include "version/version.hpp"
#include <fstream>
#include <string>

namespace adam 
{
    static constexpr uint32_t configuration_file_magic = 0xadacf116;

    configuration_item::configuration_item(const string_hashed& item_name, const configuration_parameter_list& default_params)
        : m_parameters(item_name)
    {
        for (const auto& [name, param] : default_params.get_children())
        {
            if (param) m_parameters.add(param->clone());
        }
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
        m_parameters.clear();
        for (auto& [name, param] : root_list->get_children())
            m_parameters.add(std::move(param));

        return ifs.good();
    }
}