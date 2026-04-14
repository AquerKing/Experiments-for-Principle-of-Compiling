#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "LayeredDFAGraph.h"
#include "State.h"
#include "StateMachine.h"
#include "Token.h"
#include "Utils.h"

struct ProgramRuntimeInfo {
  std::vector<std::vector<uint32_t>> KeywordList;
  std::vector<std::vector<uint32_t>> SeparatorList;
  std::vector<std::vector<uint32_t>> ArithmeticOperatorList;
  std::vector<std::vector<uint32_t>> RelationalOperatorList;

  enum class ArgumentBitFlag : uint8_t {
    ConfigFilePath = 1 << 0,
    InputFilePath = 1 << 1,
    ConfigReady = 1 << 2,
    StateMachinesBuilt = 1 << 3,
  };

  uint8_t ArgumentFlag = 0;

  bool HasArgument(ArgumentBitFlag Flag) const {
    return (ArgumentFlag & static_cast<uint8_t>(Flag)) != 0;
  }
};

// Global variable to hold the runtime information of the program such as
// keywords, separators, operators, etc. This will be populated from the
// configuration file and used throughout the program.
static ProgramRuntimeInfo RuntimeInfo;
static StateMachine KeywordStateMachine;
static StateMachine SeparatorStateMachine;
static StateMachine ArithmeticOperatorStateMachine;
static StateMachine RelationalOperatorStateMachine;
static StateMachine TokenStateMachine;
static StateMachine NumberStateMachine;

// Charsets
static std::unordered_set<uint32_t> NumberCharset = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};
static std::unordered_set<uint32_t> IdentifierCharset = {
    '_', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A',
    'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
static std::unordered_set<uint32_t> WhitespaceCharset = {' ', '\t', '\n', '\r'};
static std::unordered_set<uint32_t> SeparatorCharset = {'(', ')', '{', '}',
                                                        '[', ']', ';', ','};
static std::unordered_set<uint32_t> ArithmeticOperatorCharset = {'+', '-', '*',
                                                                 '/'};
static std::unordered_set<uint32_t> RelationalOperatorCharset = {'<', '>', '='};

void PrintStartupMessage(std::vector<std::string_view> &Args);
void PrintUsageMessage(std::vector<std::string_view> &Args);
void LoadConfiguration(std::vector<std::string_view> &Args);
void AnalyzeInputFile(std::vector<std::string_view> &Args);

void BuildStateMachines();
std::vector<Token> AnalyzeSource(const std::vector<uint32_t> &SourceCode);

static const std::unordered_map<
    std::string_view, std::function<void(std::vector<std::string_view> &)>>
    CommandHandlers = {
        {"-h", PrintStartupMessage}, {"--help", PrintStartupMessage},
        {"-c", LoadConfiguration},   {"--config", LoadConfiguration},
        {"-f", AnalyzeInputFile},    {"--file", AnalyzeInputFile},
};

int main(int argc, char *argv[]) {
  // Convert the command-line arguments to a vector of string views for easier
  // handling and to avoid unnecessary copying of strings.
  std::vector<std::string_view> AllArgs;
  AllArgs.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    AllArgs.emplace_back(argv[i]);
  }

  // Check if the user has provided any command-line arguments. If not, print
  // the startup message and exit. If the user has provided arguments, check
  // if any of them match the known command handlers and execute the
  // corresponding handler function.
  if (AllArgs.size() == 1) {
    PrintStartupMessage(AllArgs);
  }

  // Iterate through the command-line arguments starting from the second
  // argument (the first argument is the program name) and check if any of
  // them match the known command handlers. If a match is found, execute the
  // corresponding handler function and pass the remaining arguments to it.
  for (size_t i = 1; i < AllArgs.size(); ++i) {
    const auto &Arg = AllArgs[i];
    auto HandlerIt = CommandHandlers.find(Arg);
    if (HandlerIt != CommandHandlers.end()) {
      // If a handler is found for the current argument, collect the
      // remaining arguments that are not options (i.e., do not start with
      // '-') and pass them to the handler function.
      std::vector<std::string_view> HandlerArgs;
      HandlerArgs.reserve(3);
      for (size_t j = i + 1; j < AllArgs.size(); ++j) {
        if (AllArgs[j][0] == '-') {
          break;
        }
        HandlerArgs.emplace_back(AllArgs[j]);
      }

      HandlerIt->second(HandlerArgs);
    }
  }

  return 0;
}

