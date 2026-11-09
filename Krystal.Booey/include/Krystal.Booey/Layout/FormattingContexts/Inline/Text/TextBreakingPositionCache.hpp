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

#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextBreakingPositionContext.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::layout
{
  /// @brief Caches where text content breaks into inline text items, so that long text doesn't need breaking
  /// again when its content changes elsewhere or it's laid out again.
  // FIXME: Evict after a few idle seconds when the cache grows past its soft threshold, once there are
  // timers; it's evicted only at its hard cap for now.
  class TextBreakingPositionCache
  {
    KRYS_NON_COPY_MOVABLE(TextBreakingPositionCache);

  public:
    static constexpr size_t MinimumRequiredTextLengthForContentBreakCache = 5;
    static constexpr size_t MinimumRequiredContentBreaks = 3;

    struct Key
    {
      utf8_string Text;
      TextBreakingPositionContext Context;

      KRYS_NODISCARD friend bool operator==(const Key &, const Key &) noexcept = default;
    };
    using List = SmallList<size_t, 8>;

  private:
    struct KeyHash
    {
      KRYS_NODISCARD size_t operator()(const Key &key) const noexcept;
    };

    using TextBreakingPositionMap = HashMap<Key, List, KeyHash>;
    TextBreakingPositionMap _breakingPositionMap;
    size_t _cachedContentSize {0};

  public:
    KRYS_NODISCARD static TextBreakingPositionCache &Singleton() noexcept;

    TextBreakingPositionCache() noexcept = default;

    void Set(const Key &key, List &&breakingPositionList) noexcept;
    KRYS_NODISCARD const List *Get(const Key &key) const noexcept;

    void Clear() noexcept;

  private:
    void Evict() noexcept;
  };
}
