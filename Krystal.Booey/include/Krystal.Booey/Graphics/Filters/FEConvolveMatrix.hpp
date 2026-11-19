/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Zoltan Herczeg <zherczeg@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief How a filter primitive reads past its input's edges.
  enum class EdgeModeType : uint8
  {
    Unknown,
    Duplicate,
    Wrap,
    None,
  };

  /// @brief The convolve matrix filter effect: a kernel convolved with its input.
  class FEConvolveMatrix final : public FilterEffect
  {
  private:
    SizeI _kernelSize;
    float _divisor;
    float _bias;
    PointI _targetOffset;
    EdgeModeType _edgeMode;
    PointF _kernelUnitLength;
    bool _preserveAlpha;
    List<float> _kernelMatrix;

  public:
    KRYS_NODISCARD static Ref<FEConvolveMatrix>
      Create(const SizeI &kernelSize, float divisor, float bias, const PointI &targetOffset,
             EdgeModeType edgeMode, const PointF &kernelUnitLength, bool preserveAlpha,
             const List<float> &kernelMatrix,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEConvolveMatrix &other) const noexcept;

    KRYS_NODISCARD SizeI KernelSize() const noexcept
    {
      return _kernelSize;
    }

    void SetKernelSize(const SizeI &kernelSize) noexcept;

    KRYS_NODISCARD const List<float> &Kernel() const noexcept
    {
      return _kernelMatrix;
    }

    void SetKernel(const List<float> &kernel) noexcept;

    KRYS_NODISCARD float Divisor() const noexcept
    {
      return _divisor;
    }

    bool SetDivisor(float divisor) noexcept;

    KRYS_NODISCARD float Bias() const noexcept
    {
      return _bias;
    }

    bool SetBias(float bias) noexcept;

    KRYS_NODISCARD PointI TargetOffset() const noexcept
    {
      return _targetOffset;
    }

    bool SetTargetOffset(const PointI &targetOffset) noexcept;

    KRYS_NODISCARD EdgeModeType EdgeMode() const noexcept
    {
      return _edgeMode;
    }

    bool SetEdgeMode(EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD PointF KernelUnitLength() const noexcept
    {
      return _kernelUnitLength;
    }

    bool SetKernelUnitLength(const PointF &kernelUnitLength) noexcept;

    KRYS_NODISCARD bool PreserveAlpha() const noexcept
    {
      return _preserveAlpha;
    }

    bool SetPreserveAlpha(bool preserveAlpha) noexcept;

  private:
    FEConvolveMatrix(const SizeI &kernelSize, float divisor, float bias, const PointI &targetOffset,
                     EdgeModeType edgeMode, const PointF &kernelUnitLength, bool preserveAlpha,
                     const List<float> &kernelMatrix, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEConvolveMatrix>(*this, other);
    }

    KRYS_NODISCARD RectF CalculateImageRect(const Filter &filter, Span<const RectF> inputImageRects,
                                            const RectF &primitiveSubregion) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };

  TextStream &operator<<(TextStream &ts, const EdgeModeType &type) noexcept;
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEConvolveMatrix)
