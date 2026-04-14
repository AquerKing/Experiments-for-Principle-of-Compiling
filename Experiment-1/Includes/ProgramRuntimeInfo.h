#pragma once

#include <cstdint>
#include <vector>

struct ProgramRuntimeInfo {
  std::vector<std::vector<uint8_t>> KeywordList;
  std::vector<std::vector<uint8_t>> SeparatorList;
  std::vector<std::vector<uint8_t>> ArithmeticOperatorList;
  std::vector<std::vector<uint8_t>> RelationalOperatorList;

  enum class ArgumentBitFlag : uint8_t {
    ConfigFilePath = 1 << 0,
    InputFilePath = 1 << 1,
    ConfigReady = 1 << 2,
    StateMachinesBuilt = 1 << 3,
  };

  uint8_t ArgumentFlag = 0;

  bool HasArgument(ArgumentBitFlag Flag) const {
    return (ArgumentFlag & static_cast<uint8_t>(Flag)) != 0;
  }
};

extern ProgramRuntimeInfo RuntimeInfo;
