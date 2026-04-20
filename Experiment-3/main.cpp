// #include "ui/mainwindow.h"
//
// #include <QApplication>
//
// int main(int argc, char *argv[]) {
//   QApplication a(argc, argv);
//   MainWindow w;
//   w.show();
//   return QCoreApplication::exec();
// }

#include "grammar.h"
#include "symbols.h"
#include "utils.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

template <typename T> class MyStack : public std::stack<T, std::vector<T>> {
public:
  std::vector<T> &get_vector() { return this->c; }

  const std::vector<T> &get_vector() const { return this->c; }
};

template <typename T, typename Container = std::deque<T>>
class MyQueue : public std::queue<T, Container> {
public:
  std::vector<T> get_vector() const {
    return std::vector<T>(this->c.begin(), this->c.end());
  }
};

struct Action {
  enum Type { Shift, Reduce, Accept, Error } type;
  uint64_t value; // For Shift, it's the next state; for Reduce, it's the
                  // production rule index
};

std::unordered_map<uint64_t, std::unordered_map<uint64_t, Action>> actionTable;
std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> gotoTable;

void buildAnalysisTable(SymbolManager &symbolManager);
bool isActionTableItemExists(uint64_t state, uint64_t symbolId);
bool isGotoTableItemExists(uint64_t state, uint64_t symbolId);
void printAnalysisStep(const SymbolManager &symbolManager,
                       const MyStack<uint64_t> &stateStack,
                       const MyStack<uint64_t> &symbolStack,
                       const MyQueue<uint64_t> &inputQueue,
                       const Action &action,
                       const std::vector<GenerativeExpression> &productions);
std::string convertNumberVectorToString(const std::vector<uint64_t> &numbers);

void analyzeInputString(
    std::string &input, SymbolManager &symbolManager,
    const std::vector<GenerativeExpression> &generativeExpressions);

int main() {
  SymbolManager symbolManager;
  std::vector<std::string> generativeExpressions = {
      "E->E+T", "E->T", "T->T*F", "T->F", "F->(E)", "F->i",
  };
  std::vector<GenerativeExpression> parsedExpressions;

  for (const auto &expr : generativeExpressions) {
    std::vector<GenerativeExpression> parsed =
        GrammarUtils::ParseGenerativeExpressions(expr, symbolManager);
    parsedExpressions.insert(parsedExpressions.end(), parsed.begin(),
                             parsed.end());
  }

  // Build the LR(1) analysis table manually based on the parsed generative
  // expressions.
  buildAnalysisTable(symbolManager);

  std::string input;
  std::cout << "Enter an input string to analyze (end with #): ";
  std::cin >> input;
  analyzeInputString(input, symbolManager, parsedExpressions);

  return 0;
}

