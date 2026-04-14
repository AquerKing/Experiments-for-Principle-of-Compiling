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
  static std::unordered_map<TokenType, std::string_view> TokenTypeString = {
      {TokenType::Invalid, "Invalid"},
      {TokenType::Token, "Token"},
      {TokenType::Error, "Error"},
      {TokenType::ArithmeticOperator, "ArithmeticOperator"},
      {TokenType::RelationalOperator, "RelationalOperator"},
      {TokenType::Keyword, "Keyword"},
      {TokenType::Separator, "Separator"},
      {TokenType::UnsignedNumber, "UnsignedNumber"},
  };
  std::cout << "[Analyzer] Token{ Type: " << TokenTypeString[Token.Type]
            << ", Position: " << Token.Context.ToString() << ", Content: \""
            << ConvertU8VectorToString(Token.Content) << "\" }" << std::endl;
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
