#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Token.h"
#include "Types.h"

typedef ulong StateID;
typedef uchar InputType;

extern const StateID StartStateID;
extern const StateID DefaultEndStateID;

class TokenCache {
public:
  void Append(InputType Input) { Sequence.emplace_back(Input); }

  void Clear() { Sequence.clear(); }

  std::vector<InputType> GetCache() const { return Sequence; }

  void SetType(TokenType NewType) { Type = NewType; }

  Token GetToken() const {
    Token Token;

    if (!Token.Content.empty()) {
      Token.Type = Type;
      Token.Context = ContextInfo;
      Token.Content = Sequence;
    } else {
      Token.Type = TokenType::Invalid;
    }

    return Token;
  }

private:
  std::vector<InputType> Sequence;
  TokenContextInfo ContextInfo;
  TokenType Type;
};

enum class StateType : u8 {
  Start,
  End,
  Error,
  Intermediate,
};

struct StateConfig {

  struct StatePostTransitionStrategy {
    enum Strategy : u8 {
      Append,
      Ignore,
      Clear,
    };

    Strategy Strategy;
    std::function<void(TokenCache &, InputType Input)> OperatorCallback;
  };

  StateType Type;
  StatePostTransitionStrategy Strategy;
  std::unordered_map<InputType, StateID> TransitionMap;
};

class State {
public:
  State(StateID ID) : ID(ID) {}

  StateID Transit(InputType Input) {
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
    StateType = Config.Type;
  }

  void TryCallTransitionCallback(TokenCache &Cache, InputType Input) {
    if (!Strategy.OperatorCallback) {
      return;
    }
    Strategy.OperatorCallback(Cache, Input);
  }

  StateType GetStateType() const { return StateType; }

private:
  StateID ID;
  StateConfig::StatePostTransitionStrategy Strategy;
  StateType StateType;
  std::unordered_map<InputType, StateID> TransitionMap;
};

class StateManager {
public:
  enum class StateConfigUpdateResult : u8 {
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

    StateConfig StartStateConfig;
    StartStateConfig.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, InputType Input) { Cache.Append(Input); }};
    StartStateConfig.Type = StateType::Start;
    UpdateStateConfig(0, StartStateConfig);

    StateConfig DefaultEndStateConfig;
    StartStateConfig.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, InputType Input) { Cache.Append(Input); }};
    DefaultEndStateConfig.Type = StateType::End;
    UpdateStateConfig(0, StartStateConfig);
  }

  StateManager(const StateManager &Other) = delete;
  StateManager &operator=(StateManager &Other) = delete;
  StateManager(StateManager &&Other) = delete;
  StateManager &operator=(StateManager &&Other) = delete;

  StateID CreateState() {
    StateID NewID = AllocateStateID();
    StateIDMap[NewID] = std::make_shared<State>(NewID);
    return NewID;
  }

  StateID CreateStateByID(StateID ID) {
    CheckID(ID);

    StateID NewID = ID;

    if (IsStateIdOccupied(NewID)) {
      NewID = AllocateStateID();
    }

    StateIDMap[NewID] = std::make_shared<State>(NewID);
    return NewID;
  }

  std::weak_ptr<State> GetStateObject(StateID ID) const {
    auto It = StateIDMap.find(ID);
    if (It == StateIDMap.end()) {
      return {};
    }
    return It->second;
  }

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

  bool IsStateIdOccupied(StateID ID) const {
    return StateIDMap.find(ID) != StateIDMap.end();
  }

  StateConfigUpdateResult UpdateStateConfig(StateID ID,
                                            const StateConfig &Config) {
    std::shared_ptr<State> State = GetStateObject(ID).lock();

    if (!State) {
      return StateConfigUpdateResult::NonexistedState;
    }

    State->UpdateConfig(Config);

    return StateConfigUpdateResult::Success;
  }

private:
  StateID AllocateStateID() {
    for (; NextStateID >= 3; ++NextStateID) {
      if (UsedID.count(NextStateID) == 0) {
        UsedID.insert(NextStateID);
        return NextStateID++;
      }
    }
    throw std::runtime_error("StateManager: State count overflowed.");
  }

  StateID NextStateID;
  std::unordered_set<StateID> UsedID;
  std::unordered_map<StateID, std::shared_ptr<State>> StateIDMap;
};