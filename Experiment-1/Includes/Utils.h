#pragma once

#include <cstdint>
#include <vector>
#include <string>

inline std::vector<uint32_t> ConvertStringToU32Vector(std::string Str) {
  std::vector<uint32_t> Result;
  Result.reserve(Str.size());
  for (char Ch : Str) {
    Result.emplace_back(static_cast<uint32_t>(Ch));
  }
  return Result;
}