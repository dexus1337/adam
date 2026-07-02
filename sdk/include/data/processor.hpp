#pragma once

/**
 * @file    processor.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"
#include "data/registry-item.hpp"
#include "memory/buffer/buffer.hpp"
#include "commander/messages/message-structs.hpp"
#include "commander/messages/command.hpp"
#include "types/vector-double-buffer.hpp"

namespace adam 
{
    class data_format;
    class connection;

    /**
     * @class processor
     * @brief A base class for data format processors, providing a common interface for processing data in different formats used in the ADAM system.
     */
    class ADAM_SDK_API processor : public registry_item
    {
    public:

        friend class registry;

        #pragma pack(push, 1)
        struct basic_info
        {
            char            name[max_name_length];
            bool            is_unavailable;
            string_hash     type;
            string_hash     type_module;
            string_hash     input_datatype;
            string_hash     input_datatype_module;
            string_hash     output_datatype;
            string_hash     output_datatype_module;
            buffer_handle   state_buffer_handle;
            uint16_t        user_parameters;

            void setup(const string_hashed& n, string_hash t, string_hash tm, bool unavail = false, buffer_handle bh = buffer_handle())
            {
                type = t;
                type_module = tm;
                is_unavailable = unavail;
                state_buffer_handle = bh;
                user_parameters = 0;
                input_datatype = 0;
                input_datatype_module = 0;
                output_datatype = 0;
                output_datatype_module = 0;
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
            }
        };
        #pragma pack(pop)
        static_assert(sizeof(processor::basic_info) <= command::get_max_data_length(), "processor::basic_info exceeds maximum command data size");

        struct unavailable_info : public configuration_item
        {
            string_hash type;
            string_hash type_module;
            bool        is_filter;

            unavailable_info(const string_hashed& item_name) : configuration_item(item_name), type(0), type_module(0), is_filter(false) { }
        };

        struct state_buffer_data
        {
            uint64_t total_buffers_handled;
            uint64_t total_bytes_handled;
            uint64_t total_buffers_discarded;
            uint64_t total_bytes_discarded;
            uint8_t  user_data_array[1];

            template<typename T> T& user_data() { return *reinterpret_cast<T*>(user_data_array); }
        };

        /** @brief Retrieves the default configuration parameters for processors. */
        static const configuration_parameter_list& get_default_parameters();

        /** @brief Destroys the data processor object and cleans up resources. */
        virtual ~processor();

        vector_double_buffer<connection*>& connections() { return m_connections; }
        
        const data_format* get_input_format()  const { return m_format_input; }
        const data_format* get_output_format() const { return m_format_output; }

        buffer*             get_state_buffer()      const { return m_state_buffer; }
        state_buffer_data*  get_state_buffer_data() const { return m_state_buffer->data_as<state_buffer_data>(); }

        virtual const string_hashed_ct& get_type_name() const = 0;

        /** @brief Data management routine, arrives here, and may be changed to another buffer */
        virtual bool handle_data(buffer*& buf) = 0;

    protected:

        /** @brief Constructs a new data processor object. */
        processor(const string_hashed& item_name, uint32_t state_buffer_size = (sizeof(state_buffer_data) / sizeof(uintptr_t) + 1) * sizeof(uintptr_t));

        const data_format* m_format_input;     /**< The data format of input data. */
        const data_format* m_format_output;    /**< The data format of data after leaving this class. */

        buffer* m_state_buffer;                /**< The state buffer of the processor. */
        vector_double_buffer<connection*> m_connections; /**< Connections where this processor is assigned to. */
    };
}