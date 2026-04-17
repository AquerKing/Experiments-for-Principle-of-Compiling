#include "grammar.h"
#include "symbols.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <vector>

TEST(SymbolSequenceTest, ParseGenerativeExpression) {
  {
    std::string ExpressionString = "E->ABc";
    SymbolManager Manager;
    std::vector<GenerativeExpression> Expressions =
        GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
    EXPECT_EQ(Manager.GetSymbolCount(),
              SymbolManager::ReservedSymbolIdCount + 4);
    EXPECT_EQ(Expressions.size(), 1);
    EXPECT_EQ(Expressions[0].ToString(), "E->ABc");
  }
  {
    std::string ExpressionString = "E->BC|Ac|a";
    SymbolManager Manager;
    std::vector<GenerativeExpression> Expressions =
        GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
    EXPECT_EQ(Manager.GetSymbolCount(),
              SymbolManager::ReservedSymbolIdCount + 5);
    EXPECT_EQ(Expressions.size(), 3);
    EXPECT_EQ(Expressions[0].ToString(), "E->BC");
    EXPECT_EQ(Expressions[1].ToString(), "E->Ac");
    EXPECT_EQ(Expressions[2].ToString(), "E->a");
  }
  {
    std::string ExpressionString = "T->F|T*F\nF";
    SymbolManager Manager;
    std::vector<GenerativeExpression> Expressions =
        GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
    EXPECT_EQ(Manager.GetSymbolCount(),
              SymbolManager::ReservedSymbolIdCount + 4);
    EXPECT_EQ(Expressions.size(), 2);
    EXPECT_EQ(Expressions[0].ToString(), "T->F");
    EXPECT_EQ(Expressions[1].ToString(), "T->T*F\nF");
  }
}