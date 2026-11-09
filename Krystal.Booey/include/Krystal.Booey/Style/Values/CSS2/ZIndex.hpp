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

#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"

namespace krys::boo
{
  class RenderStyle;
  class RenderStyleProperties;
  class StyleBoxData;
}

namespace krys::boo::style
{
  class ComputedStyleBase;
  class ComputedStyleProperties;

  // <'z-index'> = auto | <integer>
  // https://drafts.csswg.org/css2/#propdef-z-index
  struct ZIndex
  {
    using Value = Integer<>;

    constexpr ZIndex(css::keywords::Auto) noexcept
    {
    }

    constexpr ZIndex(Value value) noexcept : _isAuto {false}, _value {value}
    {
    }

    constexpr ZIndex(Value::ResolvedValueType value) noexcept : _isAuto {false}, _value {value}
    {
    }

    constexpr ZIndex(css::ValueLiteral<css::IntegerUnit::Integer> literal) noexcept
        : _isAuto {false}, _value {literal}
    {
    }

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return _isAuto;
    }
    KRYS_NODISCARD constexpr bool IsValue() const noexcept
    {
      return !_isAuto;
    }
    KRYS_NODISCARD constexpr Maybe<Value> TryValue() const noexcept
    {
      return IsValue() ? Maybe<Value> {_value} : null;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (_isAuto)
      {
        return visitor(css::keywords::Auto {});
      }
      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const ZIndex &) const noexcept = default;

  private:
    // NOTE: This type is stored represented using an explicit `bool` and `Value`, rather than a `Variant`,
    // to allow compact storage and efficient construction in `StyleBoxData`. It is not using a
    // `css::ValueOrKeyword` to preserve the entire `int` value range for `z-index`. If we determine its ok
    // for `z-index` to only have `MAX_INT - 1` values, we can switch this out for `css::ValueOrKeyword`
    // with a custom `MarkableTraits`.
    friend class ComputedStyleBase;
    friend class ComputedStyleProperties;
    friend class boo::RenderStyle;
    friend class boo::RenderStyleProperties;
    friend class boo::StyleBoxData;

    constexpr ZIndex(bool isAuto, Value value) noexcept : _isAuto {isAuto}, _value {value}
    {
    }

    bool _isAuto {true};
    Value _value {0};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ZIndex>
  {
    KRYS_NODISCARD ZIndex operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ZIndex>
  {
    KRYS_NODISCARD bool CanBlend(const ZIndex &, const ZIndex &) noexcept;
    KRYS_NODISCARD ZIndex Blend(const ZIndex &, const ZIndex &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ZIndex)
