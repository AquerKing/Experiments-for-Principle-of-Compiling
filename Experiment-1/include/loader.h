#pragma once

#include <istream>
#include <queue>

namespace lex {

class SourceLoader final {
public:
  SourceLoader() = default;
  ~SourceLoader() = default;

  /**
   * Loads a single line from the input stream.
   * @param input_stream The input stream to read from.
   */
  void LoadSingleLine(std::istream &input_stream);

  /**
   * Gets the next line from the loaded lines.
   * @return The next line.
   */
  std::string GetLine() noexcept;

  /**
   * Fetches the next line without removing it from the queue.
   * @return The next line if available, otherwise an empty string.
   */
  std::string FetchLine() const noexcept;

  /**
   * Removes the next line from the queue.
   */
  void RemoveLine() noexcept;

  /**
   * Clears all loaded lines.
   */
  void ClearLines() noexcept;

  /**
   * Checks if there are any lines loaded.
   * @return True if there are lines available, false otherwise.
   */
  bool HasLines() const noexcept;

private:
  std::queue<std::string> lines;
};

} // namespace lex