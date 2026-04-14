#include "CommandLine.h"

#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "AnalyzerCore.h"
#include "ProgramRuntimeInfo.h"
#include "Token.h"
#include "Utils.h"

extern ProgramRuntimeInfo RuntimeInfo;

namespace {

void PrintStartupMessage(std::vector<std::string_view> &) {
  std::cout << "Welcome to the Lexical Analyzer!" << std::endl;
  std::cout << std::endl;
}

void PrintUsageMessage(std::vector<std::string_view> &) {
  std::cout << "Options:" << std::endl;
  std::cout << "  -c, --config     Specify the configuration file for the "
               "lexical analyzer"
            << std::endl;
  std::cout << "  -h, --help       Show this help message" << std::endl;
  std::cout << "  -f, --file       Specify the input file to analyze"
            << std::endl;
}

void LoadConfiguration(std::vector<std::string_view> &Args) {
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

  std::string Content((std::istreambuf_iterator<char>(ConfigFile)),
                      std::istreambuf_iterator<char>());

  if (ConfigFile.bad()) {
    std::cerr << "Error: Failed to read configuration file: " << ConfigFilePath
              << std::endl;
    return;
  }

  nlohmann::json ConfigJson;
  try {
    ConfigJson = nlohmann::json::parse(Content);
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "Error: Failed to parse configuration file: " << e.what()
              << std::endl;
    return;
  }

  try {
    auto LoadWordList = [](const nlohmann::json &JsonArray) {
      std::vector<std::vector<uint8_t>> WordList;
      WordList.reserve(JsonArray.size());

      for (const auto &Item : JsonArray) {
        WordList.emplace_back(
            ConvertStringToU8Vector(Item.get<std::string>()));
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

void AnalyzeInputFile(std::vector<std::string_view> &Args) {
  if (!RuntimeInfo.HasArgument(
          ProgramRuntimeInfo::ArgumentBitFlag::StateMachinesBuilt)) {
    BuildStateMachines();
  }

  const auto &SourceFilePath = Args[0];
  std::cout << "Message: Loading configuration from: " << SourceFilePath
            << std::endl;

  std::ifstream ConfigFile(SourceFilePath.data());

  if (!ConfigFile.is_open()) {
    std::cerr << "Error: Failed to open configuration file: " << SourceFilePath
              << std::endl;
    return;
  }

  std::string Content((std::istreambuf_iterator<char>(ConfigFile)),
                      std::istreambuf_iterator<char>());

  if (ConfigFile.bad()) {
    std::cerr << "Error: Failed to read configuration file: " << SourceFilePath
              << std::endl;
    return;
  }

  std::vector<uint8_t> SourceCode = ConvertStringToU8Vector(Content);
  std::vector<Token> RecognizedTokens = AnalyzeSource(SourceCode);

  for (const auto &Token : RecognizedTokens) {
    PrintToken(Token);
  }
}

const std::unordered_map<std::string_view,
                         std::function<void(std::vector<std::string_view> &)>>
    CommandHandlers = {
        {"-h", PrintStartupMessage}, {"--help", PrintStartupMessage},
        {"-c", LoadConfiguration},   {"--config", LoadConfiguration},
        {"-f", AnalyzeInputFile},    {"--file", AnalyzeInputFile},
};

} // namespace

void HandleCommandLine(int argc, char *argv[]) {
  std::vector<std::string_view> AllArgs;
  AllArgs.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    AllArgs.emplace_back(argv[i]);
  }

  if (AllArgs.size() == 1) {
    std::vector<std::string_view> EmptyArgs;
    PrintStartupMessage(EmptyArgs);
    return;
  }

  for (size_t i = 1; i < AllArgs.size(); ++i) {
    const auto &Arg = AllArgs[i];
    auto HandlerIt = CommandHandlers.find(Arg);
    if (HandlerIt != CommandHandlers.end()) {
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
}
