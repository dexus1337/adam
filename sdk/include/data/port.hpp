#pragma once

/**
 * @file    port.hpp
 * @author  dexus1337
 * @brief   Defines a base class for ports, providing a common interface for handling data flow in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"
#include "data/registry-item.hpp"
#include "types/vector-double-buffer.hpp"
#include "types/map-double-buffer.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"
#include "memory/buffer/buffer.hpp"

#include <cstdint>
#include <memory>
#include <cstring>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

namespace adam 
{

    class connection;
    class data_inspector;

    /**
     * @class port
     * @brief A base class for ports, providing a common interface for handling data flow in the ADAM system.
     * 
     *        Provides the following configuration parameters:
     *        - type:           The type of the port, used for identification and lookup in the ADAM system.
     * 
     */
    class ADAM_SDK_API port : public registry_item
    {
    public:
    
        enum direction : uint8_t
        {
            direction_invalid  = 0,
            direction_in       = 1 << 0,
            direction_out      = 1 << 1,
            direction_inout    = direction_in | direction_out
        };

        enum state : uint8_t
        {
            state_stopped,
            state_started,
            state_running,
            state_inactive
        };

        #pragma pack(push, 1)
        struct basic_info
        {
            char            name[max_name_length];
            direction       dir;
            bool            started;
            bool            is_unavailable;
            string_hash     type;
            string_hash     type_module;
            buffer_handle   state_buffer_handle;
            uint16_t        user_parameters;

            void setup(const string_hashed& n, string_hash t, string_hash tm, bool unavail = false, buffer_handle bh = buffer_handle())
            {
                type = t;
                type_module = tm;
                dir = direction_invalid;
                state_buffer_handle = bh;
                is_unavailable = unavail;
                started = false;
                is_unavailable = false;
                user_parameters = 0;
                std::strncpy(name, n.c_str(), sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
            }
        };
        #pragma pack(pop)
        static_assert(sizeof(port::basic_info) <= command::get_max_data_length(), "port::basic_info exceeds maximum command data size");

        #pragma pack(push, 1)
        struct status_event_info
        {
            string_hash port_hash;
        };
        #pragma pack(pop)
        static_assert(sizeof(port::status_event_info) <= command::get_max_data_length(), "port::status_event_info exceeds maximum command data size");

        struct unavailable_info : public configuration_item
        {
            string_hash type;
            string_hash type_module;

            unavailable_info(const string_hashed& item_name) : configuration_item(item_name), type(0), type_module(0) { }
        };

        struct state_buffer_data
        {
            state    cur_state;
            uint64_t total_buffers_handled;
            uint64_t total_bytes_handled;
            uint64_t total_buffers_discarded;
            uint64_t total_bytes_discarded;
            uint8_t  user_data_array[1];

            template<typename T> T& user_data() { return *reinterpret_cast<T*>(user_data_array); }
        };
        

        static const configuration_parameter_list& get_default_parameters();

        virtual ~port();

        virtual const string_hashed_ct& get_type_name() const = 0;
        virtual direction get_direction() const = 0;

        inline buffer*              get_state_buffer()      const { return m_state_buffer; }
        inline state_buffer_data*   get_state_buffer_data() const { return m_state_buffer->data_as<state_buffer_data>(); }

        inline state                get_state()             const { return get_state_buffer_data()->cur_state; }
        inline bool                 is_running()            const { return get_state_buffer_data()->cur_state == state_running; }
        inline bool                 is_started()            const { return m_started != nullptr && m_started->get_value(); }

        const vector_double_buffer<std::shared_ptr<data_inspector>>&  get_inspectors()        const { return m_inspectors; }
        const vector_double_buffer<connection*>&                      get_in_connections()    const { return m_in_connections; }
        const vector_double_buffer<connection*>&                      get_out_connections()   const { return m_out_connections; }

        void add_inspector(std::shared_ptr<data_inspector> inspector);
        void remove_inspector(std::shared_ptr<data_inspector> inspector);

        void add_as_connection_input(connection* conn);
        void remove_as_connection_input(connection* conn);

        void add_as_connection_output(connection* conn);
        void remove_as_connection_output(connection* conn);

        void rebuild_formats_database();

        virtual void reset_state_buffer();

        /** @brief Data management routine */
        virtual bool handle_data(buffer* buf, data_direction dir);

        /** @brief Starts the port. */
        virtual bool start();

        /** @brief Stops the port. */
        virtual bool stop();

        /** @brief Protoype function for data input */
        virtual bool read(buffer*& buff) = 0;

        /** @brief Protoype function for data output. */
        virtual bool write(buffer* buff) = 0;

    protected:

        /** @brief Worker function for threaded ports. */
        virtual void worker();

        void set_state(state cur) { get_state_buffer_data()->cur_state = cur; }

        port(const string_hashed& item_name, uint32_t state_buffer_size = (sizeof(state_buffer_data) / sizeof(uintptr_t) + 1) * sizeof(uintptr_t));

        bool                                                    m_b_threaded;       /**< Indicates whether this port runs in its own Thread or not */
        std::thread                                             m_thread;           /**< The thread object for threaded ports */

        vector_double_buffer<connection*>                       m_in_connections;   /**< Connections where this port acts as an input. */
        vector_double_buffer<connection*>                       m_out_connections;  /**< Connections where this port acts as an output. */
        vector_double_buffer<std::shared_ptr<data_inspector>>   m_inspectors;       /**< Zero or many data inspectors. All incoming data will be forwarded to them */

        map_double_buffer<string_hash, const data_format*>      m_formats;          /**< Database of unique data formats active on this port. */
        std::unordered_map<string_hash, buffer*>                m_parse_cache;      /**< Cache of parsed data formats active on this port. */

        buffer*                                                 m_state_buffer;     /**< A special buffer used for storing and sharing this port's runtime statistics, such as total buffers/bytes handled and current active state. The data format of this buffer is expected to be a simple binary blob matching the structure of port::state_buffer_data. */

        configuration_parameter_boolean*                        m_started;          /**< Cached pointer to the started parameter as it will be frequently accessed. */

        bool                                                    m_use_spinlock;     /**< If true, a spinlock will be used to protect the write method access. */
        std::atomic_flag                                        m_spinlock = ATOMIC_FLAG_INIT; /**< Spinlock used for write synchronization. */
    };
}