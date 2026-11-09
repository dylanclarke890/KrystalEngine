/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Zoltan Herczeg
 * Copyright (C) 2021-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A lighting filter effect: its input's alpha lit as a surface, by a light source.
  class FELighting : public FilterEffect
  {
  protected:
    Color _lightingColor;
    float _surfaceScale;
    float _diffuseConstant;
    float _specularConstant;
    float _specularExponent;
    float _kernelUnitLengthX;
    float _kernelUnitLengthY;
    const Ref<LightSource> _lightSource;

  public:
    KRYS_NODISCARD bool operator==(const FELighting &other) const noexcept;

    KRYS_NODISCARD const Color &LightingColor() const noexcept
    {
      return _lightingColor;
    }
    bool SetLightingColor(const Color &lightingColor) noexcept;

    KRYS_NODISCARD float SurfaceScale() const noexcept
    {
      return _surfaceScale;
    }
    bool SetSurfaceScale(float surfaceScale) noexcept;

    KRYS_NODISCARD float DiffuseConstant() const noexcept
    {
      return _diffuseConstant;
    }
    KRYS_NODISCARD float SpecularConstant() const noexcept
    {
      return _specularConstant;
    }
    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent;
    }

    KRYS_NODISCARD float KernelUnitLengthX() const noexcept
    {
      return _kernelUnitLengthX;
    }
    bool SetKernelUnitLengthX(float kernelUnitLengthX) noexcept;

    KRYS_NODISCARD float KernelUnitLengthY() const noexcept
    {
      return _kernelUnitLengthY;
    }
    bool SetKernelUnitLengthY(float kernelUnitLengthY) noexcept;

    KRYS_NODISCARD LightSource &GetLightSource() const noexcept
    {
      return *_lightSource;
    }

  protected:
    FELighting(Type type, const Color &lightingColor, float surfaceScale, float diffuseConstant,
               float specularConstant, float specularExponent, float kernelUnitLengthX,
               float kernelUnitLengthY, Ref<LightSource> &&lightSource,
               const DestinationColorSpace &colorSpace) noexcept;

    using FilterEffect::operator==;

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;
  };
}
