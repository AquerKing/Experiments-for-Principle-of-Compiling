#include "utils.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

bool CharacterUtils::IsUpperCase(char c) { return c >= 'A' && c <= 'Z'; }

bool CharacterUtils::IsLowerCase(char c) { return c >= 'a' && c <= 'z'; }

std::vector<GenerativeExpression>
GrammarUtils::ParseGenerativeExpressions(const std::string &input,
                                         SymbolManager &Manager) {
  if (input.empty()) {
    return {};
  }

  std::vector<GenerativeExpression> expressions;

  if (!CharacterUtils::IsUpperCase(input[0])) {
    throw std::invalid_argument("The first character of a generative "
                                "expression must be an uppercase letter.");
  }

  if (input.size() < 3 || input[1] != '-' || input[2] != '>') {
    throw std::invalid_argument(
        "Invalid generative expression format. Expected format: "
        "<Source>-><Target1><Target2>...");
  }

  uint64_t SourceNonTerminatorId = 0;
  if (const std::string Source = input.substr(0, 1);
      !Manager.IsSymbolExisted(Source)) {
    SourceNonTerminatorId =
        Manager.CreateSymbol(Source, SymbolType::NonTerminator);
    if (SourceNonTerminatorId == 0) {
      throw std::runtime_error("Failed to create non-terminator symbol for "
                               "generative expression source.");
    }
  }

  std::string TargetsExpresion = input.substr(3);
  if (TargetsExpresion.empty()) {
    throw std::invalid_argument(
        "Generative expression must have at least one target symbol.");
  }

  std::vector<std::vector<uint64_t>> AlternativeTargets;
  std::string_view RemainingTargetsExpression = TargetsExpresion;

  while (!RemainingTargetsExpression.empty()) {
    size_t DelimiterPos = RemainingTargetsExpression.find_first_of('|');

    if (DelimiterPos == std::string_view::npos) {
      if (!RemainingTargetsExpression.empty()) {
        AlternativeTargets.push_back(
            ParseSymbolSequence(RemainingTargetsExpression, Manager));
        RemainingTargetsExpression = RemainingTargetsExpression.substr(0, 0);
      }
      break;
    }

    AlternativeTargets.push_back(ParseSymbolSequence(
        RemainingTargetsExpression.substr(0, DelimiterPos), Manager));
    RemainingTargetsExpression =
        RemainingTargetsExpression.substr(DelimiterPos + 1);
  }

  for (const auto &Target : AlternativeTargets) {
    expressions.emplace_back(SourceNonTerminatorId, Target,
                             &Manager);
  }

  return expressions;
}

std::vector<uint64_t>
GrammarUtils::ParseSymbolSequence(const std::string_view &Input,
                                  SymbolManager &Manager) {
  std::vector<uint64_t> SymbolIds;

  for (char c : Input) {
    if (c == '|') {
      throw std::runtime_error(
          "The '|' character is not allowed in symbol sequences. It is "
          "reserved "
          "for separating alternative generative expressions.");
    }

    if (c == '@') {
      SymbolIds.push_back(Terminator::Epsilon.SymbolId);
      continue;
    }

    if (CharacterUtils::IsUpperCase(c)) {
      uint64_t NonTerminatorId =
          Manager.FetchSymbolIdByValue({c}, SymbolType::NonTerminator);
      SymbolIds.push_back(NonTerminatorId);
    } else {
      uint64_t TerminatorId =
          Manager.FetchSymbolIdByValue({c}, SymbolType::Terminator);
      SymbolIds.push_back(TerminatorId);
    }
  }

  return SymbolIds;
}