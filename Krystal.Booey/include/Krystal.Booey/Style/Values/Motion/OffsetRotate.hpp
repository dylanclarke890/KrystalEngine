/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'offset-rotate'> = [ auto | reverse ] || <angle>
  // https://drafts.fxtf.org/motion/#propdef-offset-rotate
  struct OffsetRotate
  {
    using Angle = style::Angle<css::All, float>;

    constexpr OffsetRotate(css::keywords::Auto keyword) noexcept : _angle {0}, _autoKeyword {keyword}
    {
    }
    constexpr OffsetRotate(Maybe<css::keywords::Auto> autoKeyword, Angle angle) noexcept
        : _angle(angle), _autoKeyword(autoKeyword)
    {
    }

    KRYS_NODISCARD bool HasAuto() const noexcept
    {
      return _autoKeyword.has_value();
    }
    KRYS_NODISCARD Maybe<css::keywords::Auto> AutoKeyword() const noexcept
    {
      return _autoKeyword;
    }

    KRYS_NODISCARD Angle GetAngle() const noexcept
    {
      return _angle;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (_autoKeyword)
      {
        return visitor(css::SpaceSeparatedTuple {*_autoKeyword, _angle});
      }
      return visitor(_angle);
    }

    KRYS_NODISCARD constexpr bool operator==(const OffsetRotate &) const noexcept = default;

  private:
    Angle _angle;
    Maybe<css::keywords::Auto> _autoKeyword;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<OffsetRotate>
  {
    KRYS_NODISCARD OffsetRotate operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<OffsetRotate>
  {
    KRYS_NODISCARD bool CanBlend(const OffsetRotate &, const OffsetRotate &) noexcept;
    KRYS_NODISCARD OffsetRotate Blend(const OffsetRotate &, const OffsetRotate &,
                                      const BlendingContext &) noexcept;
  };

  // MARK: - Evaluation
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::OffsetRotate)
