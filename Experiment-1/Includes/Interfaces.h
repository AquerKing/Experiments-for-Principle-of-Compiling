#pragma once

#include <string>

class IPrintable {
  virtual std::string ToString() const = 0;
};