#include "data/format-can.hpp"
#include "data/can-analyzer.hpp"

namespace adam::modules::can
{
    data_format data_format_can = data_format("can", nullptr, nullptr, new can_analyzer(), get_adam_module());
}