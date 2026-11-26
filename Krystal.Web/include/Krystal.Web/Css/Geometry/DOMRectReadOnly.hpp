/*
 * Copyright (C) 2016-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include <cmath>
#include <limits>

namespace krys::web
{
  /// @see https://drafts.csswg.org/geometry-1/#dictdef-domrectinit
  struct DOMRectInit
  {
    double X {0};
    double Y {0};
    double Width {0};
    double Height {0};
  };

  /// @brief The NaN-safe minimum of @p a and @p b: NaN if either is NaN, otherwise the smaller.
  /// @see https://drafts.csswg.org/geometry-1/#nan-safe-minimum
  KRYS_NODISCARD constexpr double NaNSafeMinimum(double a, double b) noexcept
  {
    if (a != a || b != b)
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return a < b ? a : b;
  }

  /// @brief The NaN-safe maximum of @p a and @p b: NaN if either is NaN, otherwise the larger.
  /// @see https://drafts.csswg.org/geometry-1/#nan-safe-maximum
  KRYS_NODISCARD constexpr double NaNSafeMaximum(double a, double b) noexcept
  {
    if (a != a || b != b)
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return a > b ? a : b;
  }

  /// @brief A rectangle whose coordinates can't be changed through it.
  /// @see https://drafts.csswg.org/geometry-1/#DOMRect
  class DOMRectReadOnly : public RefCounted<DOMRectReadOnly>
  {
  protected:
    // Any of these can be NaN or Inf.
    double _x {0};
    double _y {0};
    double _width {0};  // Can be negative.
    double _height {0}; // Can be negative.

    DOMRectReadOnly() noexcept = default;

    DOMRectReadOnly(double x, double y, double width, double height) noexcept
        : _x(x), _y(y), _width(width), _height(height)
    {
    }

  public:
#pragma region DOMRectReadOnly - https://drafts.csswg.org/geometry-1/#domrectreadonly

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrectreadonly
    KRYS_NODISCARD static Ref<DOMRectReadOnly> Create(double x = 0, double y = 0, double width = 0,
                                                      double height = 0) noexcept
    {
      // 1. Let rect be a new DOMRectReadOnly or DOMRect object as appropriate.
      // 2. Set rect's variables x coordinate to x, y coordinate to y, width dimension to width and height
      //    dimension to height.
      // 3. Return rect.
      return AdoptRef(*new DOMRectReadOnly(x, y, width, height));
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-fromrect
    KRYS_NODISCARD static Ref<DOMRectReadOnly> FromRect(const DOMRectInit &other) noexcept
    {
      // https://drafts.csswg.org/geometry-1/#create-a-domrectreadonly-from-the-dictionary
      // 1. Let rect be a new DOMRectReadOnly or DOMRect as appropriate.
      // 2. Set rect's variables x coordinate to other's x dictionary member, y coordinate to other's y
      //    dictionary member, width dimension to other's width dictionary member and height dimension to
      //    other's height dictionary member.
      // 3. Return rect.
      return Create(other.X, other.Y, other.Width, other.Height);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-x
    KRYS_NODISCARD double X() const noexcept
    {
      return _x;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-y
    KRYS_NODISCARD double Y() const noexcept
    {
      return _y;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-width
    KRYS_NODISCARD double Width() const noexcept
    {
      return _width;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-height
    KRYS_NODISCARD double Height() const noexcept
    {
      return _height;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-top
    KRYS_NODISCARD double Top() const noexcept
    {
      return NaNSafeMinimum(_y, _y + _height);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-right
    KRYS_NODISCARD double Right() const noexcept
    {
      return NaNSafeMaximum(_x, _x + _width);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-bottom
    KRYS_NODISCARD double Bottom() const noexcept
    {
      return NaNSafeMaximum(_y, _y + _height);
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domrectreadonly-domrect-left
    KRYS_NODISCARD double Left() const noexcept
    {
      return NaNSafeMinimum(_x, _x + _width);
    }

#pragma endregion

    KRYS_NODISCARD RectF ToRectF() const noexcept
    {
      return RectF(static_cast<float>(_x), static_cast<float>(_y), static_cast<float>(_width),
                   static_cast<float>(_height));
    }
  };
}
