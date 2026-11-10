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

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css::mq
{
  /// @brief Evaluates media queries against a document, or against a fixed result when there's no document.
  class MediaQueryEvaluator : public GenericMediaQueryEvaluator<MediaQueryEvaluator>
  {
  private:
    CSSOMStringAtom _mediaType;
    WeakPtr<const dom::Document> _document;
    const RenderStyle *_rootElementStyle {nullptr}; // NOTE: must outlive evaluator.
    EvaluationResult _staticMediaConditionResult {EvaluationResult::Unknown};

  public:
    MediaQueryEvaluator(const CSSOMStringAtom &mediaType, const dom::Document &document,
                        const RenderStyle *rootElementStyle) noexcept;

    explicit MediaQueryEvaluator(const CSSOMStringAtom &mediaType = CSSOMStringAtom::Null(),
                                 EvaluationResult mediaConditionResult = EvaluationResult::False) noexcept;

    KRYS_NODISCARD bool Evaluate(const MediaQueryList &list) const noexcept;
    KRYS_NODISCARD bool Evaluate(const MediaQuery &query) const noexcept;

    KRYS_NODISCARD bool EvaluateMediaType(const MediaQuery &query) const noexcept;

    KRYS_NODISCARD MediaQueryDynamicDependency
      CollectDynamicDependencies(const MediaQueryList &queries) const noexcept;
    KRYS_NODISCARD MediaQueryDynamicDependency
      CollectDynamicDependencies(const MediaQuery &query) const noexcept;

    KRYS_NODISCARD bool IsPrintMedia() const noexcept;
  };
}
