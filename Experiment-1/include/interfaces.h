#pragma once

#include <string>

namespace Lexical {

class IStringConvertable {
    virtual std::string ToString() const = 0;
};

} // namespace Lexical