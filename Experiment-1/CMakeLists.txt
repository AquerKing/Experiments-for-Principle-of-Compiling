cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("Includes")

add_executable("Lexical-Analyzer"
    "Main.cpp"

    "Source/State.cpp"
    "Source/Token.cpp"
)

# Tests
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_executable("Test-Lexical-Analyzer-StateMachine"
    "Tests/TestStateMachine.cpp"

    "Source/State.cpp"
    "Source/Token.cpp"
)

target_link_libraries("Test-Lexical-Analyzer-StateMachine" PRIVATE GTest::gtest_main)

gtest_discover_tests("Test-Lexical-Analyzer-StateMachine")

# add_test(
#     NAME "Test-Lexical-Analyzer-StateMachine"
#     COMMAND "Test-Lexical-Analyzer-StateMachine"
# )
