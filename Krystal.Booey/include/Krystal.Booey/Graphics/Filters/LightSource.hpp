/*
 * Copyright (C) 2008 Alex Mathews <possessedpenguinbob@gmail.com>
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Zoltan Herczeg <zherczeg@webkit.org>
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatPoint3D.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  enum class LightType : uint8
  {
    LS_DISTANT,
    LS_POINT,
    LS_SPOT
  };

  class Filter;
  class FilterImage;

  /// @brief The light of a lighting filter effect.
  class LightSource : public RefCounted<LightSource>
  {
  public:
    struct ComputedLightingData
    {
      FloatPoint3D LightVector;
      FloatPoint3D ColorVector;
      float LightVectorLength {0.f};
    };

    struct PaintingData
    {
      ComputedLightingData InitialLightingData;
      FloatPoint3D DirectionVector;
      float ConeCutOffLimit {0.f};
      float ConeFullLight {0.f};
    };

  private:
    LightType _type;

  public:
    explicit LightSource(LightType type) noexcept : _type(type)
    {
    }

    virtual ~LightSource() noexcept = default;

    KRYS_NODISCARD virtual bool operator==(const LightSource &other) const noexcept
    {
      return _type == other._type;
    }

    KRYS_NODISCARD LightType Type() const noexcept
    {
      return _type;
    }

    virtual TextStream &ExternalRepresentation(TextStream &) const noexcept = 0;

    virtual void InitPaintingData(const Filter &, const FilterImage &result,
                                  PaintingData &) const noexcept = 0;
    // z is a float number, since it is the alpha value scaled by a user
    // specified "surfaceScale" constant, which type is <number> in the SVG standard.
    // x and y are in the coordinates of the FilterEffect's buffer.
    KRYS_NODISCARD virtual ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x,
                                                                         int32 y, float z) const noexcept = 0;

    virtual bool SetAzimuth(float) noexcept
    {
      return false;
    }
    virtual bool SetElevation(float) noexcept
    {
      return false;
    }

    // These are in user space coordinates.
    virtual bool SetX(float) noexcept
    {
      return false;
    }
    virtual bool SetY(float) noexcept
    {
      return false;
    }
    virtual bool SetZ(float) noexcept
    {
      return false;
    }
    virtual bool SetPointsAtX(float) noexcept
    {
      return false;
    }
    virtual bool SetPointsAtY(float) noexcept
    {
      return false;
    }
    virtual bool SetPointsAtZ(float) noexcept
    {
      return false;
    }

    virtual bool SetSpecularExponent(float) noexcept
    {
      return false;
    }
    virtual bool SetLimitingConeAngle(float) noexcept
    {
      return false;
    }

  protected:
    template <typename TLightSource>
    KRYS_NODISCARD static bool AreEqual(const TLightSource &a, const LightSource &b) noexcept
    {
      auto *bType = DynamicDowncast<TLightSource>(b);
      return bType && a.operator==(*bType);
    }
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(ClassName, LightSourceType)                                  \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ClassName)                                               \
    KRYS_NODISCARD static bool IsType(const krys::boo::LightSource &source) noexcept                         \
    {                                                                                                        \
      return source.Type() == krys::boo::LightSourceType;                                                    \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
