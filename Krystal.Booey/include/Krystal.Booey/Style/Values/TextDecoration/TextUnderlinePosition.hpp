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

namespace krys::boo::style
{
  // <'text-underline-position'> = auto | [ from-font | under ] || [ left | right ]
  // https://drafts.csswg.org/css-text-decor-4/#propdef-text-underline-position

  enum class TextUnderlinePositionValue : uint8
  {
    FromFont,
    Under,
    Left,
    Right,
  };

  using TextUnderlinePositionValueEnumSet = css::SpaceSeparatedEnumSet<TextUnderlinePositionValue>;

  struct TextUnderlinePosition
  {
    using EnumSet = TextUnderlinePositionValueEnumSet;
    using value_type = TextUnderlinePositionValueEnumSet::value_type;

    enum class Side : uint8
    {
      NoPreference,
      Left,
      Right
    };

    constexpr TextUnderlinePosition(css::keywords::Auto) noexcept : _value {}
    {
    }
    constexpr TextUnderlinePosition(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr TextUnderlinePosition(value_type value) noexcept : TextUnderlinePosition {EnumSet {value}}
    {
    }
    constexpr TextUnderlinePosition(std::initializer_list<value_type> initializerList) noexcept
        : TextUnderlinePosition {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static TextUnderlinePosition FromRaw(EnumSet::StorageType rawValue) noexcept
    {
      return EnumSet::FromRaw(rawValue);
    }
    KRYS_NODISCARD constexpr EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(TextUnderlinePositionValue e) const noexcept
    {
      return _value.Contains(e);
    }
    KRYS_NODISCARD constexpr bool ContainsAny(EnumSet other) const noexcept
    {
      return _value.ContainsAny(other.Value);
    }
    KRYS_NODISCARD constexpr bool ContainsAll(EnumSet other) const noexcept
    {
      return _value.ContainsAll(other.Value);
    }
    KRYS_NODISCARD constexpr bool ContainsOnly(EnumSet other) const noexcept
    {
      return _value.ContainsOnly(other.Value);
    }

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return _value.IsEmpty();
    }
    KRYS_NODISCARD constexpr bool IsFromFont() const noexcept
    {
      return Contains(TextUnderlinePositionValue::FromFont);
    }
    KRYS_NODISCARD constexpr bool IsUnder() const noexcept
    {
      return Contains(TextUnderlinePositionValue::Under);
    }

    KRYS_NODISCARD constexpr Side VerticalTypographySide() const noexcept
    {
      if (Contains(TextUnderlinePositionValue::Left))
      {
        return Side::Left;
      }
      if (Contains(TextUnderlinePositionValue::Right))
      {
        return Side::Right;
      }
      return Side::NoPreference;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsAuto())
      {
        return visitor(css::keywords::Auto {});
      }
      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const TextUnderlinePosition &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextUnderlinePosition>
  {
    KRYS_NODISCARD TextUnderlinePosition operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextUnderlinePosition)
