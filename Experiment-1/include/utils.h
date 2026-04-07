#pragma once

#include "interfaces.h"
#include <memory>
#include <unordered_map>

namespace Lexical {

enum class TokenType : uint {
#ifdef LEXICAL_EXPERIMENT_ONLY
  Error = 0 Keyword = 1,
  Seperator = 2,
  ArithmeticOperator = 3,
  LogicalOperator = 4,
  UnsignedNumber = 5,
  Word = 6,
#else
  Keyword,
  Seperator,
  Number,
  Operator,
  Word,
  Error,
  Invalid,
#endif
};

class TokenTypeUtils final {
public:
  TokenTypeUtils &GetInstance() {
    if (obj == nullptr) {
      obj.reset(new TokenTypeUtils);
    }
    return *obj;
  }

  /** @brief Converts token type to string.
   *  @param type The token type to convert.
   *  @return The string representation of the token type.
   */
  std::string ConvertToString(TokenType type) const noexcept;

private:
  TokenTypeUtils() = default;

  std::unique_ptr<TokenTypeUtils> obj;

  static std::unordered_map<TokenType, std::string> token_type_string_map;
};

struct Token : public IStringConvertable {
  struct Position : public IStringConvertable {
    typedef unsigned long long u64;
    u64 row, column;

    /** @brief Converts position into string formatted in (row, column).
     *  @return The string representation of the position.
     */
    std::string ToString() const override;
  };

  TokenType type = TokenType::Invalid;
  std::string token;

  /** @brief Converts token into string formatted in (type, token).
   *  @return The string representation of the token.
   */
  // TODO: Whether to implement a method of formatting into a string.
};

} // namespace Lexical