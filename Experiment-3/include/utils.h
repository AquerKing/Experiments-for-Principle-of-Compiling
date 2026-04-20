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
  ParseGenerativeExpressions(const std::string &Input,
                             SymbolManager &SymbolManager);

  static std::vector<uint64_t>
  ParseSymbolSequence(const std::string_view &Input,
                      SymbolManager &SymbolManager);
};

class SymbolUtils final {
public:
  SymbolUtils() = delete;
  ~SymbolUtils() = delete;
  SymbolUtils(const SymbolUtils &) = delete;
  SymbolUtils &operator=(const SymbolUtils &) = delete;
  SymbolUtils(SymbolUtils &&) = delete;
  SymbolUtils &operator=(SymbolUtils &&) = delete;

public:
  static std::string SymbolSequenceToString(const std::vector<uint64_t> &Seq,
                                            const SymbolManager &SymbolManager);
};