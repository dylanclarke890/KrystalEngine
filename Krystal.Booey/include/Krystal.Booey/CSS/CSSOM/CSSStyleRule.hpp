#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class CSSRuleList;
  class CSSStyleProperties;
  class StylePropertyMap;
  class StyleRuleCSSStyleProperties;

  /// @brief A style rule, as scripts see it. It holds nested rules as a grouping rule does, and its rule in
  /// the rule model becomes one that can hold them when the first is inserted.
  /// @see https://drafts.csswg.org/cssom/#the-cssstylerule-interface
  class CSSStyleRule final : public CSSRule
  {
  private:
    // A css::StyleRuleWithNesting when it has nested rules, or had them.
    Ref<css::StyleRule> _rule;
    RefPtr<StyleRuleCSSStyleProperties> _style;
    RefPtr<StylePropertyMap> _styleMap;
    // The nested rules' wrappers, made when first asked for, and their live list.
    mutable List<RefPtr<CSSRule>> _childRules;
    mutable UniquePtr<CSSRuleList> _cssRules;

    CSSStyleRule(css::StyleRule &rule, CSSStyleSheet *parent) noexcept;

  public:
    ~CSSStyleRule() noexcept override;

    KRYS_NODISCARD static Ref<CSSStyleRule> Create(css::StyleRule &rule, CSSStyleSheet *parent) noexcept;

#pragma region CSSStyleRule - https://drafts.csswg.org/cssom/#the-cssstylerule-interface

    /// @brief The serialisation of the rule's selector list, as written (nested selectors aren't resolved
    /// against their parents).
    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-selectortext
    KRYS_NODISCARD CSSOMString SelectorText() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-selectortext
    void SetSelectorText(const CSSOMString &text) noexcept;

    /// @brief The rule's declarations, as a declaration block whose parent rule is this one.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstylerule-style
    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

#pragma endregion

#pragma region CSSGroupingRule - https://drafts.csswg.org/cssom/#the-cssgroupingrule-interface

    /// @brief The nested rules, as a live list.
    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-cssrules
    KRYS_NODISCARD CSSRuleList &CssRules() const noexcept;

    /// @brief Inserts @p rule, a rule or declarations, among the nested rules at @p index.
    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-insertrule
    ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;

    /// @brief Removes the nested rule at @p index.
    /// @see https://drafts.csswg.org/cssom/#dom-cssgroupingrule-deleterule
    ExceptionOr<void> DeleteRule(uint32 index) noexcept;

#pragma endregion

    /// @brief The rule's declarations, as a property map.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssstylerule-stylemap
    KRYS_NODISCARD StylePropertyMap &StyleMap() noexcept;

    // FIXME: Not CSSOM. Remove.
    KRYS_NODISCARD css::StyleRule &GetStyleRule() const noexcept
    {
      return *_rule;
    }

    // For CSSRuleList.
    KRYS_NODISCARD uint32 Length() const noexcept;

    KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept;

  private:
    KRYS_NODISCARD css::RuleType StyleRuleType() const noexcept final
    {
      return css::RuleType::Style;
    }

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    /// @see https://drafts.csswg.org/cssom/#serialize-a-css-rule
    KRYS_NODISCARD CSSOMString CssText(const css::SerialisationContext &context) const noexcept final;

    void Reattach(css::Rule &rule) noexcept final;

    void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept final;

    KRYS_NODISCARD const List<Ref<css::Rule>> &NestedRules() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::CSSRule &rule) noexcept
  {
    return rule.StyleRuleType() == krys::boo::css::RuleType::Style;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
