#include "grammar.h"
#include "symbols.h"
#include "utils.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <unordered_set>
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

TEST(PredictiveAnalysisTableTest, ConstructPredictiveAnalysisTable) {
  std::vector<std::string> ExpresssionStrings = {
      "E->TG",      "G->+TG|-TG", "G->@",   "T->FS",
      "S->*FS|/FS", "S->@",       "F->(E)", "F->i",
  };

  SymbolManager Manager;
  std::vector<GenerativeExpression> Expressions;
  for (const auto &ExpressionString : ExpresssionStrings) {
    std::vector<GenerativeExpression> ParsedExpressions =
        GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
    Expressions.insert(Expressions.end(), ParsedExpressions.begin(),
                       ParsedExpressions.end());
  }

  EXPECT_EQ(Manager.GetSymbolCount(),
            SymbolManager::ReservedSymbolIdCount + 12);
  EXPECT_EQ(Expressions[0].ToString(), "E->TG");
  EXPECT_EQ(Expressions[1].ToString(), "G->+TG");
  EXPECT_EQ(Expressions[2].ToString(), "G->-TG");
  EXPECT_EQ(Expressions[3].ToString(), "G->ε");
  EXPECT_EQ(Expressions[4].ToString(), "T->FS");
  EXPECT_EQ(Expressions[5].ToString(), "S->*FS");
  EXPECT_EQ(Expressions[6].ToString(), "S->/FS");
  EXPECT_EQ(Expressions[7].ToString(), "S->ε");
  EXPECT_EQ(Expressions[8].ToString(), "F->(E)");
  EXPECT_EQ(Expressions[9].ToString(), "F->i");

  GenerativeExpressionPreprocessor Preprocessor(&Manager);
  Preprocessor.CalculateFirstAndFollowSets(Expressions);

  // Check the first sets of symbols.
  {
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("("),
          Manager.GetSymbolIdByValue("i"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("F")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("*"),
          Manager.GetSymbolIdByValue("/"),
          Terminator::Epsilon.SymbolId,
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("S")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("("),
          Manager.GetSymbolIdByValue("i"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("T")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("+"),
          Manager.GetSymbolIdByValue("-"),
          Terminator::Epsilon.SymbolId,
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("G")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("("),
          Manager.GetSymbolIdByValue("i"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("E")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("+"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("+")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("-"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("-")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("*"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("*")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("/"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("/")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue("("),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue("(")),
          ExpectedFirstSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFirstSet = {
          Manager.GetSymbolIdByValue(")"),
      };
      EXPECT_EQ(
          Preprocessor.GetFirstSetOfSymbol(Manager.GetSymbolIdByValue(")")),
          ExpectedFirstSet);
    }
  }

  // Check the follow sets of symbols.
  {
    {
      std::unordered_set<uint64_t> ExpectedFollowSet = {
          Manager.GetSymbolIdByValue(")"),
          Terminator::EndSymbol.SymbolId,
      };
      EXPECT_EQ(
          Preprocessor.GetFollowSetOfSymbol(Manager.GetSymbolIdByValue("E")),
          ExpectedFollowSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFollowSet = {
          Manager.GetSymbolIdByValue(")"),
          Terminator::EndSymbol.SymbolId,
      };
      EXPECT_EQ(
          Preprocessor.GetFollowSetOfSymbol(Manager.GetSymbolIdByValue("G")),
          ExpectedFollowSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFollowSet = {
          Manager.GetSymbolIdByValue("+"),
          Manager.GetSymbolIdByValue("-"),
          Manager.GetSymbolIdByValue(")"),
          Terminator::EndSymbol.SymbolId,
      };
      EXPECT_EQ(
          Preprocessor.GetFollowSetOfSymbol(Manager.GetSymbolIdByValue("T")),
          ExpectedFollowSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFollowSet = {
          Manager.GetSymbolIdByValue("+"),
          Manager.GetSymbolIdByValue("-"),
          Manager.GetSymbolIdByValue(")"),
          Terminator::EndSymbol.SymbolId,
      };
      EXPECT_EQ(
          Preprocessor.GetFollowSetOfSymbol(Manager.GetSymbolIdByValue("S")),
          ExpectedFollowSet);
    }
    {
      std::unordered_set<uint64_t> ExpectedFollowSet = {
          Manager.GetSymbolIdByValue("*"), Manager.GetSymbolIdByValue("/"),
          Manager.GetSymbolIdByValue("+"), Manager.GetSymbolIdByValue("-"),
          Manager.GetSymbolIdByValue(")"), Terminator::EndSymbol.SymbolId,
      };
      EXPECT_EQ(
          Preprocessor.GetFollowSetOfSymbol(Manager.GetSymbolIdByValue("F")),
          ExpectedFollowSet);
    }
  }
}

TEST(PredictiveAnalysisTableTest, BuildPredictiveAnalysisTable) {
  std::vector<std::string> ExpresssionStrings = {
      "E->TG",      "G->+TG|-TG", "G->@",   "T->FS",
      "S->*FS|/FS", "S->@",       "F->(E)", "F->i",
  };

  SymbolManager Manager;
  std::vector<GenerativeExpression> Expressions;
  for (const auto &ExpressionString : ExpresssionStrings) {
    std::vector<GenerativeExpression> ParsedExpressions =
        GrammarUtils::ParseGenerativeExpressions(ExpressionString, Manager);
    Expressions.insert(Expressions.end(), ParsedExpressions.begin(),
                       ParsedExpressions.end());
  }

  GenerativeExpressionPreprocessor Preprocessor(&Manager);
  Preprocessor.CalculateFirstAndFollowSets(Expressions);

  PredictiveAnalysisTable Table;
  Table.BuildFromPreprocessor(Expressions, Preprocessor);

  // Test the items in the predictive analysis table.
  {
    EXPECT_EQ(Table
                  .GetItem(Manager.GetSymbolIdByValue("S"),
                           Manager.GetSymbolIdByValue("*"))
                  ->ToString(),
              "S->*FS");
  }
  {
    EXPECT_EQ(Table
                  .GetItem(Manager.GetSymbolIdByValue("F"),
                           Manager.GetSymbolIdByValue("("))
                  ->ToString(),
              "F->(E)");
  }
  {
    EXPECT_EQ(Table
                  .GetItem(Manager.GetSymbolIdByValue("T"),
                           Manager.GetSymbolIdByValue("("))
                  ->ToString(),
              "T->FS");
  }
  {
    EXPECT_EQ(Table.GetItem(Manager.GetSymbolIdByValue("E"),
                            Manager.GetSymbolIdByValue(")")),
              nullptr);
  }
}