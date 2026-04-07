#include <sstream>

#include "utils.h"

namespace lex {

// clang-format off
std::unordered_map<TokenType, std::string>
    TokenTypeUtils::token_type_string_map = {
#ifdef LEXICAL_EXPERIMENT_ONLY
        {TokenType::Error, "Error"},
        {TokenType::Keyword, "Keyword"},
        {TokenType::Seperator, "Seperator"},
        {TokenType::ArithmeticOperator, "ArithmeticOperator"},
        {TokenType::LogicalOperator, "LogicalOperator"},
        {TokenType::UnsignedNumber, "UnsignedNumber"},
        {TokenType::Word, "Word"},
#else
        {TokenType::Keyword, "Keyword"},
        {TokenType::Seperator, "Seperator"},
        {TokenType::Number, "Number"},
        {TokenType::Operator, "Operator"},
        {TokenType::Word, "Word"},
        {TokenType::Error, "Error"},
        {TokenType::Invalid, "Invalid"},
#endif
};
// clang-format on

std::string Token::Position::ToString() const {
  std::ostringstream oss;
  oss << "(" << row << ", " << column << ")";
  return oss.str();
}

} // namespace lex