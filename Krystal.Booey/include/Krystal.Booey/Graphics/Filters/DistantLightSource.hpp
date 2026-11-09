/*
 * Copyright (C) 2008 Alex Mathews <possessedpenguinbob@gmail.com>
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A light infinitely far away, in a direction.
  class DistantLightSource : public LightSource
  {
  private:
    float _azimuth;
    float _elevation;

  public:
    KRYS_NODISCARD static Ref<DistantLightSource> Create(float azimuth, float elevation) noexcept;

    KRYS_NODISCARD bool operator==(const DistantLightSource &) const noexcept;

    // These are in degrees.
    KRYS_NODISCARD float Azimuth() const noexcept
    {
      return _azimuth;
    }
    KRYS_NODISCARD float Elevation() const noexcept
    {
      return _elevation;
    }

    bool SetAzimuth(float) noexcept override;
    bool SetElevation(float) noexcept override;

    void InitPaintingData(const Filter &, const FilterImage &result, PaintingData &) const noexcept override;
    KRYS_NODISCARD ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x, int32 y,
                                                                 float z) const noexcept final;

    TextStream &ExternalRepresentation(TextStream &) const noexcept override;

  private:
    DistantLightSource(float azimuth, float elevation) noexcept;

    KRYS_NODISCARD bool operator==(const LightSource &other) const noexcept override
    {
      return AreEqual<DistantLightSource>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(DistantLightSource, LightType::LS_DISTANT)
