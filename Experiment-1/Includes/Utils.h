#pragma once

#include "State.h"
#include "Token.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <iostream>

inline size_t Utf8CodePointLength(unsigned char LeadByte) {
  if ((LeadByte & 0x80) == 0) {
    return 1;
  }
  if ((LeadByte & 0xE0) == 0xC0) {
    return 2;
  }
  if ((LeadByte & 0xF0) == 0xE0) {
    return 3;
  }
  if ((LeadByte & 0xF8) == 0xF0) {
    return 4;
  }
  return 1;
}

inline uint32_t DecodeUtf8CodePoint(std::string_view Text, size_t &Index) {
  const auto *Bytes = reinterpret_cast<const unsigned char *>(Text.data());
  const size_t Length = Text.size();

  if (Index >= Length) {
    return 0;
  }

  const unsigned char LeadByte = Bytes[Index];
  const size_t SequenceLength = Utf8CodePointLength(LeadByte);
  if (SequenceLength == 1 || Index + SequenceLength > Length) {
    ++Index;
    return LeadByte;
  }

  uint32_t CodePoint = 0;
  if (SequenceLength == 2) {
    CodePoint = LeadByte & 0x1F;
  } else if (SequenceLength == 3) {
    CodePoint = LeadByte & 0x0F;
  } else {
    CodePoint = LeadByte & 0x07;
  }

  for (size_t Offset = 1; Offset < SequenceLength; ++Offset) {
    const unsigned char ContinuationByte = Bytes[Index + Offset];
    if ((ContinuationByte & 0xC0) != 0x80) {
      ++Index;
      return LeadByte;
    }
    CodePoint = (CodePoint << 6) | (ContinuationByte & 0x3F);
  }

  Index += SequenceLength;
  return CodePoint;
}

inline size_t GetUtf8DisplayWidth(std::string_view Text) {
  size_t Width = 0;
  for (size_t Index = 0; Index < Text.size();) {
    const uint32_t CodePoint = DecodeUtf8CodePoint(Text, Index);
    if (CodePoint <= 0x1F || (CodePoint >= 0x7F && CodePoint <= 0x9F)) {
      continue;
    }

    if (CodePoint < 0x80) {
      ++Width;
    } else {
      Width += 2;
    }
  }
  return Width;
}

inline void PrintPaddedCell(std::string_view Text, size_t ColumnWidth) {
  std::cout << Text;
  const size_t TextWidth = GetUtf8DisplayWidth(Text);
  if (TextWidth < ColumnWidth) {
    std::cout << std::string(ColumnWidth - TextWidth, ' ');
  }
}

inline std::vector<uint8_t> ConvertStringToU8Vector(std::string Str) {
  std::vector<uint8_t> Result;
  Result.reserve(Str.size());
  for (char Ch : Str) {
    Result.emplace_back(static_cast<uint8_t>(Ch));
  }
  return Result;
}

inline std::string ConvertU8VectorToString(const std::vector<uint8_t> &Vec) {
  std::string Result;
  Result.reserve(Vec.size());
  for (uint8_t Ch : Vec) {
    Result.push_back(static_cast<char>(Ch));
  }
  return Result;
}

inline void PrintToken(const Token &Token) {
  // static std::unordered_map<TokenType, std::string_view> TokenTypeString = {
  //     {TokenType::Invalid, "Invalid"},
  //     {TokenType::Token, "Token"},
  //     {TokenType::Error, "Error"},
  //     {TokenType::ArithmeticOperator, "ArithmeticOperator"},
  //     {TokenType::RelationalOperator, "RelationalOperator"},
  //     {TokenType::Keyword, "Keyword"},
  //     {TokenType::Separator, "Separator"},
  //     {TokenType::UnsignedNumber, "UnsignedNumber"},
  // };
  // std::cout << "[Analyzer] Token{ Type: " << TokenTypeString[Token.Type]
  //           << ", Position: " << Token.Context.ToString() << ", Content: \""
  //           << ConvertU8VectorToString(Token.Content) << "\" }" << std::endl;
  static std::unordered_map<TokenType, std::string_view> TokenTypeString = {
      {TokenType::Invalid, "Invalid"},
      {TokenType::Token, "标识符"},
      {TokenType::Error, "Error"},
      {TokenType::ArithmeticOperator, "算术运算符"},
      {TokenType::RelationalOperator, "关系运算符"},
      {TokenType::Keyword, "关键字"},
      {TokenType::Separator, "分界符"},
      {TokenType::UnsignedNumber, "无符号数"},
  };
  static bool IsFirstOutput = true;
  static uint64_t ColumnWidth = 24;
  if (IsFirstOutput) {
    IsFirstOutput = false;
    PrintPaddedCell("单词", ColumnWidth);
    PrintPaddedCell("二元序列", ColumnWidth);
    PrintPaddedCell("类型", ColumnWidth);
    PrintPaddedCell("位置", ColumnWidth);
    std::cout << std::endl;
  }

  std::string TupleString;
  if (Token.Type == TokenType::Error) {
    TupleString = "Error";
  } else {
    TupleString = "(" + std::to_string(static_cast<uint8_t>(Token.Type)) +
                  ", " + ConvertU8VectorToString(Token.Content) + ")";
  }

  PrintPaddedCell(ConvertU8VectorToString(Token.Content), ColumnWidth);
  PrintPaddedCell(TupleString, ColumnWidth);
  PrintPaddedCell(TokenTypeString[Token.Type], ColumnWidth);
  PrintPaddedCell(Token.Context.ToString(), ColumnWidth);
  std::cout << std::endl;
}

inline std::unordered_map<uint8_t, StateID>
MakeTransitionMap(std::unordered_set<uint8_t> &CharSet, StateID NextStateID) {
  std::unordered_map<uint8_t, StateID> TransitionMap;
  for (uint8_t Char : CharSet) {
    TransitionMap[Char] = NextStateID;
  }
  return TransitionMap;
}

inline std::unordered_map<uint8_t, StateID>
MakeTransitionMap(StateID TargetState, bool ShouldAcceptNumber = false,
                  bool ShouldAcceptCharacter = false,
                  bool ShouldAcceptDot = false) {
  std::unordered_map<uint8_t, StateID> TransitionMap;
  if (ShouldAcceptNumber) {
    for (int i = 0; i < 10; ++i) {
      TransitionMap['0' + i] = TargetState;
    }
  }
  if (ShouldAcceptCharacter) {
    for (int i = 0; i < 26; ++i) {
      TransitionMap['a' + i] = TargetState;
      TransitionMap['A' + i] = TargetState;
    }
    TransitionMap['_'] = TargetState;
  }
  return TransitionMap;
}
