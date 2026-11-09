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
  // <'contain'> = none | strict@(aliased-to=[size layout paint style]) | content@(aliased-to=[layout paint
  // style]) | [ [size | inline-size] || layout || style || paint ]
  // https://drafts.csswg.org/css-contain-2/#contain-property

  enum class ContainValue : uint8
  {
    Size,
    InlineSize,
    Layout,
    Style,
    Paint,
  };

  using ContainValueEnumSet = css::SpaceSeparatedEnumSet<ContainValue>;

  struct Contain
  {
    using EnumSet = ContainValueEnumSet;
    using value_type = ContainValueEnumSet::value_type;

    static constexpr ContainValueEnumSet strict {ContainValue::Size, ContainValue::Layout,
                                                 ContainValue::Style, ContainValue::Paint};
    static constexpr ContainValueEnumSet content {ContainValue::Layout, ContainValue::Style,
                                                  ContainValue::Paint};

    constexpr Contain(css::keywords::None) noexcept : _value {}
    {
    }
    constexpr Contain(css::keywords::Strict) noexcept : _value {strict}
    {
    }
    constexpr Contain(css::keywords::Content) noexcept : _value {content}
    {
    }
    constexpr Contain(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr Contain(value_type value) noexcept : Contain {EnumSet {value}}
    {
    }
    constexpr Contain(std::initializer_list<value_type> initializerList) noexcept
        : Contain {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static Contain FromRaw(EnumSet::StorageType rawValue) noexcept
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

    KRYS_NODISCARD constexpr bool Contains(value_type e) const noexcept
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
    constexpr void Add(EnumSet other) noexcept
    {
      _value.Value.Add(other.Value);
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

      // Handle "strict" and "content" shorthands
      if (_value == strict)
      {
        return visitor(css::keywords::Strict {});
      }
      if (_value == content)
      {
        return visitor(css::keywords::Content {});
      }

      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const Contain &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Contain>
  {
    KRYS_NODISCARD Contain operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Contain)
