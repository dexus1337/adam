#pragma once

/**
 * @file    port-internal.hpp
 * @author  dexus1337
 * @brief   Defines a in-out internal port for passing data between connections directly
 * @version 1.0
 * @date    17.05.2026
 */

 
#include "api/api-sdk.hpp"
#include "data/port-types/port-in-out.hpp"


namespace adam 
{
    /**
     * @class port_internal
     * @brief Defines an InOut internal port for passing data between connections directly
     */
    class ADAM_SDK_API port_internal : public port_in_out
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "internal"_ct; }

        /** @brief Constructs a new internal port object. */
        port_internal(const string_hashed& item_name);

        /** @brief Destroys the internal port object and cleans up resources. */
        ~port_internal();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };
        
        /** @brief Data management routine */
        virtual bool handle_data(buffer* buf, data_direction dir) override;

        /** @brief Needs to be overwritten. Since this is an internal port, it does not read or write data on its own */
        virtual bool read(buffer*& buff) override { (void)buff; return false;}

        /** @brief Needs to be overwritten. Since this is an internal port, it does not read or write data on its own */
        virtual bool write(buffer* buff) override { (void)buff; return false;}
    };
}