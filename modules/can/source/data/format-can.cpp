#include "data/format-can.hpp"
#include "data/can-analyzer.hpp"
#include "data/can-parser.hpp"

/**
 * @file    format-can.cpp
 * @author  dexus1337
 * @brief   Defines the CAN data format registration with parser and analyzer.
 * @version 1.0
 * @date    24.08.2026
 */

namespace adam::modules::can
{
    using namespace adam::string_hashed_ct_literals;

    static adam::default_factory<adam::parser, can_parser> can_parser_factory;

    data_format data_format_can = data_format("can"_ct, &can_parser_factory, nullptr, new can_analyzer(), get_adam_module());
}