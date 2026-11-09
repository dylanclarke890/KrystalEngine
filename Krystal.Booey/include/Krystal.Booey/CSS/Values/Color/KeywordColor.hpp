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

#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct KeywordColor
  {
    ValueId Value;

    KRYS_NODISCARD constexpr bool operator==(const KeywordColor &) const noexcept = default;
  };

  KRYS_NODISCARD bool IsAbsoluteColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsCurrentColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsSystemColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsDeprecatedSystemColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsColorKeyword(ValueId, ColorType allowedColorTypes) noexcept;

  KRYS_NODISCARD boo::Color ColorFromAbsoluteKeyword(ValueId) noexcept;
  KRYS_NODISCARD boo::Color ColorFromKeyword(ValueId, style::ColorOptions) noexcept;

  KRYS_NODISCARD boo::Color CreateColor(const KeywordColor &, PlatformColorResolutionState &) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const KeywordColor &) noexcept;
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const KeywordColor &) noexcept;

  template <>
  struct Serialiser<KeywordColor>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const KeywordColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<KeywordColor>
  {
    constexpr void operator()(ComputedStyleDependencies &, const KeywordColor &) noexcept
    {
    }
  };

  template <>
  struct ValueChildrenVisitor<KeywordColor>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const KeywordColor &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}
