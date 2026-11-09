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
  // <legacy-position> = left | right | center
  // https://drafts.csswg.org/css-align/#valdef-justify-items-legacy
  using LegacyPosition = Variant<css::keywords::Left, css::keywords::Right, css::keywords::Center>;

  // Used by alignment types in bitfields.
  enum class LegacyPositionKind : uint8
  {
    None,
    Left,
    Right,
    Center
  };

  KRYS_NODISCARD constexpr LegacyPositionKind ComputeKind(Maybe<LegacyPosition> position) noexcept
  {
    if (!position)
    {
      return LegacyPositionKind::None;
    }
    return krys::SwitchOn(
      *position, [](css::keywords::Left) { return LegacyPositionKind::Left; },
      [](css::keywords::Right) { return LegacyPositionKind::Right; },
      [](css::keywords::Center) { return LegacyPositionKind::Center; });
  }

  constexpr decltype(auto) VisitLegacyPosition(auto &&primaryKeyword, LegacyPositionKind kind,
                                               auto &&visitor) noexcept
  {
    switch (kind)
    {
      case LegacyPositionKind::None:
        // `legacy` on its own computes to `normal`.
        // https://drafts.csswg.org/css-align/#valdef-justify-items-legacy
        return visitor(css::keywords::Normal {});
      case LegacyPositionKind::Left:
        return visitor(css::SpaceSeparatedTuple {primaryKeyword, css::keywords::Left {}});
      case LegacyPositionKind::Right:
        return visitor(css::SpaceSeparatedTuple {primaryKeyword, css::keywords::Right {}});
      case LegacyPositionKind::Center:
        return visitor(css::SpaceSeparatedTuple {primaryKeyword, css::keywords::Center {}});
    }
    krys_unreachable();
  }
}
