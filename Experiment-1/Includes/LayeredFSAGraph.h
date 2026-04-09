#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class StateMachine;

typedef std::tuple<uint64_t, uint8_t> PositionTuple;
struct PositionTupleHash {
  std::size_t operator()(const PositionTuple &k) const {
    auto [Column, Character] = k;
    return std::hash<ulong>{}(Column) ^ (std::hash<uint8_t>{}(Character) << 1);
  }
};

class LayeredFSAGraph {
  friend class StateMachine;

public:
  void AddWordList(std::vector<std::vector<uint32_t>> &List) {
    if (GraphBuilt) {
      throw std::runtime_error(
          "LayeredFSAGraph: Graph built, anything new refused.");
    }
    WordList.insert(WordList.end(), List.begin(), List.end());
  }

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
        } else if (j == WordList[i].size() - 1) {
          EndStates.insert(ThisStateID);
          if (TransitionMaps.find(ThisStateID) == TransitionMaps.end()) {
            TransitionMaps[ThisStateID] = {};
          }
        }

        if (j > 0) {
          std::tuple<uint64_t, uint8_t> LastPositionTuple =
              std::make_tuple(j - 1, WordList[i][j - 1]);
          TransitionMaps[StateIDMap[LastPositionTuple]][WordList[i][j]] =
              ThisStateID;
        }
      }
    }

    GraphBuilt = true;
  }

  bool IsGraphBuilt() const { return GraphBuilt; }

private:
  uint64_t GetNextStateID() {
    if (NextStateID < 3) {
      throw std::runtime_error("LayeredFSAGraph: State count overflowed.");
    }
    return NextStateID++;
  }

  uint64_t NextStateID = 3;
  std::unordered_set<uint64_t> EndStates;
  std::unordered_map<std::tuple<uint64_t, uint8_t>, uint64_t, PositionTupleHash>
      StateIDMap;
  // std::vector<std::tuple<uint64_t, uint64_t, uint8_t>> Transitions;
  std::unordered_map<uint64_t, std::unordered_map<uint8_t, uint64_t>>
      TransitionMaps;
  std::vector<std::vector<uint8_t>> WordList;
  bool GraphBuilt = false;
};