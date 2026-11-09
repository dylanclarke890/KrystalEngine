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

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo
{
  class css::Value;
  class RenderStyle;

  namespace style
  {
    // NOTE: the keyword value "infinity" is represented as the standard double value
    // `std::numeric_limits<double>::infinity()`.
    using SuperellipseFunction = css::FunctionNotation<css::ValueId::Superellipse, Number<css::NonNegative>>;

    // https://drafts.csswg.org/css-borders-4/#typedef-corner-shape-value
    struct CornerShapeValue
    {
      SuperellipseFunction Superellipse;

      constexpr CornerShapeValue(css::keywords::Round) noexcept : Superellipse {2.0}
      {
      }
      constexpr CornerShapeValue(css::keywords::Scoop) noexcept : Superellipse {0.5}
      {
      }
      constexpr CornerShapeValue(css::keywords::Bevel) noexcept : Superellipse {1.0}
      {
      }
      constexpr CornerShapeValue(css::keywords::Notch) noexcept : Superellipse {0.0}
      {
      }
      constexpr CornerShapeValue(css::keywords::Straight) noexcept
          : Superellipse {std::numeric_limits<double>::infinity()}
      {
      }
      constexpr CornerShapeValue(css::keywords::Squircle) noexcept : Superellipse {4.0}
      {
      }
      constexpr CornerShapeValue(SuperellipseFunction value) noexcept : Superellipse {value}
      {
      }

      template <typename F>
      decltype(auto) SwitchOn(F &&functor) const noexcept
      {
        if (*this == CornerShapeValue(css::keywords::Round {}))
        {
          return functor(css::keywords::Round {});
        }
        if (*this == CornerShapeValue(css::keywords::Scoop {}))
        {
          return functor(css::keywords::Scoop {});
        }
        if (*this == CornerShapeValue(css::keywords::Bevel {}))
        {
          return functor(css::keywords::Bevel {});
        }
        if (*this == CornerShapeValue(css::keywords::Notch {}))
        {
          return functor(css::keywords::Notch {});
        }
        if (*this == CornerShapeValue(css::keywords::Straight {}))
        {
          return functor(css::keywords::Straight {});
        }
        if (*this == CornerShapeValue(css::keywords::Squircle {}))
        {
          return functor(css::keywords::Squircle {});
        }
        return functor(Superellipse);
      }

      KRYS_NODISCARD bool operator==(const CornerShapeValue &) const noexcept = default;
    };

    // https://drafts.csswg.org/css-borders-4/#propdef-corner-shape
    using CornerShape = css::MinimallySerialisingSpaceSeparatedRectCorners<CornerShapeValue>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<CornerShapeValue>
    {
      KRYS_NODISCARD CornerShapeValue operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<CornerShapeValue>
    {
      KRYS_NODISCARD CornerShapeValue Blend(const CornerShapeValue &, const CornerShapeValue &,
                                            const BlendingContext &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::CornerShapeValue)
