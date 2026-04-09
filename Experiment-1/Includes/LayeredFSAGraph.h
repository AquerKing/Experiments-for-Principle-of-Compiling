#pragma once

#include <functional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Types.h"

class StateMachine;

namespace std {

template <> struct hash<std::tuple<unsigned long long, unsigned int>> {
  size_t
  operator()(const std::tuple<unsigned long long, unsigned int> &t) const {
    auto [first, second] = t;

    return std::hash<unsigned long long>{}(first) ^
           (std::hash<unsigned int>{}(second) << 1);
  }
};

} // namespace std

class LayeredFSAGraph {
  friend class StateMachine;

public:
  void AddWordList(std::vector<std::vector<uchar>> List) {
    if (GraphBuilt) {
      throw std::runtime_error(
          "LayeredFSAGraph: Graph built, anything new refused.");
    }
    WordList.insert(WordList.end(), List.begin(), List.end());
  }

  void BuildGraph() {
    // Traverse all words to allocate state ids and record transitions
    for (ulong i = 0; i < WordList.size(); ++i) {
      // Ignore empty word
      if (WordList[i].empty()) {
        continue;
      }

      for (ulong j = 0; j < WordList[i].size(); ++j) {
        std::tuple<ulong, uchar> PositionTuple =
            std::make_tuple(j, WordList[i][j]);

        // Allocate a new state id if not existed
        if (StateIDMap.find(PositionTuple) == StateIDMap.end()) {
          StateIDMap[PositionTuple] = GetNextStateID();
          if (j == WordList[i].size() - 1) {
            EndStates.insert(StateIDMap[PositionTuple]);
          }
        }

        // Record transitions
        if (j == 0) {
          TransitionMaps[0][WordList[i][j]] = StateIDMap[PositionTuple];
        } else {
          std::tuple<ulong, uchar> LastPositionTuple =
              std::make_tuple(j - 1, WordList[i][j - 1]);
          TransitionMaps[StateIDMap[LastPositionTuple]][WordList[i][j]] =
              StateIDMap[PositionTuple];
        }
      }
    }

    GraphBuilt = true;
  }

  bool IsGraphBuilt() const { return GraphBuilt; }

private:
  ulong GetNextStateID() {
    if (NextStateID < 3) {
      throw std::runtime_error("LayeredFSAGraph: State count overflowed.");
    }
    return NextStateID++;
  }

  ulong NextStateID = 3;
  std::unordered_set<ulong> EndStates;
  std::unordered_map<std::tuple<ulong, uchar>, ulong> StateIDMap;
  // std::vector<std::tuple<ulong, ulong, uchar>> Transitions;
  std::unordered_map<ulong, std::unordered_map<uchar, ulong>> TransitionMaps;
  std::vector<std::vector<uchar>> WordList;
  bool GraphBuilt = false;
};