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

  uint64_t SymbolId = 1;

protected:
  std::string Value;
};

class Terminator final : public Symbol {
public:
  static Terminator Epsilon;

public:
  Terminator(const uint64_t Id) : Symbol(Id) {}
  Terminator(const uint64_t Id, std::string_view Value) : Symbol(Id, Value) {}

  SymbolType GetType() const override;
};

class NonTerminator final : public Symbol {
public:
  NonTerminator(const uint64_t Id) : Symbol(Id) {}
  NonTerminator(const uint64_t Id, std::string_view Value)
      : Symbol(Id, Value) {}

  SymbolType GetType() const override;
};

class SymbolManager final {
  friend class GenerativeExpression;

public:
  SymbolManager();
  ~SymbolManager() = default;

  uint64_t CreateSymbol(const std::string &Value, const SymbolType Type);
  uint64_t
  FetchSymbolIdByName(const std::string &Value,
                      SymbolType TypeIfNotExisted = SymbolType::Invalid);
  Symbol *GetSymbol(const uint64_t Id) const;
  bool IsSymbolExisted(const std::string &Value) const;
  uint64_t GetSymbolCount() const;

private:
  uint64_t GetNextSymbolId();

  uint64_t NextSymbolId = 1;
  std::unordered_map<std::string, uint64_t> SymbolIdsByValue;
  std::unordered_set<uint64_t> UsedSymbolIds;
  std::unordered_map<uint64_t, std::shared_ptr<Symbol>> Symbols;
};
