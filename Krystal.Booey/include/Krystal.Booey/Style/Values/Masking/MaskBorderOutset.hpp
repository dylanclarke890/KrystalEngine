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

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <mask-border-outset-value> = <length [0,∞]> | <number [0,∞]>
  struct MaskBorderOutsetValue
  {
    using Length = style::Length<css::NonNegative, float>;
    using Number = style::Number<css::NonNegative, float>;

    constexpr MaskBorderOutsetValue(Length length) noexcept : _value {length}
    {
    }
    constexpr MaskBorderOutsetValue(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _value {Length {literal}}
    {
    }

    constexpr MaskBorderOutsetValue(Number number) noexcept : _value {number}
    {
    }
    constexpr MaskBorderOutsetValue(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : _value {Number {literal}}
    {
    }

    KRYS_NODISCARD constexpr bool IsLength() const noexcept
    {
      return krys::HoldsAlternative<Length>(_value);
    }
    KRYS_NODISCARD constexpr bool IsNumber() const noexcept
    {
      return krys::HoldsAlternative<Number>(_value);
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return SwitchOn([&](auto &value) { return value == 0; });
    }

    KRYS_NODISCARD constexpr bool operator==(const MaskBorderOutsetValue &) const noexcept = default;

    KRYS_NODISCARD constexpr bool HasSameType(const MaskBorderOutsetValue &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    friend struct Blending<MaskBorderOutsetValue>;

    Variant<Length, Number> _value {Number {0}};
  };

  // <'mask-border-outset'> = [ <length [0,∞]> | <number [0,∞]> ]{1,4}
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-outset
  struct MaskBorderOutset
  {
    using Value = MaskBorderOutsetValue;
    using Edges = css::MinimallySerialisingSpaceSeparatedRectEdges<Value>;

    Edges Values {Value {Value::Number {0}}};

    MaskBorderOutset(Edges values) noexcept : Values {krys::move(values)}
    {
    }
    MaskBorderOutset(Value top, Value right, Value bottom, Value left) noexcept
        : Values {top, right, bottom, left}
    {
    }
    MaskBorderOutset(Value value) noexcept : Values {krys::move(value)}
    {
    }
    MaskBorderOutset(Value::Length length) noexcept : Values {length}
    {
    }
    MaskBorderOutset(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : Values {Value::Length {literal}}
    {
    }
    MaskBorderOutset(Value::Number number) noexcept : Values {number}
    {
    }
    MaskBorderOutset(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : Values {Value::Number {literal}}
    {
    }

    KRYS_NODISCARD bool IsZero() const noexcept
    {
      return Values.AllOf([](auto &edge) { return edge.IsZero(); });
    }

    KRYS_NODISCARD bool operator==(const MaskBorderOutset &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(MaskBorderOutset, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MaskBorderOutset>
  {
    KRYS_NODISCARD MaskBorderOutset operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<MaskBorderOutset>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const MaskBorderOutset &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<MaskBorderOutsetValue>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderOutsetValue &, const MaskBorderOutsetValue &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderOutsetValue &,
                                                                      const MaskBorderOutsetValue &) noexcept;
    KRYS_NODISCARD MaskBorderOutsetValue Blend(const MaskBorderOutsetValue &, const MaskBorderOutsetValue &,
                                               const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<MaskBorderOutset>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderOutset &, const MaskBorderOutset &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const MaskBorderOutset &,
                                                                      const MaskBorderOutset &) noexcept;
    KRYS_NODISCARD MaskBorderOutset Blend(const MaskBorderOutset &, const MaskBorderOutset &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::MaskBorderOutset)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaskBorderOutsetValue)
