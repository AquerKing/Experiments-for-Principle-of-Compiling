#include "StateMachine.h"
#include "Utils.h"

#include <iostream>

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
  std::cout << "Token.Content: " << Tokens[0] << std::endl;
}

int main() {
  TestConstructStateMachine();
  return 0;
}