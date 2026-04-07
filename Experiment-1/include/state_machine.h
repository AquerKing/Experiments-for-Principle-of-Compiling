#pragma once

#include <any>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "types.h"

namespace lex {

template <typename InputType> class __State;
template <typename InputType> class __FinalState;

template <typename InputType, typename IntermediateStateType,
          typename FinalStateType>
class __IStateful {
  /**
   * Checks if the state is a final state.
   * @return true if the state is final, false otherwise.
   */
  virtual bool IsFinalState() const = 0;
  virtual IntermediateStateType *GetIntermediateState() = 0;
  virtual FinalStateType *GetFinalState() = 0;
};

/**
 * @brief A class representing a state in the state machine. This class should
 * not be used directly, only for inner implementation of the state machine.
 * @tparam InputType The type of input that can trigger transitions.
 */
template <typename InputType>
class __State : public __IStateful<InputType, __State<InputType>,
                                   __FinalState<InputType>> {
public:
  bool IsFinalState() const override { return false; }
  __State<InputType> *GetIntermediateState() override { return this; }
  __FinalState<InputType> *GetFinalState() override { return nullptr; }

  __State<InputType>(uint id) : state_id(id) {}

  __State &ReceiveInput(const InputType &input);

private:
  uint state_id;
  std::unordered_map<InputType, __IStateful<InputType, __State<InputType>,
                                            __FinalState<InputType>>>
      transitions;
};

/**
 * @brief A class representing a final state in the state machine. This class
 * should not be used directly, only for inner implementation of the state
 * machine.
 * @tparam InputType The type of input that can trigger transitions.
 */
template <typename InputType>
class __FinalState : public __IStateful<InputType, __State<InputType>,
                                        __FinalState<InputType>> {
public:
  bool IsFinalState() const override { return true; }
  __State<InputType> *GetIntermediateState() override { return nullptr; }
  __FinalState<InputType> *GetFinalState() override { return this; }

  __FinalState(uint state_id) : state_id(state_id) {}
  __FinalState(uint state_id, std::any result)
      : state_id(state_id), result(result) {}

  /**
   * @brief Gets the result associated with the final state.
   * @return The result as a std::any object.
   */
  std::any GetResult() const noexcept { return result; }

private:
  std::any result;
  uint state_id;
};

template <typename InputType> class StateMachine {
public:
  StateMachine() {
    states.emplace_back(0);                    // Initial state
    states.emplace_back(1, std::any("Error")); // Final state
    state_id_index_map[0] = 0;                 // Initial state ID
    state_id_index_map[1] = 1;                 // Final state ID
  }

  ~StateMachine() = default;

  /**
   * @brief Adds a new intermediate state to the state machine.
   * @return The ID of the newly added state.
   */
  uint AddState() {
    uint new_state_id = states.size();
    states.emplace_back(new_state_id);
    state_id_index_map[new_state_id] = states.size() - 1;
    return new_state_id;
  }

  /**
   * @brief Adds a new final state to the state machine.
   * @param result The result associated with the final state.
   * @return The ID of the newly added state.
   */
  uint AddFinalState(std::any result = {}) {
    uint new_state_id = states.size();
    states.emplace_back(new_state_id, result);
    state_id_index_map[new_state_id] = states.size() - 1;
    return new_state_id;
  }

  /**
   * @brief Gets the ID of the current state.
   * @return The ID of the current state.
   */
  uint GetCurrentStateId() const noexcept { return current_state_id; }

  /**
   * @brief Receives an input and transitions to the next state.
   * @param input The input to process.
   */
  void ReceiveInput(const InputType &input) {
    auto &current_state = states[state_id_index_map[current_state_id]];
    auto next_state_ptr =
        current_state.GetIntermediateState()->ReceiveInput(input);
    if (next_state_ptr == nullptr) {
      current_state_id = 1; // Transition to error state
    } else {
      current_state_id = next_state_ptr->state_id;
    }
  }

  /**
   * @brief Checks if the current state is a final state.
   * @return True if the current state is a final state, false otherwise.
   */
  bool ReachFinalState() const noexcept {
    auto &current_state = states[state_id_index_map.at(current_state_id)];
    return current_state.IsFinalState();
  }

  /**
   * @brief Gets the result associated with the final state.
   * @return The result as a std::any object.
   */
  std::any GetFinalStateResult() const {
    auto &current_state = states[state_id_index_map.at(current_state_id)];
    if (!current_state.IsFinalState()) {
      throw std::logic_error("Current state is not a final state.");
    }
    return current_state.GetFinalState()->GetResult();
  }

private:
  std::vector<
      __IStateful<InputType, __State<InputType>, __FinalState<InputType>>>
      states;
  std::unordered_map<uint, uint> state_id_index_map;
  uint current_state_id = 0;
};

} // namespace lex