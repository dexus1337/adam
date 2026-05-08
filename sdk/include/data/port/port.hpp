#pragma once

/**
 * @file    port.hpp
 * @author  dexus1337
 * @brief   Defines a base class for ports, providing a common interface for handling data flow in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "configuration/configuration-item.hpp"
#include "types/vector-double-buffer.hpp"
#include <memory>

namespace adam 
{
    class buffer;
    class data_format;
    class connection;
    class data_inspector;

    /**
     * @class port
     * @brief A base class for ports, providing a common interface for handling data flow in the ADAM system.
     */
    class ADAM_SDK_API port : public configuration_item
    {
    public:

        /** @brief Destroys the port object and cleans up resources. */
        virtual ~port();

        const data_format* get_data_format() const { return m_data_format; }

        vector_double_buffer<connection*>&                      connections()   { return m_connections; }
        vector_double_buffer<std::shared_ptr<data_inspector>>&  inspectors()    { return m_inspectors; }

        /** @brief Data management routine */
        virtual bool handle_data(buffer* buffer);

    protected:

        /** @brief Constructs a new port object. */
        port(const string_hashed& item_name, const configuration_parameter_list& default_params = configuration_parameter_list());

        const data_format* m_data_format;                                       /**< The data format associated with this port, used for parsing/serializing data. */

        vector_double_buffer<connection*>                       m_connections;  /**< Each port needs to know where to send/recieve data from */
        vector_double_buffer<std::shared_ptr<data_inspector>>   m_inspectors;   /**< Zero or many data inspectors. All incoming data will be forwarded to them */

    };
}