void buildAnalysisTable(SymbolManager &symbolManager) {
  actionTable.clear();
  gotoTable.clear();

  // build action table
  actionTable[0][symbolManager.GetSymbolIdByValue("i")] = {Action::Shift, 5};
  actionTable[0][symbolManager.GetSymbolIdByValue("(")] = {Action::Shift, 4};
  actionTable[1][symbolManager.GetSymbolIdByValue("+")] = {Action::Shift, 6};
  actionTable[1][symbolManager.GetSymbolIdByValue("#")] = {Action::Accept, 0};
  actionTable[2][symbolManager.GetSymbolIdByValue("+")] = {Action::Reduce, 2};
  actionTable[2][symbolManager.GetSymbolIdByValue("*")] = {Action::Shift, 7};
  actionTable[2][symbolManager.GetSymbolIdByValue(")")] = {Action::Reduce, 2};
  actionTable[2][symbolManager.GetSymbolIdByValue("#")] = {Action::Reduce, 2};
  actionTable[3][symbolManager.GetSymbolIdByValue("+")] = {Action::Reduce, 4};
  actionTable[3][symbolManager.GetSymbolIdByValue("*")] = {Action::Reduce, 4};
  actionTable[3][symbolManager.GetSymbolIdByValue(")")] = {Action::Reduce, 4};
  actionTable[3][symbolManager.GetSymbolIdByValue("#")] = {Action::Reduce, 4};
  actionTable[4][symbolManager.GetSymbolIdByValue("i")] = {Action::Shift, 5};
  actionTable[4][symbolManager.GetSymbolIdByValue("(")] = {Action::Shift, 4};
  actionTable[5][symbolManager.GetSymbolIdByValue("+")] = {Action::Reduce, 6};
  actionTable[5][symbolManager.GetSymbolIdByValue("*")] = {Action::Reduce, 6};
  actionTable[5][symbolManager.GetSymbolIdByValue(")")] = {Action::Reduce, 6};
  actionTable[5][symbolManager.GetSymbolIdByValue("#")] = {Action::Reduce, 6};
  actionTable[6][symbolManager.GetSymbolIdByValue("i")] = {Action::Shift, 5};
  actionTable[6][symbolManager.GetSymbolIdByValue("(")] = {Action::Shift, 4};
  actionTable[7][symbolManager.GetSymbolIdByValue("i")] = {Action::Shift, 5};
  actionTable[7][symbolManager.GetSymbolIdByValue("(")] = {Action::Shift, 4};
  actionTable[8][symbolManager.GetSymbolIdByValue("+")] = {Action::Shift, 6};
  actionTable[8][symbolManager.GetSymbolIdByValue(")")] = {Action::Shift, 11};
  actionTable[9][symbolManager.GetSymbolIdByValue("+")] = {Action::Reduce, 1};
  actionTable[9][symbolManager.GetSymbolIdByValue("*")] = {Action::Shift, 7};
  actionTable[9][symbolManager.GetSymbolIdByValue(")")] = {Action::Reduce, 1};
  actionTable[9][symbolManager.GetSymbolIdByValue("#")] = {Action::Reduce, 1};
  actionTable[10][symbolManager.GetSymbolIdByValue("+")] = {Action::Reduce, 3};
  actionTable[10][symbolManager.GetSymbolIdByValue("*")] = {Action::Reduce, 3};
  actionTable[10][symbolManager.GetSymbolIdByValue(")")] = {Action::Reduce, 3};
  actionTable[10][symbolManager.GetSymbolIdByValue("#")] = {Action::Reduce, 3};
  actionTable[11][symbolManager.GetSymbolIdByValue("+")] = {Action::Reduce, 5};
  actionTable[11][symbolManager.GetSymbolIdByValue("*")] = {Action::Reduce, 5};
  actionTable[11][symbolManager.GetSymbolIdByValue(")")] = {Action::Reduce, 5};
  actionTable[11][symbolManager.GetSymbolIdByValue("#")] = {Action::Reduce, 5};

  // build goto table
  gotoTable[0][symbolManager.GetSymbolIdByValue("E")] = 1;
  gotoTable[0][symbolManager.GetSymbolIdByValue("T")] = 2;
  gotoTable[0][symbolManager.GetSymbolIdByValue("F")] = 3;
  gotoTable[4][symbolManager.GetSymbolIdByValue("E")] = 8;
  gotoTable[4][symbolManager.GetSymbolIdByValue("T")] = 2;
  gotoTable[4][symbolManager.GetSymbolIdByValue("F")] = 3;
  gotoTable[6][symbolManager.GetSymbolIdByValue("T")] = 9;
  gotoTable[6][symbolManager.GetSymbolIdByValue("F")] = 3;
  gotoTable[7][symbolManager.GetSymbolIdByValue("F")] = 10;
}

bool isActionTableItemExists(uint64_t state, uint64_t symbolId) {
  return actionTable.find(state) != actionTable.end() &&
         actionTable[state].find(symbolId) != actionTable[state].end();
}

bool isGotoTableItemExists(uint64_t state, uint64_t symbolId) {
  return gotoTable.find(state) != gotoTable.end() &&
         gotoTable[state].find(symbolId) != gotoTable[state].end();
}

void analyzeInputString(
    std::string &input, SymbolManager &symbolManager,
    const std::vector<GenerativeExpression> &generativeExpressions) {
  MyStack<uint64_t> stateStack;
  MyStack<uint64_t> symbolStack;
  MyQueue<uint64_t> inputQueue;

  // Convert input string to symbol IDs and push onto symbol stack
  try {
    if (input.find_first_of('#') == std::string::npos) {
      input += '#'; // Append end symbol if not present
    } else if (input.find_first_of('#') != input.size() - 1) {
      input.erase(input.find_first_of('#') + 1); // Remove characters after #
    }

    for (char ch : input) {
      inputQueue.push(symbolManager.GetSymbolIdByValue({ch}));
    }
  } catch (const std::invalid_argument &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return;
  }

  // Initialize state stack with initial state
  stateStack.push(0);
  symbolStack.push(Terminator::EndSymbol.SymbolId);

  // Main analysis loop
  while (true) {
    // Get current state and next input symbol
    uint64_t currentState = stateStack.top();
    uint64_t nextInput = inputQueue.front();

    if (!isActionTableItemExists(currentState, nextInput)) {
      std::cerr << "Error: No action defined for state " << currentState
                << " and input symbol "
                << symbolManager.GetSymbol(nextInput)->ToString() << std::endl;
      return;
    }

    Action &action = actionTable[currentState][nextInput];

    printAnalysisStep(symbolManager, stateStack, symbolStack, inputQueue,
                      action, generativeExpressions);

    if (action.type == Action::Shift) {

      stateStack.push(action.value);
      symbolStack.push(nextInput);
      inputQueue.pop();
    } else if (action.type == Action::Reduce) {
      // production index is 1-based
      const GenerativeExpression &production =
          generativeExpressions[action.value - 1];
      for (size_t i = 0; i < production.GetTargets().size(); ++i) {
        stateStack.pop();
        symbolStack.pop();
      }

      stateStack.push(gotoTable[stateStack.top()][production.GetSource()]);
      symbolStack.push(production.GetSource());
    } else if (action.type == Action::Accept) {
      std::cout << "Input string is accepted by the grammar." << std::endl;
      break;
    }
  }
}

