#pragma once

#include <string>

#include "types.h"

namespace lex {

class Cache {
public:
  Cache(uint cache_size);
  Cache(const Cache &other) = default;
  Cache(Cache &&other) noexcept = default;
  Cache &operator=(const Cache &other) = default;
  Cache &operator=(Cache &&other) noexcept = default;

  /** @brief Loads content into the cache.
   *  @param source The source string to load into the cache.
   *  @return The size of the loaded content.
   */
  uint LoadCache(std::string source);

  /** @brief Gets the current pointer position in the cache.
   *  @return The current pointer position.
   */
  uint GetCachePointer() const;

  /** @brief Sets the current pointer position in the cache. Attention: This
   * method should be avoided if possible, as it can lead to inconsistencies in
   * the cache state.
   *  @param pointer The new pointer position.
   */
  void SetCachePointer(uint pointer);

  /** @brief Gets the content of the cache starting from the current pointer.
   *  @return The content of the cache as a string.
   */
  std::string GetCacheContent() const;

  /** @brief Gets the size of the cache.
   *  @return The size of the cache.
   */
  uint GetCacheSize() const;

  /** @brief Sets the size of the cache.
   *  @param size The new cache size.
   */
  void SetCacheSize(uint size);

  /** @brief Clears the cache.
   */
  void ClearCache();

private:
  std::string cache;
  uint cache_size;
  uint cache_pointer;
};

} // namespace lex