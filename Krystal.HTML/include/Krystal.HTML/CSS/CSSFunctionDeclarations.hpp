#pragma once

#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/Properties/CSSInternalStyleProperties.hpp"

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  class CSSFunctionDeclarations final : public CSSRule
  {
    friend class CSSParser;

    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSFunctionDeclarations);

  protected:
    Ref<CSSInternalStyleProperties> _style;

    CSSFunctionDeclarations(Ref<CSSInternalStyleProperties> style, RawPtr<CSSStyleSheet> stylesheet) noexcept
        : CSSRule(CSSRuleType::FunctionDeclarations, stylesheet), _style(Krys::Move(style))
    {
    }

  public:
    ~CSSFunctionDeclarations() noexcept override = default;

#pragma region CSSRule - https://drafts.csswg.org/cssom/#cssrule

    /// @see https://drafts.csswg.org/cssom/#dom-cssrule-csstext
    KRYS_NODISCARD CSSOMString CssText() const noexcept final
    {
      // TODO
      return {};
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSFunctionDeclarations)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSRule &rule) noexcept
  {
    return rule.IsCSSFunctionDeclarations();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();