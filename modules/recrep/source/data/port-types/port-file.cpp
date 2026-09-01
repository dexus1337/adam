#include "data/port-types/port-file.hpp"

#include <cstdlib>
#include <array>


namespace adam::modules::recrep
{
    struct format_entry
    {
        string_hashed_ct name;
        string_hashed_ct extension;
    };

    static constexpr std::array<format_entry, 2> supported_formats =
    {{
        { "pcap"_ct, ".pcap"_ct },
        { "rff"_ct,  ".rff"_ct  }
    }};

    const std::unordered_map<string_hash, string_hashed_ct>& port_file_base::get_format_map()
    {
        static const std::unordered_map<string_hash, string_hashed_ct> map = []()
        {
            std::unordered_map<string_hash, string_hashed_ct> m;
            for (const auto& entry : supported_formats)
                m.emplace(entry.extension.get_hash(), entry.name);
            return m;
        }();
        return map;
    }

    std::string_view port_file_base::get_extension_for_format(const string_hashed& format)
    {
        for (const auto& entry : supported_formats)
        {
            if (entry.name == format)
                return entry.extension.c_str();
        }
        return ".pcap";
    }

    string_hash port_file_base::resolve_file_format(const std::string& path, const string_hashed& requested_format)
    {
        const string_hashed ext(std::filesystem::path(path).extension().string());

        for (const auto& entry : supported_formats)
        {
            if (entry.extension != ext)
                continue;

            if (requested_format == "any"_ct || entry.name == requested_format)
                return entry.name.get_hash();

            return 0;
        }

        return 0;
    }

    configuration_parameter_string::presets_container port_file_base::create_data_format_presets(bool include_any)
    {
        configuration_parameter_string::presets_container presets = {};
        size_t index = 0;

        if (include_any)
        {
            presets.emplace(string_hashed(std::to_string(index++)), std::make_unique<adam::configuration_parameter_string>("any"_ct, "any"_ct));
        }

        for (const auto& entry : supported_formats)
        {
            presets.emplace(string_hashed(std::to_string(index++)), std::make_unique<adam::configuration_parameter_string>(entry.name, entry.name));
        }

        return presets;
    }

    string_hashed port_file_base::get_default_storage_path()
    {
        #if defined(ADAM_PLATFORM_ANDROID)
        const char* user_dir = std::getenv("ADAM_USER_DIR");
        if (user_dir && user_dir[0] != '\0')
        {
            std::string p = user_dir;
            if (p.back() != '/')
                p += '/';
            return string_hashed(p);
        }
        return "/storage/emulated/0/"_ct;
        #else
        return ""_ct;
        #endif
    }

    std::string port_file_base::resolve_path(const string_hashed& base_path, const std::string& file_name)
    {
        if (base_path.empty())
            return file_name;

        return (std::filesystem::path(base_path.c_str()) / file_name).string();
    }
}
