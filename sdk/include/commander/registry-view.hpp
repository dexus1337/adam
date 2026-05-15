#pragma once

/**
 * @file    registry-view.hpp
 * @author  dexus1337
 * @brief   Defines views for controller registry elements.
 * @version 1.0
 * @date    13.05.2026
 */

#include "api/sdk-api.hpp"
#include "types/string-hashed.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

namespace adam 
{
    /** @struct port_view */
    struct port_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed module_name;
        bool is_active = false;
    };

    /** @struct filter_view */
    struct filter_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed module_name;
    };

    /** @struct converter_view */
    struct converter_view
    {
        string_hashed name;
        string_hashed type;
        string_hashed module_name;
    };

    /** @struct connection_view */
    struct connection_view
    {
        string_hashed name;
        std::vector<string_hashed::hash_datatype> inputs;
        std::vector<string_hashed::hash_datatype> outputs;
        std::vector<string_hashed::hash_datatype> filters;
        std::vector<string_hashed::hash_datatype> converters;
        bool is_active = false;
    };

    /**
     * @class registry_view
     * @brief Holds a local view of the controller's registry elements.
     */
    class ADAM_SDK_API registry_view
    {
    public:
        using port_map       = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<port_view>>;
        using filter_map     = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<filter_view>>;
        using converter_map  = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<converter_view>>;
        using connection_map = std::unordered_map<string_hashed::hash_datatype, std::unique_ptr<connection_view>>;

        port_map&       ports()         { return m_ports; }
        filter_map&     filters()       { return m_filters; }
        converter_map&  converters()    { return m_converters; }
        connection_map& connections()   { return m_connections; }

        const port_map&       get_ports() const         { return m_ports; }
        const filter_map&     get_filters() const       { return m_filters; }
        const converter_map&  get_converters() const    { return m_converters; }
        const connection_map& get_connections() const   { return m_connections; }

        void clear()
        {
            m_ports.clear();
            m_filters.clear();
            m_converters.clear();
            m_connections.clear();
        }

    private:
        port_map       m_ports;
        filter_map     m_filters;
        converter_map  m_converters;
        connection_map m_connections;
    };
}