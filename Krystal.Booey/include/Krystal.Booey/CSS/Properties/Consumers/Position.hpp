/*
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

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <position> | <bg-position>
    // https://drafts.csswg.org/css-values/#position

    // MARK: <position> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumePosition(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <position-x> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumePositionX(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <position-y> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumePositionY(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <position> (unresolved)
    KRYS_NODISCARD Maybe<Position> ConsumePositionUnresolved(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept;

    // MARK: <bg-position> (unresolved)
    KRYS_NODISCARD Maybe<Position> ConsumeBackgroundPositionUnresolved(TokenRange &tokens,
                                                                       PropertyParserState &state) noexcept;

    // MARK: <position-x> (unresolved)
    KRYS_NODISCARD Maybe<PositionX> ConsumePositionXUnresolved(TokenRange &tokens,
                                                               PropertyParserState &state) noexcept;

    // MARK: <position-y> (unresolved)
    KRYS_NODISCARD Maybe<PositionY> ConsumePositionYUnresolved(TokenRange &tokens,
                                                               PropertyParserState &state) noexcept;

    // MARK: Subset / Special case parsers.

    // NOTE: This is only used by the `<-webkit-radial-gradient()>` and `<transform-origin>` parsers.
    KRYS_NODISCARD Maybe<Position>
      ConsumeOneOrTwoComponentPositionUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept;

    // NOTE: This is only used by the `<horizontal-line-command>` parser
    KRYS_NODISCARD Maybe<TwoComponentPositionHorizontal>
      ConsumeTwoComponentPositionHorizontalUnresolved(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;

    // NOTE: This is only used by the `<vertical-line-command>` parser
    KRYS_NODISCARD Maybe<TwoComponentPositionVertical>
      ConsumeTwoComponentPositionVerticalUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept;
  }
}