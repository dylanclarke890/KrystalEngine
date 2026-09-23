#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSConditionRule.hpp"

namespace krys::boo::css
{
  class MediaList;
  class StyleRuleMedia;

  namespace MQ
  {
    struct MediaQuery;
    using MediaQueryList = SmallList<MediaQuery>;
  }

  class CSSMediaRule final : public CSSConditionRule
  {
  public:
    static Ref<CSSMediaRule> create(StyleRuleMedia &rule, CSSStyleSheet *sheet) noexcept
    {
      return AdoptRef(*new CSSMediaRule(rule, sheet));
    }

    virtual ~CSSMediaRule() noexcept;

    MediaList *media() const noexcept;

  private:
    friend class MediaList;

    CSSMediaRule(StyleRuleMedia &, CSSStyleSheet *) noexcept;

    StyleRuleType styleRuleType() const noexcept final
    {
      return StyleRuleType::Media;
    }
    String cssText() const noexcept final;
    String cssText(const SerialisationContext &) const noexcept final;
    String conditionText() const noexcept final;

    const MQ::MediaQueryList &mediaQueries() const noexcept;
    void setMediaQueries(MQ::MediaQueryList &&) noexcept;

    mutable RefPtr<MediaList> m_mediaCSSOMWrapper;
  };

}

SPECIALIZE_TYPE_TRAITS_CSS_RULE(CSSMediaRule, StyleRuleType::Media)
