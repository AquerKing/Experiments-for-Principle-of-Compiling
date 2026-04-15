#include "AnalyzerCore.h"

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "LayeredDFAGraph.h"
#include "ProgramRuntimeInfo.h"
#include "State.h"
#include "StateMachine.h"
#include "Token.h"
#include "Utils.h"

static StateMachine KeywordStateMachine;
static StateMachine SeparatorStateMachine;
static StateMachine ArithmeticOperatorStateMachine;
static StateMachine RelationalOperatorStateMachine;
static StateMachine TokenStateMachine;
static StateMachine NumberStateMachine;

static std::unordered_set<uint8_t> NumberCharset = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};
static std::unordered_set<uint8_t> IdentifierCharset = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_',
};
static std::unordered_set<uint8_t> WhitespaceCharset = {' ', '\t', '\n', '\r'};
static std::unordered_set<uint8_t> SeparatorCharset = {'(', ')', '{', '}',
                                                       '[', ']', ';', ','};
static std::unordered_set<uint8_t> ArithmeticOperatorCharset = {'+', '-', '*',
                                                                '/'};
static std::unordered_set<uint8_t> RelationalOperatorCharset = {'<', '>', '='};

void BuildStateMachines() {
  if (RuntimeInfo.HasArgument(
          ProgramRuntimeInfo::ArgumentBitFlag::StateMachinesBuilt)) {
    std::cerr << "Error: State machines have already been built. Please "
                 "restart the program to rebuild the state machines."
              << std::endl;
    return;
  }

  LayeredDFAGraph KeywordDFA;
  LayeredDFAGraph SeparatorDFA;
  LayeredDFAGraph ArithmeticOperatorDFA;
  LayeredDFAGraph RelationalOperatorDFA;

  KeywordDFA.AddWordList(RuntimeInfo.KeywordList);
  SeparatorDFA.AddWordList(RuntimeInfo.SeparatorList);
  ArithmeticOperatorDFA.AddWordList(RuntimeInfo.ArithmeticOperatorList);
  RelationalOperatorDFA.AddWordList(RuntimeInfo.RelationalOperatorList);

  KeywordDFA.BuildGraph();
  SeparatorDFA.BuildGraph();
  ArithmeticOperatorDFA.BuildGraph();
  RelationalOperatorDFA.BuildGraph();

  StateFlagStrategy KeywordSMFlagStrategy = {
      TokenType::Token,
      TokenType::Keyword,
      TokenType::Invalid,
  };
  StateFlagStrategy SeparatorSMFlagStrategy = {
      TokenType::Error,
      TokenType::Separator,
      TokenType::Error,
  };
  StateFlagStrategy ArithmeticOperatorSMFlagStrategy = {
      TokenType::Error,
      TokenType::ArithmeticOperator,
      TokenType::Error,
  };
  StateFlagStrategy RelationalOperatorSMFlagStrategy = {
      TokenType::Error,
      TokenType::RelationalOperator,
      TokenType::Error,
  };

  KeywordDFA.SetStateFlagStrategy(KeywordSMFlagStrategy);
  SeparatorDFA.SetStateFlagStrategy(SeparatorSMFlagStrategy);
  ArithmeticOperatorDFA.SetStateFlagStrategy(ArithmeticOperatorSMFlagStrategy);
  RelationalOperatorDFA.SetStateFlagStrategy(RelationalOperatorSMFlagStrategy);

  KeywordStateMachine.BuildFromLayeredDFAGraph(KeywordDFA);
  SeparatorStateMachine.BuildFromLayeredDFAGraph(SeparatorDFA);
  ArithmeticOperatorStateMachine.BuildFromLayeredDFAGraph(
      ArithmeticOperatorDFA);
  RelationalOperatorStateMachine.BuildFromLayeredDFAGraph(
      RelationalOperatorDFA);

  // Build token state machine for identifiers and numbers.
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();

    std::unordered_map<uint8_t, StateID> TransitionMap =
        MakeTransitionMap(3, false, true);

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::Start;
    Config.CacheFlag = TokenType::Invalid;

    Manager.UpdateStateConfig(0, Config);
  }
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();
    Manager.CreateStateByID(3);

    std::unordered_map<uint8_t, StateID> TransitionMap =
        MakeTransitionMap(3, true, true);

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::End;
    Config.CacheFlag = TokenType::Token;

    Manager.UpdateStateConfig(3, Config);
  }

  // Build number state machine with the following rules:
  // 1. A number can only start with a digit.
  // 2. After the first digit, it can have more digits or a dot.
  // 3. If it has a dot, it can have more digits after the dot, but it cannot
  // have another dot.
  {
    StateManager &Manager = NumberStateMachine.GetStateManager();

    std::unordered_map<uint8_t, StateID> TransitionMap =
        MakeTransitionMap(3, true, false);
    TransitionMap['.'] = 4;
    TransitionMap.merge(MakeTransitionMap(1, false, true, false));

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::Start;
    Config.CacheFlag = TokenType::Invalid;

    Manager.UpdateStateConfig(0, Config);
  }
  {
    StateManager &Manager = NumberStateMachine.GetStateManager();
    Manager.CreateStateByID(3);

    std::unordered_map<uint8_t, StateID> TransitionMap =
        MakeTransitionMap(3, true, false, false);
    TransitionMap['.'] = 4;
    TransitionMap.merge(MakeTransitionMap(1, false, true, false));

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::End;
    Config.CacheFlag = TokenType::UnsignedNumber;

    Manager.UpdateStateConfig(3, Config);
  }
  {
    StateManager &Manager = NumberStateMachine.GetStateManager();
    Manager.CreateStateByID(4);

    std::unordered_map<uint8_t, StateID> TransitionMap =
        MakeTransitionMap(4, true, false, false);
    TransitionMap.merge(MakeTransitionMap(1, false, true, false));

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::End;
    Config.CacheFlag = TokenType::UnsignedNumber;

    Manager.UpdateStateConfig(4, Config);
  }
  {
    StateManager &Manager = NumberStateMachine.GetStateManager();

    StateConfig Config;
    Config.CacheFlag = TokenType::Error;
    Config.Type = StateType::Error;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = MakeTransitionMap(1, true, true, true);

    Manager.UpdateStateConfig(1, Config);
  }

  RuntimeInfo.ArgumentFlag |= static_cast<uint8_t>(
      ProgramRuntimeInfo::ArgumentBitFlag::StateMachinesBuilt);
  std::cout << "Message: State machines built successfully." << std::endl;
}

