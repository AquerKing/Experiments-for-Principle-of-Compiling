#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Token.h"

class StateManager;

typedef uint64_t StateID;

extern const StateID StartStateID;
extern const StateID DefaultEndStateID;

/** A cache for storing token information. */
class TokenCache {
public:
  void Append(uint8_t Input) { Sequence.emplace_back(Input); }

  void Clear() { Sequence.clear(); }

  std::vector<uint8_t> GetCache() const { return Sequence; }

  void SetType(TokenType NewType) { Type = NewType; }

  Token GetToken() const {
    Token Token;

    if (!Sequence.empty()) {
      Token.Type = Type;
      Token.Context = ContextInfo;
      Token.Content.assign(Sequence.begin(), Sequence.end());
    } else {
      Token.Type = TokenType::Invalid;
    }

    return Token;
  }

private:
  std::vector<uint8_t> Sequence;
  TokenContextInfo ContextInfo;
  TokenType Type;
};

enum class StateType : uint8_t {
  Start,
  End,
  Error,
  Intermediate,
};

/** Configuration for a state in the state machine. */
struct StateConfig {

  struct StatePostTransitionStrategy {
    enum Strategy : uint8_t {
      Append,
      Ignore,
      Clear,
    };

    Strategy Strategy;
    std::function<void(TokenCache &, uint8_t Input)> OperatorCallback;
  };

  StateType Type;
  TokenType CacheFlag;
  StatePostTransitionStrategy Strategy;
  std::unordered_map<uint8_t, StateID> TransitionMap;
};

/** Strategy for handling state flags. */
struct StateFlagStrategy {
  TokenType DefaultIntermediateStateFlag = TokenType::Invalid;
  TokenType DefaultEndStateFlag = TokenType::Token;
  TokenType DefaultReservedEndStateFlag = TokenType::Error;
};

/** A class representing a state in the state machine. */
class State {
  friend class StateManager;

public:
  State(StateID ID) : ID(ID) {}

  StateID Transit(uint8_t Input) {
    StateID NextStateID;

    if (TransitionMap.find(Input) != TransitionMap.end()) {
      NextStateID = TransitionMap[Input];
    } else {
      NextStateID = DefaultEndStateID;
    }

    return NextStateID;
  }

  void UpdateConfig(const StateConfig &Config) {
    Strategy = Config.Strategy;
    TransitionMap = Config.TransitionMap;
    Type = Config.Type;
    TypeFlag = Config.CacheFlag;
  }

  void UpdateType(StateType NewType) { Type = NewType; }

  void ExecuteStrategy(TokenCache &Cache, uint8_t Input) {
    Cache.SetType(TypeFlag);

    if (!Strategy.OperatorCallback) {
      return;
    }
    Strategy.OperatorCallback(Cache, Input);
  }

  StateType GetStateType() const { return Type; }

private:
  StateID ID;
  StateConfig::StatePostTransitionStrategy Strategy;
  TokenType TypeFlag;
  StateType Type;
  std::unordered_map<uint8_t, StateID> TransitionMap;
};

class StateManager {
public:
  enum class StateConfigUpdateResult : uint8_t {
    Success,
    NonexistedState,
  };

public:
  StateManager(StateType DefaultEndStateType = StateType::End)
      : NextStateID(2) {
    UsedID.insert(0); // Start state
    UsedID.insert(1); // Default end state
    StateIDMap[0] = std::make_shared<State>(0);
    StateIDMap[1] = std::make_shared<State>(1);

    // Initialize start state and default end state configs
    StateConfig StartStateConfig;
    StartStateConfig.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
      [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    StartStateConfig.Type = StateType::Start;
    StartStateConfig.CacheFlag = TokenType::Invalid;
    UpdateStateConfig(0, StartStateConfig);

    // Default end state config will be updated in BuildFromLayeredDFAGraph
    // based on the provided strategy
    StateConfig DefaultEndStateConfig;
    StartStateConfig.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
      [](TokenCache &Cache, uint8_t Input) { Cache.Append(Input); }};
    DefaultEndStateConfig.Type = StateType::End;
    DefaultEndStateConfig.CacheFlag = TokenType::Error;
    UpdateStateConfig(1, StartStateConfig);
  }

  StateManager(const StateManager &Other) = delete;
  StateManager &operator=(StateManager &Other) = delete;
  StateManager(StateManager &&Other) = delete;
  StateManager &operator=(StateManager &&Other) = delete;

