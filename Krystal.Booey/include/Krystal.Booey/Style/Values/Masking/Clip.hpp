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
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <clip-edge> = <length> | auto
  struct ClipEdge : css::ValueOrKeyword<Length<>, css::keywords::Auto>
  {
    using Base::Base;
    using Length = typename Base::Value;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<Length> TryLength() const noexcept
    {
      return TryValue();
    }
  };

  // <rect()> = rect( <clip-edge> , <clip-edge> , <clip-edge> , <clip-edge> )
  struct ClipRect
  {
    css::FunctionNotation<css::ValueId::Rect, css::CommaSeparatedRectEdges<ClipEdge>> Value;

    ClipRect(css::keywords::Auto keyword) noexcept : Value {css::CommaSeparatedRectEdges<ClipEdge> {keyword}}
    {
    }

    template <typename T>
    ClipRect(T top, T right, T bottom, T left) noexcept
        : Value {css::CommaSeparatedRectEdges<ClipEdge> {krys::move(top), krys::move(right),
                                                         krys::move(bottom), krys::move(left)}}
    {
    }

    KRYS_NODISCARD bool IsAllAuto() const noexcept
    {
      return Value->AllOf([](auto &side) { return side.IsAuto(); });
    }
    KRYS_NODISCARD bool IsAnyAuto() const noexcept
    {
      return Value->AnyOf([](auto &side) { return side.IsAuto(); });
    }

    KRYS_NODISCARD bool operator==(const ClipRect &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(ClipRect, Value);

  // <'clip'> = <rect()> | auto
  // https://drafts.fxtf.org/css-masking/#propdef-clip
  struct Clip
  {
    Clip(css::keywords::Auto) noexcept : _value {}
    {
    }

    Clip(const ClipRect &rect) noexcept : _value {rect}
    {
    }

    Clip(ClipRect &&rect) noexcept : _value {krys::move(rect)}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return !_value;
    }
    KRYS_NODISCARD bool IsRect() const noexcept
    {
      return !IsAuto();
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsAuto() || _value->IsAllAuto())
      {
        return visitor(css::keywords::Auto {});
      }
      return visitor(*_value);
    }

    KRYS_NODISCARD bool operator==(const Clip &) const noexcept = default;

  private:
    friend struct Blending<Clip>;

    Maybe<ClipRect> _value;
  };

  // MARK: Conversion

  template <>
  struct CSSValueConversion<Clip>
  {
    KRYS_NODISCARD Clip operator()(BuilderState &, const css::Value &) noexcept;
  };

  // `ClipRect` is special-cased to return a `css::ValueId::Rect`.
  template <>
  struct CSSValueCreation<ClipRect>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const ClipRect &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Clip>
  {
    KRYS_NODISCARD bool CanBlend(const Clip &, const Clip &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const Clip &, const Clip &) noexcept;
    KRYS_NODISCARD Clip Blend(const Clip &, const Clip &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::ClipRect);
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ClipEdge);
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Clip);
