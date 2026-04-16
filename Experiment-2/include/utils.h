#pragma once

#include "grammar.h"
#include "symbols.h"
#include <string_view>

class CharacterUtils final {
public:
  static bool IsUpperCase(char c);
  static bool IsLowerCase(char c);
};



class GrammarUtils final {
public:
  GrammarUtils() = delete;
  ~GrammarUtils() = delete;
  GrammarUtils(const GrammarUtils &) = delete;
  GrammarUtils &operator=(const GrammarUtils &) = delete;
  GrammarUtils(GrammarUtils &&) = delete;
  GrammarUtils &operator=(GrammarUtils &&) = delete;

public:
  static std::vector<GenerativeExpression>
  ParseGenerativeExpressions(const std::string &input,
                             SymbolManager &symbolManager);

  static std::vector<uint64_t>
  ParseSymbolSequence(const std::string_view &input, SymbolManager &symbolManager);
};