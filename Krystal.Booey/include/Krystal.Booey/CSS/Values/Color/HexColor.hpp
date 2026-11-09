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

#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct HexColor
  {
    SRGBA<uint8> Value;

    KRYS_NODISCARD constexpr bool operator==(const HexColor &) const noexcept = default;
  };

  KRYS_NODISCARD inline boo::Color CreateColor(const HexColor &unresolved,
                                               PlatformColorResolutionState &) noexcept
  {
    return boo::Color {unresolved.Value};
  }

  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const HexColor &) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr bool ContainsColorSchemeDependentColor(const HexColor &) noexcept
  {
    return false;
  }

  template <>
  struct Serialiser<HexColor>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const HexColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<HexColor>
  {
    constexpr void operator()(ComputedStyleDependencies &, const HexColor &) noexcept
    {
    }
  };

  template <>
  struct ValueChildrenVisitor<HexColor>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const HexColor &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}
