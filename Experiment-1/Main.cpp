#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "LayeredDFAGraph.h"
#include "StateMachine.h"
#include "Utils.h"

struct ProgramRuntimeInfo {
    std::vector<std::vector<uint32_t>> KeywordList;
    std::vector<std::vector<uint32_t>> SeparatorList;
    std::vector<std::vector<uint32_t>> ArithmeticOperatorList;
    std::vector<std::vector<uint32_t>> RelationalOperatorList;

    bool IsConfigReady = false;
};

// Global variable to hold the runtime information of the program such as
// keywords, separators, operators, etc. This will be populated from the
// configuration file and used throughout the program.
static ProgramRuntimeInfo RuntimeInfo;
static StateMachine LexicalAnalyzer;

void PrintStartupMessage(std::vector<std::string_view> &Args);
void PrintUsageMessage(std::vector<std::string_view> &Args);
void LoadConfiguration(std::vector<std::string_view> &Args);

void BuildStateMachines();

static const std::unordered_map<
    std::string_view, std::function<void(std::vector<std::string_view> &)>>
    CommandHandlers = {
        {"-h", PrintStartupMessage},
        {"--help", PrintStartupMessage},
        {"-c", LoadConfiguration},
        {"--config", LoadConfiguration},
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
        std::cerr << "Error: Failed to open configuration file: "
                  << ConfigFilePath << std::endl;
        return;
    }

    // Read the entire content of the configuration file into a string. If
    // reading fails, print an error message and return.
    std::string Content((std::istreambuf_iterator<char>(ConfigFile)),
                        std::istreambuf_iterator<char>());

    if (ConfigFile.bad()) {
        std::cerr << "Error: Failed to read configuration file: "
                  << ConfigFilePath << std::endl;
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

        RuntimeInfo.KeywordList =
            LoadWordList(ConfigJson.at("keywords"));
        RuntimeInfo.SeparatorList =
            LoadWordList(ConfigJson.at("separators"));
        RuntimeInfo.ArithmeticOperatorList =
            LoadWordList(ConfigJson.at("arithmetic_operators"));
        RuntimeInfo.RelationalOperatorList =
            LoadWordList(ConfigJson.at("relational_operators"));

        RuntimeInfo.IsConfigReady = true;
        std::cout << "Message: Configuration loaded successfully." << std::endl;
    } catch (const nlohmann::json::type_error &e) {
        std::cerr << "Error: Invalid configuration format: " << e.what()
                  << std::endl;
    }
}

void BuildStateMachines() {
  LayeredDFAGraph KeywordDFA;
  LayeredDFAGraph SeparatorDFA;
  LayeredDFAGraph ArithmeticOperatorDFA;
  LayeredDFAGraph RelationalOperatorDFA;

  KeywordDFA.AddWordList(RuntimeInfo.KeywordList);
  SeparatorDFA.AddWordList(RuntimeInfo.SeparatorList);
  ArithmeticOperatorDFA.AddWordList(RuntimeInfo.ArithmeticOperatorList);
  RelationalOperatorDFA.AddWordList(RuntimeInfo.RelationalOperatorList);

  
}