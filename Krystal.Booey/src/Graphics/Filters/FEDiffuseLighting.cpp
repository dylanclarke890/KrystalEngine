/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FEDiffuseLighting.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<FEDiffuseLighting> FEDiffuseLighting::Create(const Color &lightingColor, float surfaceScale,
                                                   float diffuseConstant, float kernelUnitLengthX,
                                                   float kernelUnitLengthY, Ref<LightSource> &&lightSource,
                                                   const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEDiffuseLighting(lightingColor, surfaceScale, diffuseConstant, kernelUnitLengthX,
                                           kernelUnitLengthY, krys::move(lightSource), colorSpace));
  }

  FEDiffuseLighting::FEDiffuseLighting(const Color &lightingColor, float surfaceScale, float diffuseConstant,
                                       float kernelUnitLengthX, float kernelUnitLengthY,
                                       Ref<LightSource> &&lightSource,
                                       const DestinationColorSpace &colorSpace) noexcept
      : FELighting(FilterEffect::Type::FEDiffuseLighting, lightingColor, surfaceScale, diffuseConstant, 0, 0,
                   kernelUnitLengthX, kernelUnitLengthY, krys::move(lightSource), colorSpace)
  {
  }

  bool FEDiffuseLighting::SetDiffuseConstant(float diffuseConstant) noexcept
  {
    diffuseConstant = std::max(diffuseConstant, 0.0f);
    if (_diffuseConstant == diffuseConstant)
    {
      return false;
    }
    _diffuseConstant = diffuseConstant;
    return true;
  }

  TextStream &FEDiffuseLighting::ExternalRepresentation(TextStream &ts,
                                                        FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feDiffuseLighting";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" surfaceScale=\"" << _surfaceScale << u8'"';
    ts << u8" diffuseConstant=\"" << _diffuseConstant << u8'"';
    ts << u8" kernelUnitLength=\"" << _kernelUnitLengthX << u8", " << _kernelUnitLengthY << u8'"';

    ts << u8"]\n";
    return ts;
  }
}
