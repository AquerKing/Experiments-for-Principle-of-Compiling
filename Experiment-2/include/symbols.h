#pragma once

#include "interface.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>

enum class SymbolType {
  Invalid,
  Terminator,
  NonTerminator,
  Epsilon,
};

class Symbol : public IStringConvertable {
public:
  Symbol(const uint64_t Id) : SymbolId(Id) {}
  Symbol(const uint64_t Id, std::string_view Value)
      : SymbolId(Id), Value(Value) {}

  virtual SymbolType GetType() const = 0;

  std::string ToString() const override;

  uint64_t SymbolId = 0;

protected:
  SymbolType Type;
  std::string Value;
};

class Terminator final : public Symbol {
public:
  /** The epsilon symbol. */
  static Terminator Epsilon;
  static Terminator EndSymbol;

public:
  Terminator(const uint64_t Id) : Symbol(Id) {}
  Terminator(const uint64_t Id, std::string_view Value) : Symbol(Id, Value) {}

  /**
   * Gets the type of the symbol.
   * @return The type of the symbol.
   */
  SymbolType GetType() const override;
};

class NonTerminator final : public Symbol {

public:
  NonTerminator(const uint64_t Id) : Symbol(Id) {}
  NonTerminator(const uint64_t Id, std::string_view Value)
      : Symbol(Id, Value) {}

  /**
   * Gets the type of the symbol.
   * @return The type of the symbol.
   */
  SymbolType GetType() const override;
};

class SymbolManager final {
  friend class GenerativeExpression;

public:
  static const uint64_t ReservedSymbolIdCount = 2;

public:
  SymbolManager();
  ~SymbolManager() = default;

  /**
   * Creates a new symbol.
   * @param Value The value of the symbol.
   * @param Type The type of the symbol.
   * @return The ID of the created symbol.
   */
  uint64_t CreateSymbol(const std::string &Value, const SymbolType Type);

  /**
   * Fetches the ID of a symbol by its value.
   * @param Value The value of the symbol.
   * @param TypeIfNotExisted The type of the symbol created when it does not
   * exist.
   * @return The ID of the symbol.
   */
  uint64_t
  FetchSymbolIdByValue(const std::string &Value,
                       SymbolType TypeIfNotExisted = SymbolType::Invalid);

  /**
   * Gets a symbol by its ID.
   * @param Id The ID of the symbol.
   * @return A pointer to the symbol.
   */
  Symbol *GetSymbol(const uint64_t Id) const;

  /**
   * Checks if a symbol exists by its value.
   * @param Value The value of the symbol.
   * @return True if the symbol exists, false otherwise.
   */
  bool IsSymbolExisted(const std::string &Value) const;

  /**
   * Gets the count of symbols.
   * @return The number of symbols.
   */
  uint64_t GetSymbolCount() const;

  /**
   * Gets the ID of the start symbol.
   * @return The ID of the start symbol.
   */
  uint64_t GetStartSymbolId() const;

  void SetStartSymbolId(uint64_t Id);

private:
  /**
   * Gets the next available symbol ID.
   * @return The next available symbol ID.
   */
  uint64_t GetNextSymbolId();

  uint64_t NextSymbolId = ReservedSymbolIdCount;
  std::unordered_map<std::string, uint64_t> SymbolIdsByValue;
  std::unordered_set<uint64_t> UsedSymbolIds;
  std::unordered_map<uint64_t, std::shared_ptr<Symbol>> Symbols;
  uint64_t StartSymbolId = 0;
};
