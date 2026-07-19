#pragma once

#include "Krystal.HTML/CSS/Enums/CSSRuleType.hpp"
#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSStyleSheet;

  /// @see https://www.w3.org/TR/cssom-1/#cssrule
  class CSSRule : public RefCounted<CSSRule>,
                  public CanMakeWeakPtr<CSSRule>,
                  public CanMakeCheckedPtr<CSSStyleSheet>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSRule);

  private:
    CSSRuleType _type : BitCount<CSSRuleType>() {CSSRuleType::Unknown};
    CheckedPtr<CSSRule> _parentRule;
    CheckedPtr<CSSStyleSheet> _parentStylesheet;

  protected:
    CSSRule(CSSRuleType type, RawPtr<CSSStyleSheet> stylesheet) noexcept;

  public:
    virtual ~CSSRule() noexcept = default;

#pragma region CSSRule - https://www.w3.org/TR/cssom-1/#cssrule

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssrule-csstext
    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssrule-csstext
    ExceptionOr<void> CssText(const CSSOMString &text) noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssrule-parentrule
    KRYS_NODISCARD RefPtr<CSSRule> ParentRule() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssrule-parentstylesheet
    KRYS_NODISCARD RefPtr<CSSStyleSheet> ParentStyleSheet() const noexcept;

    /// @see https://www.w3.org/TR/cssom-1/#dom-cssrule-type
    KRYS_NODISCARD uint8 Type() const noexcept;

#pragma endregion

    /// @brief Custom version of Type() that includes all CSSRuleType values, including those that are not
    /// exposed via the spec's Type() method.
    KRYS_NODISCARD CSSRuleType RuleType() const noexcept
    {
      return _type;
    }
  };
}