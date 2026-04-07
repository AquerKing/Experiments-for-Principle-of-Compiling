#include "loader.h"

namespace lex {

void SourceLoader::LoadSingleLine(std::istream &input_stream) {
  std::string line;
  if (std::getline(input_stream, line)) {
    lines.push(line);
  }
}

std::string SourceLoader::GetLine() noexcept {
  if (lines.empty()) {
    return {};
  }
  std::string line = lines.front();
  lines.pop();
  return line;
}

std::string SourceLoader::FetchLine() const noexcept {
  if (lines.empty()) {
    return {};
  }
  return lines.front();
}

void SourceLoader::RemoveLine() noexcept {
  if (!lines.empty()) {
    lines.pop();
  }
}

void SourceLoader::ClearLines() noexcept {
  while (!lines.empty()) {
    lines.pop();
  }
}

bool SourceLoader::HasLines() const noexcept { return !lines.empty(); }

} // namespace lex