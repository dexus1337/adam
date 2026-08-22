#pragma once

/**
 * @file    parser.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format parsers, providing a common interface for parsing data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-core.hpp"
#include "configuration/configuration-item.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"

namespace adam 
{
    class buffer;

    /**
     * @class parser
     * @brief A base class for data format parsers, providing a common interface for parsing data in different formats used in the ADAM system.
     */
    class ADAM_CORE_API parser : public configuration_item
    {
    public:

        static const configuration_parameter_list& get_default_parameters();

        virtual ~parser() = default;

        virtual bool parse(class buffer* buf, class buffer*& internal_data) = 0;

    protected:

        parser(const string_hashed& item_name = "parser"_ct, const configuration_parameter_list& default_params = get_default_parameters());

    };
}