#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "LayeredFSAGraph.h"
#include "StateMachine.h"
#include "Token.h"
#include "Utils.h"

void TestKeywordStateMachine() {
  StateMachine SM;
  LayeredFSAGraph Graph;

  std::vector<std::vector<uint32_t>> WordList = {
      ConvertStringToU32Vector("hello"),
      ConvertStringToU32Vector("hi"),
      ConvertStringToU32Vector("hijack"),
  };

  Graph.AddWordList(WordList);
  Graph.BuildGraph();

  SM.BuildFromLayeredFSAGraph(Graph);

  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("hello");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("hello"));
    assert(Tokens.front().IsValid());
  }
  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("hi");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("hi"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("hijack");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("hijack"));
    assert(Tokens.front().IsValid());
  }
  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("hijacke");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 0);
  }
  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("helloe");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 0);
  }
}

void TestSeparatorStateMachine() {
  StateMachine SM;
  LayeredFSAGraph Graph;

  std::vector<std::vector<uint32_t>> WordList = {
      {'('}, {')'}, {','}, {';'}, {'['}, {']'},
  };

  Graph.AddWordList(WordList);
  Graph.BuildGraph();
  SM.BuildFromLayeredFSAGraph(Graph);

  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("(([],);");
    std::vector<Token> Tokens = SM.ReceiveInputs(Inputs);
  }
}

int main() {
  std::cout << ">>> TestStateMachine Start <<<" << std::endl;
  TestKeywordStateMachine();
  std::cout << ">>> TestStateMachine End <<<" << std::endl;
}