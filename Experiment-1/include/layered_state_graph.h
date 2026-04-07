#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "state_machine.h"
#include "types.h"

namespace lex {

class LayeredStateGraph final {
  typedef std::unordered_map<uint, std::unordered_map<char, uint>>
      StatePositionMap;

public:
  LayeredStateGraph() = default;
  LayeredStateGraph(std::vector<std::string> word_lists)
      : word_lists(std::move(word_lists)) {}
  ~LayeredStateGraph() = default;

  LayeredStateGraph(const LayeredStateGraph &) = delete;
  LayeredStateGraph &operator=(const LayeredStateGraph &) = delete;
  LayeredStateGraph(LayeredStateGraph &&) = default;
  LayeredStateGraph &operator=(LayeredStateGraph &&) = default;

  void BuildStateGraph();

  StateMachine<char> ConstructStateMachine() const;

private:
  std::vector<std::string> word_lists;

  bool is_built = false;

  StatePositionMap state_position_map;
  std::vector<std::tuple<uint, uint, char>> transitions;
  uint state_count =
      2; // Start with 2 states: 0 for start state, 1 for error state
  std::unordered_set<uint> final_state_ids;
  std::unordered_map<uint, std::string> final_state_value;
};

} // namespace lex