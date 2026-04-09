#pragma once

#include <vector>

#include "Types.h"

inline std::vector<uchar> ConvertStringToUcharVector(std::string Str) {
  std::vector<uchar> Result;
  Result.reserve(Str.size());
  for (char Ch : Str) {
    Result.emplace_back(static_cast<uchar>(Ch));
  }
  return Result;
}