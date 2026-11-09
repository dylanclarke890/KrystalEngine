/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToTypedColor.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::css
{
  template <typename D>
  struct AbsoluteColorResolver
  {
    using Descriptor = D;

    CSSColorParseTypeWithCalc<Descriptor> Components;
    uint32 NestingLevel;
  };

  template <typename D>
  KRYS_NODISCARD boo::Color Resolve(const AbsoluteColorResolver<D> &absolute,
                                    const ToLengthConversionData &conversionData) noexcept
  {
    // Evaluated any calc values to their corresponding channel value.
    auto components =
      StyleColorParseType<D> {style::ConvertToStyle(std::get<0>(absolute.Components), conversionData),
                              style::ConvertToStyle(std::get<1>(absolute.Components), conversionData),
                              style::ConvertToStyle(std::get<2>(absolute.Components), conversionData),
                              style::ConvertToStyle(std::get<3>(absolute.Components), conversionData)};

    // Normalize values into their numeric form, forming a validated typed color.
    auto typedColor = ConvertToTypedColor<D>(components, 1.0);

    // Convert the validated typed color into a `boo::Color`,
    return ConvertToColor<D, ColorFunctionForm::Absolute>(typedColor, absolute.NestingLevel);
  }

  // This resolve function should only be called if the components have been checked and don't require
  // conversion data to be resolved.
  template <typename D>
  KRYS_NODISCARD boo::Color ResolveNoConversionDataRequired(const AbsoluteColorResolver<D> &absolute) noexcept
  {
    krys_debug_assert(!ComponentsRequireConversionData<D>(absolute.Components));

    // Evaluated any calc values to their corresponding channel value.
    auto components = StyleColorParseType<D> {
      style::ConvertToStyleNoConversionDataRequired(std::get<0>(absolute.Components)),
      style::ConvertToStyleNoConversionDataRequired(std::get<1>(absolute.Components)),
      style::ConvertToStyleNoConversionDataRequired(std::get<2>(absolute.Components)),
      style::ConvertToStyleNoConversionDataRequired(std::get<3>(absolute.Components))};

    // Normalize values into their numeric form, forming a validated typed color.
    auto typedColor = ConvertToTypedColor<D>(components, 1.0);

    // Convert the validated typed color into a `boo::Color`,
    return ConvertToColor<D, ColorFunctionForm::Absolute>(typedColor, absolute.NestingLevel);
  }
}
