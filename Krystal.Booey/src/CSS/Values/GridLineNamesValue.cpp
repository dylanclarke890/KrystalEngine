#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include <utility>

namespace krys::boo::css
{
  CSSOMString GridLineNamesValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    CSSOMString result;
    result.push_back(u8'[');
    bool first = true;
    for (const auto &name : _names)
    {
      if (!std::exchange(first, false))
      {
        result.push_back(u8' ');
      }
      SerialiseIdentifier(name, result);
    }
    result.push_back(u8']');
    return result;
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
