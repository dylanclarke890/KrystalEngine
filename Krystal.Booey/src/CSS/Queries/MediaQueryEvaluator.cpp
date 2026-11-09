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

#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::css::mq
{
  MediaQueryEvaluator::MediaQueryEvaluator(const CSSOMStringAtom &mediaType, const dom::Document &document,
                                           const RenderStyle *rootElementStyle) noexcept
      : GenericMediaQueryEvaluator(), _mediaType(mediaType), _document(CreateWeakPtr(&document)),
        _rootElementStyle(rootElementStyle)
  {
  }

  MediaQueryEvaluator::MediaQueryEvaluator(const CSSOMStringAtom &mediaType,
                                           EvaluationResult mediaConditionResult) noexcept
      : GenericMediaQueryEvaluator(), _mediaType(mediaType), _staticMediaConditionResult(mediaConditionResult)
  {
  }

  bool MediaQueryEvaluator::Evaluate(const MediaQueryList &list) const noexcept
  {
    if (list.empty())
    {
      return true;
    }

    for (auto &query : list)
    {
      if (Evaluate(query))
      {
        return true;
      }
    }
    return false;
  }

  bool MediaQueryEvaluator::Evaluate(const MediaQuery &query) const noexcept
  {
    bool isNegated = query.Prefix && *query.Prefix == Prefix::Not;

    if (!EvaluateMediaType(query))
    {
      return isNegated;
    }

    auto result = [&]
    {
      if (!query.Condition)
      {
        return EvaluationResult::True;
      }

      auto document = ShareRefPtr(_document.get());
      if (!document || !_rootElementStyle)
      {
        return _staticMediaConditionResult;
      }

      if (!document->View() || !document->DocumentElement())
      {
        return EvaluationResult::Unknown;
      }

      FeatureEvaluationContext context {
        CreateWeakRef(*document),
        {*_rootElementStyle, _rootElementStyle, nullptr, document->GetRenderView()},
        nullptr};
      return EvaluateCondition(*query.Condition, context);
    }();

    switch (result)
    {
      case EvaluationResult::Unknown: return false;
      case EvaluationResult::True:    return !isNegated;
      case EvaluationResult::False:   return isNegated;
    }

    krys_unreachable();
  }

  bool MediaQueryEvaluator::EvaluateMediaType(const MediaQuery &query) const noexcept
  {
    if (query.MediaType == CSSOMStringAtom::Null() || query.MediaType == CSSOMStringAtom::Empty())
    {
      return true;
    }
    if (query.MediaType == AllAtom())
    {
      return true;
    }
    return query.MediaType == _mediaType;
  }

  MediaQueryDynamicDependency
    MediaQueryEvaluator::CollectDynamicDependencies(const MediaQueryList &queries) const noexcept
  {
    auto result = MediaQueryDynamicDependency::None;

    for (auto &query : queries)
    {
      result |= CollectDynamicDependencies(query);
    }

    return result;
  }

  MediaQueryDynamicDependency
    MediaQueryEvaluator::CollectDynamicDependencies(const MediaQuery &query) const noexcept
  {
    if (!EvaluateMediaType(query))
    {
      return MediaQueryDynamicDependency::None;
    }

    auto result = MediaQueryDynamicDependency::None;

    TraverseFeatures(query,
                     [&](const Feature &feature)
                     {
                       if (!feature.Schema)
                       {
                         return;
                       }
                       result |= feature.Schema->Dependencies;
                     });

    return result;
  }

  bool MediaQueryEvaluator::IsPrintMedia() const noexcept
  {
    return _mediaType == PrintAtom();
  }
}
