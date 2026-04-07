#include "cache.h"

namespace lex {

Cache::Cache(uint cache_size) : cache_size(cache_size), cache_pointer(0) {
  cache.reserve(cache_size);
  cache.clear();
}

uint Cache::LoadCache(std::string source) {
  if (source.size() > cache_size) {
    cache = source.substr(0, cache_size);
  } else {
    cache = source;
  }
  return cache.size();
}

uint Cache::GetCachePointer() const { return cache_pointer; }

void Cache::SetCachePointer(uint pointer) { cache_pointer = pointer; }

std::string Cache::GetCacheContent() const {
  return cache.substr(cache_pointer);
}

uint Cache::GetCacheSize() const { return cache_size; }

void Cache::SetCacheSize(uint size) { cache_size = size; }

void Cache::ClearCache() {
  cache.clear();
  cache_pointer = 0;
}

} // namespace lex
