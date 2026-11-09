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

#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"

namespace krys::boo::style
{
  // <'font-variant-east-asian'> = normal | [ <east-asian-variant-values> || <east-asian-width-values> ||
  //   ruby ]
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-variant-east-asian
  struct FontVariantEastAsian
  {
    using Platform = boo::FontVariantEastAsianValues;

    constexpr FontVariantEastAsian(css::keywords::Normal) noexcept : _platform {}
    {
    }

    constexpr FontVariantEastAsian(Platform value) noexcept : _platform {value}
    {
    }

    KRYS_NODISCARD constexpr Platform GetPlatform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _platform.Variant == FontVariantEastAsianVariant::Normal
             && _platform.Width == FontVariantEastAsianWidth::Normal
             && _platform.Ruby == FontVariantEastAsianRuby::Normal;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNormal())
      {
        return visitor(css::keywords::Normal {});
      }

      auto variantValue = [](auto value)
        -> Maybe<Variant<css::keywords::Jis78, css::keywords::Jis83, css::keywords::Jis90,
                         css::keywords::Jis04, css::keywords::Simplified, css::keywords::Traditional>>
      {
        switch (value)
        {
          case FontVariantEastAsianVariant::Normal:      return std::nullopt;
          case FontVariantEastAsianVariant::Jis78:       return css::keywords::Jis78 {};
          case FontVariantEastAsianVariant::Jis83:       return css::keywords::Jis83 {};
          case FontVariantEastAsianVariant::Jis90:       return css::keywords::Jis90 {};
          case FontVariantEastAsianVariant::Jis04:       return css::keywords::Jis04 {};
          case FontVariantEastAsianVariant::Simplified:  return css::keywords::Simplified {};
          case FontVariantEastAsianVariant::Traditional: return css::keywords::Traditional {};
        }
        krys_unreachable();
      };
      auto widthValue =
        [](auto value) -> Maybe<Variant<css::keywords::FullWidth, css::keywords::ProportionalWidth>>
      {
        switch (value)
        {
          case FontVariantEastAsianWidth::Normal:       return std::nullopt;
          case FontVariantEastAsianWidth::Full:         return css::keywords::FullWidth {};
          case FontVariantEastAsianWidth::Proportional: return css::keywords::ProportionalWidth {};
        }
        krys_unreachable();
      };
      auto rubyValue = [](auto value) -> Maybe<css::keywords::Ruby>
      {
        switch (value)
        {
          case FontVariantEastAsianRuby::Normal: return std::nullopt;
          case FontVariantEastAsianRuby::Yes:    return css::keywords::Ruby {};
        }
        krys_unreachable();
      };

      return visitor(css::SpaceSeparatedTuple {
        variantValue(_platform.Variant),
        widthValue(_platform.Width),
        rubyValue(_platform.Ruby),
      });
    }

    KRYS_NODISCARD constexpr bool operator==(const FontVariantEastAsian &) const noexcept = default;

  private:
    Platform _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariantEastAsian>
  {
    KRYS_NODISCARD FontVariantEastAsian operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontVariantEastAsian)
