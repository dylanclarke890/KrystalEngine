#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"

namespace Krys::HTML
{
  /// @deprecated The @charset rule is deprecated and is only here for completeness.
  class CSSCharsetRule final : public CSSRule
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSCharsetRule);

  public:
    CSSCharsetRule(RawPtr<CSSStyleSheet> stylesheet) noexcept : CSSRule(CSSRuleType::Charset, stylesheet)
    {
    }

    ~CSSCharsetRule() noexcept override = default;

#pragma region CSSCharsetRule

    KRYS_NODISCARD CSSOMString Encoding() const noexcept
    {
      return u8"";
    }

    ExceptionOr<void> Encoding(const CSSOMString &encoding) noexcept
    {
      return ExceptionCode::NoModificationAllowedError;
    }

#pragma endregion

#pragma region CSSRule - https://drafts.csswg.org/cssom/#cssrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    KRYS_NODISCARD CSSOMString CssText() const noexcept final
    {
      return u8"@charset \"\";";
    }

#pragma endregion
  };
}