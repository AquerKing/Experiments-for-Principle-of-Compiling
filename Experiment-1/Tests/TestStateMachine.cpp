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
      ConvertStringToU32Vector("do"),     ConvertStringToU32Vector("end"),
      ConvertStringToU32Vector("for"),    ConvertStringToU32Vector("if"),
      ConvertStringToU32Vector("printf"), ConvertStringToU32Vector("scanf"),
      ConvertStringToU32Vector("then"),   ConvertStringToU32Vector("while"),
  };

  Graph.AddWordList(WordList);
  Graph.BuildGraph();

  SM.BuildFromLayeredFSAGraph(Graph);

  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("do");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("do"));
    assert(Tokens.front().IsValid());
  }
  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("end");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("end"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("for");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("for"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("if");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("if"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("printf");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("printf"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("scanf");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("scanf"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("then");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("then"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("while");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 1);
    assert(Tokens.front().Content == ConvertStringToU32Vector("while"));
    assert(Tokens.front().IsValid());
  }
  {
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("whileif");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
    assert(Tokens.size() == 0);
  }

  std::cout << " -- Keyword State Machine Test Completed -- " << std::endl;
}

void TestSeparatorStateMachine() {
  StateMachine SM;
  LayeredFSAGraph Graph;

  std::vector<std::vector<uint32_t>> WordList = {
      {static_cast<uint32_t>('(')}, {static_cast<uint32_t>(')')},
      {static_cast<uint32_t>(',')}, {static_cast<uint32_t>(';')},
      {static_cast<uint32_t>('[')}, {static_cast<uint32_t>(']')},
  };

  Graph.AddWordList(WordList);
  Graph.BuildGraph();
  SM.BuildFromLayeredFSAGraph(Graph);

  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("(([],);");
    std::vector<Token> Tokens = SM.ReceiveInputs(Inputs);
    assert(Tokens.size() == 7);
    assert(Tokens.at(0).Content == ConvertStringToU32Vector("("));
    assert(Tokens.at(1).Content == ConvertStringToU32Vector("("));
    assert(Tokens.at(2).Content == ConvertStringToU32Vector("["));
    assert(Tokens.at(3).Content == ConvertStringToU32Vector("]"));
    assert(Tokens.at(4).Content == ConvertStringToU32Vector(","));
    assert(Tokens.at(5).Content == ConvertStringToU32Vector(")"));
    assert(Tokens.at(6).Content == ConvertStringToU32Vector(";"));
  }

  std::cout << " -- Separator State Machine Test Completed -- " << std::endl;
}

void TestOperatorStateMachine() {
  StateMachine SM;
  LayeredFSAGraph Graph;

  std::vector<std::vector<uint32_t>> WordList = {
      {static_cast<uint32_t>('+')},   {static_cast<uint32_t>('-')},
      {static_cast<uint32_t>('*')},   {static_cast<uint32_t>('/')},
      ConvertStringToU32Vector("<"),  ConvertStringToU32Vector("<="),
      ConvertStringToU32Vector("="),  ConvertStringToU32Vector(">"),
      ConvertStringToU32Vector(">="), ConvertStringToU32Vector("<>"),
  };

  Graph.AddWordList(WordList);
  Graph.BuildGraph();
  SM.BuildFromLayeredFSAGraph(Graph);

  {
    SM.Reset();
    std::vector<uint32_t> Inputs = ConvertStringToU32Vector("+");
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
  }

  std::cout << " -- Operator State Machine Test Completed -- " << std::endl;
}

int main() {
  std::cout << ">>> TestStateMachine Start <<<" << std::endl;
  TestKeywordStateMachine();
  TestSeparatorStateMachine();
  std::cout << ">>> TestStateMachine End <<<" << std::endl;
}