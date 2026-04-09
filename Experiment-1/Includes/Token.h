#pragma once

#include <iomanip>
#include <sstream>
#include <vector>

#include "Interfaces.h"
#include "Types.h"

enum class TokenType : u8 {
  Invalid,
  Keyword,
  Separater,
  ArithmeticOperator,
  RelationalOperator,
  UnsignedNumber,
  Token,
};

struct TokenContextInfo : public IPrintable {
  static TokenContextInfo Invalid;

  u64 Row, Column;

  TokenContextInfo(const u64 NewRow = 0, const u64 NewColumn = 0)
      : Row(NewRow), Column(NewColumn) {}

  bool operator==(const TokenContextInfo &Other) {
    return Row == Other.Row && Column == Other.Column;
  }

  void UpdateInfo(const u64 NewRow, const u64 NewColumn) {
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
  bool IsValid() const { return Type == TokenType::Invalid; }

  TokenType Type;
  TokenContextInfo Context;
  std::vector<uchar> Content;
};

inline std::basic_ostream<char, std::char_traits<char>> &
operator<<(std::basic_ostream<char, std::char_traits<char>> &__os,
           const Token &Token) {
  for (uchar Char : Token.Content) {
    __os << static_cast<char>(Char);
  }
  return __os;
}
