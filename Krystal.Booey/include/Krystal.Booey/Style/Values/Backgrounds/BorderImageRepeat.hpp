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
  // <'border-image-repeat'> = [ stretch | repeat | round | space ]{1,2}
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-repeat
  struct BorderImageRepeat
  {
    css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> Values {NinePieceImageRule::Stretch,
                                                                            NinePieceImageRule::Stretch};

    constexpr BorderImageRepeat(
      css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> values) noexcept
        : Values {values}
    {
    }
    constexpr BorderImageRepeat(NinePieceImageRule horizontalRule, NinePieceImageRule verticalRule) noexcept
        : Values {horizontalRule, verticalRule}
    {
    }
    constexpr BorderImageRepeat(NinePieceImageRule rule) noexcept : Values {rule, rule}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Stretch) noexcept
        : BorderImageRepeat {NinePieceImageRule::Stretch}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Repeat) noexcept
        : BorderImageRepeat {NinePieceImageRule::Repeat}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Round) noexcept : BorderImageRepeat {NinePieceImageRule::Round}
    {
    }
    constexpr BorderImageRepeat(css::keywords::Space) noexcept : BorderImageRepeat {NinePieceImageRule::Space}
    {
    }

    KRYS_NODISCARD constexpr NinePieceImageRule HorizontalRule() const noexcept
    {
      return Values.Width();
    }
    KRYS_NODISCARD constexpr NinePieceImageRule VerticalRule() const noexcept
    {
      return Values.Height();
    }

    KRYS_NODISCARD constexpr bool operator==(const BorderImageRepeat &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(BorderImageRepeat, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BorderImageRepeat>
  {
    KRYS_NODISCARD BorderImageRepeat operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::BorderImageRepeat)
