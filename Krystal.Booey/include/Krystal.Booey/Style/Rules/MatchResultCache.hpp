/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class StyleProperties;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  /// @brief A cached match result for an element, with the properties of its inline style that changed since
  /// it was cached.
  struct CachedMatchResult
  {
    style::UnadjustedStyle UnadjustedStyle;
    PropertyCascade::IncludedProperties ChangedProperties;
    /// @brief The cached style, to update once the element's style is rebuilt. Never null.
    RenderStyle *StyleToUpdate {};
  };

  /// @brief Caches the match results of elements with a mutable inline style, so that an inline style change
  /// can skip selector matching and apply only the changed properties.
  class MatchResultCache
  {
    KRYS_NON_COPY_MOVABLE(MatchResultCache);

    struct Entry;

  private:
    HashMap<const dom::Element *, UniquePtr<Entry>> _entries;
    uint32 _setsSinceLastPrune {0};

  public:
    MatchResultCache() noexcept;
    ~MatchResultCache() noexcept;

    KRYS_NODISCARD Maybe<CachedMatchResult> ResultWithCurrentInlineStyle(const dom::Element &) noexcept;
    static void Update(CachedMatchResult &, const RenderStyle &) noexcept;
    void UpdateForFastPathInherit(const dom::Element &, const RenderStyle &parentStyle) noexcept;
    void Set(const dom::Element &, const UnadjustedStyle &) noexcept;

  private:
    KRYS_NODISCARD Entry *Find(const dom::Element &) noexcept;
    void PruneDeadEntries() noexcept;

    KRYS_NODISCARD static bool
      IsUsableAfterInlineStyleChange(const MatchResultCache::Entry &,
                                     const css::StyleProperties &inlineStyle) noexcept;
    KRYS_NODISCARD static PropertyCascade::IncludedProperties
      ComputeAndUpdateChangedProperties(MatchResultCache::Entry &) noexcept;
  };
}
