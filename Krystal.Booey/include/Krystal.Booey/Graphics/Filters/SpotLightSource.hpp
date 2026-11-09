/*
 * Copyright (C) 2008 Alex Mathews <possessedpenguinbob@gmail.com>
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

#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A light at a point, shining at another, within a cone.
  class SpotLightSource : public LightSource
  {
  private:
    FloatPoint3D _position;
    FloatPoint3D _pointsAt;
    mutable FloatPoint3D _bufferPosition;
    float _specularExponent;
    float _limitingConeAngle;

  public:
    KRYS_NODISCARD static Ref<SpotLightSource> Create(const FloatPoint3D &position,
                                                      const FloatPoint3D &pointsAt, float specularExponent,
                                                      float limitingConeAngle) noexcept;

    KRYS_NODISCARD bool operator==(const SpotLightSource &) const noexcept;

    KRYS_NODISCARD const FloatPoint3D &Position() const noexcept
    {
      return _position;
    }
    KRYS_NODISCARD const FloatPoint3D &Direction() const noexcept
    {
      return _pointsAt;
    }
    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent;
    }
    KRYS_NODISCARD float LimitingConeAngle() const noexcept
    {
      return _limitingConeAngle;
    }

    bool SetX(float) noexcept override;
    bool SetY(float) noexcept override;
    bool SetZ(float) noexcept override;
    bool SetPointsAtX(float) noexcept override;
    bool SetPointsAtY(float) noexcept override;
    bool SetPointsAtZ(float) noexcept override;

    bool SetSpecularExponent(float) noexcept override;
    bool SetLimitingConeAngle(float) noexcept override;

    void InitPaintingData(const Filter &, const FilterImage &result, PaintingData &) const noexcept override;
    KRYS_NODISCARD ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x, int32 y,
                                                                 float z) const noexcept final;

    TextStream &ExternalRepresentation(TextStream &) const noexcept override;

  private:
    SpotLightSource(const FloatPoint3D &position, const FloatPoint3D &direction, float specularExponent,
                    float limitingConeAngle) noexcept;

    KRYS_NODISCARD bool operator==(const LightSource &other) const noexcept override
    {
      return AreEqual<SpotLightSource>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(SpotLightSource, LightType::LS_SPOT)
