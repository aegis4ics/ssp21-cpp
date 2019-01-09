
#include "crypto/LogMessagePrinter.h"

#include "openpal/logging/LogMacros.h"
#include "openpal/logging/HexLogging.h"

#include <inttypes.h>

using namespace openpal;

namespace ssp21
{

    LogMessagePrinter::LogMessagePrinter(const Logger& logger, LogLevel level, uint32_t max_hex_bytes_per_line) :
        logger_(logger),
        level_(level),
        max_hex_bytes_per_line_(max_hex_bytes_per_line)
    {

    }

    void LogMessagePrinter::print(const char* message)
    {
        logger_.log(level_, LOCATION, message);
    }

    void LogMessagePrinter::print(const char* name, uint64_t value)
    {
        char message[max_log_entry_size];
        SAFE_STRING_FORMAT(message, max_log_entry_size, "%s: %" PRIu64, name, value);
        logger_.log(level_, LOCATION, message);
    }

    void LogMessagePrinter::print(const char* name, const char* value)
    {
        char message[max_log_entry_size];
        SAFE_STRING_FORMAT(message, max_log_entry_size, "%s: %s", name, value);
        logger_.log(level_, LOCATION, message);
    }

    void LogMessagePrinter::print(const char* name, const seq32_t& data)
    {
        char message[max_log_entry_size];
        SAFE_STRING_FORMAT(message, max_log_entry_size, "%s (length = %u)", name, data.length());
        logger_.log(level_, LOCATION, message);

        // TODO: Fix this
        //HexLogging::log(logger_, level_, data, ':', max_hex_bytes_per_line_, max_hex_bytes_per_line_);
    }

}
