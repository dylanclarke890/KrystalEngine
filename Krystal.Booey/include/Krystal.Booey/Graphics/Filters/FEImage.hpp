/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Dirk Schulze <krit@webkit.org>
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
#include "Krystal.Booey/Graphics/Images/SourceImage.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The image filter effect: an image, or an element drawn into an image buffer.
  class FEImage final : public FilterEffect
  {
  private:
    SourceImage _sourceImage;
    FloatRect _sourceImageRect;
    svg::SVGPreserveAspectRatioValue _preserveAspectRatio;

  public:
    KRYS_NODISCARD static Ref<FEImage>
      Create(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
             const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept;

    KRYS_NODISCARD bool operator==(const FEImage &other) const noexcept;

    KRYS_NODISCARD const SourceImage &GetSourceImage() const noexcept
    {
      return _sourceImage;
    }
    void SetImageSource(SourceImage &&sourceImage) noexcept
    {
      _sourceImage = krys::move(sourceImage);
    }

    KRYS_NODISCARD FloatRect SourceImageRect() const noexcept
    {
      return _sourceImageRect;
    }
    KRYS_NODISCARD const svg::SVGPreserveAspectRatioValue &PreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatio;
    }

  private:
    FEImage(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
            const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEImage>(*this, other);
    }

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 0u;
    }

    // FEImage results are always in DestinationColorSpace::SRGB()
    void SetOperatingColorSpace(const DestinationColorSpace &) noexcept override
    {
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEImage)
