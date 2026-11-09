/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class TimingFunction;
}

namespace krys::boo::css
{
  struct ParserContext;
  class TokenRange;
  class Value;

  struct PropertyParserState;
  struct EasingFunction;

  namespace PropertyParserHelpers
  {
    // <easing-function> = linear | ease | ease-in | ease-out | ease-in-out | step-start | step-end |
    // <linear()> | <cubic-bezier()> | <steps()> NOTE: also includes non-standard <spring()>.
    // https://drafts.csswg.org/css-easing/#typedef-easing-function

    // MARK: <easing-function> consuming (unresolved)
    KRYS_NODISCARD Maybe<EasingFunction> ConsumeUnresolvedEasingFunction(TokenRange &tokens,
                                                                         PropertyParserState &state) noexcept;

    // MARK: <easing-function> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeEasingFunction(TokenRange &tokens,
                                                       PropertyParserState &state) noexcept;

    // MARK: <easing-function> parsing (TimingFunction)
    /// @brief Parses the whole of `string`, leading and trailing whitespace aside, as an easing function.
    /// @return Null if it isn't one.
    KRYS_NODISCARD RefPtr<TimingFunction>
      ParseEasingFunctionDeprecated(const CSSOMString &string, const ParserContext &context) noexcept;
  }
}