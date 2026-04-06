#pragma once

#include "interfaces.h"

namespace Lexical {

struct Token {
    struct Position : public IStringConvertable {
        typedef unsigned long long u64;
        u64 row, column;
    };
};

} // namespace Lexical