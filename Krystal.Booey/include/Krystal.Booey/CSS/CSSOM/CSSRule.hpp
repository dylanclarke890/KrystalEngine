#pragma once

#include "Krystal.Booey/CSS/Parser/NestedParserContext.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
{
  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class RuleBase;
  class Rule;
  class RuleWithNesting;

  struct ParserContext;

  namespace CSS
  {
    struct SerializationContext;
  }

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

    void SetParentStyleSheet(CSSStyleSheet *stylesheet) noexcept
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

    KRYS_NODISCARD CSSRule *ParentRule() const
    {
      return _parentIsRule ? _parentRule : nullptr;
    }

    KRYS_NODISCARD bool HasStyleRuleAncestor() const;

    KRYS_NODISCARD NestedParserContext NestedContext() const noexcept;

    KRYS_NODISCARD virtual RefPtr<RuleWithNesting> PrepareChildStyleRuleForNesting(Rule &);

    virtual void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &)
    {
    }

    ExceptionOr<void> SetCssText(const String &);

    KRYS_NODISCARD virtual bool IsGroupingRule() const noexcept
    {
      return false;
    }

  protected:
    explicit CSSRule(CSSStyleSheet *parent) noexcept
        : _hasCachedSelectorText(false), _parentIsRule(false), _parentStyleSheet(parent)
    {
    }

    bool HasCachedSelectorText() const
    {
      return _hasCachedSelectorText;
    }
    void SetHasCachedSelectorText(bool hasCachedSelectorText) const
    {
      _hasCachedSelectorText = hasCachedSelectorText;
    }

    const ParserContext &ParserContext() const;
  };
}