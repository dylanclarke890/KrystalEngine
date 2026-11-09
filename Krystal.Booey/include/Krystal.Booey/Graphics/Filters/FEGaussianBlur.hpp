/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/FEConvolveMatrix.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Geometry/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The Gaussian blur filter effect.
  class FEGaussianBlur final : public FilterEffect
  {
  private:
    float _stdX;
    float _stdY;
    EdgeModeType _edgeMode;

  public:
    KRYS_NODISCARD static Ref<FEGaussianBlur> Create(float x, float y, EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD bool operator==(const FEGaussianBlur &other) const noexcept;

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

    KRYS_NODISCARD EdgeModeType EdgeMode() const noexcept
    {
      return _edgeMode;
    }
    bool SetEdgeMode(EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD static IntSize CalculateKernelSize(const Filter &filter, FloatSize stdDeviation) noexcept;
    /// @brief The size of the box blur kernels that approximate a Gaussian blur of `stdDeviation`.
    KRYS_NODISCARD static IntSize CalculateUnscaledKernelSize(FloatSize stdDeviation) noexcept;
    /// @brief How far a blur of `stdDeviation` spreads past each edge.
    KRYS_NODISCARD static IntSize CalculateOutsetSize(FloatSize stdDeviation) noexcept;

    KRYS_NODISCARD static IntOutsets CalculateOutsets(const FloatSize &stdDeviation) noexcept;

  private:
    FEGaussianBlur(float x, float y, EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEGaussianBlur>(*this, other);
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD bool ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept override;

    KRYS_NODISCARD EnumSet<FilterRenderingMode> SupportedFilterRenderingModes(
      EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept override;

    KRYS_NODISCARD Maybe<GraphicsStyle> CreateGraphicsStyle(GraphicsContext &context,
                                                            const Filter &filter) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEGaussianBlur)
