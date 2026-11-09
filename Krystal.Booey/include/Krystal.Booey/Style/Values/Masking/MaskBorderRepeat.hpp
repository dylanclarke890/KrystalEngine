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
  // <'mask-border-repeat'> = [ stretch | repeat | round | space ]{1,2}
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-repeat
  struct MaskBorderRepeat
  {
    css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> Values {NinePieceImageRule::Stretch,
                                                                            NinePieceImageRule::Stretch};

    constexpr MaskBorderRepeat(
      css::MinimallySerialisingSpaceSeparatedSize<NinePieceImageRule> values) noexcept
        : Values {values}
    {
    }
    constexpr MaskBorderRepeat(NinePieceImageRule horizontalRule, NinePieceImageRule verticalRule) noexcept
        : Values {horizontalRule, verticalRule}
    {
    }
    constexpr MaskBorderRepeat(NinePieceImageRule rule) noexcept : Values {rule, rule}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Stretch) noexcept
        : MaskBorderRepeat {NinePieceImageRule::Stretch}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Repeat) noexcept : MaskBorderRepeat {NinePieceImageRule::Repeat}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Round) noexcept : MaskBorderRepeat {NinePieceImageRule::Round}
    {
    }
    constexpr MaskBorderRepeat(css::keywords::Space) noexcept : MaskBorderRepeat {NinePieceImageRule::Space}
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

    KRYS_NODISCARD constexpr bool operator==(const MaskBorderRepeat &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(MaskBorderRepeat, Values);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MaskBorderRepeat>
  {
    KRYS_NODISCARD MaskBorderRepeat operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::MaskBorderRepeat)
