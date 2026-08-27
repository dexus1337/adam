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

        inline float                      get_lat()   const { return static_cast<float>(m_p_lat->get_value()); }
        inline float                      get_lon()   const { return static_cast<float>(m_p_lon->get_value()); }
        inline const adam::string_hashed& get_label() const { return m_p_name->get_value(); }

        inline void                       set_lat(float lat)                         { m_p_lat->set_value(static_cast<double>(lat)); }
        inline void                       set_lon(float lon)                         { m_p_lon->set_value(static_cast<double>(lon)); }
        inline void                       set_label(const adam::string_hashed& name) { m_p_name->set_value(name); }

    protected:
        adam::configuration_parameter_double* m_p_lat  = nullptr;
        adam::configuration_parameter_double* m_p_lon  = nullptr;
        adam::configuration_parameter_string* m_p_name = nullptr;
    };
}
