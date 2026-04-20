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
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

struct Action {
  enum Type { Shift, Reduce, Accept, Error } type;
  uint64_t value; // For Shift, it's the next state; for Reduce, it's the
                  // production rule index
};

std::unordered_map<uint64_t, std::unordered_map<uint64_t, Action>> actionTable;
std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> gotoTable;

void buildAnalysisTable(SymbolManager& symbolManager);

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
    generativeExpressions.insert(generativeExpressions.end(), parsed.begin(),
                                 parsed.end());
  }

  return 0;
}

void buildAnalysisTable(SymbolManager& symbolManager) {
  actionTable.clear();
  gotoTable.clear();

  // build action table
  actionTable[0][symbolManager.Get("i")] = {Action::Shift, 5};
  actionTable[0][symbolManager.("(")] = {Action::Shift, 4};
}