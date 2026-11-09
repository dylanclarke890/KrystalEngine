/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2010 Renata Hodovan <reni@inf.u-szeged.hu>
 * Copyright (C) 2011 Gabor Loki <loki@webkit.org>
 * Copyright (C) 2017-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FETurbulence.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FETurbulence> FETurbulence::Create(TurbulenceType type, float baseFrequencyX, float baseFrequencyY,
                                         int32 numOctaves, float seed, bool stitchTiles,
                                         const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(
      *new FETurbulence(type, baseFrequencyX, baseFrequencyY, numOctaves, seed, stitchTiles, colorSpace));
  }

  FETurbulence::FETurbulence(TurbulenceType type, float baseFrequencyX, float baseFrequencyY,
                             int32 numOctaves, float seed, bool stitchTiles,
                             const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FETurbulence, colorSpace), _type(type),
        _baseFrequencyX(baseFrequencyX), _baseFrequencyY(baseFrequencyY), _numOctaves(numOctaves),
        _seed(seed), _stitchTiles(stitchTiles)
  {
  }

  bool FETurbulence::operator==(const FETurbulence &other) const noexcept
  {
    return FilterEffect::operator==(other) && _type == other._type && _baseFrequencyX == other._baseFrequencyX
           && _baseFrequencyY == other._baseFrequencyY && _numOctaves == other._numOctaves
           && _seed == other._seed && _stitchTiles == other._stitchTiles;
  }

  bool FETurbulence::SetType(TurbulenceType type) noexcept
  {
    if (_type == type)
    {
      return false;
    }
    _type = type;
    return true;
  }

  bool FETurbulence::SetBaseFrequencyY(float baseFrequencyY) noexcept
  {
    if (_baseFrequencyY == baseFrequencyY)
    {
      return false;
    }
    _baseFrequencyY = baseFrequencyY;
    return true;
  }

  bool FETurbulence::SetBaseFrequencyX(float baseFrequencyX) noexcept
  {
    if (_baseFrequencyX == baseFrequencyX)
    {
      return false;
    }
    _baseFrequencyX = baseFrequencyX;
    return true;
  }

  bool FETurbulence::SetSeed(float seed) noexcept
  {
    if (_seed == seed)
    {
      return false;
    }
    _seed = seed;
    return true;
  }

  bool FETurbulence::SetNumOctaves(int32 numOctaves) noexcept
  {
    if (_numOctaves == numOctaves)
    {
      return false;
    }
    _numOctaves = numOctaves;
    return true;
  }

  bool FETurbulence::SetStitchTiles(bool stitch) noexcept
  {
    if (_stitchTiles == stitch)
    {
      return false;
    }
    _stitchTiles = stitch;
    return true;
  }

  FloatRect FETurbulence::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                             const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts, TurbulenceType type) noexcept
    {
      switch (type)
      {
        case TurbulenceType::Unknown:      ts << u8"UNKNOWN"; break;
        case TurbulenceType::Turbulence:   ts << u8"TURBULENCE"; break;
        case TurbulenceType::FractalNoise: ts << u8"NOISE"; break;
      }
      return ts;
    }
  }

  TextStream &FETurbulence::ExternalRepresentation(TextStream &ts,
                                                   FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feTurbulence";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" type=\"" << GetType() << u8'"';
    ts << u8" baseFrequency=\"" << BaseFrequencyX() << u8", " << BaseFrequencyY() << u8'"';
    ts << u8" seed=\"" << Seed() << u8'"';
    ts << u8" numOctaves=\"" << NumOctaves() << u8'"';
    ts << u8" stitchTiles=\"" << StitchTiles() << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
