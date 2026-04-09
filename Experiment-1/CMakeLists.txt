cmake_minimum_required(VERSION 3.20)

include_directories("Includes")

add_executable("Lexical-Analyzer"
    "Main.cpp"

    "Source/State.cpp"
    "Source/Token.cpp"

    "Source/State.cpp"
    "Source/Token.cpp"
)

add_executable("Lexical-Analyzer-Test"
    "Tests/TestStateMachine.cpp"

    "Source/State.cpp"
    "Source/Token.cpp"
)

target_include_directories("Lexical-Analyzer-Test"
    PRIVATE
    "Includes"
)

add_test(NAME "Lexical-Analyzer-Test" COMMAND "Lexical-Analyzer-Test")
