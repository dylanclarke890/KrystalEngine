#pragma once

#include "Krystal.HTML/CSS/Enums/CSSRuleFlag.hpp"
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

  /// @see https://drafts.csswg.org/cssom/#the-cssrule-interface
  class CSSRule : public RefCounted<CSSRule>,
                  public CanMakeWeakPtr<CSSRule>,
                  public CanMakeCheckedPtr<CSSStyleSheet>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSRule);

  private:
    CSSRuleType _type : BitCount<CSSRuleType>() {CSSRuleType::Unknown};
    CSSRuleFlag _flags : BitCount<CSSRuleFlag>() {CSSRuleFlag::None};
    CheckedPtr<CSSRule> _parentRule;
    CheckedPtr<CSSStyleSheet> _parentStylesheet;

  protected:
    CSSRule(CSSRuleType type, RawPtr<CSSStyleSheet> stylesheet) noexcept;

  public:
    virtual ~CSSRule() noexcept = default;

#pragma region CSSRule - https://drafts.csswg.org/cssom/#cssrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    ExceptionOr<void> CssText(const CSSOMString &text) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-parentrule
    KRYS_NODISCARD RefPtr<CSSRule> ParentRule() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-parentstylesheet
    KRYS_NODISCARD RefPtr<CSSStyleSheet> ParentStyleSheet() const noexcept;

    /// @deprecated
    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-type
    KRYS_NODISCARD uint8 Type() const noexcept;

#pragma endregion

    /// @brief Custom version of Type() that includes all supported CSSRuleType values, including those that
    /// are not exposed via the spec's Type() method.
    KRYS_NODISCARD CSSRuleType RuleType() const noexcept
    {
      return _type;
    }

  protected:
#pragma region Type Checks

    KRYS_NODISCARD bool IsCSSCharsetRule() const noexcept
    {
      return _type == CSSRuleType::Charset;
    }

    KRYS_NODISCARD bool IsCSSGroupingRule() const noexcept
    {
      return HasFlag(_flags, CSSRuleFlag::IsGroupingRule);
    }

    KRYS_NODISCARD bool IsCSSImportRule() const noexcept
    {
      return _type == CSSRuleType::Import;
    }

    KRYS_NODISCARD bool IsCSSMarginRule() const noexcept
    {
      return _type == CSSRuleType::Margin;
    }

    KRYS_NODISCARD bool IsCSSMediaRule() const noexcept
    {
      return _type == CSSRuleType::Media;
    }

    KRYS_NODISCARD bool IsCSSNamespaceRule() const noexcept
    {
      return _type == CSSRuleType::Namespace;
    }

    KRYS_NODISCARD bool IsCSSPageRule() const noexcept
    {
      return _type == CSSRuleType::Page;
    }

    KRYS_NODISCARD bool IsCSSStyleRule() const noexcept
    {
      return _type == CSSRuleType::Style;
    }

#pragma endregion
  };
}