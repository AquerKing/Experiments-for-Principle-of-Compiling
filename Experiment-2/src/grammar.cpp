#include "grammar.h"

std::string GenerativeExpression::ToString() const {
  if (Manager != nullptr && IsValid()) {
    std::string Result = Manager->Symbols.at(Source)->ToString() + "->";
    for (const auto &TargetId : Targets) {
      Result += Manager->Symbols.at(TargetId)->ToString();
    }
    return Result;
  }
  return "< Invalid GenerativeExpression >";
}