void PrintStartupMessage(std::vector<std::string_view> &Args) {
  std::cout << "Welcome to the Lexical Analyzer!" << std::endl;
  std::cout << std::endl;
}

void PrintUsageMessage(std::vector<std::string_view> &Args) {
  std::cout << "Options:" << std::endl;
  std::cout << "  -c, --config     Specify the configuration file for the "
               "lexical analyzer"
            << std::endl;
  std::cout << "  -h, --help       Show this help message" << std::endl;
  std::cout << "  -f, --file       Specify the input file to analyze"
            << std::endl;
}

void LoadConfiguration(std::vector<std::string_view> &Args) {
  // Check if the user has provided a configuration file path as an argument.
  // If not, print an error message and the usage message, and return.
  if (Args.empty()) {
    std::cerr << "Error: No configuration file specified." << std::endl;
    PrintUsageMessage(Args);
    return;
  }

  const auto &ConfigFilePath = Args[0];
  std::cout << "Message: Loading configuration from: " << ConfigFilePath
            << std::endl;

  std::ifstream ConfigFile(ConfigFilePath.data());

  if (!ConfigFile.is_open()) {
    std::cerr << "Error: Failed to open configuration file: " << ConfigFilePath
              << std::endl;
    return;
  }

  // Read the entire content of the configuration file into a string. If
  // reading fails, print an error message and return.
  std::string Content((std::istreambuf_iterator<char>(ConfigFile)),
                      std::istreambuf_iterator<char>());

  if (ConfigFile.bad()) {
    std::cerr << "Error: Failed to read configuration file: " << ConfigFilePath
              << std::endl;
    return;
  }

  // Parse the JSON content of the configuration file and populate the
  // RuntimeInfo structure. If parsing fails, print an error message and
  // return.
  nlohmann::json ConfigJson;
  try {
    ConfigJson = nlohmann::json::parse(Content);
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "Error: Failed to parse configuration file: " << e.what()
              << std::endl;
    return;
  }

  // Convert the relevant fields from the JSON configuration to the
  // appropriate format (e.g., vectors of uint32_t) and store them in the
  // RuntimeInfo structure. If any of the fields are missing or have an
  // invalid format, print an error message and return.
  try {
    auto LoadWordList = [](const nlohmann::json &JsonArray) {
      std::vector<std::vector<uint32_t>> WordList;
      WordList.reserve(JsonArray.size());

      for (const auto &Item : JsonArray) {
        WordList.emplace_back(
            ConvertStringToU32Vector(Item.get<std::string>()));
      }

      return WordList;
    };

    RuntimeInfo.KeywordList = LoadWordList(ConfigJson.at("keywords"));
    RuntimeInfo.SeparatorList = LoadWordList(ConfigJson.at("separators"));
    RuntimeInfo.ArithmeticOperatorList =
        LoadWordList(ConfigJson.at("arithmetic_operators"));
    RuntimeInfo.RelationalOperatorList =
        LoadWordList(ConfigJson.at("relational_operators"));

    RuntimeInfo.ArgumentFlag |= static_cast<uint8_t>(
        ProgramRuntimeInfo::ArgumentBitFlag::ConfigFilePath);
    std::cout << "Message: Configuration loaded successfully." << std::endl;
  } catch (const nlohmann::json::type_error &e) {
    std::cerr << "Error: Invalid configuration format: " << e.what()
              << std::endl;
  }

  RuntimeInfo.ArgumentFlag |=
      static_cast<uint8_t>(ProgramRuntimeInfo::ArgumentBitFlag::ConfigFilePath);

  BuildStateMachines();
}

