#pragma once

#include "Krystal.Booey/CSS/Parser/ParserEnum.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class Rule;
  class StyleRule;
  class StyleRuleWithNesting;
  struct ParserContext;
  struct SerialisationContext;

  /// @brief The base of the rules exposed to scripts. Each wraps a rule of the internal rule model.
  /// @see https://drafts.csswg.org/cssom/#the-cssrule-interface
  class CSSRule : public RefCounted<CSSRule>, public CanMakeWeakPtr<CSSRule>
  {
  private:
    mutable bool _hasCachedSelectorText : 1 {false};
    bool _parentIsRule : 1 {false};
    union
    {
      CSSRule *_parentRule;
      CSSStyleSheet *_parentStyleSheet;
    };

  protected:
    explicit CSSRule(CSSStyleSheet *parent) noexcept : _parentStyleSheet(parent)
    {
    }

  public:
    virtual ~CSSRule() noexcept = default;

    /// @brief The rule's `type`: its rule type, or 0 for the rule types added after the enumeration was
    /// frozen.
    KRYS_NODISCARD uint16 TypeForCSSOM() const noexcept;

    KRYS_NODISCARD virtual css::RuleType StyleRuleType() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsGroupingRule() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    KRYS_NODISCARD virtual CSSOMString CssText(const SerialisationContext &) const noexcept
    {
      return CssText();
    }

    /// @brief Points the wrapper at @p rule, the copy of its rule made when the style sheet was copied on
    /// write.
    virtual void Reattach(Rule &rule) noexcept = 0;

    void SetParentStyleSheet(CSSStyleSheet *styleSheet) noexcept
    {
      _parentIsRule = false;
      _parentStyleSheet = styleSheet;
    }

    void SetParentRule(CSSRule *rule) noexcept
    {
      _parentIsRule = true;
      _parentRule = rule;
    }

    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept
    {
      if (_parentIsRule)
      {
        return _parentRule ? _parentRule->ParentStyleSheet() : nullptr;
      }
      return _parentStyleSheet;
    }

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept
    {
      return _parentIsRule ? _parentRule : nullptr;
    }

    KRYS_NODISCARD bool HasStyleRuleAncestor() const noexcept;

    /// @brief The innermost style or scope rule this rule is nested inside, if any.
    KRYS_NODISCARD css::NestedContext GetNestedContext() const noexcept;

    /// @brief Replaces the child @p rule with a style rule that can hold nested rules.
    /// @return The new rule, or null if @p rule isn't a child of this rule.
    KRYS_NODISCARD virtual RefPtr<StyleRuleWithNesting>
      PrepareChildStyleRuleForNesting(StyleRule &rule) noexcept;

    virtual void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &) noexcept
    {
    }

    dom::ExceptionOr<void> SetCssText(const CSSOMString &text) noexcept;

  protected:
    KRYS_NODISCARD bool HasCachedSelectorText() const noexcept
    {
      return _hasCachedSelectorText;
    }

    void SetHasCachedSelectorText(bool hasCachedSelectorText) const noexcept
    {
      _hasCachedSelectorText = hasCachedSelectorText;
    }

    KRYS_NODISCARD const ParserContext &GetParserContext() const noexcept;
  };
}
