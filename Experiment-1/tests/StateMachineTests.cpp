#include <any>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "layered_state_graph.h"
#include "state_machine.h"

using lex::StateMachine;

static void TestDefaultConstruction() {
  StateMachine<char> machine;
  assert(machine.GetCurrentStateId() == 0);
  assert(!machine.DoesReachFinalState());
  machine.Reset();
  assert(machine.GetCurrentStateId() == 0);
}

static void TestAddStatesAndTransitions() {
  StateMachine<char> machine;

  uint intermediate_state = machine.AddState();
  uint final_state = machine.AddFinalState(std::any(std::string("done")));

  assert(intermediate_state == 2);
  assert(final_state == 3);
  assert(machine.GetCurrentStateId() == 0);
  assert(!machine.DoesReachFinalState());

  machine.AddTransition(0, intermediate_state, 'a');
  auto result = machine.ReceiveInput('a');
  assert(result == StateMachine<char>::StateTransitionResult::Unfinished);
  assert(machine.GetCurrentStateId() == intermediate_state);
  assert(!machine.DoesReachFinalState());

  machine.Reset();
  assert(machine.GetCurrentStateId() == 0);

  machine.AddTransition(0, final_state, 'b');
  machine.ReceiveInput('b');
  assert(machine.GetCurrentStateId() == final_state);
  assert(machine.DoesReachFinalState());
  assert(std::any_cast<std::string>(machine.GetFinalStateResult()) == "done");
}

static void TestStateMachineTransitions() {
  StateMachine<char> machine;

  std::vector<std::string> words = {"hello", "world", "hi"};

  lex::LayeredStateGraph layered_state_graph(words);
  layered_state_graph.BuildStateGraph();
  machine = layered_state_graph.ConstructStateMachine();

  std::vector<char> test_input = {'h', 'e', 'l', 'l', 'o'};
  assert(machine.GetCurrentStateId() == 0);
  machine.ReceiveInputs(test_input);
  assert(machine.GetCurrentStateId() == 6);
  assert(machine.CheckStateType(machine.GetCurrentStateId()) ==
         lex::StateType::Final);

  machine.Reset();
  test_input = {'w', 'o', 'r', 'l', 'd'};
  machine.ReceiveInputs(test_input);
  assert(machine.GetCurrentStateId() == 10);
  assert(machine.CheckStateType(machine.GetCurrentStateId()) ==
         lex::StateType::Final);
  machine.Reset();

  test_input = {'h', 'c', 'f', 'e', 'd'};
  machine.ReceiveInput('h');
  assert(machine.GetCurrentStateId() == 2);
  machine.ReceiveInput('c');
  assert(machine.GetCurrentStateId() == 1);
  assert(machine.CheckStateType(machine.GetCurrentStateId()) ==
         lex::StateType::Error);
}

int main() {
  std::cout << "Running StateMachine construction tests...\n";
  TestDefaultConstruction();
  TestAddStatesAndTransitions();
  TestStateMachineTransitions();
  std::cout << "StateMachine construction tests passed.\n";
  return 0;
}
