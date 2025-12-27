#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include <cassert>

namespace Krys::Gfx
{
  template <typename CacheKey, typename ResourceHandle>
  class ResourceHandleCache
  {
    struct CacheResource
    {
      ResourceHandle Handle;
      uint16 ReferenceCount {1u};
    };

    Map<CacheKey, CacheResource> _cache;

  public:
    void Add(const CacheKey &key, ResourceHandle handle) noexcept
    {
      auto it = _cache.find(key);
      if (it != _cache.end())
      {
        assert(it->second.Handle.Id == handle.Id && "Mismatched handles for the same key.");
        it->second.ReferenceCount++;
      }
      else
      {
        _cache[key] = {.Handle = handle, .ReferenceCount = 1u};
      }
    }

    KRYS_NODISCARD ResourceHandle Get(const CacheKey &key) noexcept
    {
      auto it = _cache.find(key);
      if (it != _cache.end())
      {
        it->second.ReferenceCount++;
        return it->second.Handle;
      }
      return ResourceHandle {};
    }

    bool Remove(ResourceHandle handle) noexcept
    {
      auto it = std::find_if(_cache.begin(), _cache.end(),
                             [handle](const auto &pair) { return pair.second.Handle.Id == handle.Id; });
      if (it != _cache.end())
      {
        assert(it->second.ReferenceCount > 0u && "Reference count is already zero.");
        if (--it->second.ReferenceCount == 0u)
        {
          _cache.erase(it);
          return true;
        }
      }
      return false;
    }

    void Clear() noexcept
    {
      _cache.clear();
    }

    auto begin() noexcept
    {
      return _cache.begin();
    }

    auto end() noexcept
    {
      return _cache.end();
    }
  };
}