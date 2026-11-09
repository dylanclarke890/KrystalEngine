/*
 * Copyright (C) 2008 Alex Mathews <possessedpenguinbob@gmail.com>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Color/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffectApplier.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffectGeometry.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterFunction.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsStyle.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  class Filter;

  /// @brief One filter primitive.
  class FilterEffect : public FilterFunction
  {
  protected:
    DestinationColorSpace _operatingColorSpace {DestinationColorSpace::SRGB()};

  public:
    KRYS_NODISCARD virtual bool operator==(const FilterEffect &other) const noexcept;

    KRYS_NODISCARD const DestinationColorSpace &OperatingColorSpace() const noexcept
    {
      return _operatingColorSpace;
    }
    virtual void SetOperatingColorSpace(const DestinationColorSpace &colorSpace) noexcept
    {
      _operatingColorSpace = colorSpace;
    }

    KRYS_NODISCARD uint32 NumberOfImageInputs() const noexcept
    {
      return FilterType() == FilterEffect::Type::SourceGraphic ? 1u : NumberOfEffectInputs();
    }

    /// @brief Takes the effect's inputs from the top of @p stack, the topmost first; none when there aren't
    /// enough.
    KRYS_NODISCARD FilterImageVector TakeImageInputs(FilterImageVector &stack) const noexcept;

    /// @brief The effect applied to @p inputs, or its result already in @p results; null when it can't be.
    KRYS_NODISCARD RefPtr<FilterImage> Apply(const Filter &filter, Span<const Ref<FilterImage>> inputs,
                                             FilterResults &results,
                                             const Maybe<FilterEffectGeometry> &geometry = null) noexcept;

    /// @brief The style that applies the effect in the graphics context, following @p input, and where its
    /// result goes.
    KRYS_NODISCARD FilterStyle
      CreateFilterStyle(GraphicsContext &context, const Filter &filter, const FilterStyle &input,
                        const Maybe<FilterEffectGeometry> &geometry = null) const noexcept;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;

  protected:
    explicit FilterEffect(Type type,
                          const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    template <typename FilterEffectType>
    KRYS_NODISCARD static bool AreEqual(const FilterEffectType &a, const FilterEffect &b) noexcept
    {
      auto *bType = DynamicDowncast<FilterEffectType>(b);
      return bType && a.operator==(*bType);
    }

    KRYS_NODISCARD virtual uint32 NumberOfEffectInputs() const noexcept
    {
      return 1u;
    }

    KRYS_NODISCARD FloatRect
      CalculatePrimitiveSubregion(const Filter &filter, Span<const FloatRect> inputPrimitiveSubregions,
                                  const Maybe<FilterEffectGeometry> &geometry) const noexcept;

    KRYS_NODISCARD virtual FloatRect CalculateImageRect(const Filter &filter,
                                                        Span<const FloatRect> inputImageRects,
                                                        const FloatRect &primitiveSubregion) const noexcept;

    // Solid black image with different alpha values.
    KRYS_NODISCARD virtual bool ResultIsAlphaImage(Span<const Ref<FilterImage>>) const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool ResultIsValidPremultiplied() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual const DestinationColorSpace &
      ResultColorSpace(Span<const Ref<FilterImage>>) const noexcept
    {
      return _operatingColorSpace;
    }
    virtual void TransformInputsColorSpace(Span<const Ref<FilterImage>> inputs) const noexcept;

    /// @brief The applier for the filter's rendering mode: the accelerated backend's, from @p results.
    KRYS_NODISCARD UniquePtr<FilterEffectApplier> CreateApplier(const Filter &filter,
                                                                const FilterResults &results) const noexcept;

    KRYS_NODISCARD virtual Maybe<GraphicsStyle> CreateGraphicsStyle(GraphicsContext &,
                                                                    const Filter &) const noexcept
    {
      return null;
    }

    KRYS_NODISCARD RefPtr<FilterImage> Apply(const Filter &filter, FilterImage &input,
                                             FilterResults &results) noexcept override;
    KRYS_NODISCARD FilterStyleVector CreateFilterStyles(GraphicsContext &context, const Filter &filter,
                                                        const FilterStyle &input) const noexcept override;
  };

  TextStream &operator<<(TextStream &ts, const FilterEffect &filterEffect) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::FilterEffect)
  KRYS_NODISCARD static bool IsType(const krys::boo::FilterFunction &function) noexcept
  {
    return function.IsFilterEffect();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