void BuildStateMachines() {
  if (RuntimeInfo.HasArgument(
          ProgramRuntimeInfo::ArgumentBitFlag::StateMachinesBuilt)) {
    std::cerr << "Error: State machines have already been built. Please "
                 "restart the program to rebuild the state machines."
              << std::endl;
    return;
  }

  LayeredDFAGraph KeywordDFA;
  LayeredDFAGraph SeparatorDFA;
  LayeredDFAGraph ArithmeticOperatorDFA;
  LayeredDFAGraph RelationalOperatorDFA;

  KeywordDFA.AddWordList(RuntimeInfo.KeywordList);
  SeparatorDFA.AddWordList(RuntimeInfo.SeparatorList);
  ArithmeticOperatorDFA.AddWordList(RuntimeInfo.ArithmeticOperatorList);
  RelationalOperatorDFA.AddWordList(RuntimeInfo.RelationalOperatorList);

  KeywordDFA.BuildGraph();
  SeparatorDFA.BuildGraph();
  ArithmeticOperatorDFA.BuildGraph();
  RelationalOperatorDFA.BuildGraph();

  KeywordStateMachine.BuildFromLayeredDFAGraph(KeywordDFA);
  SeparatorStateMachine.BuildFromLayeredDFAGraph(SeparatorDFA);
  ArithmeticOperatorStateMachine.BuildFromLayeredDFAGraph(
      ArithmeticOperatorDFA);
  RelationalOperatorStateMachine.BuildFromLayeredDFAGraph(
      RelationalOperatorDFA);

  // Build token state machine manually
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();

    // Update intermediate state config
    std::unordered_map<uint32_t, StateID> TransitionMap;
    for (int i = 0; i < 26; ++i) {
      TransitionMap['a' + i] = 3;
      TransitionMap['A' + i] = 3;
    }
    TransitionMap['_'] = 3;

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::Start;
    Config.CacheFlag = TokenType::Invalid;

    Manager.UpdateStateConfig(0, Config);
  }
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();
    Manager.CreateStateByID(3);

    // Update intermediate state config
    std::unordered_map<uint32_t, StateID> TransitionMap;
    for (int i = 0; i < 26; ++i) {
      TransitionMap['a' + i] = 3;
      TransitionMap['A' + i] = 3;
    }
    for (int i = 0; i < 10; ++i) {
      TransitionMap['0' + i] = 3;
    }
    TransitionMap['_'] = 3;

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::End;
    Config.CacheFlag = TokenType::Token;

    Manager.UpdateStateConfig(3, Config);
  }

  // Build number state machine manually
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();

    std::unordered_map<uint32_t, StateID> TransitionMap;
    for (int i = 0; i < 10; ++i) {
      TransitionMap['0' + i] = 3;
    }
    TransitionMap['.'] = 4;

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::Start;
    Config.CacheFlag = TokenType::Invalid;

    Manager.UpdateStateConfig(0, Config);
  }
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();
    Manager.CreateStateByID(3);

    std::unordered_map<uint32_t, StateID> TransitionMap;
    for (int i = 0; i < 10; ++i) {
      TransitionMap['0' + i] = 3;
    }
    TransitionMap['.'] = 4;

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::End;
    Config.CacheFlag = TokenType::UnsignedNumber;

    Manager.UpdateStateConfig(3, Config);
  }
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();
    Manager.CreateStateByID(4);

    std::unordered_map<uint32_t, StateID> TransitionMap;
    for (int i = 0; i < 10; ++i) {
      TransitionMap['0' + i] = 4;
    }

    StateConfig Config;
    Config.Strategy = {
        StateConfig::StatePostTransitionStrategy::Append,
        [](TokenCache &Cache, uint32_t Input) { Cache.Append(Input); }};
    Config.TransitionMap = TransitionMap;
    Config.Type = StateType::End;
    Config.CacheFlag = TokenType::UnsignedNumber;

    Manager.UpdateStateConfig(4, Config);
  }
  {
    StateManager &Manager = TokenStateMachine.GetStateManager();

    StateConfig Config;
    Config.CacheFlag = TokenType::Error;

    Manager.UpdateStateConfig(1, Config);
  }

  RuntimeInfo.ArgumentFlag |= static_cast<uint8_t>(
      ProgramRuntimeInfo::ArgumentBitFlag::StateMachinesBuilt);
  std::cout << "Message: State machines built successfully." << std::endl;
}

