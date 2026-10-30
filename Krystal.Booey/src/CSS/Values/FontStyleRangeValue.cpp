#include "Krystal.Booey/CSS/Values/FontStyleRangeValue.hpp"

namespace krys::boo::css
{
  CSSOMString FontStyleRangeValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    RefPtr obliqueValues = ObliqueValues;
    Ref fontStyleValue = FontStyleValue;
    if (!obliqueValues)
    {
      return fontStyleValue->CssText(context);
    }

    CSSOMString result = fontStyleValue->CssText(context);
    result.push_back(u8' ');
    result.append(obliqueValues->CssText(context));
    return result;
  }

  bool FontStyleRangeValue::Equals(const FontStyleRangeValue &other) const noexcept
  {
    return CompareValue(FontStyleValue, other.FontStyleValue)
           && CompareValuePtr(ObliqueValues, other.ObliqueValues);
  }
}
