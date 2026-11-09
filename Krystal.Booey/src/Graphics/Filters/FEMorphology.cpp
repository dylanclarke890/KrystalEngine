/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2017-2022 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FEMorphology.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<FEMorphology> FEMorphology::Create(MorphologyOperatorType type, float radiusX, float radiusY,
                                         const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEMorphology(type, radiusX, radiusY, colorSpace));
  }

  FEMorphology::FEMorphology(MorphologyOperatorType type, float radiusX, float radiusY,
                             const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEMorphology, colorSpace), _type(type),
        _radiusX(std::max(0.0f, radiusX)), _radiusY(std::max(0.0f, radiusY))
  {
  }

  bool FEMorphology::operator==(const FEMorphology &other) const noexcept
  {
    return FilterEffect::operator==(other) && _type == other._type && _radiusX == other._radiusX
           && _radiusY == other._radiusY;
  }

  bool FEMorphology::SetMorphologyOperator(MorphologyOperatorType type) noexcept
  {
    if (_type == type)
    {
      return false;
    }
    _type = type;
    return true;
  }

  bool FEMorphology::SetRadiusX(float radiusX) noexcept
  {
    radiusX = std::max(0.0f, radiusX);
    if (_radiusX == radiusX)
    {
      return false;
    }
    _radiusX = radiusX;
    return true;
  }

  bool FEMorphology::SetRadiusY(float radiusY) noexcept
  {
    radiusY = std::max(0.0f, radiusY);
    if (_radiusY == radiusY)
    {
      return false;
    }
    _radiusY = radiusY;
    return true;
  }

  FloatRect FEMorphology::CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                             const FloatRect &primitiveSubregion) const noexcept
  {
    auto imageRect = inputImageRects[0];
    imageRect.Inflate(filter.ResolvedSize({_radiusX, _radiusY}));
    return filter.ClipToMaxEffectRect(imageRect, primitiveSubregion);
  }

  bool FEMorphology::ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept
  {
    return inputs[0]->IsAlphaImage();
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts, const MorphologyOperatorType &type) noexcept
    {
      switch (type)
      {
        case MorphologyOperatorType::Unknown: ts << u8"UNKNOWN"; break;
        case MorphologyOperatorType::Erode:   ts << u8"ERODE"; break;
        case MorphologyOperatorType::Dilate:  ts << u8"DILATE"; break;
      }
      return ts;
    }
  }

  TextStream &FEMorphology::ExternalRepresentation(TextStream &ts,
                                                   FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feMorphology";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" operator=\"" << MorphologyOperator() << u8'"';
    ts << u8" radius=\"" << RadiusX() << u8", " << RadiusY() << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
