#define LEXICAL_EXPERIMENT_ONLY

#include <iostream>
#include <string>
#include <vector>

#include "layered_state_graph.h"
#include "state_machine.h"

int main() {
  std::vector<std::string> word_lists = {"hello", "world", "hi"};
  std::vector<std::string> test_lists = {"hello", "you", "are", "nice",
                                         "world", "guy", "hi"};
  lex::LayeredStateGraph layered_state_graph(word_lists);
  layered_state_graph.BuildStateGraph();
  lex::StateMachine<char> state_machine =
      layered_state_graph.ConstructStateMachine();
  for (const auto &word : test_lists) {
    state_machine.Reset();
    for (char c : word) {
      uint current_state_id = state_machine.GetCurrentStateId();
      lex::StateMachine<char>::StateTransitionResult result =
          state_machine.ReceiveInput(c);
      current_state_id = state_machine.GetCurrentStateId();
      if (result == lex::StateMachine<char>::StateTransitionResult::Success) {
        std::cout << "Reached a final state while processing word: " << word
                  << std::endl
                  << std::flush;
        break;
      }
    }
    if (!state_machine.DoesReachFinalState()) {
      std::cout << "Did not reach a final state for word: " << word << std::endl
                << std::flush;
    }
  }
  return 0;
}