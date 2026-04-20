#pragma once

#include "interface.h"
#include "symbols.h"

#include <cstdint>
#include <functional>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct UInt64VectorHash {
  size_t operator()(const std::vector<uint64_t> &Values) const noexcept {
    size_t Seed = 0;
    for (const uint64_t Value : Values) {
      Seed ^=
          std::hash<uint64_t>{}(Value) + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
    }
    return Seed;
  }
};

/** Represents a generative expression in the grammar. */
class GenerativeExpression final : public IStringConvertable {
  friend class GenerativeExpressionPreprocessor;

public:
  GenerativeExpression(uint64_t Source, std::vector<uint64_t> Targets,
                       SymbolManager *Manager);

  /**
   * Converts the generative expression to a string.
   * @return The string representation of the generative expression.
   */
  std::string ToString() const override;

  /**
   * Checks if the generative expression is valid.
   * @return True if the expression is valid, false otherwise.
   */
  bool IsValid() const;

private:
  SymbolManager *Manager;
  uint64_t Source;
  std::vector<uint64_t> Targets;
};

class GenerativeExpressionPreprocessor {
public:
  GenerativeExpressionPreprocessor(SymbolManager *Manager) : Manager(Manager) {}

  void CalculateFirstAndFollowSets(
      const std::vector<GenerativeExpression> &Expressions);

  bool IsPreprocessed() const;

private:
  void
  PreprocessExpressions(const std::vector<GenerativeExpression> &Expressions);

  void CalculateFirstSets(const std::vector<GenerativeExpression> &Expressions);

  void
  CalculateFollowSets(const std::vector<GenerativeExpression> &Expressions);

  std::unordered_map<uint64_t, std::unordered_set<uint64_t>> FirstSets;
  std::unordered_map<uint64_t, std::unordered_set<uint64_t>> FollowSets;
  std::unordered_map<
      uint64_t, std::unordered_set<std::vector<uint64_t>, UInt64VectorHash>>
      GenerativeTargetOfNonTerminators;
  std::unordered_map<
      uint64_t, std::unordered_set<std::vector<uint64_t>, UInt64VectorHash>>
      SuffixesOfNonTerminators;
  SymbolManager *Manager;
  bool Preprocessed = false;
  bool IsExpressionConverted = false;
};

class PredictiveAnalysisTable {
public:
  PredictiveAnalysisTable() = default;

  /**
   * Sets an item in the predictive analysis table.
   * @param NonTerminatorId The ID of the non-terminal symbol.
   * @param TerminatorId The ID of the terminal symbol.
   * @param Expression The generative expression to set.
   */
  void SetItem(uint64_t NonTerminatorId, uint64_t TerminatorId,
               GenerativeExpression *Expression);

  /**
   * Gets an item from the predictive analysis table.
   * @param NonTerminatorId The ID of the non-terminal symbol.
   * @param TerminatorId The ID of the terminal symbol.
   * @return The generative expression associated with the given IDs.
   */
  GenerativeExpression *GetItem(uint64_t NonTerminatorId,
                                uint64_t TerminatorId) const;

  /**
   * Checks if an item exists in the predictive analysis table.
   * @param NonTerminatorId The ID of the non-terminal symbol.
   * @param TerminatorId The ID of the terminal symbol.
   * @return True if the item exists, false otherwise.
   */
  bool IsItemExisted(uint64_t NonTerminatorId, uint64_t TerminatorId) const;

private:
  SymbolManager *Manager;
  std::unordered_map<uint64_t,
                     std::unordered_map<uint64_t, GenerativeExpression *>>
      Table;
};
