#include "data/parser.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

namespace adam 
{
    const configuration_parameter_list& parser::get_default_parameters()
    {
        static const configuration_parameter_list params = []()
        {
            configuration_parameter_list p;
            auto user_params = std::make_unique<configuration_parameter_list_sorted>("user_parameters"_ct);
            p.add(std::move(user_params));
            return p;
        }();

        return params;
    }

    parser::parser(const string_hashed& item_name, const configuration_parameter_list& default_params)
        : configuration_item(item_name, default_params)
    {
    }
}
