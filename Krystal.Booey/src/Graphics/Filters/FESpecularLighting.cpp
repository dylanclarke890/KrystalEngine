/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/FESpecularLighting.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<FESpecularLighting> FESpecularLighting::Create(const Color &lightingColor, float surfaceScale,
                                                     float specularConstant, float specularExponent,
                                                     float kernelUnitLengthX, float kernelUnitLengthY,
                                                     Ref<LightSource> &&lightSource,
                                                     const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FESpecularLighting(lightingColor, surfaceScale, specularConstant, specularExponent,
                                            kernelUnitLengthX, kernelUnitLengthY, krys::move(lightSource),
                                            colorSpace));
  }

  FESpecularLighting::FESpecularLighting(const Color &lightingColor, float surfaceScale,
                                         float specularConstant, float specularExponent,
                                         float kernelUnitLengthX, float kernelUnitLengthY,
                                         Ref<LightSource> &&lightSource,
                                         const DestinationColorSpace &colorSpace) noexcept
      : FELighting(FilterEffect::Type::FESpecularLighting, lightingColor, surfaceScale, 0, specularConstant,
                   specularExponent, kernelUnitLengthX, kernelUnitLengthY, krys::move(lightSource),
                   colorSpace)
  {
  }

  bool FESpecularLighting::SetSpecularConstant(float specularConstant) noexcept
  {
    specularConstant = std::max(specularConstant, 0.0f);
    if (_specularConstant == specularConstant)
    {
      return false;
    }
    _specularConstant = specularConstant;
    return true;
  }

  bool FESpecularLighting::SetSpecularExponent(float specularExponent) noexcept
  {
    specularExponent = std::clamp(specularExponent, 1.0f, 128.0f);
    if (_specularExponent == specularExponent)
    {
      return false;
    }
    _specularExponent = specularExponent;
    return true;
  }

  TextStream &FESpecularLighting::ExternalRepresentation(TextStream &ts,
                                                         FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feSpecularLighting";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" surfaceScale=\"" << _surfaceScale << u8'"';
    ts << u8" specualConstant=\"" << _specularConstant << u8'"';
    ts << u8" specularExponent=\"" << _specularExponent << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
