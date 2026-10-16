#include "Krystal.Booey/CSS/Values/UnicodeRangeValue.hpp"
#include "Krystal.Core/Text/HexNumber.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CSSOMString UnicodeRangeValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    StringBuilder result;
    result.Append(u8"U+", Hex(_from, HexConversionMode::Lowercase));
    if (_from != _to)
    {
      result.Append(u8'-', Hex(_to, HexConversionMode::Lowercase));
    }
    return krys::move(result).ToString();
  }

  bool UnicodeRangeValue::Equals(const UnicodeRangeValue &other) const noexcept
  {
    return _from == other._from && _to == other._to;
  }
}
