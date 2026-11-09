/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Rules/MatchResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  class CustomPropertyData;
  class Resolver;

  /// @brief Caches the styles built from identical matched declarations, so an element whose declarations
  /// match an earlier element's copies its non-inherited properties instead of applying them.
  class MatchedDeclarationsCache
  {
    KRYS_NON_COPY_MOVABLE(MatchedDeclarationsCache);

  public:
    struct Entry
    {
      RefPtr<const style::MatchResult> MatchResult;
      UniquePtr<const boo::RenderStyle> RenderStyle;
      UniquePtr<const boo::RenderStyle> ParentRenderStyle;

      KRYS_NODISCARD bool IsUsableAfterHighPriorityProperties(const boo::RenderStyle &) const noexcept;
    };

    struct Result
    {
      const MatchedDeclarationsCache::Entry &Entry;
      bool InheritedEqual;
    };

  private:
    WeakPtr<const Resolver> _owner;
    HashMap<uint32, List<Entry>> _entries;
    uint32 _additionsSinceLastSweep {0};
    Timer _sweepTimer;

  public:
    explicit MatchedDeclarationsCache(const Resolver &) noexcept;
    ~MatchedDeclarationsCache() noexcept;

    KRYS_NODISCARD static bool IsCacheable(const dom::Element &, const RenderStyle &,
                                           const RenderStyle &parentStyle) noexcept;
    KRYS_NODISCARD static uint32 ComputeHash(const MatchResult &,
                                             const CustomPropertyData &inheritedCustomProperties) noexcept;

    KRYS_NODISCARD Maybe<Result> Find(uint32 hash, const MatchResult &,
                                      const CustomPropertyData &inheritedCustomProperties,
                                      const RenderStyle &) noexcept;
    void Add(const RenderStyle &, const RenderStyle &parentStyle, uint32 hash, const MatchResult &) noexcept;
    void Remove(uint32 hash) noexcept;

    // Every N additions to the matched declaration cache trigger a sweep where entries holding
    // the last reference to a style declaration are garbage collected.
    void Invalidate() noexcept;
    void ClearEntriesAffectedByViewportUnits() noexcept;

  private:
    template <typename TCallback>
    void RemoveAllMatching(const TCallback &matches) noexcept;

    void Sweep() noexcept;
  };
}
