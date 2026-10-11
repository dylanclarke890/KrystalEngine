#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"

namespace krys::boo::css
{
  Ref<BorderImageWidthValue> BorderImageWidthValue::Create(Quad widths, bool overridesBorderWidths) noexcept
  {
    return AdoptRef(*new BorderImageWidthValue(krys::move(widths), overridesBorderWidths));
  }

  CSSOMString BorderImageWidthValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    // The border-image-width longhand can't set m_overridesBorderWidths to true, so serialize as empty
    // string.
    // This can only be created by the -webkit-border-image shorthand, which will not serialize as empty
    // string in this case. This is an unconventional relationship between a longhand and a shorthand, which
    // we may want to revise.
    if (_overridesBorderWidths)
    {
      return CSSOMString();
    }

    return _widths.CssText(context);
  }

  bool BorderImageWidthValue::Equals(const BorderImageWidthValue &other) const noexcept
  {
    return _widths.Equals(other._widths) && _overridesBorderWidths == other._overridesBorderWidths;
  }
}