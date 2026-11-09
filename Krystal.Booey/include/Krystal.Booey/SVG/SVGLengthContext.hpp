/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2019-2025 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/SVG/SVGLengthValue.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class ToLengthConversionData;
}

namespace krys::boo::dom
{
  template <typename T>
  class ExceptionOr;
}

namespace krys::boo::style
{
  struct PreferredSize;
  struct SVGCenterCoordinateComponent;
  struct SVGCoordinateComponent;
  struct SVGRadius;
  struct SVGRadiusComponent;
  struct SVGStrokeDasharrayValue;
  struct SVGStrokeDashoffset;
  struct StrokeWidth;
  struct ZoomFactor;
  struct ZoomNeeded;
}

namespace krys::boo::svg
{
  class SVGElement;

  class SVGLengthContext
  {
  private:
    WeakPtr<SVGElement> _context;
    mutable Maybe<FloatSize> _viewportSize;

  public:
    explicit SVGLengthContext(const SVGElement *, const Maybe<FloatSize> &viewportSize = {}) noexcept;
    ~SVGLengthContext() noexcept;

    template <typename T>
    KRYS_NODISCARD static FloatRect ResolveRectangle(const SVGElement *context, T &element,
                                                     SVGUnitTypes::SVGUnitType type,
                                                     const FloatRect &viewport) noexcept
    {
      return ResolveRectangle(context, type, viewport, element.X(), element.Y(), element.Width(),
                              element.Height());
    }

    template <typename T>
    KRYS_NODISCARD static FloatRect ResolveRectangle(const T &element, SVGUnitTypes::SVGUnitType type,
                                                     const FloatRect &viewport) noexcept
    {
      return ResolveRectangle(&element, element, type, viewport);
    }

    KRYS_NODISCARD static FloatRect ResolveRectangle(const SVGElement *, SVGUnitTypes::SVGUnitType,
                                                     const FloatRect &viewport, const SVGLengthValue &x,
                                                     const SVGLengthValue &y, const SVGLengthValue &width,
                                                     const SVGLengthValue &height) noexcept;
    KRYS_NODISCARD static FloatPoint ResolvePoint(const SVGElement *, SVGUnitTypes::SVGUnitType,
                                                  const SVGLengthValue &x, const SVGLengthValue &y) noexcept;
    KRYS_NODISCARD static float ResolveLength(const SVGElement *, SVGUnitTypes::SVGUnitType,
                                              const SVGLengthValue &) noexcept;

    KRYS_NODISCARD float ValueForLength(const style::PreferredSize &, style::ZoomFactor usedZoom,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;
    KRYS_NODISCARD float ValueForLength(const style::SVGCenterCoordinateComponent &, style::ZoomNeeded,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;
    KRYS_NODISCARD float ValueForLength(const style::SVGCoordinateComponent &, style::ZoomNeeded,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;
    KRYS_NODISCARD float ValueForLength(const style::SVGRadius &, style::ZoomNeeded,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;
    KRYS_NODISCARD float ValueForLength(const style::SVGRadiusComponent &, style::ZoomNeeded,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;
    KRYS_NODISCARD float ValueForLength(const style::SVGStrokeDasharrayValue &, style::ZoomNeeded,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;
    KRYS_NODISCARD float ValueForLength(const style::SVGStrokeDashoffset &, style::ZoomNeeded,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;
    KRYS_NODISCARD float ValueForLength(const style::StrokeWidth &, style::ZoomNeeded,
                                        SVGLengthMode = SVGLengthMode::Other) noexcept;

    KRYS_NODISCARD dom::ExceptionOr<float> ResolveValueToUserUnits(float, const css::LengthPercentageUnit &,
                                                                   SVGLengthMode) const noexcept;
    KRYS_NODISCARD dom::ExceptionOr<css::LengthPercentage<>>
      ResolveValueFromUserUnits(float, const css::LengthPercentageUnit &, SVGLengthMode) const noexcept;

    KRYS_NODISCARD Maybe<FloatSize> ViewportSize() const noexcept;

  private:
    KRYS_NODISCARD dom::ExceptionOr<float>
      ConvertValueFromUserUnitsToPercentage(float value, SVGLengthMode) const noexcept;
    KRYS_NODISCARD dom::ExceptionOr<float>
      ConvertValueFromPercentageToUserUnits(float value, SVGLengthMode) const noexcept;
    KRYS_NODISCARD static float ConvertValueFromPercentageToUserUnits(float value, SVGLengthMode,
                                                                      FloatSize) noexcept;

    KRYS_NODISCARD dom::ExceptionOr<float> ConvertValueFromUserUnitsToEXS(float) const noexcept;
    KRYS_NODISCARD dom::ExceptionOr<float> ConvertValueFromEXSToUserUnits(float) const noexcept;

    KRYS_NODISCARD Maybe<FloatSize> ComputeViewportSize() const noexcept;
    KRYS_NODISCARD float ComputeNonCalcLength(float, css::LengthUnit) const noexcept;
    KRYS_NODISCARD float RemoveZoomFromFontOrRootFontRelativeLength(float value,
                                                                    css::LengthUnit) const noexcept;

    KRYS_NODISCARD Maybe<css::ToLengthConversionData> CssConversionData() const noexcept;
    KRYS_NODISCARD RefPtr<const SVGElement> ProtectedContext() const noexcept;

    template <typename TSize>
    KRYS_NODISCARD float ValueForSizeType(const TSize &, style::ZoomFactor usedZoom,
                                          SVGLengthMode = SVGLengthMode::Other) noexcept
    requires(TSize::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed
             || TSize::Calc::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed);
    template <typename TSize>
    KRYS_NODISCARD float ValueForSizeType(const TSize &, style::ZoomNeeded,
                                          SVGLengthMode = SVGLengthMode::Other) noexcept;
  };
}
