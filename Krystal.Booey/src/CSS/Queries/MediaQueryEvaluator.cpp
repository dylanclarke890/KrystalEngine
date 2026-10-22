#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::css::mq
{
  KRYS_NODISCARD static const CSSOMStringAtom &PrintAtom() noexcept
  {
    static const CSSOMStringAtom print {u8"print"};
    return print;
  }

  MediaQueryEvaluator::MediaQueryEvaluator(const CSSOMStringAtom &mediaType, const dom::Document &document,
                                           const RenderStyle *rootElementStyle) noexcept
      : GenericMediaQueryEvaluator(), _mediaType(mediaType),
        _document(WeakPtr<const dom::Document>::WithRef(document)), _rootElementStyle(rootElementStyle)
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
        WeakRef<const dom::Document>::WithRef(*document),
        {*_rootElementStyle, _rootElementStyle, nullptr, RenderViewOf(*document)},
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
