#pragma once

#include "Krystal.Booey/CSS/CSSRule.hpp"

namespace krys::boo::css
{
  /// @deprecated The @charset rule is deprecated and is only here for use by the parser.
  class CSSCharsetRule final : public CSSRule
  {
    friend class Parser;

    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSCharsetRule);

  private:
    CSSCharsetRule(CSSStyleSheet *stylesheet) noexcept : CSSRule(RuleType::Charset, stylesheet)
    {
    }

  public:
    ~CSSCharsetRule() noexcept override = default;

#pragma region CSSCharsetRule

    KRYS_NODISCARD CSSOMString Encoding() const noexcept
    {
      return u8"";
    }

    dom::ExceptionOr<void> Encoding(const CSSOMString &encoding) noexcept
    {
      return dom::ExceptionCode::NoModificationAllowedError;
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSCharsetRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsCSSCharsetRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();