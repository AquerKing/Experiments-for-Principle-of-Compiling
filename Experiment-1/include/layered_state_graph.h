#pragma once

#include <string>
#include <vector>

#include "types.h"

namespace lex {

class LayeredStateGraph final {
public:
  LayeredStateGraph() = default;
  LayeredStateGraph(std::vector<std::string> word_lists)
      : word_lists(std::move(word_lists)) {}
  ~LayeredStateGraph() = default;

  LayeredStateGraph(const LayeredStateGraph &) = delete;
  LayeredStateGraph &operator=(const LayeredStateGraph &) = delete;
  LayeredStateGraph(LayeredStateGraph &&) = default;
  LayeredStateGraph &operator=(LayeredStateGraph &&) = default;

private:
  std::vector<std::string> word_lists;
};

} // namespace lex