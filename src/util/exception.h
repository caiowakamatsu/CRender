#pragma once

#include <util/logger.h>

#include <cstdlib>

namespace cr
{
    inline void exit(std::string_view reason, int error_code = 1)
    {
        fmt::print("Exiting program with fatal error: {}", reason);
        std::exit(error_code);
    }
}
