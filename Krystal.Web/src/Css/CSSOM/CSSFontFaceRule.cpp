#include "Krystal.Web/Css/CSSOM/CSSFontFaceRule.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Web/Css/CSSOM/CSSFontFaceDescriptors.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/Properties/StyleProperties.hpp"
#include "Krystal.Web/Css/Rules/Rule.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"

namespace krys::web
{
  namespace
  {
    /// @brief `@font-face {`, the declarations with a space either side, and `}`; `@font-face { }` without
    /// any.
    KRYS_NODISCARD CSSOMString FontFaceRuleText(const CSSOMString &declarations) noexcept
    {
      StringBuilder builder;
      builder.Append(u8"@font-face { ");
      if (!declarations.empty())
      {
        builder.Append(declarations, u8' ');
      }
      builder.Append(u8'}');
      return krys::move(builder).ToString();
    }
  }

  CSSFontFaceRule::CSSFontFaceRule(css::FontFaceRule &rule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _rule(ShareRef(rule))
  {
  }

  CSSFontFaceRule::~CSSFontFaceRule() noexcept
  {
    // The descriptors can outlive the rule, which is no longer theirs.
    if (_style)
    {
      _style->ClearParentRule();
    }
  }

  Ref<CSSFontFaceRule> CSSFontFaceRule::Create(css::FontFaceRule &rule, CSSStyleSheet *parent) noexcept
  {
    return AdoptRef(*new CSSFontFaceRule(rule, parent));
  }

  CSSFontFaceDescriptors &CSSFontFaceRule::Style() noexcept
  {
    if (!_style)
    {
      _style = CSSFontFaceDescriptors::Create(_rule->MutableProperties(), *this);
    }
    return *_style;
  }

  CSSOMString CSSFontFaceRule::CssText() const noexcept
  {
    return CssText(css::DefaultSerialisationContext());
  }

  CSSOMString CSSFontFaceRule::CssText(const css::SerialisationContext &context) const noexcept
  {
    return FontFaceRuleText(_rule->Properties().AsText(context));
  }

  void CSSFontFaceRule::Reattach(css::Rule &rule) noexcept
  {
    _rule = ShareRef(Downcast<css::FontFaceRule>(rule));
    if (_style)
    {
      _style->Reattach(_rule->MutableProperties());
    }
  }
}
