#pragma once

/**
 * @file    processor.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"
#include "configuration/configuration-item.hpp"

namespace adam 
{
    class buffer;
    class data_format;

    /**
     * @class data_processor
     * @brief A base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
     */
    class ADAM_SDK_API data_processor : public configuration_item
    {
    public:

        /** @brief Destroys the data processor object and cleans up resources. */
        ~data_processor();
        
        const data_format* get_input_data_format()  const { return m_format_input; }
        const data_format* get_output_data_format() const { return m_format_output; }

        /** @brief Data management routine, arrives here, and may be changed to another buffer */
        virtual bool handle_data(buffer*& buffer) = 0;

    protected:

        /** @brief Constructs a new data processor object. */
        data_processor(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        const data_format* m_format_input;     /**< The data format of input data. */
        const data_format* m_format_output;    /**< The data format of data after leaving this class. */

    };
}