void printAnalysisStep(const SymbolManager &symbolManager,
                       const MyStack<uint64_t> &stateStack,
                       const MyStack<uint64_t> &symbolStack,
                       const MyQueue<uint64_t> &inputQueue,
                       const Action &action,
                       const std::vector<GenerativeExpression> &productions) {
  static bool isFirstStep = true;
  static uint64_t step = 1;
  static uint64_t stepHeaderWidth = 5;
  static uint64_t stateStackWidth = 20;
  static uint64_t symbolStackWidth = 20;
  static uint64_t inputQueueWidth = 20;
  static uint64_t actionWidth = 30;

  if (isFirstStep) {
    // clang-format off
    std::cout << std::left << std::setw(stepHeaderWidth)  << "Step"
              << std::left << std::setw(stateStackWidth) << "State Stack"
              << std::left << std::setw(symbolStackWidth) << "Symbol Stack"
              << std::left << std::setw(inputQueueWidth) << "Input Queue"
              << std::left << std::setw(actionWidth) << "Action"
              << std::endl;
    // clang-format on
    //   // clang-format off
    //   std::cout << std::setw(3)  << step++
    //             << std::setw(16) <<
    //             SymbolUtils::SymbolSequenceToString(stateStack.get_vector(),
    //                                                    symbolManager)
    //             << std::setw(20) <<
    //             SymbolUtils::SymbolSequenceToString(symbolStack.get_vector(),
    //                                                    symbolManager)
    //             << std::setw(16) <<
    //             SymbolUtils::SymbolSequenceToString(inputQueue.get_vector(),
    //                                                    symbolManager)
    //             << std::setw(20) << ""
    //             << std::endl;
    //   // clang-format on
    isFirstStep = false;
  }

  // Construct action string for display
  std::string actionStr;

  switch (action.type) {
  case Action::Shift:
    actionStr = "ACTION[0, " +
                symbolManager.GetSymbol(action.value)->ToString() + "] = S(" +
                std::to_string(action.value) + "), PUSH(" +
                std::to_string(action.value) + ")";
    break;
  case Action::Reduce:
    actionStr =
        "R(" + std::to_string(action.value) +
        "): " + productions[action.value - 1].ToString() + ", GOTO[" +
        std::to_string(stateStack.top()) + ", " +
        symbolManager.GetSymbol(productions[action.value - 1].GetSource())
            ->ToString() +
        "] = " +
        std::to_string(gotoTable[stateStack.top()][symbolStack.top()]) +
        ", PUSH";
    break;
  case Action::Accept:
    actionStr = "ACCEPT";
    break;
  case Action::Error:
    actionStr = "ERROR";
    break;
  }

  // clang-format off
  std::cout << std::left << std::setw(stepHeaderWidth)  << step++
            << std::left << std::setw(stateStackWidth) << convertNumberVectorToString(stateStack.get_vector())
            << std::left << std::setw(symbolStackWidth) << SymbolUtils::SymbolSequenceToString(symbolStack.get_vector(),
                                                   symbolManager)
            << std::left << std::setw(inputQueueWidth) << SymbolUtils::SymbolSequenceToString(inputQueue.get_vector(),
                                                   symbolManager)
            << std::left << std::setw(actionWidth) << actionStr
            << std::endl;
  // clang-format on
}

std::string convertNumberVectorToString(const std::vector<uint64_t> &numbers) {
  std::string result;
  for (uint64_t num : numbers) {
    if (num < 10) {
      result += std::to_string(num);
    } else {
      result += "{" + std::to_string(num) + "}";
    }
  }
  return result;
}