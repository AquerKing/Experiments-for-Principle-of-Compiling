#pragma once

#include <cstdint>
#include <string>
#include <vector>

inline std::vector<uint32_t> ConvertStringToU32Vector(std::string Str) {
  std::vector<uint32_t> Result;
  Result.reserve(Str.size());
  for (char Ch : Str) {
    Result.emplace_back(static_cast<uint32_t>(Ch));
  }
  return Result;
}

inline std::string ConvertU32VectorToString(const std::vector<uint32_t> &Vec) {
  std::string Result;
  Result.reserve(Vec.size());
  for (uint32_t Ch : Vec) {
    Result.push_back(static_cast<char>(Ch));
  }
  return Result;
}