#pragma once

#include <cstdint>
#include <vector>

#include "Token.h"

void BuildStateMachines();
std::vector<Token> AnalyzeSource(const std::vector<uint8_t> &SourceCode);
