#pragma once

#include <string>

namespace lex {

class IStringConvertable {
  /**
   * Converts the object to a string representation.
   * @return The string representation of the object.
   */
  virtual std::string ToString() const = 0;
};

} // namespace lex