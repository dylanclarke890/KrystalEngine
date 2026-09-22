#pragma once

#include "Krystal.Booey/CSS/Parser/NestedParserContext.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo::dom
{
  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class Rule;
  class RuleBase;
  class RuleWithNesting;

  struct ParserContext;
  struct SerialisationContext;

  class CSSRule : public RefCounted<CSSRule>, public CanMakeWeakPtr<CSSRule>
  {
  private:
    mutable bool _hasCachedSelectorText : 1;
    bool _parentIsRule : 1;
    union
    {
      CSSRule *_parentRule;
      CSSStyleSheet *_parentStyleSheet;
    };

  protected:
    explicit CSSRule(CSSStyleSheet *parent) noexcept
        : _hasCachedSelectorText(false), _parentIsRule(false), _parentStyleSheet(parent)
    {
    }

  public:
    virtual ~CSSRule() = default;

    KRYS_NODISCARD ushort TypeForCSSOM() const noexcept;

    KRYS_NODISCARD virtual RuleType Type() const noexcept = 0;

    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    KRYS_NODISCARD virtual CSSOMString CssText(const SerialisationContext &context) const noexcept
    {
      (void)context;
      return CssText();
    }

    virtual void Reattach(RuleBase &rule) noexcept = 0;

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

    KRYS_NODISCARD NestedParserContext NestedContext() const noexcept;

    KRYS_NODISCARD virtual RefPtr<RuleWithNesting> PrepareChildStyleRuleForNesting(Rule &) noexcept;

    virtual void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &) noexcept
    {
    }

    dom::ExceptionOr<void> SetCssText(const CSSOMString &) noexcept;

    KRYS_NODISCARD virtual bool IsGroupingRule() const noexcept
    {
      return false;
    }

  protected:
    KRYS_NODISCARD bool HasCachedSelectorText() const noexcept
    {
      return _hasCachedSelectorText;
    }

    void SetHasCachedSelectorText(bool hasCachedSelectorText) const noexcept
    {
      _hasCachedSelectorText = hasCachedSelectorText;
    }

    KRYS_NODISCARD const ParserContext &ParserContext() const noexcept;
  };
}