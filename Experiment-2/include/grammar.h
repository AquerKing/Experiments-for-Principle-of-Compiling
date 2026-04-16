#pragma once

#include "interface.h"
#include "symbols.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/** Represents a generative expression in the grammar. */
class GenerativeExpression final : public IStringConvertable {
public:
  GenerativeExpression(uint64_t Source, std::vector<uint64_t> Targets,
                       SymbolManager* Manager)
      : Source(Source), Targets(std::move(Targets)), Manager(Manager) {}

  std::string ToString() const override;

  bool IsValid() const { return Source != 0 && !Targets.empty(); }

private:
  SymbolManager* Manager;
  uint64_t Source;
  std::vector<uint64_t> Targets;
};