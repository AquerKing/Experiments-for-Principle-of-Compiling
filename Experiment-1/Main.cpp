#include "StateMachine.h"
#include "Utils.h"

#include <iostream>


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

  std::vector<uint32_t> Inputs = ConvertStringToU32Vector("hello");
  std::vector<Token> Tokens = SM.ReceiveInputs(Inputs);
  std::cout << "Token.Content: " << Tokens.front() << std::endl;
}

int main() {
  TestKeywordStateMachine();
  return 0;
}