  /**
   * @brief Create a State object and return its ID. State IDs 0, 1 and 2 are
   * pre-allocated for start state, default end state and reserved end state, so
   * the first created state ID starts from 3.
   *
   * @return StateID
   */
  StateID CreateState() {
    StateID NewID = AllocateStateID();
    StateIDMap[NewID] = std::make_shared<State>(NewID);
    return NewID;
  }

  /**
   * @brief Create a State object with a specific ID.
   *
   * @param ID The ID for the new state.
   * @return StateID
   */
  StateID CreateStateByID(StateID ID) {
    CheckID(ID);

    StateID NewID = ID;

    if (IsStateIdOccupied(NewID)) {
      NewID = AllocateStateID();
    }

    StateIDMap[NewID] = std::make_shared<State>(NewID);
    return NewID;
  }

  /**
   * @brief Get a weak pointer to the state object by its ID.
   *
   * @param ID The ID of the state to retrieve.
   * @return std::weak_ptr<State>
   */
  std::weak_ptr<State> GetStateObject(StateID ID) const {
    auto It = StateIDMap.find(ID);
    if (It == StateIDMap.end()) {
      return {};
    }
    return It->second;
  }

  /**
   * @brief Check if a state ID is valid.
   *
   * @param ID The ID to check.
   */
  void CheckID(StateID ID) {
    auto It = StateIDMap.find(ID);
    if (It == StateIDMap.end()) {
      if (UsedID.count(ID) > 0) {
        UsedID.erase(ID);
      }
    } else {
      if (UsedID.count(ID) == 0) {
        UsedID.insert(ID);
      }
    }
  }

  /**
   * @brief Check if a state ID is occupied.
   *
   * @param ID The ID to check.
   * @return bool
   */
  bool IsStateIdOccupied(StateID ID) const {
    return StateIDMap.find(ID) != StateIDMap.end();
  }

  /**
   * @brief Update the configuration of a state.
   *
   * @param ID The ID of the state to update.
   * @param Config The new configuration for the state.
   * @return StateConfigUpdateResult
   */
  StateConfigUpdateResult UpdateStateConfig(StateID ID,
                                            const StateConfig &Config) {
    assert(StateIDMap.find(ID) != StateIDMap.end());

    std::shared_ptr<State> State = StateIDMap.at(ID);

    if (!State) {
      return StateConfigUpdateResult::NonexistedState;
    }

    State->UpdateConfig(Config);

    return StateConfigUpdateResult::Success;
  }

  /**
   * @brief Update the type of a state.
   *
   * @param ID The ID of the state to update.
   * @param Type The new type for the state.
   */
  void UpdateStateType(StateID ID, StateType Type) {
    assert(StateIDMap.find(ID) != StateIDMap.end());

    std::shared_ptr<State> State = StateIDMap.at(ID);
    State->UpdateType(Type);
  }

  /**
   * @brief Get the type of a state.
   *
   * @param ID The ID of the state to retrieve.
   * @return StateType
   */
  StateType GetStateType(StateID ID) const {
    assert(StateIDMap.find(ID) != StateIDMap.end());
    return StateIDMap.at(ID)->GetStateType();
  }

  /**
   * @brief Update the flag of a state.
   *
   * @param ID The ID of the state to update.
   * @param Type The new flag for the state.
   */
  void UpdateStateFlag(StateID ID, TokenType Type) {
    assert(StateIDMap.find(ID) != StateIDMap.end());
    GetStateObject(ID).lock()->TypeFlag = Type;
  }

private:
  /**
   * @brief Allocate a new state ID. State IDs 0, 1 and 2 are pre-allocated for
   * start state, default end state and reserved end state, so the first
   * allocated state ID starts from 3.
   *
   * @return StateID
   */
  StateID AllocateStateID() {
    for (; NextStateID >= 3; ++NextStateID) {
      if (UsedID.count(NextStateID) == 0) {
        UsedID.insert(NextStateID);
        return NextStateID++;
      }
    }
    throw std::runtime_error("StateManager: State count overflowed.");
  }

  StateID NextStateID; // State IDs 0, 1 and 2 are pre-allocated for start
                       // state, default end state and reserved end state
  std::unordered_set<StateID>
      UsedID; // Used state IDs, used for state ID allocation
  std::unordered_map<StateID, std::shared_ptr<State>>
      StateIDMap; // StateID -> State object mapping
};