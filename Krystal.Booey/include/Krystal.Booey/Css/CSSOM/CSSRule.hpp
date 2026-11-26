#pragma once

#include "Krystal.Booey/Css/Parser/ParserEnum.hpp"
#include "Krystal.Booey/Css/Rules/RuleType.hpp"
#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashSet.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class CSSStyleSheet;
  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  struct ParserContext;
  class Rule;
  struct SerialisationContext;
  class StyleRule;
  class StyleRuleWithNesting;
}

namespace krys::boo
{
  /// @brief A CSS rule as scripts see it: a wrapper of a rule in the internal rule model, made when a script
  /// first asks for it.
  /// @see https://drafts.csswg.org/cssom/#the-cssrule-interface
  class CSSRule : public RefCounted<CSSRule>, public CanMakeWeakPtr<CSSRule>
  {
  private:
    // The rule holding this one, if any; otherwise the style sheet holding it, if any.
    CSSRule *_parentRule {nullptr};
    CSSStyleSheet *_parentStyleSheet {nullptr};

  protected:
    explicit CSSRule(CSSStyleSheet *parentStyleSheet) noexcept : _parentStyleSheet(parentStyleSheet)
    {
    }

  public:
    virtual ~CSSRule() noexcept = default;

#pragma region CSSRule - https://drafts.csswg.org/cssom/#the-cssrule-interface

    /// @brief The serialisation of the rule.
    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    /// @brief Setting `cssText` does nothing.
    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    static ExceptionOr<void> SetCssText(const CSSOMString &text) noexcept;

    /// @brief The rule holding this one, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-parentrule
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept
    {
      return _parentRule;
    }

    /// @brief The style sheet holding this rule, directly or through its parent rules.
    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-parentstylesheet
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept
    {
      return _parentRule ? _parentRule->ParentStyleSheet() : _parentStyleSheet;
    }

    /// @brief The deprecated `type`: the number the CSSOM gave the rule's kind, or 0 for the kinds that came
    /// after it stopped giving them.
    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-type
    KRYS_NODISCARD uint16 TypeForCSSOM() const noexcept;

#pragma endregion

    /// @brief The kind of rule this wraps.
    KRYS_NODISCARD virtual css::RuleType StyleRuleType() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsGroupingRule() const noexcept
    {
      return false;
    }

    /// @brief The rule serialised in @p context, which can replace the URLs in it.
    KRYS_NODISCARD virtual CSSOMString CssText(const css::SerialisationContext &) const noexcept
    {
      return CssText();
    }

    /// @brief Points the wrapper at @p rule, the copy of its rule made when its style sheet's contents were
    /// copied to be changed.
    virtual void Reattach(css::Rule &rule) noexcept = 0;

    /// @brief Makes @p styleSheet the rule's parent, and the rule a top-level rule.
    void SetParentStyleSheet(CSSStyleSheet *styleSheet) noexcept
    {
      _parentRule = nullptr;
      _parentStyleSheet = styleSheet;
    }

    /// @brief Makes @p rule the rule's parent; null for a rule removed from its parent, which then has no
    /// style sheet either.
    void SetParentRule(CSSRule *rule) noexcept
    {
      _parentRule = rule;
      _parentStyleSheet = nullptr;
    }

    /// @brief Whether a style rule holds this one, directly or through other rules.
    KRYS_NODISCARD bool HasStyleRuleAncestor() const noexcept;

    /// @brief What the innermost style or scope rule holding this one makes of it when parsing, if any.
    KRYS_NODISCARD css::NestedContext GetNestedContext() const noexcept;

    /// @brief Replaces the child @p rule with a style rule that can hold nested rules.
    /// @return The new rule, or null if @p rule isn't a child of this rule.
    KRYS_NODISCARD virtual RefPtr<css::StyleRuleWithNesting>
      PrepareChildStyleRuleForNesting(css::StyleRule &rule) noexcept;

    /// @brief Adds the style sheets this rule imports, and those they import, to @p childStyleSheets.
    virtual void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept;

  protected:
    /// @brief The parser context of the rule's style sheet, for parsing text set on it.
    KRYS_NODISCARD const css::ParserContext &GetParserContext() const noexcept;
  };
}
