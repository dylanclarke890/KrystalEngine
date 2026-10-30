#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class MediaList;
  class MediaRule;

  /// @brief An `@media` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-cssmediarule-interface
  class CSSMediaRule final : public CSSConditionRule
  {
    friend class MediaList;

  private:
    mutable RefPtr<MediaList> _mediaCSSOMWrapper;

    CSSMediaRule(MediaRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSMediaRule() noexcept;

    KRYS_NODISCARD static Ref<CSSMediaRule> Create(MediaRule &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD MediaList *Media() const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Media;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept final;
    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept;
    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSMediaRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Media;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
