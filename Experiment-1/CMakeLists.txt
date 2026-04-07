cmake_minimum_required(VERSION 3.20)

include_directories("include")

add_executable("lexical-analyzer"
    "src/main.cpp"

    # "include/state_machine.cpp"
    "include/layered_state_graph.cpp"
    # "include/utils.cpp"
)

add_executable("lexical-analyzer-tests"
    "tests/StateMachineTests.cpp"

    "include/layered_state_graph.cpp"
)

target_include_directories("lexical-analyzer-tests" PRIVATE "include")
add_test(NAME lexical-analyzer-tests COMMAND lexical-analyzer-tests)
