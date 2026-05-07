#pragma once

/**
 * @file    adam-sdk.hpp
 * @author  dexus1337
 * @brief   Umbrella header for the ADAM SDK, including all necessary headers for using the SDK in external applications and modules.
 * @version 1.0
 * @date    25.04.2026
 */

#include "api/api.hpp"
#include "configuration/configuration-item.hpp"
#include "configuration/parameters/configuration-parameter.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-boolean.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "controller/controller.hpp"
#include "commander/command.hpp"
#include "controller/response.hpp"
#include "controller/registry.hpp"
#include "commander/commander.hpp"
#include "data/format.hpp"
#include "data/parser/parser.hpp"
#include "data/serializer/serializer.hpp"
#include "data/processor.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "data/port/port.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/connection.hpp"
#include "data/inspector.hpp"
#include "memory/memory.hpp"
#include "memory/memory-signaled.hpp"
#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "module/module.hpp"
#include "string/string-hashed.hpp"
#include "string/string-hashed-ct.hpp"
#include "queue/queue-shared.hpp"
#include "queue/queue-shared-duplex.hpp"
#include "version/version.hpp"
#include "logger/log.hpp"
#include "logger/logger.hpp"
#include "logger/logger-sink.hpp"
#include "os/os.hpp"