std::vector<Token> AnalyzeSource(const std::vector<uint8_t> &SourceCode) {
  const int Length = SourceCode.size();
  std::vector<Token> RecognizedTokens;

  if (Length == 0) {
    std::cerr << "Error: Source code is empty." << std::endl;
    return {};
  }

  auto CheckChar = [](const std::vector<uint8_t> &Source, size_t Index,
                      uint8_t Char) {
    return Index >= 0 && Index < Source.size() && Source[Index] == Char;
  };

  auto GetSubStringByCharset = [](const std::vector<uint8_t> &Source,
                                  size_t StartIndex,
                                  const std::unordered_set<uint8_t> &CharSet) {
    std::vector<uint8_t> Result;
    size_t Index = StartIndex;

    while (Index >= 0 && Index < Source.size() &&
           CharSet.find(Source[Index]) != CharSet.end()) {
      Result.push_back(Source[Index]);
      ++Index;
    }

    return Result;
  };

  auto IsWhitespace = [](uint32_t Char) {
    return Char == ' ' || Char == '\t' || Char == '\n' || Char == '\r';
  };

  auto IsIdentifierStartChar = [](uint32_t Char) {
    return (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') ||
           Char == '_';
  };

  enum class ScannerState : uint8_t {
    InComment = 1 << 0,
    InMultiLineComment = 1 << 1,
  };

  uint8_t ScannerFlag = 0;
  TokenContextInfo ContextInfo = {1, 1};
  std::vector<Token> Tokens;
  for (size_t i = 0; i < SourceCode.size(); ++i) {
    uint8_t Char = SourceCode[i];

    if (IsWhitespace(Char)) {
      if (Char == '\n' || Char == '\r') {
        if (Char == '\r' && CheckChar(SourceCode, i + 1, '\n')) {
          ++i;
        }
        ContextInfo.Row += 1;
        ContextInfo.Column = 1;
        ScannerFlag &= ~static_cast<uint8_t>(ScannerState::InComment);
      } else {
        ContextInfo.Column += 1;
      }
      continue;
    }

    if (i < SourceCode.size() - 1 && Char == '/' && SourceCode[i + 1] == '/' &&
        !(ScannerFlag &
          static_cast<uint8_t>(ScannerState::InMultiLineComment))) {
      ScannerFlag |= static_cast<uint8_t>(ScannerState::InComment);
      i += 1;
      ContextInfo.Column += 2;
      continue;
    } else if (i < SourceCode.size() - 1 && Char == '/' &&
               SourceCode[i + 1] == '*' &&
               !(ScannerFlag & static_cast<uint8_t>(ScannerState::InComment)) &&
               !(ScannerFlag &
                 static_cast<uint8_t>(ScannerState::InMultiLineComment))) {
      ScannerFlag |= static_cast<uint8_t>(ScannerState::InMultiLineComment);
      i += 1;
      ContextInfo.Column += 2;
      continue;
    } else if (i < SourceCode.size() - 1 && Char == '*' &&
               SourceCode[i + 1] == '/') {
      ScannerFlag &= ~static_cast<uint8_t>(ScannerState::InComment);
      ScannerFlag &= ~static_cast<uint8_t>(ScannerState::InMultiLineComment);
      i += 1;
      ContextInfo.Column += 2;
      continue;
    }

    if (ScannerFlag & static_cast<uint8_t>(ScannerState::InComment) ||
        ScannerFlag & static_cast<uint8_t>(ScannerState::InMultiLineComment)) {
      ContextInfo.Column += 1;
      continue;
    }

    if (CheckChar(SourceCode, i, '_') || (Char >= 'a' && Char <= 'z') ||
        (Char >= 'A' && Char <= 'Z')) {
      std::vector<uint8_t> Identifier =
          GetSubStringByCharset(SourceCode, i, IdentifierCharset);
      i += Identifier.size() - 1;
      KeywordStateMachine.SetContextInfo(ContextInfo.Row, ContextInfo.Column);

      Tokens = KeywordStateMachine.ReceiveInputs(
          Identifier, StateMachine::TokenGenerationStrategy::GenerateAtLast);

      if (Tokens.empty()) {
        TokenStateMachine.SetContextInfo(ContextInfo.Row, ContextInfo.Column);
        Tokens = TokenStateMachine.ReceiveInputs(
            Identifier, StateMachine::TokenGenerationStrategy::GenerateAtLast);
        TokenStateMachine.Reset();
      }

      ContextInfo.Column += Identifier.size();

      if (Tokens.size() != 1) {
        throw std::runtime_error("Failed to recognize identifier: " +
                                 ConvertU8VectorToString(Identifier));
      }

      RecognizedTokens.emplace_back(Tokens[0]);
      KeywordStateMachine.Reset();
    } else if (Char >= '0' && Char <= '9' || Char == '.') {
      static std::unordered_set<uint8_t> Charset = NumberCharset;
      Charset.insert('.');

      std::vector<uint8_t> Number =
          GetSubStringByCharset(SourceCode, i, Charset);
      i += Number.size() - 1;
      NumberStateMachine.SetContextInfo(ContextInfo.Row, ContextInfo.Column);
      ContextInfo.Column += Number.size();

      Tokens = NumberStateMachine.ReceiveInputs(
          Number, StateMachine::TokenGenerationStrategy::GenerateAtLast);

      if (Tokens.size() != 1) {
        throw std::runtime_error("Failed to recognize number: " +
                                 ConvertU8VectorToString(Number));
      }

      RecognizedTokens.emplace_back(Tokens[0]);
      NumberStateMachine.Reset();
    } else if (SeparatorCharset.find(Char) != SeparatorCharset.end()) {
      std::vector<uint8_t> SeparatorSequence =
          GetSubStringByCharset(SourceCode, i, SeparatorCharset);
      i += SeparatorSequence.size() - 1;
      SeparatorStateMachine.SetContextInfo(ContextInfo.Row, ContextInfo.Column);
      ContextInfo.Column += SeparatorSequence.size();

      Tokens = SeparatorStateMachine.ReceiveInputs(
          SeparatorSequence,
          StateMachine::TokenGenerationStrategy::GenerateSoonIfPossible);
      if (!Tokens.size()) {
        throw std::runtime_error("Failed to recognize separator: " +
                                 ConvertU8VectorToString(SeparatorSequence));
      }

      RecognizedTokens.insert(RecognizedTokens.end(), Tokens.begin(),
                              Tokens.end());
      SeparatorStateMachine.Reset();
    } else if (ArithmeticOperatorCharset.find(Char) !=
               ArithmeticOperatorCharset.end()) {
      std::vector<uint8_t> String =
          GetSubStringByCharset(SourceCode, i, ArithmeticOperatorCharset);
      i += String.size() - 1;
      ArithmeticOperatorStateMachine.SetContextInfo(ContextInfo.Row,
                                                    ContextInfo.Column);
      ContextInfo.Column += String.size();

      Tokens = ArithmeticOperatorStateMachine.ReceiveInputs(
          String, StateMachine::TokenGenerationStrategy::GenerateAtLast);
      if (!Tokens.size()) {
        throw std::runtime_error("Failed to recognize arithmetic operator: " +
                                 ConvertU8VectorToString(String));
      }

      RecognizedTokens.insert(RecognizedTokens.end(), Tokens.begin(),
                              Tokens.end());
      ArithmeticOperatorStateMachine.Reset();

    } else if (RelationalOperatorCharset.find(Char) !=
               RelationalOperatorCharset.end()) {
      std::vector<uint8_t> String =
          GetSubStringByCharset(SourceCode, i, RelationalOperatorCharset);
      i += String.size() - 1;
      RelationalOperatorStateMachine.SetContextInfo(ContextInfo.Row,
                                                    ContextInfo.Column);
      ContextInfo.Column += String.size();

      Tokens = RelationalOperatorStateMachine.ReceiveInputs(
          String, StateMachine::TokenGenerationStrategy::GenerateAtLast);
      if (!Tokens.size()) {
        throw std::runtime_error("Failed to recognize relational operator: " +
                                 ConvertU8VectorToString(String));
      }

      RecognizedTokens.insert(RecognizedTokens.end(), Tokens.begin(),
                              Tokens.end());
      RelationalOperatorStateMachine.Reset();
    } else {
      Token NewToken;
      NewToken.Type = TokenType::Error;
      NewToken.Context = ContextInfo;
      NewToken.Content = {Char};
      Tokens = {NewToken};
      RecognizedTokens.push_back(NewToken);
      ContextInfo.Column += 1;
    }

    // DEBUG
    for (const auto &Token : Tokens) {
      PrintToken(Token);
    }
  }

  return RecognizedTokens;
}