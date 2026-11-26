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

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"

namespace krys::web
{
  class Document;
  class RenderStyle;
}

namespace krys::web::style
{
  struct Filter;
}

namespace krys::web::css
{
  struct Filter;
  struct ParserContext;
  struct PropertyParserState;
  class TokenRange;
  class Value;

  namespace parse_property
  {
    // https://drafts.fxtf.org/filter-effects/#FilterProperty

    // MARK: <'filter'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeFilter(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <'filter'> consuming (unresolved)
    KRYS_NODISCARD Maybe<Filter> ConsumeUnresolvedFilter(TokenRange &tokens,
                                                         PropertyParserState &state) noexcept;

    // MARK: <'filter'> parsing (raw)

    /// @brief @p string parsed as a `filter` value and resolved against @p style; empty if it isn't one.
    /// @note What the canvas 2D context's `filter` is set with.
    KRYS_NODISCARD Maybe<style::Filter> ParseFilterValueListOrNoneRaw(const CSSOMString &string,
                                                                      const ParserContext &context,
                                                                      const Document &document,
                                                                      RenderStyle &style) noexcept;
  }
}
