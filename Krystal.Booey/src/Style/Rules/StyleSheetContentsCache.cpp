/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Rules/StyleSheetContentsCache.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

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
