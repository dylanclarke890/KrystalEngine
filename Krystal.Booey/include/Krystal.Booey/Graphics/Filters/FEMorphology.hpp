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
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  enum class MorphologyOperatorType : uint8
  {
    Unknown,
    Erode,
    Dilate
  };

  /// @brief The morphology filter effect: its input thinned (eroded) or fattened (dilated).
  class FEMorphology final : public FilterEffect
  {
  private:
    MorphologyOperatorType _type;
    float _radiusX;
    float _radiusY;

  public:
    KRYS_NODISCARD static Ref<FEMorphology>
      Create(MorphologyOperatorType type, float radiusX, float radiusY,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEMorphology &other) const noexcept;

    KRYS_NODISCARD MorphologyOperatorType MorphologyOperator() const noexcept
    {
      return _type;
    }
    bool SetMorphologyOperator(MorphologyOperatorType type) noexcept;

    KRYS_NODISCARD float RadiusX() const noexcept
    {
      return _radiusX;
    }
    bool SetRadiusX(float radiusX) noexcept;

    KRYS_NODISCARD float RadiusY() const noexcept
    {
      return _radiusY;
    }
    bool SetRadiusY(float radiusY) noexcept;

  private:
    FEMorphology(MorphologyOperatorType type, float radiusX, float radiusY,
                 const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEMorphology>(*this, other);
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD bool ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEMorphology)
