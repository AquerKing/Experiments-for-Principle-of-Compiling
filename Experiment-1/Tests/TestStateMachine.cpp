#include <cassert>
#include <iostream>
#include <vector>

#include "LayeredFSAGraph.h"
#include "StateMachine.h"
#include "Types.h"
#include "Utils.h"

void TestConstructStateMachine() {
  StateMachine SM;
  LayeredFSAGraph Graph;

  std::vector<std::vector<uchar>> WordList = {
      ConvertStringToUcharVector("hello"),
      ConvertStringToUcharVector("hi"),
      ConvertStringToUcharVector("hijack"),
  };

  Graph.AddWordList(WordList);
  Graph.BuildGraph();

  SM.BuildFromLayeredFSAGraph(Graph);

  std::vector<uchar> Inputs = ConvertStringToUcharVector("hello");
  std::vector<Token> Tokens = SM.ReceiveInputs(Inputs);
  assert(Tokens.size() == 1);
  std::cout << "Token.Content: " << Tokens[0] << std::endl;
}

int main() {
  std::cout << ">>> TestStateMachine Start <<<" << std::endl;
  TestConstructStateMachine();
  std::cout << ">>> TestStateMachine End <<<" << std::endl;
}