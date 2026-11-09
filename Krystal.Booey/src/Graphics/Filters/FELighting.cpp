/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Zoltan Herczeg
 * Copyright (C) 2018-2022 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Filters/FELighting.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"
#include <algorithm>

namespace krys::boo
{
  FELighting::FELighting(Type type, const Color &lightingColor, float surfaceScale, float diffuseConstant,
                         float specularConstant, float specularExponent, float kernelUnitLengthX,
                         float kernelUnitLengthY, Ref<LightSource> &&lightSource,
                         const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(type, colorSpace), _lightingColor(lightingColor), _surfaceScale(surfaceScale),
        _diffuseConstant(std::max(diffuseConstant, 0.0f)),
        _specularConstant(std::max(specularConstant, 0.0f)),
        _specularExponent(std::clamp(specularExponent, 1.0f, 128.0f)), _kernelUnitLengthX(kernelUnitLengthX),
        _kernelUnitLengthY(kernelUnitLengthY), _lightSource(krys::move(lightSource))
  {
  }

  bool FELighting::operator==(const FELighting &other) const noexcept
  {
    return FilterEffect::operator==(other) && _lightingColor == other._lightingColor
           && _surfaceScale == other._surfaceScale && _diffuseConstant == other._diffuseConstant
           && _specularConstant == other._specularConstant && _specularExponent == other._specularExponent
           && _kernelUnitLengthX == other._kernelUnitLengthX && _kernelUnitLengthY == other._kernelUnitLengthY
           && ArePointingToEqualData(_lightSource, other._lightSource);
  }

  bool FELighting::SetSurfaceScale(float surfaceScale) noexcept
  {
    if (_surfaceScale == surfaceScale)
    {
      return false;
    }
    _surfaceScale = surfaceScale;
    return true;
  }

  bool FELighting::SetLightingColor(const Color &lightingColor) noexcept
  {
    if (_lightingColor == lightingColor)
    {
      return false;
    }
    _lightingColor = lightingColor;
    return true;
  }

  bool FELighting::SetKernelUnitLengthX(float kernelUnitLengthX) noexcept
  {
    if (_kernelUnitLengthX == kernelUnitLengthX)
    {
      return false;
    }
    _kernelUnitLengthX = kernelUnitLengthX;
    return true;
  }

  bool FELighting::SetKernelUnitLengthY(float kernelUnitLengthY) noexcept
  {
    if (_kernelUnitLengthY == kernelUnitLengthY)
    {
      return false;
    }
    _kernelUnitLengthY = kernelUnitLengthY;
    return true;
  }

  FloatRect FELighting::CalculateImageRect(const Filter &filter, Span<const FloatRect>,
                                           const FloatRect &primitiveSubregion) const noexcept
  {
    return filter.MaxEffectRect(primitiveSubregion);
  }
}
