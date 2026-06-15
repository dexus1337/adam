#include "module/module-serial.hpp"

#include "data/port-types/port-serial.hpp"


namespace adam::modules::serial
{
    static module_serial global_instance = module_serial();

    static default_factory<port, port_serial> global_port_serial_factory = default_factory<port, port_serial>();

    module_serial::module_serial() : module("serial", version)
    {
        m_descriptions[static_cast<size_t>(adam::language_english)] = 
            std::string("Provides support for communication over operating system serial ports.\n"
                        "This includes COM ports on Windows and tty devices on Linux.");
        m_descriptions[static_cast<size_t>(adam::language_german)]  = 
            std::string("Bietet Unterstützung für die Kommunikation über serielle Schnittstellen des Betriebssystems.\n"
                        "Dies umfasst COM-Ports unter Windows und tty-Geräte unter Linux.");

        // Export the factory for the controller to dynamically create this port type!
        m_port_factories.emplace
        (
            port_serial::type_name(), 
            registry::factory_data_port(&global_port_serial_factory, &port_serial::get_default_parameters(), port_serial::direction)
        );

    }

    module_serial::~module_serial() 
    {
        // Clean up resources if needed
    }
}

extern "C" adam::module* get_adam_module() 
{
    return &adam::modules::serial::global_instance;
}