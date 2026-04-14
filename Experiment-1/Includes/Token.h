#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <vector>

#include "Interfaces.h"

enum class TokenType : uint8_t {
  Invalid,
  Keyword,
  Separator,
  ArithmeticOperator,
  RelationalOperator,
  UnsignedNumber,
  Token,
  Error,
};

struct TokenContextInfo : public IPrintable {
  static TokenContextInfo Invalid;

  uint64_t Row, Column;

  TokenContextInfo(const uint64_t NewRow = 0, const uint64_t NewColumn = 0)
      : Row(NewRow), Column(NewColumn) {}

  bool operator==(const TokenContextInfo &Other) {
    return Row == Other.Row && Column == Other.Column;
  }

  void UpdateInfo(const uint64_t NewRow, const uint64_t NewColumn) {
    Row = NewRow;
    Column = NewColumn;
  }

  std::string ToString() const override {
    std::ostringstream oss;
    oss << "(" << std::left << std::setw(4) << Row << "," << std::left
        << std::setw(4) << Column << ")";
    return oss.str();
  }
};

struct Token {
  Token() = default;
  Token(TokenType Type, TokenContextInfo Context,
        std::vector<uint8_t> &Sequence)
      : Type(Type), Context(Context), Content(Sequence) {}

  bool IsValid() const { return Type != TokenType::Invalid; }

  TokenType Type;
  TokenContextInfo Context;
  std::vector<uint8_t> Content;
};

inline std::basic_ostream<char, std::char_traits<char>> &
operator<<(std::basic_ostream<char, std::char_traits<char>> &__os,
           const Token &Token) {
  for (uint8_t Char : Token.Content) {
    __os << static_cast<char>(Char);
  }
  return __os;
}
