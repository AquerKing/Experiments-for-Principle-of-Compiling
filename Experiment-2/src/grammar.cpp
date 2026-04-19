#include "grammar.h"
#include "symbols.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <sys/types.h>
#include <unordered_set>
#include <utility>
#include <vector>

GenerativeExpression::GenerativeExpression(uint64_t Source,
                                           std::vector<uint64_t> Targets,
                                           SymbolManager *Manager)
    : Manager(Manager), Source(Source), Targets(std::move(Targets)) {}

bool GenerativeExpression::IsValid() const {
  return Source != 0 && !Targets.empty();
}

const std::vector<uint64_t> &GenerativeExpression::GetTargets() const {
  return Targets;
}

std::string GenerativeExpression::ToString() const {
  if (Manager != nullptr && IsValid()) {
    std::string Result = Manager->Symbols.at(Source)->ToString() + "->";
    for (const auto &TargetId : Targets) {
      Result += Manager->Symbols.at(TargetId)->ToString();
    }
    return Result;
  }
  return "< Invalid GenerativeExpression >";
}

void GenerativeExpressionPreprocessor::CalculateFirstAndFollowSets(
    const std::vector<GenerativeExpression> &Expressions) {
  assert(Manager);

  PreprocessExpressions(Expressions);

  CalculateFirstSets(Expressions);
  CalculateFollowSets(Expressions);

  Preprocessed = true;
}

bool GenerativeExpressionPreprocessor::IsPreprocessed() const {
  return Preprocessed;
}

void GenerativeExpressionPreprocessor::PreprocessExpressions(
    const std::vector<GenerativeExpression> &Expressions) {
  GenerativeTargetOfNonTerminators.clear();
  SuffixesOfNonTerminators.clear();

  // Preprocess the generative expressions to fill in the
  // GenerativeTargetOfNonTerminators and SuffixesOfNonTerminators maps.
  for (const auto &Expression : Expressions) {
    if (!Expression.IsValid()) {
      throw std::runtime_error("Invalid generative expression found.");
    }

    // Fill in the GenerativeTargetOfNonTerminators map.
    GenerativeTargetOfNonTerminators[Expression.Source].insert(
        Expression.Targets);

    // Fill in the SuffixesOfNonTerminators map.
    const std::vector<uint64_t> &SymbolSequence = Expression.Targets;
    for (size_t i = 0; i < SymbolSequence.size(); ++i) {
      if (Manager->GetSymbol(SymbolSequence.at(i))->GetType() ==
          SymbolType::NonTerminator) {
        if (i == SymbolSequence.size() - 1) {
          SuffixesOfNonTerminators[SymbolSequence.at(i)].insert(
              std::vector<uint64_t>{Terminator::Epsilon.SymbolId});
        } else {
          SuffixesOfNonTerminators[SymbolSequence.at(i)].insert(
              std::vector<uint64_t>(SymbolSequence.begin() + i,
                                    SymbolSequence.end()));
        }
      }
    }
  }

  IsExpressionConverted = true;
}

void GenerativeExpressionPreprocessor::CalculateFirstSets(
    const std::vector<GenerativeExpression> &Expressions) {
  static std::function<void(uint64_t SymbolId)> DFS = [this](
                                                          uint64_t SymbolId) {
    // Implementation for calculating first sets

    if (Manager->GetSymbol(SymbolId)->GetType() == SymbolType::Terminator) {
      // If the symbol is a terminator, its first set is itself.
      FirstSets[SymbolId].insert(SymbolId);
      return;
    } else {
      // If the symbol is a non-terminator, we need to calculate its first
      // set
      for (const auto &TargetSequence :
           GenerativeTargetOfNonTerminators[SymbolId]) {
        bool FirstSetReady = false;
        for (const auto &TargetSymbolId : TargetSequence) {
          if (!FirstSetReady) {
            DFS(TargetSymbolId);
            FirstSets[SymbolId].insert(FirstSets[TargetSymbolId].begin(),
                                       FirstSets[TargetSymbolId].end());

            // If the first set of the target symbol does not contain epsilon,
            // we can stop here.
            if (FirstSets[TargetSymbolId].count(Terminator::Epsilon.SymbolId) ==
                0) {
              FirstSetReady = true;
            }
          } else if (Manager->GetSymbol(TargetSymbolId)->GetType() ==
                     SymbolType::Terminator) {
            DFS(TargetSymbolId);
          }
        }
      }
    }
  };

  for (const auto &Expression : Expressions) {
    DFS(Expression.Source);
  }
}

