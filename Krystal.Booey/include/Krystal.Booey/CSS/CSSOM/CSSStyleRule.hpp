#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class CSSRuleList;
  class CSSStyleProperties;
  class StyleRuleCSSStyleProperties;

  /// @brief A style rule, exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-cssstylerule-interface
  // FIXME: styleMap (DeclaredStylePropertyMap) comes with the Typed OM.
  class CSSStyleRule final : public CSSRule
  {
  private:
    Ref<StyleRule> _styleRule;
    RefPtr<StyleRuleCSSStyleProperties> _propertiesCSSOMWrapper;

    mutable List<RefPtr<CSSRule>> _childRuleCSSOMWrappers;
    mutable UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

    CSSStyleRule(StyleRule &rule, CSSStyleSheet *parent) noexcept;
    CSSStyleRule(StyleRuleWithNesting &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSStyleRule() noexcept;

    KRYS_NODISCARD static Ref<CSSStyleRule> Create(StyleRule &rule, CSSStyleSheet *sheet) noexcept;
    KRYS_NODISCARD static Ref<CSSStyleRule> Create(StyleRuleWithNesting &rule, CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;
    void SetSelectorText(const CSSOMString &selectorText) noexcept;

    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

    // FIXME: Not CSSOM. Remove.
    KRYS_NODISCARD StyleRule &GetStyleRule() const noexcept
    {
      return *_styleRule;
    }

    KRYS_NODISCARD CSSRuleList &CssRules() const noexcept;
    dom::ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;
    dom::ExceptionOr<void> DeleteRule(uint32 index) noexcept;
    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept;

  private:
    KRYS_NODISCARD RuleType StyleRuleType() const noexcept final
    {
      return RuleType::Style;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) const noexcept final;
    KRYS_NODISCARD CSSOMString CssTextInternal(const CSSOMString &declarations,
                                               const StringBuilder &rules) const noexcept;
    void Reattach(Rule &rule) noexcept final;
    void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept final;

    KRYS_NODISCARD CSSOMString GenerateSelectorText() const noexcept;
    KRYS_NODISCARD const List<Ref<Rule>> &NestedRules() const noexcept;
    void CssTextForRules(StringBuilder &rules) const noexcept;
    void CssTextForRulesWithReplacementURLs(StringBuilder &rules,
                                            const SerialisationContext &context) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Style;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
