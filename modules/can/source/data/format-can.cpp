#include "data/format-can.hpp"

namespace adam::modules::can
{
    data_format data_format_can = data_format("can", nullptr, nullptr, nullptr, get_adam_module());
}