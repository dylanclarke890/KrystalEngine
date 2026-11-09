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
  // <'text-emphasis-position'> = [ over | under ] && [ right | left ]?@(default=right)
  // https://drafts.csswg.org/css-text-decor-4/#propdef-text-emphasis-position

  enum class TextEmphasisPositionValue : uint8
  {
    Over,
    Under,
    Left,
    Right,
  };

  using TextEmphasisPositionValueEnumSet = css::SpaceSeparatedEnumSet<TextEmphasisPositionValue>;

  // FIXME: This could be packed into 2 bits if we didn't use an EnumSet.
  struct TextEmphasisPosition
  {
    using EnumSet = TextEmphasisPositionValueEnumSet;
    using value_type = TextEmphasisPositionValueEnumSet::value_type;

    constexpr TextEmphasisPosition(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr TextEmphasisPosition(value_type value) noexcept : TextEmphasisPosition {EnumSet {value}}
    {
    }
    constexpr TextEmphasisPosition(std::initializer_list<value_type> initializerList) noexcept
        : TextEmphasisPosition {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static TextEmphasisPosition FromRaw(EnumSet::StorageType rawValue) noexcept
    {
      return EnumSet::FromRaw(rawValue);
    }
    KRYS_NODISCARD constexpr EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(TextEmphasisPositionValue e) const noexcept
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

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (Contains(TextEmphasisPositionValue::Over))
      {
        if (Contains(TextEmphasisPositionValue::Left))
        {
          return visitor(css::SpaceSeparatedTuple {css::keywords::Over {}, css::keywords::Left {}});
        }
        return visitor(css::SpaceSeparatedTuple {css::keywords::Over {}});
      }
      else
      {
        if (Contains(TextEmphasisPositionValue::Left))
        {
          return visitor(css::SpaceSeparatedTuple {css::keywords::Under {}, css::keywords::Left {}});
        }
        return visitor(css::SpaceSeparatedTuple {css::keywords::Under {}});
      }
    }

    KRYS_NODISCARD constexpr bool operator==(const TextEmphasisPosition &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextEmphasisPosition>
  {
    KRYS_NODISCARD TextEmphasisPosition operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextEmphasisPosition)
