/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"

namespace krys::boo::style
{
  enum class ShadowStyle : bool
  {
    Normal,
    Inset
  };

  template <typename T>
  concept Shadow = requires(const T &shadow, const style::ZoomFactor &zoomFactor) {
    { GetShadowStyle(shadow) } -> std::same_as<ShadowStyle>;
    { IsInset(shadow) } -> std::same_as<bool>;
    { PaintingSpread(shadow, zoomFactor) } -> std::same_as<LayoutUnit>;
  };

  template <typename ShadowType>
  using ShadowList = css::CommaSeparatedFixedList<ShadowType>;
  template <typename ShadowType>
  struct Shadows : css::ListOrNone<ShadowList<ShadowType>>
  {
    using css::ListOrNone<ShadowList<ShadowType>>::ListOrNone;
  };

  KRYS_NODISCARD LayoutUnit PaintingExtent(Shadow auto const &shadow,
                                           const style::ZoomFactor &zoomFactor) noexcept
  {
    // Blurring uses a Gaussian function whose std. deviation is _radius/2, and which in theory
    // extends to infinity. In 8-bit contexts, however, rounding causes the effect to become
    // undetectable at around 1.4x the radius.
    constexpr const float radiusExtentMultiplier = 1.4f;
    return LayoutUnit {std::ceil(shadow.Blur.ResolveZoom(zoomFactor) * radiusExtentMultiplier)};
  }

  KRYS_NODISCARD LayoutUnit PaintingExtentAndSpread(Shadow auto const &shadow,
                                                    const style::ZoomFactor &zoomFactor) noexcept
  {
    return PaintingExtent(shadow, zoomFactor) + PaintingSpread(shadow, zoomFactor);
  }

  template <Shadow ShadowType>
  KRYS_NODISCARD LayoutBoxExtent ShadowOutsetExtent(const Shadows<ShadowType> &shadows,
                                                    const style::ZoomFactor &zoomFactor) noexcept
  {
    LayoutUnit top;
    LayoutUnit right;
    LayoutUnit bottom;
    LayoutUnit left;

    for (const auto &shadow : shadows)
    {
      if (IsInset(shadow))
      {
        continue;
      }

      auto extentAndSpread = PaintingExtentAndSpread(shadow, zoomFactor);

      left =
        std::min<LayoutUnit>(left, LayoutUnit(shadow.Location.x().ResolveZoom(zoomFactor)) - extentAndSpread);
      right = std::max<LayoutUnit>(right,
                                   LayoutUnit(shadow.Location.x().ResolveZoom(zoomFactor)) + extentAndSpread);
      top =
        std::min<LayoutUnit>(top, LayoutUnit(shadow.Location.y().ResolveZoom(zoomFactor)) - extentAndSpread);
      bottom = std::max<LayoutUnit>(bottom, LayoutUnit(shadow.Location.y().ResolveZoom(zoomFactor))
                                              + extentAndSpread);
    }

    return {top, right, bottom, left};
  }

  template <Shadow ShadowType>
  KRYS_NODISCARD LayoutBoxExtent ShadowInsetExtent(const Shadows<ShadowType> &shadows,
                                                   const style::ZoomFactor &zoomFactor) noexcept
  {
    LayoutUnit top;
    LayoutUnit right;
    LayoutUnit bottom;
    LayoutUnit left;

    for (const auto &shadow : shadows)
    {
      if (!IsInset(shadow))
      {
        continue;
      }

      auto extentAndSpread = PaintingExtentAndSpread(shadow, zoomFactor);

      top =
        std::max<LayoutUnit>(top, LayoutUnit(shadow.Location.y().ResolveZoom(zoomFactor)) + extentAndSpread);
      right = std::min<LayoutUnit>(right,
                                   LayoutUnit(shadow.Location.x().ResolveZoom(zoomFactor)) - extentAndSpread);
      bottom = std::min<LayoutUnit>(bottom, LayoutUnit(shadow.Location.y().ResolveZoom(zoomFactor))
                                              - extentAndSpread);
      left =
        std::max<LayoutUnit>(left, LayoutUnit(shadow.Location.x().ResolveZoom(zoomFactor)) + extentAndSpread);
    }

    return {top, right, bottom, left};
  }

