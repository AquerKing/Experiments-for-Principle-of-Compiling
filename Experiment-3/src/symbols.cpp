#include "symbols.h"

#include <cstdint>
#include <stdexcept>
#include <string>

Terminator Terminator::Epsilon(0, "ε");
Terminator Terminator::EndSymbol(1, "#");

std::string Symbol::ToString() const { return Value; }

SymbolType Terminator::GetType() const { return SymbolType::Terminator; }

SymbolType NonTerminator::GetType() const { return SymbolType::NonTerminator; }

SymbolManager::SymbolManager() {
  for (uint64_t i = 0; i < ReservedSymbolIdCount; ++i) {
    UsedSymbolIds.insert(i);
  }
  NextSymbolId = ReservedSymbolIdCount;
  Symbols[Terminator::Epsilon.SymbolId] =
      std::make_shared<Terminator>(Terminator::Epsilon);
  Symbols[Terminator::EndSymbol.SymbolId] =
      std::make_shared<Terminator>(Terminator::EndSymbol);
  SymbolIdsByValue["@"] = Terminator::Epsilon.SymbolId;
  SymbolIdsByValue["#"] = Terminator::EndSymbol.SymbolId;
}

uint64_t SymbolManager::CreateSymbol(const std::string &Value,
                                     const SymbolType Type) {
  auto it = SymbolIdsByValue.find(Value);
  if (it != SymbolIdsByValue.end()) {
    return it->second;
  }

  uint64_t id = GetNextSymbolId();
  UsedSymbolIds.insert(id);
  SymbolIdsByValue[Value] = id;

  if (Type == SymbolType::Terminator) {
    Symbols[id] = std::make_shared<Terminator>(id, Value);
  } else if (Type == SymbolType::NonTerminator) {
    Symbols[id] = std::make_shared<NonTerminator>(id, Value);
    if (StartSymbolId == 0) {
      StartSymbolId = id;
    }
  } else {
    throw std::invalid_argument(
        "Unsupported symbol type for registering symbol: ");
  }

  return id;
}

uint64_t
SymbolManager::FetchSymbolIdByValue(const std::string &Value,
                                    const SymbolType TypeIfNotExisted) {
  if (IsSymbolExisted(Value)) {
    return SymbolIdsByValue.at(Value);
  }

  if (TypeIfNotExisted == SymbolType::Invalid) {
    throw std::invalid_argument(
        "Symbol with value '" + Value +
        "' does not exist. Consider creating it first by calling CreateSymbol "
        "with the appropriate type.");
  }

  uint64_t NewSymbolId = CreateSymbol(Value, TypeIfNotExisted);

  return NewSymbolId;
}

uint64_t SymbolManager::GetSymbolIdByValue(const std::string &Value) const {
  if (!IsSymbolExisted(Value)) {
    throw std::invalid_argument("Symbol with value '" + Value +
                                "' does not exist.");
  }
  return SymbolIdsByValue.at(Value);
}

uint64_t SymbolManager::GetSymbolCount() const { return UsedSymbolIds.size(); }

uint64_t SymbolManager::GetNextSymbolId() {
  if (NextSymbolId < ReservedSymbolIdCount) {
    throw std::overflow_error("No more symbol IDs available.");
  }

  while (UsedSymbolIds.count(NextSymbolId) > 0) {
    ++NextSymbolId;
    if (NextSymbolId < ReservedSymbolIdCount) {
      throw std::overflow_error("No more symbol IDs available.");
    }
  }

  return NextSymbolId;
}

uint64_t SymbolManager::GetStartSymbolId() const {
  if (StartSymbolId < ReservedSymbolIdCount ||
      UsedSymbolIds.count(StartSymbolId) == 0 ||
      GetSymbol(StartSymbolId)->GetType() != SymbolType::NonTerminator) {
    throw std::runtime_error("Start symbol ID has not been set.");
  }
  return StartSymbolId;
}

void SymbolManager::SetStartSymbolId(uint64_t Id) {
  if (Id < ReservedSymbolIdCount || UsedSymbolIds.count(Id) == 0 ||
      GetSymbol(Id)->GetType() != SymbolType::NonTerminator) {
    throw std::invalid_argument("Invalid start symbol ID. It must be a valid "
                                "non-terminator symbol ID.");
  }
  StartSymbolId = Id;
}

bool SymbolManager::IsSymbolExisted(const std::string &Value) const {
  if (SymbolIdsByValue.find(Value) != SymbolIdsByValue.end()) {
    return true;
  }
  return false;
}

Symbol *SymbolManager::GetSymbol(const uint64_t id) const {
  if (id == 0) {
    return &Terminator::Epsilon;
  } else if (id == 1) {
    return &Terminator::EndSymbol;
  }

  auto it = Symbols.find(id);
  if (it != Symbols.end()) {
    return it->second.get();
  }

  return nullptr;
}