void GenerativeExpressionPreprocessor::CalculateFollowSets(
    const std::vector<GenerativeExpression> &Expressions) {
  // The follow set of the start symbol should contain the end symbol.
  FollowSets[Manager->GetStartSymbolId()].insert(
      Terminator::EndSymbol.SymbolId);

  bool Updated = true;
  while (Updated) {
    Updated = false;

    for (const auto &Expression : Expressions) {
      const size_t SymbolCount = Expression.Targets.size();

      for (size_t i = 0; i < SymbolCount; ++i) {
        const uint64_t SymbolId = Expression.Targets.at(i);
        const uint64_t OldFollowSetSize = FollowSets[SymbolId].size();

        if (Manager->GetSymbol(SymbolId)->GetType() == SymbolType::Terminator) {
          continue;
        }

        if (i == SymbolCount - 1) {
          FollowSets[SymbolId].insert(FollowSets[Expression.Source].begin(),
                                      FollowSets[Expression.Source].end());
        } else {
          for (size_t j = i + 1; j < SymbolCount; ++j) {
            FollowSets[SymbolId].insert(
                FirstSets[Expression.Targets.at(j)].begin(),
                FirstSets[Expression.Targets.at(j)].end());
            if (FirstSets[Expression.Targets.at(j)].count(
                    Terminator::Epsilon.SymbolId) == 0) {
              break;
            }

            FollowSets[SymbolId].erase(Terminator::Epsilon.SymbolId);

            if (j == SymbolCount - 1) {
              FollowSets[SymbolId].insert(FollowSets[Expression.Source].begin(),
                                          FollowSets[Expression.Source].end());
            }
          }
        }

        if (FollowSets[SymbolId].size() > OldFollowSetSize) {
          Updated = true;
        }
      }
    }
  }
}

const std::unordered_set<uint64_t> &
GenerativeExpressionPreprocessor::GetFirstSetOfSymbol(uint64_t SymbolId) const {
  if (Preprocessed == false) {
    throw std::runtime_error("First sets have not been calculated. Please call "
                             "CalculateFirstAndFollowSets() first.");
  }

  if (FirstSets.count(SymbolId) == 0) {
    throw std::runtime_error("Symbol not found in first sets.");
  }
  return FirstSets.at(SymbolId);
}

const std::unordered_set<uint64_t> &
GenerativeExpressionPreprocessor::GetFollowSetOfSymbol(
    uint64_t SymbolId) const {
  if (Preprocessed == false) {
    throw std::runtime_error(
        "Follow sets have not been calculated. Please call "
        "CalculateFirstAndFollowSets() first.");
  }

  if (FollowSets.count(SymbolId) == 0) {
    throw std::runtime_error("Symbol not found in follow sets.");
  }
  return FollowSets.at(SymbolId);
}

void PredictiveAnalysisTable::BuildFromPreprocessor(
    const std::vector<GenerativeExpression> &Expressions,
    const GenerativeExpressionPreprocessor &Preprocessor) {
  if (!Preprocessor.IsPreprocessed()) {
    throw std::runtime_error(
        "The preprocessor has not been preprocessed. Please call "
        "CalculateFirstAndFollowSets() first.");
  }

  for (const auto &Expression : Expressions) {
    const uint64_t SourceSymbolId = Expression.Source;

    for (const uint64_t FirstSymbolId :
         Preprocessor.FirstSets.at(Expression.Targets.at(0))) {
      if (FirstSymbolId == Terminator::Epsilon.SymbolId) {
        continue;
      }

      if (Table.count(SourceSymbolId) > 0 &&
          Table.at(SourceSymbolId).count(FirstSymbolId) > 0) {
        throw std::runtime_error(
            "The grammar is not LL(1) since there are multiple items in the "
            "predictive analysis table for the same non-terminator and "
            "terminator.");
      }

      SetItem(SourceSymbolId, FirstSymbolId,
              const_cast<GenerativeExpression *>(&Expression));
    }

    if (Preprocessor.FirstSets.at(Expression.Targets.at(0))
            .count(Terminator::Epsilon.SymbolId) > 0) {
      for (const uint64_t FollowSymbolId :
           Preprocessor.FollowSets.at(SourceSymbolId)) {
        if (Table.count(SourceSymbolId) > 0 &&
            Table.at(SourceSymbolId).count(FollowSymbolId) > 0) {
          throw std::runtime_error(
              "The grammar is not LL(1) since there are multiple items in the "
              "predictive analysis table for the same non-terminator and "
              "terminator.");
        }

        SetItem(SourceSymbolId, FollowSymbolId,
                const_cast<GenerativeExpression *>(&Expression));
      }
    }
  }
}

void PredictiveAnalysisTable::SetItem(uint64_t NonTerminatorId,
                                      uint64_t TerminatorId,
                                      GenerativeExpression *Expression) {
  if (IsItemExisted(NonTerminatorId, TerminatorId)) {
    throw std::runtime_error(
        "Items in predictive analysis table can not be overrided.");
    return;
  }

  if (Table.count(NonTerminatorId) == 0) {
    Table[NonTerminatorId] = {};
  }

  Table[NonTerminatorId][TerminatorId] = Expression;
}

GenerativeExpression *
PredictiveAnalysisTable::GetItem(uint64_t NonTerminatorId,
                                 uint64_t TerminatorId) const {
  if (!IsItemExisted(NonTerminatorId, TerminatorId)) {
    return nullptr;
  }
  return Table.at(NonTerminatorId).at(TerminatorId);
}

bool PredictiveAnalysisTable::IsItemExisted(uint64_t NonTerminatorId,
                                            uint64_t TerminatorId) const {
  if (Table.count(NonTerminatorId) == 0 ||
      Table.at(NonTerminatorId).count(TerminatorId) == 0) {
    return false;
  }
  return true;
}