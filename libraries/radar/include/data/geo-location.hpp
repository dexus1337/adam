#pragma once

/**
 * @file    geo-location.hpp
 * @author  dexus1337
 * @brief   Base geographic location configuration item
 * @version 1.0
 * @date    09.08.2026
 */

#include <adam-core.hpp>

namespace adam::lib::radar
{
    class geo_location : public adam::configuration_item
    {
    public:
        geo_location(const adam::string_hashed& item_name, const adam::configuration_parameter_list& child_params = adam::configuration_parameter_list());
        virtual ~geo_location() = default;

        float get_lat() const;
        float get_lon() const;
        const adam::string_hashed& get_label() const; 

        void set_lat(float lat);
        void set_lon(float lon);
        void set_label(const adam::string_hashed& name);

    protected:
        void cache_geo_parameters();

        adam::configuration_parameter_double* m_p_lat = nullptr;
        adam::configuration_parameter_double* m_p_lon = nullptr;
        adam::configuration_parameter_string* m_p_name = nullptr;
    };
}
