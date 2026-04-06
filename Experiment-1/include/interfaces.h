#pragma once

#include <string>

namespace Lexical {

class IStringConvertable {
    virtual std::string to_string() const = 0;
};

} // namespace Lexical