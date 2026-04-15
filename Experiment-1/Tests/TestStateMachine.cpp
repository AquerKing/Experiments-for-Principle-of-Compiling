#include <cstdint>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// 假设这些是你的头文件
#include "LayeredDFAGraph.h"
#include "State.h"
#include "StateMachine.h"
#include "Token.h"
#include "Utils.h"

// --- 辅助宏或函数 (可选，为了简化代码) ---
// 这里直接使用 ConvertStringToU8Vector，假设它已在全局或 Utils 中定义

// ============================================================
// 1. 关键字状态机测试套件
// ============================================================
class KeywordStateMachineTest : public ::testing::Test {
protected:
  StateMachine SM;
  LayeredDFAGraph Graph;

  // 在每个测试开始前运行 (SetUp)
  void SetUp() override {
    StateFlagStrategy FlagStrategy = {
        TokenType::Token,
        TokenType::Keyword,
        TokenType::Invalid,
    };
    Graph.SetStateFlagStrategy(FlagStrategy);

    std::vector<std::vector<uint8_t>> WordList = {
        ConvertStringToU8Vector("do"),     ConvertStringToU8Vector("end"),
        ConvertStringToU8Vector("for"),    ConvertStringToU8Vector("if"),
        ConvertStringToU8Vector("printf"), ConvertStringToU8Vector("scanf"),
        ConvertStringToU8Vector("then"),   ConvertStringToU8Vector("while"),
    };

    Graph.AddWordList(WordList);
    Graph.BuildGraph();
    SM.BuildFromLayeredDFAGraph(Graph);
  }

  // 在每个测试结束后运行 (TearDown)
  void TearDown() override { SM.Reset(); }
};

// 子测试：测试单个关键字 "do"
TEST_F(KeywordStateMachineTest, HandlesDoKeyword) {
  std::vector<uint8_t> Inputs = ConvertStringToU8Vector("do");
  std::vector<Token> Tokens = SM.ReceiveInputs(
      Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);

  ASSERT_EQ(Tokens.size(), 1) << "Token size should be 1";
  EXPECT_EQ(Tokens.front().Content, ConvertStringToU8Vector("do"));
  EXPECT_TRUE(Tokens.front().IsValid());
  EXPECT_EQ(Tokens.front().Type, TokenType::Keyword);
}

// 子测试：测试其他关键字
TEST_F(KeywordStateMachineTest, HandlesOtherKeywords) {
  // 使用参数化测试或者简单的循环测试多个类似情况
  std::vector<std::string> keywords = {"end",   "for",  "if",   "printf",
                                       "scanf", "then", "while"};

  for (const auto &kw : keywords) {
    SM.Reset(); // 在循环内重置
    std::vector<uint8_t> Inputs = ConvertStringToU8Vector(kw);
    std::vector<Token> Tokens = SM.ReceiveInputs(
        Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);

    // 使用 SCOPED_TRACE 可以在循环出错时打印出是哪个字符串导致的错误
    SCOPED_TRACE("Testing keyword: " + kw);

    ASSERT_EQ(Tokens.size(), 1);
    EXPECT_EQ(Tokens.front().Content, ConvertStringToU8Vector(kw));
    EXPECT_TRUE(Tokens.front().IsValid());
    EXPECT_EQ(Tokens.front().Type, TokenType::Keyword);
  }
}

// 子测试：测试非法组合 "whileif"
TEST_F(KeywordStateMachineTest, HandlesInvalidCombination) {
  std::vector<uint8_t> Inputs = ConvertStringToU8Vector("whileif");
  std::vector<Token> Tokens = SM.ReceiveInputs(
      Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);

  EXPECT_EQ(Tokens.size(), 0) << "Invalid combination should produce 0 tokens";
}

// ============================================================
// 2. 分隔符状态机测试套件
// ============================================================
class SeparatorStateMachineTest : public ::testing::Test {
protected:
  StateMachine SM;
  LayeredDFAGraph Graph;

  void SetUp() override {
    StateFlagStrategy FlagStrategy = {
        TokenType::Error,
        TokenType::Separator,
    };
    Graph.SetStateFlagStrategy(FlagStrategy);

    std::vector<std::vector<uint8_t>> WordList = {
      {static_cast<uint8_t>('(')}, {static_cast<uint8_t>(')')},
      {static_cast<uint8_t>(',')}, {static_cast<uint8_t>(';')},
      {static_cast<uint8_t>('[')}, {static_cast<uint8_t>(']')},
    };

    Graph.AddWordList(WordList);
    Graph.BuildGraph();
    SM.BuildFromLayeredDFAGraph(Graph);
  }
};

