#include "data/format-asterix.hpp"


#include "module/module-asterix.hpp"

#include "data/asterix-parser.hpp"
#include "data/asterix-encoder.hpp"
#include "data/asterix-analyzer.hpp"


namespace adam::modules::asterix
{
    static adam::default_factory<adam::parser, asterix_parser> asterix_parser_factory;
    static adam::default_factory<adam::encoder, asterix_encoder> asterix_encoder_factory;

    ADAM_ASTERIX_API data_format data_format_asterix = data_format
    (
        "asterix"_ct,
        &asterix_parser_factory,
        &asterix_encoder_factory,
        new asterix_analyzer(),
        get_adam_module()
    );
}