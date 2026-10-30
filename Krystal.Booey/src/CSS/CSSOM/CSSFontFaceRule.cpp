#include "Krystal.Booey/CSS/CSSOM/CSSFontFaceRule.hpp"
#include "Krystal.Booey/CSS/Descriptors/CSSFontFaceDescriptors.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  CSSFontFaceRule::CSSFontFaceRule(FontFaceRule &fontFaceRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _fontFaceRule(ShareRef(fontFaceRule))
  {
  }

  CSSFontFaceRule::~CSSFontFaceRule() noexcept
  {
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->ClearParentRule();
    }
  }

  Ref<CSSFontFaceRule> CSSFontFaceRule::Create(FontFaceRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSFontFaceRule(rule, sheet));
  }

  CSSFontFaceDescriptors &CSSFontFaceRule::Style() noexcept
  {
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = CSSFontFaceDescriptors::Create(_fontFaceRule->MutableProperties(), *this);
    }
    return *_propertiesCSSOMWrapper;
  }

  CSSOMString CSSFontFaceRule::CssText() const noexcept
  {
    return CssTextInternal(_fontFaceRule->Properties().AsText(DefaultSerialisationContext()));
  }

  CSSOMString CSSFontFaceRule::CssText(const SerialisationContext &context) const noexcept
  {
    return CssTextInternal(_fontFaceRule->Properties().AsText(context));
  }

  CSSOMString CSSFontFaceRule::CssTextInternal(const CSSOMString &declarations) noexcept
  {
    if (declarations.empty())
    {
      return u8"@font-face { }";
    }

    CSSOMString result;
    result.append(u8"@font-face { ").append(declarations).append(u8" }");
    return result;
  }

  void CSSFontFaceRule::Reattach(Rule &rule) noexcept
  {
    _fontFaceRule = ShareRef(Downcast<FontFaceRule>(rule));
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->Reattach(_fontFaceRule->MutableProperties());
    }
  }
}
