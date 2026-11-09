/*
 * Copyright (C) 2019-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Highlights/Highlight.hpp"
#include "Krystal.Booey/DOM/Ranges/StaticRange.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  /// @brief A document's custom highlights by name, in the order they were registered (which is the order
  /// they're painted in, below their priority).
  /// @see https://drafts.csswg.org/css-highlight-api-1/#highlight-registry
  class HighlightRegistry : public RefCounted<HighlightRegistry>
  {
  private:
    HashMap<DOMStringAtom, Ref<Highlight>> _map;
    List<DOMStringAtom> _highlightNames;

    HighlightRegistry() noexcept = default;

  public:
    KRYS_NODISCARD static Ref<HighlightRegistry> Create() noexcept
    {
      return AdoptRef(*new HighlightRegistry);
    }

    /// @brief Registers @p value as the highlight named @p key, in place of any highlight of that name.
    void Set(DOMStringAtom key, Ref<Highlight> &&value) noexcept;

    /// @brief The highlight named @p key, if any.
    KRYS_NODISCARD Highlight *Get(DOMStringAtom key) const noexcept;

    KRYS_NODISCARD bool Has(DOMStringAtom key) const noexcept
    {
      return _map.contains(key);
    }

    /// @brief Unregisters every highlight, clearing them.
    void Clear() noexcept;

    /// @brief Unregisters the highlight named @p key.
    /// @return Whether there was one.
    bool Delete(DOMStringAtom key) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _map.empty();
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _map.size();
    }

    /// @brief Adds @p value to the registry's annotation highlight (the text fragments' highlight).
    void AddAnnotationHighlightWithRange(Ref<StaticRange> &&value) noexcept;

    KRYS_NODISCARD const HashMap<DOMStringAtom, Ref<Highlight>> &Map() const noexcept
    {
      return _map;
    }

    KRYS_NODISCARD const List<DOMStringAtom> &HighlightNames() const noexcept
    {
      return _highlightNames;
    }
  };
}
