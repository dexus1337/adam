#pragma once

/**
 * @file    factory.hpp
 * @author  dexus1337
 * @brief   Defines a generic factory template for creating instantiated components (ports, filters, converters).
 * @version 1.0
 * @date    12.05.2026
 */

#include "types/string-hashed.hpp"

namespace adam 
{
    /**
     * @class factory
     * @brief A generic factory interface for creating configurable items like ports, filters, and converters.
     */
    template <typename type>
    class factory 
    {
    public:
        virtual ~factory() = default;

        /**
         * @brief Creates a new instance of the component.
         * @param name The unique name to assign to the new instance.
         * @return A pointer to the newly created instance. The caller assumes ownership.
         */
        virtual type* create(const string_hashed& name) const = 0;
    };

    /**
     * @class default_factory
     * @brief A default implementation of the factory interface that automatically invokes the constructor of type_derived.
     */
    template <typename type_base, typename type_derived>
    class default_factory : public factory<type_base>
    {
    public:
        virtual ~default_factory() = default;

        type_base* create(const string_hashed& name) const override
        {
            return new type_derived(name);
        }
    };
}