  template <Shadow ShadowType>
  KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit>
    ShadowHorizontalExtent(const Shadows<ShadowType> &shadows, const style::ZoomFactor &zoomFactor) noexcept
  {
    LayoutUnit left = 0;
    LayoutUnit right = 0;

    for (const auto &shadow : shadows)
    {
      if (IsInset(shadow))
      {
        continue;
      }

      auto extentAndSpread = PaintingExtentAndSpread(shadow, zoomFactor);

      left =
        std::min<LayoutUnit>(left, LayoutUnit(shadow.Location.x().ResolveZoom(zoomFactor)) - extentAndSpread);
      right = std::max<LayoutUnit>(right,
                                   LayoutUnit(shadow.Location.x().ResolveZoom(zoomFactor)) + extentAndSpread);
    }

    return {left, right};
  }

  template <Shadow ShadowType>
  KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit>
    ShadowVerticalExtent(const Shadows<ShadowType> &shadows, const style::ZoomFactor &zoomFactor) noexcept
  {
    LayoutUnit top = 0;
    LayoutUnit bottom = 0;

    for (const auto &shadow : shadows)
    {
      if (IsInset(shadow))
      {
        continue;
      }

      auto extentAndSpread = PaintingExtentAndSpread(shadow, zoomFactor);

      // FIXME: Why does this do a static cast to `int` but all of the other "extent" functions in this file
      // do not?
      top = std::min<LayoutUnit>(
        top, LayoutUnit(static_cast<int>(shadow.Location.y().ResolveZoom(zoomFactor))) - extentAndSpread);
      bottom = std::max<LayoutUnit>(
        bottom, LayoutUnit(static_cast<int>(shadow.Location.y().ResolveZoom(zoomFactor))) + extentAndSpread);
    }

    return {top, bottom};
  }

  template <Shadow ShadowType>
  KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit>
    ShadowBlockDirectionExtent(const Shadows<ShadowType> &shadows, WritingMode writingMode,
                               const style::ZoomFactor &zoomFactor) noexcept
  {
    return writingMode.IsHorizontal() ? ShadowVerticalExtent(shadows, zoomFactor)
                                      : ShadowHorizontalExtent(shadows, zoomFactor);
  }

  template <Shadow ShadowType>
  KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit>
    ShadowInlineDirectionExtent(const Shadows<ShadowType> &shadows, WritingMode writingMode,
                                const style::ZoomFactor &zoomFactor) noexcept
  {
    return writingMode.IsHorizontal() ? ShadowHorizontalExtent(shadows, zoomFactor)
                                      : ShadowVerticalExtent(shadows, zoomFactor);
  }

  template <Shadow ShadowType>
  void AdjustRectForShadow(LayoutRect &rect, const Shadows<ShadowType> &shadows,
                           const style::ZoomFactor &zoomFactor) noexcept
  {
    auto shadowExtent = ShadowOutsetExtent(shadows, zoomFactor);

    rect.Move(shadowExtent.Left(), shadowExtent.Top());
    rect.SetWidth(rect.Width() - shadowExtent.Left() + shadowExtent.Right());
    rect.SetHeight(rect.Height() - shadowExtent.Top() + shadowExtent.Bottom());
  }

  template <Shadow ShadowType>
  void AdjustRectForShadow(FloatRect &rect, const Shadows<ShadowType> &shadows,
                           const style::ZoomFactor &zoomFactor) noexcept
  {
    auto shadowExtent = ShadowOutsetExtent(shadows, zoomFactor);

    rect.Move(shadowExtent.Left(), shadowExtent.Top());
    rect.SetWidth(rect.Width() - shadowExtent.Left() + shadowExtent.Right());
    rect.SetHeight(rect.Height() - shadowExtent.Top() + shadowExtent.Bottom());
  }
}
