/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2010 Renata Hodovan <reni@inf.u-szeged.hu>
 * Copyright (C) 2017-2023 Apple Inc. All rights reserved.
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
  enum class TurbulenceType : uint8
  {
    Unknown,
    FractalNoise,
    Turbulence
  };

  /// @brief The turbulence filter effect: Perlin noise.
  class FETurbulence final : public FilterEffect
  {
  private:
    TurbulenceType _type;
    float _baseFrequencyX;
    float _baseFrequencyY;
    int32 _numOctaves;
    float _seed;
    bool _stitchTiles;

  public:
    KRYS_NODISCARD static Ref<FETurbulence>
      Create(TurbulenceType type, float baseFrequencyX, float baseFrequencyY, int32 numOctaves, float seed,
             bool stitchTiles,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FETurbulence &other) const noexcept;

    KRYS_NODISCARD TurbulenceType GetType() const noexcept
    {
      return _type;
    }
    bool SetType(TurbulenceType type) noexcept;

    KRYS_NODISCARD float BaseFrequencyX() const noexcept
    {
      return _baseFrequencyX;
    }
    bool SetBaseFrequencyX(float baseFrequencyX) noexcept;

    KRYS_NODISCARD float BaseFrequencyY() const noexcept
    {
      return _baseFrequencyY;
    }
    bool SetBaseFrequencyY(float baseFrequencyY) noexcept;

    KRYS_NODISCARD float Seed() const noexcept
    {
      return _seed;
    }
    bool SetSeed(float seed) noexcept;

    KRYS_NODISCARD int32 NumOctaves() const noexcept
    {
      return _numOctaves;
    }
    bool SetNumOctaves(int32 numOctaves) noexcept;

    KRYS_NODISCARD bool StitchTiles() const noexcept
    {
      return _stitchTiles;
    }
    bool SetStitchTiles(bool stitch) noexcept;

  private:
    FETurbulence(TurbulenceType type, float baseFrequencyX, float baseFrequencyY, int32 numOctaves,
                 float seed, bool stitchTiles, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FETurbulence>(*this, other);
    }

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 0u;
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FETurbulence)
