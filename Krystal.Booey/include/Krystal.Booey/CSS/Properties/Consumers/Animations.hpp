/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct ParserContext;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    KRYS_NODISCARD SmallList<Pair<ValueId, double>>
      ConsumeKeyframeKeyList(TokenRange &tokens, PropertyParserState &state) noexcept;

    /// @grammar <keyframe-selector>
    /// @see https://drafts.csswg.org/css-animations-1/#typedef-keyframe-selector
    KRYS_NODISCARD SmallList<Pair<ValueId, double>>
      ParseKeyframeKeyList(const CSSOMString &str, const ParserContext &context) noexcept;

    /// @grammar <keyframes-name>
    /// @see https://drafts.csswg.org/css-animations/#typedef-keyframes-name
    KRYS_NODISCARD RefPtr<Value> ConsumeKeyframesName(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;
  }
}