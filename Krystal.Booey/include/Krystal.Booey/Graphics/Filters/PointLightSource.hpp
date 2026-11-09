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
  /// @brief A light at a point, shining in every direction.
  class PointLightSource : public LightSource
  {
  private:
    FloatPoint3D _position;
    mutable FloatPoint3D _bufferPosition;

  public:
    KRYS_NODISCARD static Ref<PointLightSource> Create(const FloatPoint3D &position) noexcept;

    KRYS_NODISCARD bool operator==(const PointLightSource &) const noexcept;

    KRYS_NODISCARD const FloatPoint3D &Position() const noexcept
    {
      return _position;
    }
    bool SetX(float) noexcept override;
    bool SetY(float) noexcept override;
    bool SetZ(float) noexcept override;

    void InitPaintingData(const Filter &, const FilterImage &result, PaintingData &) const noexcept override;
    KRYS_NODISCARD ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x, int32 y,
                                                                 float z) const noexcept final;

    TextStream &ExternalRepresentation(TextStream &) const noexcept override;

  private:
    explicit PointLightSource(const FloatPoint3D &position) noexcept;

    KRYS_NODISCARD bool operator==(const LightSource &other) const noexcept override
    {
      return AreEqual<PointLightSource>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(PointLightSource, LightType::LS_POINT)
