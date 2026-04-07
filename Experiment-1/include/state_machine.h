#pragma once

#include <any>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "types.h"

namespace lex {

template <typename InputType> class __State;
template <typename InputType> class __FinalState;

enum class StateType { Intermediate, Final, Error };

template <typename InputType, typename IntermediateStateType,
          typename FinalStateType>
class __IStateful {
public:
  /**
   * Checks if the state is a final state.
   * @return true if the state is final, false otherwise.
   */
  virtual bool IsFinalState() const = 0;
  virtual bool IsErrorState() const noexcept = 0;
  virtual IntermediateStateType *GetIntermediateState() = 0;
  virtual FinalStateType *GetFinalState() = 0;
  virtual ~__IStateful() = default;
  virtual StateType GetStateType() const noexcept = 0;
};

/**
 * @brief A class representing a state in the state machine. This class should
 * not be used directly, only for inner implementation of the state machine.
 * @tparam InputType The type of input that can trigger transitions.
 */
template <typename InputType>
class __State : public __IStateful<InputType, __State<InputType>,
                                   __FinalState<InputType>> {
  typedef __IStateful<InputType, __State<InputType>, __FinalState<InputType>>
      __StatefulType;

public:
  bool IsFinalState() const override { return false; }
  bool IsErrorState() const noexcept override { return state_id == 1; }
  StateType GetStateType() const noexcept override { return state_type; }

  __State<InputType> *GetIntermediateState() override { return this; }
  __FinalState<InputType> *GetFinalState() override { return nullptr; }

  __State<InputType>(uint id) : state_id(id) {}

  uint ReceiveInput(const InputType &input) {
    if (transitions.find(input) == transitions.end()) {
      return 1; // No transition for this input, stay in the current state
    }
    return transitions[input];
  }

  void AddTransition(const InputType &input, uint to_state_id) {
    transitions[input] = to_state_id;
  }

  StateType state_type = StateType::Intermediate;

private:
  uint state_id;
  std::unordered_map<InputType, uint> transitions;
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
  typedef __IStateful<InputType, __State<InputType>, __FinalState<InputType>>
      __StatefulType;

public:
  bool IsFinalState() const override { return true; }
  bool IsErrorState() const noexcept override { return false; }
  StateType GetStateType() const noexcept override { return state_type; }

  __State<InputType> *GetIntermediateState() override { return nullptr; }
  __FinalState<InputType> *GetFinalState() override { return this; }

  __FinalState(uint state_id) : state_id(state_id) {}
  __FinalState(uint state_id, std::any result,
               StateType state_type = StateType::Final)
      : state_id(state_id), result(result), state_type(state_type) {}

  /**
   * @brief Gets the result associated with the final state.
   * @return The result as a std::any object.
   */
  std::any GetResult() const noexcept { return result; }

  StateType state_type = StateType::Final;

private:
  std::any result;
  uint state_id;
};

template <typename InputType> class StateMachine {
  typedef __IStateful<InputType, __State<InputType>, __FinalState<InputType>>
      __StatefulType;

public:
  enum class StateTransitionResult { Success, Error, Unfinished };

public:
  StateMachine() {
    states.push_back(std::make_shared<__State<InputType>>(0));
    states.push_back(
        std::make_shared<__FinalState<InputType>>(1, std::any("Error"), StateType::Error));
    state_id_index_map[0] = 0;
    state_id_index_map[1] = 1;
  }

  ~StateMachine() = default;

  /**
   * @brief Adds a new intermediate state to the state machine.
   * @return The ID of the newly added state.
   */
  uint AddState() {
    uint new_state_id = states.size();
    states.push_back(std::make_shared<__State<InputType>>(new_state_id));
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
    states.push_back(
        std::make_shared<__FinalState<InputType>>(new_state_id, result));
    state_id_index_map[new_state_id] = states.size() - 1;
    return new_state_id;
  }

  void AddTransition(uint from_state_id, uint to_state_id,
                     const InputType &input) {
    if (state_id_index_map.find(from_state_id) == state_id_index_map.end() ||
        state_id_index_map.find(to_state_id) == state_id_index_map.end()) {
      throw std::invalid_argument("State ID does not exist.");
    }
    auto &from_state_ptr = states[state_id_index_map[from_state_id]];
    if (from_state_ptr->IsFinalState()) {
      throw std::invalid_argument("Cannot add transition from a final state.");
    }
    auto *from_state = static_cast<__State<InputType> *>(from_state_ptr.get());
    from_state->AddTransition(input, to_state_id);
  }

  /**
   * @brief Gets the ID of the current state.
   * @return The ID of the current state.
   */
  uint GetCurrentStateId() const noexcept { return current_state_id; }

  /**
   * @brief Receives an input and transitions to the next state.
   * @param input The input to process.
   * @return True if the transition reached a final state, false otherwise.
   */
  StateTransitionResult ReceiveInput(const InputType &input) {
    auto &current_state_ptr = states[state_id_index_map[current_state_id]];
    if (current_state_ptr->IsFinalState()) {
      if (current_state_ptr->IsErrorState()) {
        return StateTransitionResult::Error; // Already in error state
      }
      return StateTransitionResult::Success; // Already in final state
    }
    auto *intermediate_state = current_state_ptr->GetIntermediateState();
    uint next_state_id = intermediate_state->ReceiveInput(input);
    current_state_id = next_state_id;
    return StateTransitionResult::Unfinished;
  }

  /**
   * @brief Receives a sequence of inputs and transitions to the next state.
   * @param inputs The vector of inputs to process.
   * @return The result of the transition.
   */
  StateTransitionResult ReceiveInputs(const std::vector<InputType> &inputs) {
    for (const auto &input : inputs) {
      StateTransitionResult result = ReceiveInput(input);
      if (result == StateTransitionResult::Error) {
        return StateTransitionResult::Error;
      }
      if (result == StateTransitionResult::Success) {
        return StateTransitionResult::Success;
      }
    }
    return StateTransitionResult::Unfinished;
  }

  /**
   * @brief Resets the state machine to its initial state.
   */
  void Reset() noexcept { current_state_id = 0; }

  /**
   * @brief Checks if the current state is a final state.
   * @return True if the current state is a final state, false otherwise.
   */
  bool DoesReachFinalState() const noexcept {
    return states[state_id_index_map.at(current_state_id)]->IsFinalState();
  }

  /**
   * @brief Gets the result associated with the final state.
   * @return The result as a std::any object.
   */
  std::any GetFinalStateResult() const {
    auto &current_state_ptr = states[state_id_index_map.at(current_state_id)];
    if (!current_state_ptr->IsFinalState()) {
      throw std::logic_error("Current state is not a final state.");
    }
    return current_state_ptr->GetFinalState()->GetResult();
  }

  StateType CheckStateType(uint state_id) const {
    if (state_id_index_map.find(state_id) == state_id_index_map.end()) {
      throw std::invalid_argument("State ID does not exist.");
    }
    return states[state_id_index_map.at(state_id)]->GetStateType();
  }

private:
  std::vector<std::shared_ptr<__StatefulType>> states;
  std::unordered_map<uint, uint> state_id_index_map;
  uint current_state_id = 0;
};

} // namespace lex