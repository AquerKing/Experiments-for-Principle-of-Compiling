#pragma once

#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "types.h"

namespace lex {

class State;
class StateMachine;

struct StateID {
  static StateID Start;
  static StateID Error;
  static StateID Invalid;

  u32 id;
};

struct StateMachineID {
  u32 id;
  u32 priority;
};

struct __StateID_Hash {
  std::size_t operator()(const StateID &state_id) const {
    return std::hash<u32>()(state_id.id);
  }
};

typedef std::vector<uchar> InputSequence, CharacterVector;
typedef std::unordered_map<uchar, std::vector<StateID>> TransitionMap;
typedef std::unordered_set<StateID, __StateID_Hash> StateIDSet;

class TransitionResult {
public:
  enum class Type {
    Intermediate,
    Final,
    Error,
  };

public:
  TransitionResult(Type type, std::optional<std::any> value = std::nullopt)
      : type(type), value(value) {}

  Type GetType() const { return type; }

  std::optional<std::any> GetValue() const { return value; }

private:
  Type type;
  std::optional<std::any> value;
};

class State {

public:
  enum class ValueContainment {
    None,
    Any,
  };

  enum class Type {
    Start,
    Intermediate,
    End,
    Error,
    Invalid,
  };

  enum class InputStrategy {
    AppendAnyInput,
    RejectAnyInput,
    AcceptSpecificInputs,
  };

public:
  State(StateID id, Type type = Type::Invalid,
        InputStrategy input_strategy = InputStrategy::RejectAnyInput,
        ValueContainment value_containment = ValueContainment::None)
      : id(id), type(type), input_strategy(input_strategy),
        value_containment(value_containment) {}

  bool Equals(const State &other) const { return id.id == other.id.id; }

  Type GetType() const { return type; }

  InputStrategy GetInputStrategy() const { return input_strategy; }

  ValueContainment GetValueContainment() const { return value_containment; }

  template <typename T> std::optional<T> GetValue() const {
    if (value.has_value()) {
      try {
        return std::any_cast<T>(value.value());
      } catch (const std::bad_any_cast &) {
        return std::nullopt;
      }
    }
    return std::nullopt;
  }

  StateIDSet ReceiveInput(uchar input) const {
    auto it = transitions.find(input);
    if (it != transitions.end()) {
      StateIDSet next_states;
      for (const auto &state_id : it->second) {
        next_states.insert(state_id);
      }
      if (next_states.empty()) {
        next_states.insert(StateID::Invalid);
      }
      return next_states;
    }
    return {StateID::Invalid};
  }

private:
  StateID id;
  Type type;
  InputStrategy input_strategy;
  ValueContainment value_containment;

  TransitionMap transitions;
  std::weak_ptr<StateMachine> state_machine;
  std::optional<std::any> value;
};

class StateMachine {
public:
  StateMachine() {
    state_ids.reserve(3);
    state_ids.emplace_back(StateID::Start);
    state_ids.emplace_back(StateID::Error);
    state_ids.emplace_back(StateID::Invalid);

    states_id_map.emplace(StateID::Start,
                          State(StateID::Start, State::Type::Start));
    states_id_map.emplace(StateID::Error,
                          State(StateID::Error, State::Type::Error));
    states_id_map.emplace(StateID::Invalid,
                          State(StateID::Invalid, State::Type::Invalid));
  }

  std::vector<TransitionResult>
  ReceiveInputs(const InputSequence &input_sequence) const {
    
  }

private:
  u32 GetNewStateID() {
    if (state_counter <= 2) {
      throw std::runtime_error("StateID counter overflow");
    }
    return state_counter++;
  }

  std::vector<StateID> state_ids;
  std::unordered_map<StateID, State, __StateID_Hash> states_id_map;

  u32 state_counter = 3; // Start, Error, Invalid are reserved
};

typedef State::Type StateType;
typedef State::InputStrategy StateInputStrategy;

} // namespace lex