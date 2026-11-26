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

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::web::css
{
  struct PropertyParserState;
  class TokenRange;
  class Value;

  namespace parse_property
  {
    enum class PathParsingOption : uint8
    {
      RejectPathFillRule,
      RejectPath,
    };

    consteval PathParsingOption MaxEnumValue(PathParsingOption) noexcept
    {
      return PathParsingOption::RejectPath;
    }

    // <basic-shape> = <circle()> | <ellipse() | <inset()> | <path()> | <polygon()> | <rect()> | <shape()> |
    // <xywh()> https://drafts.csswg.org/css-shapes/#typedef-basic-shape
    KRYS_NODISCARD RefPtr<Value> ConsumeBasicShape(TokenRange &tokens, PropertyParserState &state,
                                                   EnumSet<PathParsingOption> pathParsingOptions) noexcept;

    // <path()> = path( <'fill-rule'>? , <string> )
    // https://drafts.csswg.org/css-shapes/#funcdef-basic-shape-path
    KRYS_NODISCARD RefPtr<Value> ConsumePath(TokenRange &tokens, PropertyParserState &state) noexcept;

    // <'shape-outside'> = none | [ <basic-shape> || <shape-box> ] | <image>
    // https://drafts.csswg.org/css-shapes/#propdef-shape-outside
    KRYS_NODISCARD RefPtr<Value> ConsumeShapeOutside(TokenRange &tokens, PropertyParserState &state) noexcept;
  }
}
