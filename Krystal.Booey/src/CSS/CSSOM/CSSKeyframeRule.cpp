#include "Krystal.Booey/CSS/CSSOM/CSSKeyframeRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSKeyframesRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  CSSKeyframeRule::CSSKeyframeRule(KeyframeRule &keyframe, CSSKeyframesRule *parent) noexcept
      : CSSRule(nullptr), _keyframe(ShareRef(keyframe))
  {
    SetParentRule(parent);
  }

  CSSKeyframeRule::~CSSKeyframeRule() noexcept
  {
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->ClearParentRule();
    }
  }

  CSSOMString CSSKeyframeRule::CssText() const noexcept
  {
    return _keyframe->CssText();
  }

  CSSOMString CSSKeyframeRule::KeyText() const noexcept
  {
    return _keyframe->KeyText();
  }

  void CSSKeyframeRule::SetKeyText(const CSSOMString &text) noexcept
  {
    std::ignore = _keyframe->SetKeyText(text);
  }

  CSSStyleProperties &CSSKeyframeRule::Style() noexcept
  {
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = StyleRuleCSSStyleProperties::Create(_keyframe->MutableProperties(), *this);
    }
    return *_propertiesCSSOMWrapper;
  }

  void CSSKeyframeRule::Reattach(Rule &) noexcept
  {
    // No need to reattach, the underlying data is shareable on mutation.
    krys_debug_assert(false);
  }
}
