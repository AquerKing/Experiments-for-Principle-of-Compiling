#include "grammar.h"
#include "symbols.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(GenerativeExpressionParsingTest, ParseSingleGenerativeExpression) {
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
              SymbolManager::ReservedSymbolIdCount + 6);
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
  {
    std::string ExpressionString = "T->F|@";
    SymbolManager Manager;
    std::vector<GenerativeExpression> Expressions =
        GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
    EXPECT_EQ(Manager.GetSymbolCount(),
              SymbolManager::ReservedSymbolIdCount + 2);
    EXPECT_EQ(Expressions.size(), 2);
    EXPECT_EQ(Expressions[0].ToString(), "T->F");
    EXPECT_EQ(Expressions[1].ToString(), "T->ε");
  }
}

TEST(GenerativeExpressionParsingTest, ParseMultipleGenerativeExpressions) {
  {
    std::vector<std::string> ExpressionStrings = {
        "E->AB|a",
        "A->a|@",
        "B->bC",
        "C->abc",
    };

    SymbolManager Manager;
    std::vector<GenerativeExpression> Expressions;
    for (const auto &ExpressionString : ExpressionStrings) {
      std::vector<GenerativeExpression> ParsedExpressions =
          GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
      Expressions.insert(Expressions.end(), ParsedExpressions.begin(),
                         ParsedExpressions.end());
    }

    EXPECT_EQ(Expressions.size(), 6);
    EXPECT_EQ(Expressions[0].ToString(), "E->AB");
    EXPECT_EQ(Expressions[1].ToString(), "E->a");
    EXPECT_EQ(Expressions[2].ToString(), "A->a");
    EXPECT_EQ(Expressions[3].ToString(), "A->ε");
    EXPECT_EQ(Expressions[4].ToString(), "B->bC");
    EXPECT_EQ(Expressions[5].ToString(), "C->abc");
  }
  {
    std::vector<std::string> ExpresssionStrings = {
        "E->ABCd|e", "A->aB|@", "B->bC", "C->AdE|@|D", "D->d",
    };

    SymbolManager Manager;
    std::vector<GenerativeExpression> Expressions;
    for (const auto &ExpressionString : ExpresssionStrings) {
      std::vector<GenerativeExpression> ParsedExpressions =
          GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
      Expressions.insert(Expressions.end(), ParsedExpressions.begin(),
                         ParsedExpressions.end());
    }

    EXPECT_EQ(Expressions.size(), 9);
    EXPECT_EQ(Expressions[0].ToString(), "E->ABCd");
    EXPECT_EQ(Expressions[1].ToString(), "E->e");
    EXPECT_EQ(Expressions[2].ToString(), "A->aB");
    EXPECT_EQ(Expressions[3].ToString(), "A->ε");
    EXPECT_EQ(Expressions[4].ToString(), "B->bC");
    EXPECT_EQ(Expressions[5].ToString(), "C->AdE");
    EXPECT_EQ(Expressions[6].ToString(), "C->ε");
    EXPECT_EQ(Expressions[7].ToString(), "C->D");
    EXPECT_EQ(Expressions[8].ToString(), "D->d");
  }
}