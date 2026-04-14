#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "State.h"

class StateMachine;

typedef std::tuple<uint64_t, uint8_t> PositionTuple;

/** Hash function for PositionTuple. */
struct PositionTupleHash {
  std::size_t operator()(const PositionTuple &k) const {
    auto [Column, Character] = k;
    return std::hash<uint64_t>{}(Column) ^
           (std::hash<uint8_t>{}(Character) << 1);
  }
};

/** A graph representing a layered finite state automaton for word matching. */
class LayeredDFAGraph {
  friend class StateMachine;

public:
  void AddWordList(std::vector<std::vector<uint8_t>> &List) {
    if (GraphBuilt) {
      throw std::runtime_error(
          "LayeredDFAGraph: Graph built, anything new refused.");
    }
    WordList.insert(WordList.end(), List.begin(), List.end());
  }

  /**
   * @brief Build the graph from the added word list.
   */
  void BuildGraph() {
    // Traverse all words to allocate state ids and record transitions
    for (uint64_t i = 0; i < WordList.size(); ++i) {
      // Ignore empty word
      if (WordList[i].empty()) {
        continue;
      }

      for (uint64_t j = 0; j < WordList[i].size(); ++j) {
        std::tuple<uint64_t, uint8_t> PositionTuple =
            std::make_tuple(j, WordList[i][j]);

        // Allocate a new state id if not existed
        if (StateIDMap.find(PositionTuple) == StateIDMap.end()) {
          uint64_t NewID = GetNextStateID();
          StateIDMap[PositionTuple] = NewID;
        }

        // Record transitions
        uint64_t ThisStateID = StateIDMap[PositionTuple];
        if (j == 0) {
          TransitionMaps[0][WordList[i][j]] = ThisStateID;
        } else if (j > 0) {
          std::tuple<uint64_t, uint8_t> LastPositionTuple =
              std::make_tuple(j - 1, WordList[i][j - 1]);
          TransitionMaps[StateIDMap[LastPositionTuple]][WordList[i][j]] =
              ThisStateID;
        }

        if (j == WordList[i].size() - 1) {
          EndStates.insert(ThisStateID);
          if (TransitionMaps.find(ThisStateID) == TransitionMaps.end()) {
            TransitionMaps[ThisStateID] = {};
          }
        }
      }
    }

    GraphBuilt = true;
  }

  void UpdateStateFlagStrategy(const StateFlagStrategy &Strategy) {
    FlagStrategy = Strategy;
  }

  /**
   * @brief Check if the graph is built.
   *
   * @return bool
   */
  bool IsGraphBuilt() const { return GraphBuilt; }

private:
  /**
   * @brief Get the Next State ID for graph building. State IDs 0, 1 and 2 are
   * pre-allocated for start state, default end state and reserved end state, so
   * the next state ID starts from 3.
   *
   * @return uint64_t
   */
  uint64_t GetNextStateID() {
    if (NextStateID < 3) {
      throw std::runtime_error("LayeredDFAGraph: State count overflowed.");
    }
    return NextStateID++;
  }

  uint64_t NextStateID = 3; // Start state (0), default end state (1) and
  // reserved end state (2) are pre-allocated
  // graph building only
  std::unordered_map<uint64_t, std::unordered_map<uint8_t, uint64_t>>
      TransitionMaps; // StateID -> (Input -> StateID), used for graph building
                      // only
  std::unordered_set<uint64_t>
      EndStates; // StateIDs of end states, used for graph building only
  std::unordered_map<PositionTuple, uint64_t, PositionTupleHash>
      StateIDMap; // Position tuple (column, character) -> StateID, used for
  std::vector<std::vector<uint8_t>> WordList; // For graph building only
  StateFlagStrategy FlagStrategy; // State flag strategy for the graph, used for
                                  // state config building
  bool GraphBuilt =
      false; // Whether the graph is built, used for graph building control
};