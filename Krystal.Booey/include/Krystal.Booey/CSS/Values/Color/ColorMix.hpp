/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ColorMix
  {
    struct Component
    {
      using PercentageType = css::Percentage<Range {0, 100}>;

      css::Color Color;
      Maybe<PercentageType> Percentage;

      KRYS_NODISCARD bool operator==(const Component &) const noexcept = default;
    };

    krys::ColourInterpolationMethod ColourInterpolationMethod;
    Component MixComponents1;
    Component MixComponents2;

    KRYS_NODISCARD bool operator==(const ColorMix &) const noexcept = default;
  };

  constexpr auto DefaultInterpolationMethodForColorMix =
    ColourInterpolationMethod {ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied};

  KRYS_NODISCARD boo::Color CreateColor(const ColorMix &unresolved,
                                        PlatformColorResolutionState &state) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const ColorMix &unresolved) noexcept;
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const ColorMix &unresolved) noexcept;

  template <>
  struct Serialiser<ColorMix>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ColorMix &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<ColorMix>
  {
    void operator()(ComputedStyleDependencies &dependencies, const ColorMix &value) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<ColorMix>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const ColorMix &value) noexcept;
  };
}
