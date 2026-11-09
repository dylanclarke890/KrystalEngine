/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Geometry/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The drop shadow filter effect.
  class FEDropShadow final : public FilterEffect
  {
  private:
    float _stdX;
    float _stdY;
    float _dx;
    float _dy;
    Color _shadowColor;
    float _shadowOpacity;

  public:
    KRYS_NODISCARD static Ref<FEDropShadow> Create(float stdX, float stdY, float dx, float dy,
                                                   const Color &shadowColor, float shadowOpacity) noexcept;

    KRYS_NODISCARD bool operator==(const FEDropShadow &other) const noexcept;

    KRYS_NODISCARD float StdDeviationX() const noexcept
    {
      return _stdX;
    }
    bool SetStdDeviationX(float stdX) noexcept;

    KRYS_NODISCARD float StdDeviationY() const noexcept
    {
      return _stdY;
    }
    bool SetStdDeviationY(float stdY) noexcept;

    KRYS_NODISCARD float Dx() const noexcept
    {
      return _dx;
    }
    bool SetDx(float dx) noexcept;

    KRYS_NODISCARD float Dy() const noexcept
    {
      return _dy;
    }
    bool SetDy(float dy) noexcept;

    KRYS_NODISCARD const Color &ShadowColor() const noexcept
    {
      return _shadowColor;
    }
    bool SetShadowColor(const Color &shadowColor) noexcept;

    KRYS_NODISCARD float ShadowOpacity() const noexcept
    {
      return _shadowOpacity;
    }
    bool SetShadowOpacity(float shadowOpacity) noexcept;

    /// @brief How far a shadow at `offset`, blurred by `stdDeviation`, spreads past each edge.
    KRYS_NODISCARD static IntOutsets CalculateOutsets(const FloatSize &offset,
                                                      const FloatSize &stdDeviation) noexcept;

  private:
    FEDropShadow(float stdX, float stdY, float dx, float dy, const Color &shadowColor,
                 float shadowOpacity) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEDropShadow>(*this, other);
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD EnumSet<FilterRenderingMode> SupportedFilterRenderingModes(
      EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept override;

    KRYS_NODISCARD Maybe<GraphicsStyle> CreateGraphicsStyle(GraphicsContext &context,
                                                            const Filter &filter) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEDropShadow)