TEST_F(SeparatorStateMachineTest, ParsesComplexSeparators) {
  SM.Reset();
  std::vector<uint8_t> Inputs = ConvertStringToU8Vector("(([],);");
  std::vector<Token> Tokens = SM.ReceiveInputs(Inputs);

  ASSERT_EQ(Tokens.size(), 7);

  EXPECT_EQ(Tokens.at(0).Content, ConvertStringToU8Vector("("));
  EXPECT_EQ(Tokens.at(1).Content, ConvertStringToU8Vector("("));
  EXPECT_EQ(Tokens.at(2).Content, ConvertStringToU8Vector("["));
  EXPECT_EQ(Tokens.at(3).Content, ConvertStringToU8Vector("]"));
  EXPECT_EQ(Tokens.at(4).Content, ConvertStringToU8Vector(","));
  EXPECT_EQ(Tokens.at(5).Content, ConvertStringToU8Vector(")"));
  EXPECT_EQ(Tokens.at(6).Content, ConvertStringToU8Vector(";"));
}

// ============================================================
// 3. 算术运算符状态机测试套件
// ============================================================
class ArithmeticOperatorStateMachineTest : public ::testing::Test {
protected:
  StateMachine SM;
  LayeredDFAGraph Graph;

  void SetupGraph(std::vector<std::vector<uint8_t>> WordList) {
    StateFlagStrategy FlagStrategy = {
        TokenType::Error,
        TokenType::ArithmeticOperator,
    };
    Graph.SetStateFlagStrategy(FlagStrategy);
    Graph.AddWordList(WordList);
    Graph.BuildGraph();
    SM.BuildFromLayeredDFAGraph(Graph);
  }
};

TEST_F(ArithmeticOperatorStateMachineTest, ArithmeticOperators) {
    std::vector<std::vector<uint8_t>> WordList = {
      {static_cast<uint8_t>('+')},
      {static_cast<uint8_t>('-')},
      {static_cast<uint8_t>('*')},
      {static_cast<uint8_t>('/')},
  };
  SetupGraph(WordList);

  // Test '+'
  SM.Reset();
  auto Inputs = ConvertStringToU8Vector("+");
  auto Tokens = SM.ReceiveInputs(
      Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
  ASSERT_FALSE(Tokens.empty());
  EXPECT_EQ(Tokens.front().Type, TokenType::ArithmeticOperator);

  // Test '++' (Error case)
  SM.Reset();
  Inputs = ConvertStringToU8Vector("++");
  Tokens = SM.ReceiveInputs(
      Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
  ASSERT_FALSE(Tokens.empty());
  EXPECT_EQ(Tokens.front().Type, TokenType::Error);
}

// ============================================================
// 4. 关系运算符状态机测试套件
// ============================================================
class RelationalOperatorStateMachineTest : public ::testing::Test {
protected:
  StateMachine SM;
  LayeredDFAGraph Graph;

  void SetupGraph(std::vector<std::vector<uint8_t>> WordList) {
    StateFlagStrategy FlagStrategy = {
        TokenType::Error,
        TokenType::RelationalOperator,
    };
    Graph.SetStateFlagStrategy(FlagStrategy);
    Graph.AddWordList(WordList);
    Graph.BuildGraph();
    SM.BuildFromLayeredDFAGraph(Graph);
  }
};

TEST_F(RelationalOperatorStateMachineTest, RelationalOperators) {
  std::vector<std::vector<uint8_t>> WordList = {
      ConvertStringToU8Vector("<"),  ConvertStringToU8Vector("<="),
      ConvertStringToU8Vector("="),  ConvertStringToU8Vector(">"),
      ConvertStringToU8Vector(">="), ConvertStringToU8Vector("<>"),
  };
  SetupGraph(WordList);

  // Test '<'
  SM.Reset();
  auto Inputs = ConvertStringToU8Vector("<");
  auto Tokens = SM.ReceiveInputs(
      Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
  ASSERT_FALSE(Tokens.empty());
  EXPECT_EQ(Tokens.front().Type, TokenType::RelationalOperator);

  // Test '<='
  SM.Reset();
  Inputs = ConvertStringToU8Vector("<=");
  Tokens = SM.ReceiveInputs(
      Inputs, StateMachine::TokenGenerationStrategy::GenerateAtLast);
  ASSERT_FALSE(Tokens.empty());
  EXPECT_EQ(Tokens.front().Type, TokenType::RelationalOperator);
}

// ============================================================
// 5. 主函数
// ============================================================
int main(int argc, char **argv) {
  std::cout << ">>> TestStateMachine Start <<<" << std::endl;

  // 初始化 Google Test
  ::testing::InitGoogleTest(&argc, argv);

  // 运行所有测试
  int result = RUN_ALL_TESTS();

  std::cout << ">>> TestStateMachine End <<<" << std::endl;
  return result;
}