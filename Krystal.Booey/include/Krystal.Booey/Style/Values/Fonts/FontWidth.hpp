/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Graphics/Fonts/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-width'> = normal | <percentage [0,∞]> | ultra-condensed | extra-condensed | condensed |
  //   semi-condensed | semi-expanded | expanded | extra-expanded | ultra-expanded
  // NOTE: Computed value is always resolved to a <percentage [0,∞]>.
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-width
  struct FontWidth
  {
    using Percentage = style::Percentage<css::NonNegative>;

    FontWidth(css::keywords::Normal) noexcept : _platform {NormalWidthValue()}
    {
    }
    FontWidth(css::keywords::UltraCondensed) noexcept : _platform {UltraCondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::ExtraCondensed) noexcept : _platform {ExtraCondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::Condensed) noexcept : _platform {CondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::SemiCondensed) noexcept : _platform {SemiCondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::SemiExpanded) noexcept : _platform {SemiExpandedWidthValue()}
    {
    }
    FontWidth(css::keywords::Expanded) noexcept : _platform {ExpandedWidthValue()}
    {
    }
    FontWidth(css::keywords::ExtraExpanded) noexcept : _platform {ExtraExpandedWidthValue()}
    {
    }
    FontWidth(css::keywords::UltraExpanded) noexcept : _platform {UltraExpandedWidthValue()}
    {
    }
    FontWidth(Percentage percentage) noexcept
        : _platform {FontSelectionValue::ClampFloat(static_cast<float>(percentage.Value))}
    {
    }

    FontWidth(FontSelectionValue platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD Percentage GetPercentage() const noexcept
    {
      return Percentage {static_cast<float>(_platform)};
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      return visitor(GetPercentage());
    }

    KRYS_NODISCARD FontSelectionValue Platform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD bool operator==(const FontWidth &) const noexcept = default;

  private:
    FontSelectionValue _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontWidth>
  {
    KRYS_NODISCARD FontWidth operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<FontWidth>
  {
    KRYS_NODISCARD FontWidth Blend(const FontWidth &, const FontWidth &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontWidth)
