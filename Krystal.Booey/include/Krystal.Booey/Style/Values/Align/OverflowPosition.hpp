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
  // <overflow-position> = unsafe | safe
  // https://drafts.csswg.org/css-align/#typedef-overflow-position
  using OverflowPosition = Variant<css::keywords::Unsafe, css::keywords::Safe>;

  // Used by alignment types in bitfields.
  enum class OverflowPositionKind : uint8
  {
    None,
    Unsafe,
    Safe
  };

  KRYS_NODISCARD constexpr OverflowPositionKind ComputeKind(Maybe<OverflowPosition> overflow) noexcept
  {
    if (!overflow)
    {
      return OverflowPositionKind::None;
    }
    return krys::SwitchOn(
      *overflow, [](css::keywords::Unsafe) { return OverflowPositionKind::Unsafe; },
      [](css::keywords::Safe) { return OverflowPositionKind::Safe; });
  }

  constexpr decltype(auto) VisitOverflowPosition(auto &&primaryKeyword, OverflowPositionKind kind,
                                                 auto &&visitor) noexcept
  {
    switch (kind)
    {
      case OverflowPositionKind::None: return visitor(primaryKeyword);
      case OverflowPositionKind::Unsafe:
        return visitor(css::SpaceSeparatedTuple {css::keywords::Unsafe {}, primaryKeyword});
      case OverflowPositionKind::Safe:
        return visitor(css::SpaceSeparatedTuple {css::keywords::Safe {}, primaryKeyword});
    }
    krys_unreachable();
  }
}
