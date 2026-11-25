/*
 * Copyright (C) 2016, 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Geometry/DOMRectReadOnly.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"

namespace krys::boo
{
  /// @brief A rectangle whose coordinates can be changed.
  /// @see https://drafts.csswg.org/geometry-1/#DOMRect
  class DOMRect : public DOMRectReadOnly
  {
    DOMRect() noexcept = default;

    DOMRect(double x, double y, double width, double height) noexcept : DOMRectReadOnly(x, y, width, height)
    {
    }

  public:
#pragma region DOMRect - https://drafts.csswg.org/geometry-1/#domrect

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrectreadonly
    KRYS_NODISCARD static Ref<DOMRect> Create(double x = 0, double y = 0, double width = 0,
                                              double height = 0) noexcept
    {
      return AdoptRef(*new DOMRect(x, y, width, height));
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrect-fromrect
    KRYS_NODISCARD static Ref<DOMRect> FromRect(const DOMRectInit &other) noexcept
    {
      // https://drafts.csswg.org/geometry-1/#create-a-domrectreadonly-from-the-dictionary
      return Create(other.X, other.Y, other.Width, other.Height);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-x
    void SetX(double x) noexcept
    {
      _x = x;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-y
    void SetY(double y) noexcept
    {
      _y = y;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-width
    void SetWidth(double width) noexcept
    {
      _width = width;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-height
    void SetHeight(double height) noexcept
    {
      _height = height;
    }

#pragma endregion

    /// @brief A rectangle with @p rect's position and size.
    KRYS_NODISCARD static Ref<DOMRect> Create(const RectF &rect) noexcept
    {
      return AdoptRef(*new DOMRect(rect.X(), rect.Y(), rect.Width(), rect.Height()));
    }

    /// @brief A rectangle with @p rect's position and size.
    KRYS_NODISCARD static Ref<DOMRect> Create(const RectI &rect) noexcept
    {
      return AdoptRef(*new DOMRect(rect.X(), rect.Y(), rect.Width(), rect.Height()));
    }
  };

  static_assert(sizeof(DOMRect) == sizeof(DOMRectReadOnly));
}
