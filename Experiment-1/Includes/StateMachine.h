#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "LayeredDFAGraph.h"
#include "State.h"
#include "Token.h"

typedef uint64_t StateMachineID;

class LayeredDFAGraph;

/** A class representing a state machine. */
class StateMachine {
public:
  enum class TokenGenerationStrategy : uint8_t {
    GenerateSoonIfPossible,
    GenerateAtLast,
  };

public:
  StateMachine() : StateMachineReady(true), CurrentStateID(0) {}

  /**
   * @brief Receive an input and transition to the next state.
   *
   * @param Input The input to process.
   * @return StateID The ID of the next state.
   */
  StateID ReceiveInput(uint32_t Input) {
    std::shared_ptr<State> CurrentState =
        Manager.GetStateObject(CurrentStateID).lock();

    StateID NextStateID = CurrentState->Transit(Input);
    std::shared_ptr<State> NextState =
        Manager.GetStateObject(NextStateID).lock();

    CurrentStateID = NextStateID;
    NextState->ExecuteStrategy(Cache, Input);

    StateMachineReady = false;

    return NextStateID;
  }

  /**
   * @brief Receive a sequence of inputs and generate tokens according to the
   * specified, token generation strategy.
   *
   * @param Inputs
   * @param GenerationStrategy
   * @return std::vector<Token>
   */
  std::vector<Token> ReceiveInputs(
      std::vector<uint32_t> Inputs,
      TokenGenerationStrategy GenerationStrategy =
          StateMachine::TokenGenerationStrategy::GenerateSoonIfPossible) {
    std::vector<Token> Tokens;

    for (size_t i = 0; i < Inputs.size(); ++i) {
      uint32_t Input = Inputs[i];

      ReceiveInput(Input);

      if (!IsEndState(CurrentStateID)) {
        continue;
      }

      Token TempToken = Cache.GetToken();

      switch (GenerationStrategy) {
      case TokenGenerationStrategy::GenerateSoonIfPossible:
        if (TempToken.IsValid()) {
          Tokens.emplace_back(TempToken);
        }
        Reset();
        break;
      case TokenGenerationStrategy::GenerateAtLast:
        if (i == Inputs.size() - 1 && TempToken.IsValid()) {
          Tokens.emplace_back(Cache.GetToken());
          Reset();
        }
        break;
      }
    }

    return Tokens;
  }

  /**
   * @brief Try to get a token from the current state.
   *
   * @return std::optional<Token> The token if the current state is an end
   * state, otherwise std::nullopt.
   */
  std::optional<Token> TryGetToken() {
    std::shared_ptr<State> State =
        Manager.GetStateObject(CurrentStateID).lock();

    if (State->GetStateType() == StateType::End) {
      return Cache.GetToken();
    }

    return std::nullopt;
  }

  /**
   * @brief Check if a state is an end state.
   *
   * @param ID The ID of the state to check.
   * @return bool
   */
  bool IsEndState(StateID ID) const {
    StateType Type = Manager.GetStateType(ID);
    return Type == StateType::End || Type == StateType::Error;
  }

  /**
   * @brief Build the state machine from a layered DFA graph.
   *
   * @param Graph The layered DFA graph to build from.
   */
  void BuildFromLayeredDFAGraph(const LayeredDFAGraph &Graph) {
    if (!StateMachineReady) {
      throw std::runtime_error("StateMachine: State machine should be reset "
                               "before building.");
      return;
    }

    std::unordered_set<StateID> StateIDs;

    for (auto &State : Graph.StateIDMap) {
      StateID ID = Manager.CreateStateByID(State.second);

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
          [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
      Config.TransitionMap = Graph.TransitionMaps.at(StateID);
      Config.Type = StateType::Intermediate;
      Config.CacheFlag = Graph.FlagStrategy.DefaultIntermediateStateFlag;

      Manager.UpdateStateConfig(StateID, Config);
    }

    // Update start state config
    {
      StateConfig Config;
      Config.Strategy = {
          StateConfig::StatePostTransitionStrategy::Ignore,
          [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
      Config.TransitionMap = Graph.TransitionMaps.at(0);
      Config.Type = StateType::Start;
      Config.CacheFlag = TokenType::Invalid;

      Manager.UpdateStateConfig(0, Config);
    }

    // Update default end state config
    {
      StateConfig Config;
      Config.Strategy = {
          StateConfig::StatePostTransitionStrategy::Append,
          [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
      Config.TransitionMap = {};
      Config.Type = StateType::Error;
      Config.CacheFlag = Graph.FlagStrategy.DefaultReservedEndStateFlag;

      Manager.UpdateStateConfig(1, Config);
    }

    // Update end states' type
    for (auto StateID : Graph.EndStates) {
      Manager.UpdateStateType(StateID, StateType::End);
      Manager.UpdateStateFlag(StateID, Graph.FlagStrategy.DefaultEndStateFlag);
    }
  }

  /**
   * @brief Reset the state machine to its initial state.
   */
  void Reset() {
    CurrentStateID = StartStateID;
    ContextInfo = TokenContextInfo::Invalid;
    Cache.Clear();
    StateMachineReady = true;
  }

private:
  StateMachineID ID;
  StateManager Manager;
  StateID CurrentStateID;
  TokenCache Cache;
  TokenContextInfo ContextInfo;

  bool StateMachineReady;
};