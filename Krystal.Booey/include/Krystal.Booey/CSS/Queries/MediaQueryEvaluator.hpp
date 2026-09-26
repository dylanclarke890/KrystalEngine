#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"

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
  class MediaQueryEvaluator : public GenericMediaQueryEvaluator<MediaQueryEvaluator>
  {
  private:
    CSSOMStringAtom _mediaType;
    WeakPtr<const dom::Document> _document;
    const RenderStyle *_rootElementStyle {nullptr}; // FIXME: Switch to a smart pointer.
    EvaluationResult _staticMediaConditionResult {EvaluationResult::Unknown};

  public:
    MediaQueryEvaluator(const CSSOMStringAtom &mediaType, const dom::Document &,
                        const RenderStyle *rootElementStyle) noexcept;

    MediaQueryEvaluator(const CSSOMStringAtom &mediaType = CSSOMStringAtom::Null(),
                        EvaluationResult mediaConditionResult = EvaluationResult::False) noexcept;

    KRYS_NODISCARD bool Evaluate(const MediaQueryList &queries) const noexcept;
    
    KRYS_NODISCARD bool Evaluate(const MediaQuery &query) const noexcept;

    KRYS_NODISCARD bool EvaluateMediaType(const MediaQuery &query) const noexcept;

    KRYS_NODISCARD MediaQueryDynamicDependency
      CollectDynamicDependencies(const MediaQueryList &queries) const noexcept;

    KRYS_NODISCARD MediaQueryDynamicDependency
      CollectDynamicDependencies(const MediaQuery &query) const noexcept;

    KRYS_NODISCARD bool IsPrintMedia() const noexcept;
  };
}
