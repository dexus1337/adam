#include "data/format-asterix.hpp"


#include "module/module-asterix.hpp"

#include "data/asterix-parser.hpp"
#include "data/asterix-encoder.hpp"
#include "data/asterix-analyzer.hpp"


namespace adam::modules::asterix
{
    ADAM_ASTERIX_API data_format data_format_asterix = data_format("asterix", new asterix_parser(), new asterix_encoder(), new asterix_analyzer(), get_adam_module());
}