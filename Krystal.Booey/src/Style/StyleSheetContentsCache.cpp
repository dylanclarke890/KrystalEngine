#include "Krystal.Booey/Style/StyleSheetContentsCache.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style
{
  size_t StyleSheetContentsCache::KeyHash::operator()(const Key &key) const noexcept
  {
    return Hash::Combine(std::hash<css::CSSOMString> {}(key.first), key.second.Hash());
  }

  StyleSheetContentsCache::StyleSheetContentsCache() noexcept = default;

  StyleSheetContentsCache::~StyleSheetContentsCache() noexcept = default;

  StyleSheetContentsCache &StyleSheetContentsCache::Singleton() noexcept
  {
    static MainThreadNeverDestroyed<StyleSheetContentsCache> cache;
    return cache;
  }

  RefPtr<css::StyleSheetContents> StyleSheetContentsCache::Get(const Key &key) const noexcept
  {
    auto it = _cache.find(key);
    if (it == _cache.end())
    {
      return nullptr;
    }
    return it->second;
  }

  void StyleSheetContentsCache::Add(Key &&key, Ref<css::StyleSheetContents> contents) noexcept
  {
    krys_debug_assert(contents->IsCacheable());

    if (!_cache.try_emplace(krys::move(key), contents).second)
    {
      return;
    }
    contents->AddedToMemoryCache();

    constexpr static auto MaximumCacheSize = 256uz;
    if (_cache.size() > MaximumCacheSize)
    {
      auto toRemove = _cache.begin();
      toRemove->second->RemovedFromMemoryCache();
      _cache.erase(toRemove);
    }
  }

  void StyleSheetContentsCache::Clear() noexcept
  {
    _cache.clear();
  }
}
