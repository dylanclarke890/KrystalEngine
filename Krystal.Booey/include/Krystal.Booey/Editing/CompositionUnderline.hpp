/*
 * Copyright (C) 2017-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <compare>
#include <utility>

namespace krys::boo
{
  enum class CompositionUnderlineColor : bool
  {
    GivenColor,
    TextColor
  };

  /// @brief An underline under part of the text being composed with an input method (a clause), thick for the
  /// clause being converted.
  struct CompositionUnderline
  {
    uint32 StartOffset {0};
    uint32 EndOffset {0};
    CompositionUnderlineColor UnderlineColor {CompositionUnderlineColor::TextColor};
    boo::Color Color;
    bool Thick {false};

    friend std::strong_ordering operator<=>(const CompositionUnderline &a,
                                            const CompositionUnderline &b) noexcept
    {
      return std::make_pair(a.StartOffset, a.EndOffset) <=> std::make_pair(b.StartOffset, b.EndOffset);
    }

    friend bool operator==(const CompositionUnderline &, const CompositionUnderline &) noexcept = default;
  };
}
