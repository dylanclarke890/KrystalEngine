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
  // <baseline-position-preference> = first | last
  // https://drafts.csswg.org/css-align/#baseline-alignment-preference
  using BaselineAlignmentPreference = Variant<css::keywords::First, css::keywords::Last>;

  // Used by alignment types in bitfields.
  enum class BaselineAlignmentPreferenceKind : bool
  {
    First,
    Last
  };

  KRYS_NODISCARD constexpr BaselineAlignmentPreferenceKind
    ComputeKind(Maybe<BaselineAlignmentPreference> preference) noexcept
  {
    if (!preference)
    {
      return BaselineAlignmentPreferenceKind::First;
    }
    return krys::SwitchOn(
      *preference, [](css::keywords::First) { return BaselineAlignmentPreferenceKind::First; },
      [](css::keywords::Last) { return BaselineAlignmentPreferenceKind::Last; });
  }

  constexpr decltype(auto) VisitBaselineAlignmentPreference(auto &&primaryKeyword,
                                                            BaselineAlignmentPreferenceKind kind,
                                                            auto &&visitor) noexcept
  {
    switch (kind)
    {
      case BaselineAlignmentPreferenceKind::First: return visitor(primaryKeyword);
      case BaselineAlignmentPreferenceKind::Last:
        return visitor(css::SpaceSeparatedTuple {css::keywords::Last {}, primaryKeyword});
    }
    krys_unreachable();
  }
}
