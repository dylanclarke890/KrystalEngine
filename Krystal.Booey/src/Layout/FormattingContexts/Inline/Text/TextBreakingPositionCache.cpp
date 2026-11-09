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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextBreakingPositionCache.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo::layout
{
  namespace
  {
    // At this amount of content (string + breaking position list) we should start evicting
    constexpr size_t EvictionSoftThreshold = 500'000;
    // Do not let the cache grow beyond this
    constexpr size_t EvictionHardCapMultiplier = 5;
  }

  TextBreakingPositionContext::TextBreakingPositionContext(const RenderStyle &style) noexcept
      : WhitespaceCollapseBehavior(BehaviorForWhiteSpaceCollapse(style.GetWhiteSpaceCollapse())),
        OverflowWrap(style.GetOverflowWrap()), LineBreak(style.GetLineBreak()),
        WordBreak(style.GetWordBreak()), Locale(style.ComputedLocale().GetPlatform())
  {
  }

  size_t TextBreakingPositionCache::KeyHash::operator()(const Key &key) const noexcept
  {
    return Hash::Combine(std::hash<utf8_stringview> {}(key.Text),
                         DefaultHash<TextBreakingPositionContext>::Hash(key.Context));
  }

  TextBreakingPositionCache &TextBreakingPositionCache::Singleton() noexcept
  {
    static NeverDestroyed<TextBreakingPositionCache> cache;
    return cache;
  }

  void TextBreakingPositionCache::Evict() noexcept
  {
    while (_cachedContentSize > EvictionSoftThreshold && !_breakingPositionMap.empty())
    {
      auto evictedEntry = _breakingPositionMap.begin();
      _cachedContentSize -= (evictedEntry->first.Text.length() + 4 * evictedEntry->second.size());
      _breakingPositionMap.erase(evictedEntry);
    }
  }

  void TextBreakingPositionCache::Set(const Key &key, List &&breakingPositionList) noexcept
  {
    krys_debug_assert(!_breakingPositionMap.contains(key));

    auto evictIfNeeded = [&]
    {
      if (_cachedContentSize < EvictionSoftThreshold)
      {
        return;
      }

      krys_debug_assert(!_breakingPositionMap.empty());
      auto isBelowHardThreshold = _cachedContentSize < EvictionSoftThreshold * EvictionHardCapMultiplier;
      if (isBelowHardThreshold)
      {
        return;
      }
      Evict();
    };
    evictIfNeeded();

    _cachedContentSize += (key.Text.length() + 4 * breakingPositionList.size());
    _breakingPositionMap.insert_or_assign(key, krys::move(breakingPositionList));
  }

  const TextBreakingPositionCache::List *TextBreakingPositionCache::Get(const Key &key) const noexcept
  {
    auto iterator = _breakingPositionMap.find(key);
    if (iterator == _breakingPositionMap.end())
    {
      return {};
    }
    return &iterator->second;
  }

  void TextBreakingPositionCache::Clear() noexcept
  {
    _breakingPositionMap.clear();
    _cachedContentSize = 0;
  }
}
