#include "layered_state_graph.h"
#include "state_machine.h"
#include <cstddef>
#include <unordered_map>

namespace lex {

void LayeredStateGraph::BuildStateGraph() {
  for (size_t t = 0; t < word_lists.size(); ++t) {
    const auto &word = word_lists[t];
    if (word.empty()) {
      continue; // Skip empty words
    }

    for (size_t i = 0; i < word.size(); ++i) {
      char current_char = word[i];
      if (state_position_map[i].find(current_char) == state_position_map[i].end()) {
        state_position_map[i][current_char] = state_count++;
      }
      if (i == word.size() - 1) {
        final_state_ids.insert(state_position_map[i][current_char]);
        final_state_value[state_position_map[i][current_char]] = word;
      }
    }

    // Add transitions for the current word
    transitions.emplace_back(0, state_position_map[0][word[0]], word[0]);
    for (size_t i = 1; i < word.size(); ++i) {
      transitions.emplace_back(state_position_map[i - 1][word[i - 1]],
                               state_position_map[i][word[i]], word[i]);
    }
  }
  is_built = true;
}

StateMachine<char> LayeredStateGraph::ConstructStateMachine() const {
  if (!is_built) {
    return StateMachine<char>();
  } // Return an empty state machine if not built

  StateMachine<char> state_machine;
  std::unordered_map<uint, uint> remap_state_id;
  remap_state_id[0] = 0; // Start state
  remap_state_id[1] = 1; // Error state
  for (int i = 2; i < state_count; ++i) {
    if (final_state_ids.find(i) != final_state_ids.end()) {
      remap_state_id[i] = state_machine.AddFinalState(final_state_value.at(i));
    } else {
      remap_state_id[i] = state_machine.AddState();
    }
  }

  for (const auto &[from, to, input] : transitions) {
    state_machine.AddTransition(remap_state_id[from], remap_state_id[to],
                                input);
  }

  return state_machine;
}

} // namespace lex