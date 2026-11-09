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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <repeat-style> = repeat-x@(alias=[repeat no-repeat]) | repeat-y@(alias=[no-repeat repeat]) | [repeat |
  // space | round | no-repeat]{1,2} https://www.w3.org/TR/css-backgrounds-3/#typedef-repeat-style
  struct RepeatStyle
  {
    css::MinimallySerialisingSpaceSeparatedPoint<FillRepeat> Values;

    KRYS_NODISCARD FillRepeat X() const noexcept
    {
      return Values.x();
    }
    KRYS_NODISCARD FillRepeat Y() const noexcept
    {
      return Values.y();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (X() == Y())
      {
        return visitor(X());
      }
      if (X() == FillRepeat::Repeat && Y() == FillRepeat::NoRepeat)
      {
        return visitor(css::keywords::RepeatX {});
      }
      if (X() == FillRepeat::NoRepeat && Y() == FillRepeat::Repeat)
      {
        return visitor(css::keywords::RepeatY {});
      }
      return visitor(Values);
    }

    KRYS_NODISCARD bool operator==(const RepeatStyle &) const noexcept = default;
    bool operator==(FillRepeat other) const noexcept
    {
      return X() == other && Y() == other;
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<RepeatStyle>
  {
    KRYS_NODISCARD RepeatStyle operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::RepeatStyle)
