#pragma once

/**
 * @file    adam-sdk.hpp
 * @author  dexus1337
 * @brief   Umbrella header for the ADAM SDK, including all necessary headers for using the SDK in external applications and modules.
 * @version 1.0
 * @date    25.04.2026
 */

#include "api/api.hpp"
#include "configuration/item/configuration-item.hpp"
#include "configuration/parameter/configuration-parameter.hpp"
#include "controller/controller.hpp"
#include "commander/command.hpp"
#include "controller/response/response.hpp"
#include "commander/commander.hpp"
#include "data/format/data-format.hpp"
#include "data/format/parser/parser.hpp"
#include "data/format/serializer/serializer.hpp"
#include "data/processor/data-processor.hpp"
#include "data/processor/filter/filter.hpp"
#include "data/processor/converter/converter.hpp"
#include "memory/shared/memory-shared.hpp"
#include "memory/shared/memory-shared-signaled.hpp"
#include "memory/buffer/memory-buffer.hpp"
#include "memory/manager/memory-manager.hpp"
#include "module/module.hpp"
#include "port/port.hpp"
#include "port/input/port-input.hpp"
#include "port/output/port-output.hpp"
#include "string/string-hashed.hpp"
#include "queue/queue-shared.hpp"
#include "queue/queue-shared-duplex.hpp"
#include "version/version.hpp"
#include "logger/log.hpp"
#include "logger/logger.hpp"
#include "os/os.hpp"