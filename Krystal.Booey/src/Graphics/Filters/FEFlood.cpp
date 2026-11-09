/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/FEFlood.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEFlood> FEFlood::Create(const Color &floodColor, float floodOpacity,
                               const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEFlood(floodColor, floodOpacity, colorSpace));
  }

  FEFlood::FEFlood(const Color &floodColor, float floodOpacity,
                   const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEFlood, colorSpace), _floodColor(floodColor),
        _floodOpacity(floodOpacity)
  {
  }

  bool FEFlood::operator==(const FEFlood &other) const noexcept
  {
    return FilterEffect::operator==(other) && _floodColor == other._floodColor
           && _floodOpacity == other._floodOpacity;
  }

  bool FEFlood::SetFloodColor(const Color &color) noexcept
  {
    if (_floodColor == color)
    {
      return false;
    }
    _floodColor = color;
    return true;
  }

  bool FEFlood::SetFloodOpacity(float floodOpacity) noexcept
  {
    if (_floodOpacity == floodOpacity)
    {
      return false;
    }
    _floodOpacity = floodOpacity;
    return true;
  }

  FloatRect FEFlood::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                        const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }

  TextStream &FEFlood::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feFlood";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" flood-color=\"" << SerialisationForRenderTreeAsText(FloodColor()) << u8'"';
    ts << u8" flood-opacity=\"" << FloodOpacity() << u8'"';
    ts << u8"]\n";
    return ts;
  }
}
