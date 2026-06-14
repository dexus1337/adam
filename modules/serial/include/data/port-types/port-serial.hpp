#pragma once

/**
 * @file    port-serial.hpp
 * @author  dexus1337
 * @brief   Defines an serial Port. In-Out for bidirectional data transfer.
 * @version 1.0
 * @date    30.05.2026
 */


#include "api/api-serial.hpp"
 
#include <adam-sdk.hpp>


namespace adam::modules::serial
{
    /**
     * @class port_serial
     * @brief Defines an serial Port. In-Out for bidirectional data transfer.
     */
    class ADAM_SERIAL_API port_serial : public port_in_out
    {
    public:

        static ADAM_CONSTEXPR string_hashed_ct type_name() { return "serial"_ct; }

        static const configuration_parameter_list& get_user_parameters();

        port_serial(const string_hashed& item_name);
        virtual ~port_serial();

        virtual const string_hashed_ct& get_type_name() const override { static string_hashed_ct name = type_name(); return name; };
        
        /** @brief Starts the port. Initializes Serial Communication */
        virtual bool start();

        /** @brief Stops the port. Cleans up resources used by Serial Communication */
        virtual bool stop();

        /** @brief Protoype function for data input */
        virtual bool read(buffer*& buff);

        /** @brief Protoype function for data output. */
        virtual bool write(buffer* buff);

    private:
        #if defined(ADAM_PLATFORM_WINDOWS)
        void* m_handle;
        #else
        int m_fd;
        #endif

        adam::configuration_parameter_integer* m_rttc = nullptr;
        adam::configuration_parameter_integer* m_rttm = nullptr;
        adam::configuration_parameter_integer* m_rit = nullptr;
    };
}