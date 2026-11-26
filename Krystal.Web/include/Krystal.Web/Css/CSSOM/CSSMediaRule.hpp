#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Css/CSSOM/CSSConditionRule.hpp"
#include "Krystal.Web/Css/Queries/MediaQuery.hpp"

namespace krys::web::css
{
  class MediaRule;
}

namespace krys::web
{
  class MediaList;

  /// @brief An `@media` rule, as scripts see it.
  /// @see https://drafts.csswg.org/cssom/#the-cssmediarule-interface
  class CSSMediaRule final : public CSSConditionRule
  {
    friend class MediaList;

  private:
    mutable RefPtr<MediaList> _media;

    CSSMediaRule(css::MediaRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSMediaRule() noexcept override;

    KRYS_NODISCARD static Ref<CSSMediaRule> Create(css::MediaRule &rule, CSSStyleSheet *parent) noexcept;

    /// @brief The rule's media queries, made when first asked for.
    /// @see https://drafts.csswg.org/cssom/#dom-cssmediarule-media
    KRYS_NODISCARD MediaList *Media() const noexcept;

  private:
    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::Media;
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-css-rule
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    /// @see https://drafts.csswg.org/cssom/#serialize-a-css-rule
    KRYS_NODISCARD CSSOMString CssText(const css::SerialisationContext &context) const noexcept final;

    /// @brief The serialisation of the media queries.
    /// @see https://drafts.csswg.org/css-conditional-3/#dom-cssconditionrule-conditiontext
    KRYS_NODISCARD CSSOMString ConditionText() const noexcept final;

    KRYS_NODISCARD const css::mq::MediaQueryList &MediaQueries() const noexcept;

    void SetMediaQueries(css::mq::MediaQueryList &&queries) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CSSMediaRule)
  KRYS_NODISCARD static bool IsType(const krys::web::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::web::css::RuleType::Media;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