void AnalyzeInputFile(std::vector<std::string_view> &Args) {
  if (!RuntimeInfo.HasArgument(
          ProgramRuntimeInfo::ArgumentBitFlag::StateMachinesBuilt)) {
    BuildStateMachines();
  }
}

std::vector<Token> AnalyzeSource(const std::vector<uint32_t> &SourceCode) {
  const int Length = SourceCode.size();
  std::vector<Token> RecognizedTokens;

  if (Length == 0) {
    std::cerr << "Error: Source code is empty." << std::endl;
    return {};
  }

  auto CheckChar = [](const std::vector<uint32_t> &Source, size_t Index,
                      uint32_t Char) {
    return Index >= 0 && Index < Source.size() && Source[Index] == Char;
  };

  auto GetSubStringByCharset = [](const std::vector<uint32_t> &Source,
                                  size_t StartIndex,
                                  const std::unordered_set<uint32_t> &CharSet) {
    std::vector<uint32_t> Result;
    size_t Index = StartIndex;

    while (Index >= 0 && Index < Source.size() &&
           CharSet.find(Source[Index]) != CharSet.end()) {
      Result.push_back(Source[Index]);
      ++Index;
    }

    return Result;
  };

  auto IsWhitespace = [](uint32_t Char) {
    return Char == ' ' || Char == '\t' || Char == '\n' || Char == '\r';
  };

  auto IsIdentifierStartChar = [](uint32_t Char) {
    return (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') ||
           Char == '_';
  };

  enum class ScannerState : uint8_t {
    InComment = 1 << 0,
    InMultiLineComment = 1 << 1,
  };

  uint8_t ScannerFlag = 0;
  TokenContextInfo ContextInfo = {1, 1};
  for (size_t i = 0; i < SourceCode.size(); ++i) {
    uint32_t Char = SourceCode[i];

    if (IsWhitespace(Char)) {
      if (Char == '\n' || Char == '\r') {
        if (Char == '\r' && CheckChar(SourceCode, i + 1, '\n')) {
          ++i; // Handle Windows-style line endings (\r\n)
        }
        ContextInfo.Row += 1;
        ContextInfo.Column = 1;
        ScannerFlag &= ~static_cast<uint8_t>(ScannerState::InComment);
      } else {
        ContextInfo.Column += 1;
      }
      continue;
    }

    if (i < SourceCode.size() - 1 && Char == '/' && SourceCode[i + 1] == '/') {
      // Handle single-line comment
      ScannerFlag |= static_cast<uint8_t>(ScannerState::InComment);
      i += 2; // Skip the '//' characters
      continue;
    } else if (i < SourceCode.size() - 1 && Char == '/' &&
               SourceCode[i + 1] == '*') {
      // Handle multi-line comment
      ScannerFlag |= static_cast<uint8_t>(ScannerState::InMultiLineComment);
      i += 2; // Skip the '/*' characters
      continue;
    } else if (i < SourceCode.size() - 1 && Char == '*' &&
               SourceCode[i + 1] == '/') {
      // Handle end of multi-line comment
      ScannerFlag &= ~static_cast<uint8_t>(ScannerState::InMultiLineComment);
      i += 2; // Skip the '*/' characters
      continue;
    }

    if (ScannerFlag & static_cast<uint8_t>(ScannerState::InComment) ||
        ScannerFlag & static_cast<uint8_t>(ScannerState::InMultiLineComment)) {
      continue; // Skip characters inside comments
    }

    // Check if the current character can be the start of a token
    if (CheckChar(SourceCode, i, '_') || (Char >= 'a' && Char <= 'z') ||
        (Char >= 'A' && Char <= 'Z')) {
      // If the current character can be the start of an identifier, try to
      // recognize an identifier token starting from the current character. If
      // recognition fails, print an error message and continue to the next
      // character.
      std::vector<uint32_t> Identifier =
          GetSubStringByCharset(SourceCode, i, IdentifierCharset);
      std::vector<Token> Tokens = KeywordStateMachine.ReceiveInputs(
          Identifier, StateMachine::TokenGenerationStrategy::GenerateAtLast);

      if (Tokens.empty()) {
        Tokens = TokenStateMachine.ReceiveInputs(
            Identifier, StateMachine::TokenGenerationStrategy::GenerateAtLast);
      }

      if (Tokens.size() != 1) {
        throw std::runtime_error("Failed to recognize identifier: " +
                                 ConvertU32VectorToString(Identifier));
      }

      RecognizedTokens.emplace_back(Tokens[0]);
    } else if (Char >= '0' && Char <= '9') {
      // If the current character can be the start of a number, try to recognize
      // a number token starting from the current character. If recognition
      // fails, print an error message and continue to the next character.
      std::vector<uint32_t> Number =
          GetSubStringByCharset(SourceCode, i, NumberCharset);
      std::vector<Token> Tokens = NumberStateMachine.ReceiveInputs(
          Number, StateMachine::TokenGenerationStrategy::GenerateAtLast);

      if (Tokens.empty()) {
        throw std::runtime_error("Failed to recognize number: " +
                                 ConvertU32VectorToString(Number));
      }

      if (Tokens.size() != 1) {
        throw std::runtime_error("Failed to recognize number: " +
                                 ConvertU32VectorToString(Number));
      }

      RecognizedTokens.emplace_back(Tokens[0]);
    } else if (SeparatorCharset.find(Char) != SeparatorCharset.end()) {
      std::vector<uint32_t> String =
          GetSubStringByCharset(SourceCode, i, SeparatorCharset);
      std::vector<Token> Tokens = SeparatorStateMachine.ReceiveInputs(
          String,
          StateMachine::TokenGenerationStrategy::GenerateSoonIfPossible);
      if (!Tokens.size()) {
        throw std::runtime_error("Failed to recognize separator: " +
                                 ConvertU32VectorToString(String));
      }

      RecognizedTokens.insert(RecognizedTokens.end(), Tokens.begin(),
                              Tokens.end());
      i += String.size() - 1;
    } else if (ArithmeticOperatorCharset.find(Char) !=
               ArithmeticOperatorCharset.end()) {
      std::vector<uint32_t> String =
          GetSubStringByCharset(SourceCode, i, ArithmeticOperatorCharset);
      std::vector<Token> Tokens = ArithmeticOperatorStateMachine.ReceiveInputs(
          String, StateMachine::TokenGenerationStrategy::GenerateAtLast);
      if (!Tokens.size()) {
        throw std::runtime_error("Failed to recognize arithmetic operator: " +
                                 ConvertU32VectorToString(String));
      }

      RecognizedTokens.insert(RecognizedTokens.end(), Tokens.begin(),
                              Tokens.end());
      i += String.size() - 1;
    } else if (RelationalOperatorCharset.find(Char) !=
               RelationalOperatorCharset.end()) {
      std::vector<uint32_t> String =
          GetSubStringByCharset(SourceCode, i, RelationalOperatorCharset);
      std::vector<Token> Tokens = RelationalOperatorStateMachine.ReceiveInputs(
          String, StateMachine::TokenGenerationStrategy::GenerateAtLast);
      if (!Tokens.size()) {
        throw std::runtime_error("Failed to recognize relational operator: " +
                                 ConvertU32VectorToString(String));
      }

      RecognizedTokens.insert(RecognizedTokens.end(), Tokens.begin(),
                              Tokens.end());
      i += String.size() - 1;
    } else {
      throw std::runtime_error("Unrecognized character: " +
                               ConvertU32VectorToString({Char}));
    }
  }

  return RecognizedTokens;
}