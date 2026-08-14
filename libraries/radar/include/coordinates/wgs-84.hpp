/**
 * @file    wgs-84.hpp
 * @author  dexus1337
 * @brief   Structs and functions for the WGS84 Coordiante System
 * @version 1.0
 * @date    14.08.2026
 */


#include <adam-sdk.hpp>


namespace adam::lib::radar::coord
{
    struct wgs84 
    {
        static ADAM_CONSTEXPR double semi_major_axis       = 6378137.0;
        static ADAM_CONSTEXPR double flattenging           = 1.0 / 298.257223563;
        static ADAM_CONSTEXPR double semi_minor_axis       = semi_major_axis * (1.0 - flattenging);
        static ADAM_CONSTEXPR double eccentricity_squared  = flattenging * (2.0 - flattenging);

        double latitude;       // [-90, 90]     degree
        double longitude;      // [-180, 180]   degree
        double altitude;       //               m

        
    };
}