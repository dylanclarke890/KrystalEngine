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

#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryParser.hpp"

namespace krys::boo::css::cq
{
  /// @brief Parses the prelude of an `@container` rule.
  struct ContainerQueryParser : mq::GenericMediaQueryParser<ContainerQueryParser>
  {
    KRYS_NODISCARD static Maybe<ContainerQuery> ConsumeContainerQuery(TokenRange &tokens,
                                                                      const ParserContext &context) noexcept;
    /// @brief `<container-condition>#`, each a query.
    KRYS_NODISCARD static Maybe<ContainerQueryList>
      ConsumeContainerQueryList(TokenRange &tokens, const ParserContext &context) noexcept;

    KRYS_NODISCARD static bool IsValidFunctionId(ValueId functionId) noexcept;
    /// @brief A `<style-range>`, inside `style()`.
    KRYS_NODISCARD static Maybe<mq::StyleRange>
      ConsumeStyleRange(TokenRange &tokens, const ParserContext &context, State &state) noexcept;
    KRYS_NODISCARD static const mq::FeatureSchema *
      SchemaForFeatureName(const CSSOMStringAtom &name, const ParserContext &context, State &state) noexcept;
    KRYS_NODISCARD static List<const mq::FeatureSchema *> FeatureSchemas() noexcept;
  };
}
