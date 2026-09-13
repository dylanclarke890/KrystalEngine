#pragma once

#include "Krystal.Booey/CSS/CSSRule.hpp"
#include "Krystal.Booey/CSS/Properties/InternalStyleProperties.hpp"

namespace krys::boo::css
{
  /// @see https://drafts.csswg.org/css-nesting/#the-cssnestrule
  class CSSFunctionDeclarations final : public CSSRule
  {
    friend class Parser;

    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSFunctionDeclarations);

  protected:
    Ref<InternalStyleProperties> _style;

    CSSFunctionDeclarations(Ref<InternalStyleProperties> style, CSSStyleSheet *stylesheet) noexcept
        : CSSRule(RuleType::FunctionDeclarations, stylesheet), _style(krys::move(style))
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSFunctionDeclarations)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSRule &rule) noexcept
  {
    return rule.IsCSSFunctionDeclarations();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();