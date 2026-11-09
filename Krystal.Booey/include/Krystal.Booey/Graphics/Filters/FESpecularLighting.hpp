/*
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

#include "Krystal.Booey/Graphics/Filters/FELighting.hpp"

namespace krys::boo
{
  /// @brief The specular lighting filter effect: a shiny surface lit by its light source.
  class FESpecularLighting final : public FELighting
  {
  public:
    KRYS_NODISCARD static Ref<FESpecularLighting>
      Create(const Color &lightingColor, float surfaceScale, float specularConstant, float specularExponent,
             float kernelUnitLengthX, float kernelUnitLengthY, Ref<LightSource> &&lightSource,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FESpecularLighting &other) const noexcept
    {
      return FELighting::operator==(other);
    }

    KRYS_NODISCARD float SpecularConstant() const noexcept
    {
      return _specularConstant;
    }
    bool SetSpecularConstant(float specularConstant) noexcept;

    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent;
    }
    bool SetSpecularExponent(float specularExponent) noexcept;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;

  private:
    FESpecularLighting(const Color &lightingColor, float surfaceScale, float specularConstant,
                       float specularExponent, float kernelUnitLengthX, float kernelUnitLengthY,
                       Ref<LightSource> &&lightSource, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FESpecularLighting>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FESpecularLighting)
