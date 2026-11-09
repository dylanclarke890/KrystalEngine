/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/FEOffset.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEOffset> FEOffset::Create(float dx, float dy, const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEOffset(dx, dy, colorSpace));
  }

  FEOffset::FEOffset(float dx, float dy, const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEOffset, colorSpace), _dx(dx), _dy(dy)
  {
  }

  bool FEOffset::operator==(const FEOffset &other) const noexcept
  {
    return FilterEffect::operator==(other) && _dx == other._dx && _dy == other._dy;
  }

  bool FEOffset::SetDx(float dx) noexcept
  {
    if (_dx == dx)
    {
      return false;
    }
    _dx = dx;
    return true;
  }

  bool FEOffset::SetDy(float dy) noexcept
  {
    if (_dy == dy)
    {
      return false;
    }
    _dy = dy;
    return true;
  }

  FloatRect FEOffset::CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                         const FloatRect &primitiveSubregion) const noexcept
  {
    auto imageRect = inputImageRects[0];
    imageRect.Move(filter.ResolvedSize({_dx, _dy}));
    return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
  }

  IntOutsets FEOffset::CalculateOutsets(const FloatSize &offset) noexcept
  {
    auto adjustedOffset = ExpandedIntSize(offset);

    IntOutsets outsets;
    if (adjustedOffset.Height() < 0)
    {
      outsets.SetTop(-adjustedOffset.Height());
    }
    else
    {
      outsets.SetBottom(adjustedOffset.Height());
    }
    if (adjustedOffset.Width() < 0)
    {
      outsets.SetLeft(-adjustedOffset.Width());
    }
    else
    {
      outsets.SetRight(adjustedOffset.Width());
    }

    return outsets;
  }

  bool FEOffset::ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    return inputs[0]->IsAlphaImage();
  }

  TextStream &FEOffset::ExternalRepresentation(TextStream &ts,
                                               FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feOffset";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" dx=\"" << Dx() << u8"\" dy=\"" << Dy() << u8'"';
    ts << u8"]\n";
    return ts;
  }
}
