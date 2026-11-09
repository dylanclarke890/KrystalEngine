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

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  enum class ColorMatrixType : uint8
  {
    FECOLORMATRIX_TYPE_UNKNOWN = 0,
    FECOLORMATRIX_TYPE_MATRIX = 1,
    FECOLORMATRIX_TYPE_SATURATE = 2,
    FECOLORMATRIX_TYPE_HUEROTATE = 3,
    FECOLORMATRIX_TYPE_LUMINANCETOALPHA = 4
  };

  /// @brief The colour matrix filter effect: a matrix applied to each pixel's straight-alpha colour.
  class FEColorMatrix final : public FilterEffect
  {
  private:
    ColorMatrixType _type;
    List<float> _values;

  public:
    KRYS_NODISCARD static Ref<FEColorMatrix> Create(ColorMatrixType type, List<float> &&values) noexcept;

    KRYS_NODISCARD bool operator==(const FEColorMatrix &other) const noexcept;

    KRYS_NODISCARD ColorMatrixType GetType() const noexcept
    {
      return _type;
    }
    bool SetType(ColorMatrixType type) noexcept;

    KRYS_NODISCARD const List<float> &Values() const noexcept
    {
      return _values;
    }
    bool SetValues(const List<float> &values) noexcept;

    static void CalculateSaturateComponents(Span<float, 9> components, float value) noexcept;
    static void CalculateHueRotateComponents(Span<float, 9> components, float value) noexcept;
    KRYS_NODISCARD static List<float> NormalizedFloats(const List<float> &values) noexcept;

  private:
    FEColorMatrix(ColorMatrixType type, List<float> &&values) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEColorMatrix>(*this, other);
    }

    KRYS_NODISCARD bool ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept override;

    KRYS_NODISCARD EnumSet<FilterRenderingMode> SupportedFilterRenderingModes(
      EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept override;

    KRYS_NODISCARD Maybe<GraphicsStyle> CreateGraphicsStyle(GraphicsContext &context,
                                                            const Filter &filter) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEColorMatrix)
