/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Dirk Schulze <krit@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/FEImage.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEImage> FEImage::Create(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
                               const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept
  {
    return AdoptRef(*new FEImage(krys::move(sourceImage), sourceImageRect, preserveAspectRatio));
  }

  FEImage::FEImage(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
                   const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept
      : FilterEffect(Type::FEImage), _sourceImage(krys::move(sourceImage)), _sourceImageRect(sourceImageRect),
        _preserveAspectRatio(preserveAspectRatio)
  {
  }

  bool FEImage::operator==(const FEImage &other) const noexcept
  {
    return FilterEffect::operator==(other) && _sourceImage == other._sourceImage
           && _sourceImageRect == other._sourceImageRect
           && _preserveAspectRatio == other._preserveAspectRatio;
  }

  FloatRect FEImage::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                        const FloatRect &primitiveSubregion) const noexcept
  {
    if (_sourceImage.NativeImageIfExists())
    {
      auto imageRect = primitiveSubregion;
      auto srcRect = _sourceImageRect;
      _preserveAspectRatio.TransformRect(imageRect, srcRect);
      return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
    }

    if (_sourceImage.ImageBufferIfExists())
    {
      return filter.MaxEffectRect(primitiveSubregion);
    }

    krys_debug_assert(false);
    return FloatRect();
  }

  TextStream &FEImage::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feImage";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" image-size=\"" << _sourceImageRect.Width() << u8'x' << _sourceImageRect.Height() << u8'"';
    // FIXME: should this dump also object returned by FEImage::image() ?

    ts << u8"]\n";
    return ts;
  }
}
