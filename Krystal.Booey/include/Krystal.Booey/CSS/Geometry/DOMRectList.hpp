/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Geometry/DOMRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief A list of rectangles, such as an element's client rects.
  /// @see https://drafts.csswg.org/geometry-1/#DOMRectList
  class DOMRectList : public RefCounted<DOMRectList>
  {
  private:
    List<Ref<DOMRect>> _items;

    DOMRectList() noexcept = default;

    explicit DOMRectList(const List<QuadF> &quads) noexcept;

    explicit DOMRectList(const List<RectF> &rects) noexcept;

  public:
    ~DOMRectList() noexcept;

    /// @brief A list of the bounding boxes of @p quads.
    KRYS_NODISCARD static Ref<DOMRectList> Create(const List<QuadF> &quads) noexcept
    {
      return AdoptRef(*new DOMRectList(quads));
    }

    KRYS_NODISCARD static Ref<DOMRectList> Create(const List<RectF> &rects) noexcept
    {
      return AdoptRef(*new DOMRectList(rects));
    }

    KRYS_NODISCARD static Ref<DOMRectList> Create() noexcept
    {
      return AdoptRef(*new DOMRectList());
    }

#pragma region DOMRectList - https://drafts.csswg.org/geometry-1/#domrectlist

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectlist-length
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_items.size());
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectlist-item
    KRYS_NODISCARD DOMRect *Item(uint32 index) const noexcept
    {
      return index < _items.size() ? _items[index].get() : nullptr;
    }

#pragma endregion

    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-indices
    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < _items.size();
    }
  };
}
