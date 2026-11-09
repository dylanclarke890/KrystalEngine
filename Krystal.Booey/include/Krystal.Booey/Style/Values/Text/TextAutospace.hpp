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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Booey/Text/TextSpacing.hpp"

namespace krys::boo::style
{
  // <'text-autospace'> = normal | auto | no-autospace | [ ideograph-alpha || ideograph-numeric ]
  // FIXME: Current spec is `normal | auto | no-autospace | [ ideograph-alpha || ideograph-numeric ||
  // punctuation ] || [ insert | replace ]` https://drafts.csswg.org/css-text-4/#propdef-text-autospace
  struct TextAutospace
  {
    constexpr TextAutospace(css::keywords::Normal) noexcept : _value {boo::TextAutospace::Type::Normal}
    {
    }
    constexpr TextAutospace(css::keywords::Auto) noexcept : _value {boo::TextAutospace::Type::Auto}
    {
    }
    constexpr TextAutospace(css::keywords::NoAutospace) noexcept : _value {}
    {
    }
    constexpr TextAutospace(css::keywords::IdeographAlpha) noexcept
        : _value {boo::TextAutospace::Type::IdeographAlpha}
    {
    }
    constexpr TextAutospace(css::keywords::IdeographNumeric) noexcept
        : _value {boo::TextAutospace::Type::IdeographNumeric}
    {
    }
    constexpr TextAutospace(css::keywords::IdeographAlpha, css::keywords::IdeographNumeric) noexcept
        : _value {{boo::TextAutospace::Type::IdeographAlpha, boo::TextAutospace::Type::IdeographNumeric}}
    {
    }

    constexpr TextAutospace(boo::TextAutospace value) noexcept : _value {value}
    {
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _value.IsNormal();
    }
    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return _value.IsAuto();
    }
    KRYS_NODISCARD constexpr bool IsNoAutospace() const noexcept
    {
      return _value.IsNoAutospace();
    }
    KRYS_NODISCARD constexpr bool HasIdeographAlpha() const noexcept
    {
      return _value.HasIdeographAlpha();
    }
    KRYS_NODISCARD constexpr bool HasIdeographNumeric() const noexcept
    {
      return _value.HasIdeographNumeric();
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNormal())
      {
        return visitor(css::keywords::Normal {});
      }
      if (IsAuto())
      {
        return visitor(css::keywords::Auto {});
      }
      if (IsNoAutospace())
      {
        return visitor(css::keywords::NoAutospace {});
      }
      if (HasIdeographAlpha())
      {
        if (HasIdeographNumeric())
        {
          return visitor(
            css::SpaceSeparatedTuple {css::keywords::IdeographAlpha {}, css::keywords::IdeographNumeric {}});
        }
        return visitor(css::keywords::IdeographAlpha {});
      }
      if (HasIdeographNumeric())
      {
        return visitor(css::keywords::IdeographNumeric {});
      }

      krys_unreachable();
    }

    KRYS_NODISCARD bool ShouldApplySpacing(text_spacing::CharacterClass firstCharacterClass,
                                           text_spacing::CharacterClass secondCharacterClass) const noexcept
    {
      return _value.ShouldApplySpacing(firstCharacterClass, secondCharacterClass);
    }

    KRYS_NODISCARD bool ShouldApplySpacing(char32_t firstCharacter, char32_t secondCharacter) const noexcept
    {
      return _value.ShouldApplySpacing(firstCharacter, secondCharacter);
    }

    KRYS_NODISCARD constexpr bool operator==(const TextAutospace &) const noexcept = default;

  private:
    friend struct ToPlatform<TextAutospace>;

    boo::TextAutospace _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextAutospace>
  {
    KRYS_NODISCARD TextAutospace operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<TextAutospace>
  {
    KRYS_NODISCARD constexpr boo::TextAutospace operator()(const TextAutospace &value) noexcept
    {
      return value._value;
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextAutospace);
