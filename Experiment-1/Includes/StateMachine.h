#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "LayeredFSAGraph.h"
#include "State.h"
#include "Token.h"
#include "Types.h"

typedef ulong StateMachineID;

class LayeredFSAGraph;

class StateMachine {
public:
  StateMachine() : StateMachineReady(true), CurrentStateID(0) {}

  StateID ReceiveInput(InputType Input) {
    std::shared_ptr<State> CurrentState =
        StateManager.GetStateObject(CurrentStateID).lock();

    StateID NextStateID = CurrentState->Transit(Input);
    std::shared_ptr<State> NextState =
        StateManager.GetStateObject(NextStateID).lock();

    CurrentStateID = NextStateID;
    NextState->ExecuteStrategy(Cache, Input);

    StateMachineReady = false;

    return NextStateID;
  }

  std::vector<Token> ReceiveInputs(std::vector<InputType> Inputs) {
    std::vector<Token> Tokens;

    for (InputType &Input : Inputs) {
      ReceiveInput(Input);
      if (!IsEndState(CurrentStateID)) {
        continue;
      }

      Token Token = Cache.GetToken();
      Tokens.emplace_back(Token);

      Reset();
    }

    return Tokens;
  }

  std::optional<Token> TryGetToken() {
    std::shared_ptr<State> State =
        StateManager.GetStateObject(CurrentStateID).lock();

    if (State->GetStateType() == StateType::End) {
      return Cache.GetToken();
    }

    return std::nullopt;
  }

  bool IsEndState(StateID ID) const {
    StateType Type = StateManager.GetStateType(ID);
    return Type == StateType::End || Type == StateType::Error;
  }

  void BuildFromLayeredFSAGraph(const LayeredFSAGraph &Graph) {
    if (!StateMachineReady) {
      throw std::runtime_error("StateMachine: State machine should be reset "
                               "before building.");
      return;
    }

    std::unordered_set<StateID> StateIDs;

    for (auto &State : Graph.StateIDMap) {
      StateID ID = StateManager.CreateStateByID(State.second);

      if (State.second != ID) {
        throw std::runtime_error("StateMachine: Miss matched state ID.");
      }

      StateIDs.insert(ID);
    }

    // Update state configs
    for (auto StateID : StateIDs) {
      StateConfig Config;
      Config.Strategy = {
          StateConfig::StatePostTransitionStrategy::Append,
          [](TokenCache &Cache, InputType Input) { Cache.Append(Input); }};
      Config.TransitionMap = Graph.TransitionMaps.at(StateID);
      Config.Type = StateType::Intermediate;

      StateManager.UpdateStateConfig(StateID, Config);
    }

    // Update start state config
    {
      StateConfig Config;
      Config.Strategy = {
          StateConfig::StatePostTransitionStrategy::Ignore,
          [](TokenCache &Cache, InputType Input) { Cache.Append(Input); }};
      Config.TransitionMap = Graph.TransitionMaps.at(0);
      Config.Type = StateType::Start;

      StateManager.UpdateStateConfig(0, Config);
    }

    // Update end states' type
    for (auto StateID : Graph.EndStates) {
      StateManager.UpdateStateType(StateID, StateType::End);
    }
  }

  void Reset() {
    CurrentStateID = StartStateID;
    ContextInfo = TokenContextInfo::Invalid;
    Cache.Clear();
    StateMachineReady = true;
  }

private:
  StateMachineID ID;
  StateManager StateManager;
  StateID CurrentStateID;
  TokenCache Cache;
  TokenContextInfo ContextInfo;

  bool StateMachineReady;
};