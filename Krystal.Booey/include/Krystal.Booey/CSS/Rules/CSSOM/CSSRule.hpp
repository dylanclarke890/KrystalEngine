#pragma once

#include "Krystal.Booey/CSS/Rules/RuleFlag.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;

  /// @see https://drafts.csswg.org/cssom/#the-cssrule-interface
  class CSSRule : public RefCounted<CSSRule>,
                  public CanMakeWeakPtr<CSSRule>,
                  public CanMakeCheckedPtr<CSSRule>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSRule);

  private:
    RuleType _type : BitCount<RuleType>() {RuleType::Unknown};
    RuleFlag _flags : BitCount<RuleFlag>() {RuleFlag::None};
    CheckedPtr<CSSRule> _parentRule;
    CheckedPtr<CSSStyleSheet> _parentStylesheet;

  protected:
    CSSRule(RuleType type, CSSStyleSheet *stylesheet) noexcept;

  public:
    virtual ~CSSRule() noexcept = default;

#pragma region CSSRule - https://drafts.csswg.org/cssom/#cssrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    dom::ExceptionOr<void> CssText(const CSSOMString &text) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-parentrule
    KRYS_NODISCARD RefPtr<CSSRule> ParentRule() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-parentstylesheet
    KRYS_NODISCARD RefPtr<CSSStyleSheet> ParentStyleSheet() const noexcept;

    /// @deprecated
    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-type
    KRYS_NODISCARD uint8 Type() const noexcept;

#pragma endregion

    /// @brief Custom version of Type() that includes all supported RuleType values, including those that
    /// are not exposed via the spec's Type() method.
    KRYS_NODISCARD RuleType RuleType() const noexcept
    {
      return _type;
    }

  protected:
#pragma region Type Checks

    KRYS_NODISCARD bool IsCSSCharsetRule() const noexcept
    {
      return _type == RuleType::Charset;
    }

    KRYS_NODISCARD bool IsCSSFunctionDeclarations() const noexcept
    {
      return _type == RuleType::FunctionDeclarations;
    }

    KRYS_NODISCARD bool IsCSSGroupingRule() const noexcept
    {
      return HasFlag(_flags, RuleFlag::IsGroupingRule);
    }

    KRYS_NODISCARD bool IsCSSImportRule() const noexcept
    {
      return _type == RuleType::Import;
    }

    KRYS_NODISCARD bool IsCSSMarginRule() const noexcept
    {
      return _type == RuleType::Margin;
    }

    KRYS_NODISCARD bool IsCSSMediaRule() const noexcept
    {
      return _type == RuleType::Media;
    }

    KRYS_NODISCARD bool IsCSSNamespaceRule() const noexcept
    {
      return _type == RuleType::Namespace;
    }

    KRYS_NODISCARD bool IsCSSNestedDeclarations() const noexcept
    {
      return _type == RuleType::NestedDeclarations;
    }

    KRYS_NODISCARD bool IsCSSPageRule() const noexcept
    {
      return _type == RuleType::Page;
    }

    KRYS_NODISCARD bool IsCSSStyleRule() const noexcept
    {
      return _type == RuleType::Style;
    }

#pragma endregion
  };
}