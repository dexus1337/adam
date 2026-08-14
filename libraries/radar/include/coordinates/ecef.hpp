/**
 * @file    ecef.hpp
 * @author  dexus1337
 * @brief   Structs and functions for the ECEF (Carthesian-) Coordiante System
 * @version 1.0
 * @date    14.08.2026
 */


namespace adam::lib::radar::coord
{
    struct ecef 
    {
        double x;
        double y;
        double z;
    };
}