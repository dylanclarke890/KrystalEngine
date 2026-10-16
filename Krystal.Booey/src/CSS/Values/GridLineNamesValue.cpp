#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <utility>

namespace krys::boo::css
{
  CSSOMString GridLineNamesValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    StringBuilder result;
    result.Append(u8'[');
    bool first = true;
    for (const auto &name : _names)
    {
      if (!std::exchange(first, false))
      {
        result.Append(u8' ');
      }
      SerialiseIdentifier(name, result);
    }
    result.Append(u8']');
    return krys::move(result).ToString();
  }

  GridLineNamesValue::GridLineNamesValue(Span<const CSSOMString> names) noexcept
      : Value(ValueType::GridLineNames), _names(names.begin(), names.end())
  {
  }

  Ref<GridLineNamesValue> GridLineNamesValue::Create(Span<const CSSOMString> names) noexcept
  {
    return AdoptRef(*new GridLineNamesValue(names));
  }
}
