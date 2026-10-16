#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD CSSOMStringView FormatNonfiniteCSSNumberValuePrefix(double number) noexcept
    {
      if (number == std::numeric_limits<double>::infinity())
      {
        return u8"infinity";
      }
      if (number == -std::numeric_limits<double>::infinity())
      {
        return u8"-infinity";
      }
      krys_debug_assert(std::isnan(number));
      return u8"NaN";
    }
  }

  void FormatNonfiniteCSSNumberValue(StringBuilder &builder, const SerialisableNumber &number) noexcept
  {
    builder.Append(FormatNonfiniteCSSNumberValuePrefix(number.Value));
    if (!number.Suffix.empty())
    {
      builder.Append(u8" * 1");
    }
    builder.Append(number.Suffix);
  }

  CSSOMString FormatNonfiniteCSSNumberValue(const SerialisableNumber &number) noexcept
  {
    StringBuilder builder;
    FormatNonfiniteCSSNumberValue(builder, number);
    return krys::move(builder).ToString();
  }

  void FormatCSSNumberValue(StringBuilder &builder, const SerialisableNumber &number) noexcept
  {
    if (!std::isfinite(number.Value))
    {
      FormatNonfiniteCSSNumberValue(builder, number);
      return;
    }

    FormatCSSNumber(builder, number.Value);
    builder.Append(number.Suffix);
  }

  CSSOMString FormatCSSNumberValue(const SerialisableNumber &number) noexcept
  {
    StringBuilder builder;
    FormatCSSNumberValue(builder, number);
    return krys::move(builder).ToString();
  }
}
