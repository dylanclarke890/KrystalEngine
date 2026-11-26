/*
 * Copyright (c) 2012-2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutGeometryForward.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutUnit.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::web
{
  enum class AspectRatioFit
  {
    Shrink,
    Grow,
  };

  /// @brief @p size fitted to @p aspectRatio: keeping its height if @p fit is `Shrink` and it's too wide or
  /// `Grow` and it's too narrow, and its width otherwise.
  KRYS_NODISCARD constexpr LayoutSize FitToAspectRatio(const LayoutSize &size, const LayoutSize &aspectRatio,
                                                       AspectRatioFit fit) noexcept
  {
    float heightScale = size.Height().ToFloat() / aspectRatio.Height().ToFloat();
    float widthScale = size.Width().ToFloat() / aspectRatio.Width().ToFloat();

    if ((widthScale > heightScale) != (fit == AspectRatioFit::Grow))
    {
      return {size.Height() * aspectRatio.Width() / aspectRatio.Height(), size.Height()};
    }

    return {size.Width(), size.Width() * aspectRatio.Height() / aspectRatio.Width()};
  }

  KRYS_NODISCARD constexpr bool MightBeSaturated(const LayoutSize &size) noexcept
  {
    return size.Width().MightBeSaturated() || size.Height().MightBeSaturated();
  }

  KRYS_NODISCARD constexpr SizeI FlooredSizeI(const LayoutSize &s) noexcept
  {
    return SizeI(s.Width().Floor(), s.Height().Floor());
  }

  KRYS_NODISCARD constexpr SizeI CeiledSizeI(const LayoutSize &s) noexcept
  {
    return SizeI(s.Width().Ceil(), s.Height().Ceil());
  }

  KRYS_NODISCARD constexpr SizeI RoundedSizeI(const LayoutSize &s) noexcept
  {
    return SizeI(s.Width().Round(), s.Height().Round());
  }

  KRYS_NODISCARD inline SizeF FloorSizeToDevicePixels(const LayoutSize &size,
                                                      float pixelSnappingFactor) noexcept
  {
    return SizeF(FloorToDevicePixel(size.Width(), pixelSnappingFactor),
                 FloorToDevicePixel(size.Height(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline SizeF RoundSizeToDevicePixels(const LayoutSize &size,
                                                      float pixelSnappingFactor) noexcept
  {
    return SizeF(RoundToDevicePixel(size.Width(), pixelSnappingFactor),
                 RoundToDevicePixel(size.Height(), pixelSnappingFactor));
  }

  struct LayoutSizeLimits
  {
    LayoutSize Min;
    LayoutSize Max;

    KRYS_NODISCARD constexpr LayoutSize Clamp(const LayoutSize &size) const noexcept
    {
      return size.ConstrainedBetween(Min, Max);
    }

    KRYS_NODISCARD constexpr bool Fits(const LayoutSize &size) const noexcept
    {
      return size == Clamp(size);
    }

    KRYS_NODISCARD constexpr LayoutSize Distance(const LayoutSize &size) const noexcept
    {
      return size - Clamp(size);
    }
  };

  TextStream &operator<<(TextStream &ts, const LayoutSize &value) noexcept;
}
