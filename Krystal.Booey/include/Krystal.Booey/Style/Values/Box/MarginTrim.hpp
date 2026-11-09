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
  // <'margin-trim'> = none | [ block || inline ] | [ block-start || inline-start || block-end || inline-end ]
  // https://drafts.csswg.org/css-box/#margin-trim

  enum class MarginTrimSide : uint8
  {
    BlockStart,
    InlineStart,
    BlockEnd,
    InlineEnd,
  };

  using MarginTrimSideEnumSet = css::SpaceSeparatedEnumSet<MarginTrimSide>;

  struct MarginTrim
  {
    using EnumSet = MarginTrimSideEnumSet;
    using value_type = MarginTrimSideEnumSet::value_type;

    constexpr MarginTrim(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr MarginTrim(css::keywords::None) noexcept : _value {}
    {
    }
    constexpr MarginTrim(value_type value) noexcept : MarginTrim {EnumSet {value}}
    {
    }
    constexpr MarginTrim(std::initializer_list<value_type> initializerList) noexcept
        : MarginTrim {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static MarginTrim FromRaw(EnumSet::StorageType rawValue) noexcept
    {
      if (!rawValue)
      {
        return css::keywords::None {};
      }
      return EnumSet::FromRaw(rawValue);
    }
    KRYS_NODISCARD constexpr EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(MarginTrimSide e) const noexcept
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

    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return _value.IsEmpty();
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }

      // Handle "block", "inline" and "block inline" shorthands
      if (_value.ContainsAll({MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd})
          && !_value.ContainsAny({MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd}))
      {
        return visitor(css::keywords::Block {});
      }
      if (_value.ContainsAll({MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd})
          && !_value.ContainsAny({MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd}))
      {
        return visitor(css::keywords::Inline {});
      }
      if (_value.ContainsAll({MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd,
                              MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd}))
      {
        return visitor(css::SpaceSeparatedTuple {css::keywords::Block {}, css::keywords::Inline {}});
      }

      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const MarginTrim &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MarginTrim>
  {
    KRYS_NODISCARD MarginTrim operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MarginTrim)
