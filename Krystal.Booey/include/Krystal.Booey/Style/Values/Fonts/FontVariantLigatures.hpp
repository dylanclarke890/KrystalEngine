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
  // <'font-variant-ligatures'> = normal | none | [ <common-lig-values> || <discretionary-lig-values> ||
  // <historical-lig-values> || <contextual-alt-values> ]
  // https://drafts.csswg.org/css-fonts-4/#font-variant-ligatures-prop
  struct FontVariantLigatures
  {
    using Platform = boo::FontVariantLigaturesValues;

    constexpr FontVariantLigatures(css::keywords::Normal) noexcept : _platform {}
    {
    }

    constexpr FontVariantLigatures(css::keywords::None) noexcept
        : _platform {boo::FontVariantLigatures::No, boo::FontVariantLigatures::No,
                     boo::FontVariantLigatures::No, boo::FontVariantLigatures::No}
    {
    }

    constexpr FontVariantLigatures(Platform value) noexcept : _platform {value}
    {
    }

    KRYS_NODISCARD constexpr Platform GetPlatform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      using enum boo::FontVariantLigatures;
      return _platform.Common == Normal && _platform.Discretionary == Normal && _platform.Historical == Normal
             && _platform.Contextual == Normal;
    }

    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      using enum boo::FontVariantLigatures;
      return _platform.Common == No && _platform.Discretionary == No && _platform.Historical == No
             && _platform.Contextual == No;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      if (IsNormal())
      {
        return visitor(css::keywords::Normal {});
      }

      auto tupleValue = []<typename YesType, typename NoType>(auto value) -> Maybe<Variant<YesType, NoType>>
      {
        using enum boo::FontVariantLigatures;
        switch (value)
        {
          case Normal: return std::nullopt;
          case No:     return NoType {};
          case Yes:    return YesType {};
        }
        krys_unreachable();
      };

      return visitor(css::SpaceSeparatedTuple {
        tupleValue.template operator()<css::keywords::CommonLigatures, css::keywords::NoCommonLigatures>(
          _platform.Common),
        tupleValue.template operator()<css::keywords::DiscretionaryLigatures,
                                       css::keywords::NoDiscretionaryLigatures>(_platform.Discretionary),
        tupleValue.template operator()<css::keywords::HistoricalLigatures,
                                       css::keywords::NoHistoricalLigatures>(_platform.Historical),
        tupleValue.template operator()<css::keywords::Contextual, css::keywords::NoContextual>(
          _platform.Contextual),
      });
    }

    KRYS_NODISCARD constexpr bool operator==(const FontVariantLigatures &) const noexcept = default;

  private:
    Platform _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariantLigatures>
  {
    KRYS_NODISCARD FontVariantLigatures operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontVariantLigatures)
