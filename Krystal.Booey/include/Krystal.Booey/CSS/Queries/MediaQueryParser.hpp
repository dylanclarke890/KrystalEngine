/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::mq
{
  /// @brief Parses media query lists (`@media`, `@import`, the `media` attribute, `matchMedia()`).
  struct MediaQueryParser : public GenericMediaQueryParser<MediaQueryParser>
  {
    KRYS_NODISCARD static MediaQueryList Parse(CSSOMStringView string, const ParserContext &context) noexcept;
    KRYS_NODISCARD static MediaQueryList Parse(TokenRange tokens, const ParserContext &context) noexcept;

    /// @brief Parses a bare `<media-condition>` (as in `@import ... supports() <condition>`); an empty range
    /// is `all`.
    KRYS_NODISCARD static Maybe<MediaQuery> ParseCondition(TokenRange tokens,
                                                           const ParserContext &context) noexcept;

    /// @brief Consumes a comma-separated list. A query that doesn't parse becomes `not all`.
    KRYS_NODISCARD static MediaQueryList ConsumeMediaQueryList(TokenRange &tokens,
                                                               const ParserContext &context) noexcept;
    KRYS_NODISCARD static Maybe<MediaQuery> ConsumeMediaQuery(TokenRange &tokens,
                                                              const ParserContext &context) noexcept;

    KRYS_NODISCARD static List<const FeatureSchema *> FeatureSchemas() noexcept;
  };

  void Serialise(StringBuilder &builder, const MediaQueryList &list) noexcept;
  void Serialise(StringBuilder &builder, const MediaQuery &query) noexcept;
}
