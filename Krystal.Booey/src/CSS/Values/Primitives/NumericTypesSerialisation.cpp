#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Core/Assert.hpp"
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

  void FormatNonfiniteCSSNumberValue(CSSOMString &builder, const SerializableNumber &number) noexcept
  {
    builder.append(FormatNonfiniteCSSNumberValuePrefix(number.Value));
    if (!number.Suffix.empty())
    {
      builder.append(u8" * 1");
    }
    builder.append(number.Suffix);
  }

  CSSOMString FormatNonfiniteCSSNumberValue(const SerializableNumber &number) noexcept
  {
    CSSOMString builder;
    FormatNonfiniteCSSNumberValue(builder, number);
    return builder;
  }

  void FormatCSSNumberValue(CSSOMString &builder, const SerializableNumber &number) noexcept
  {
    if (!std::isfinite(number.Value))
    {
      FormatNonfiniteCSSNumberValue(builder, number);
      return;
    }

    FormatCSSNumber(builder, number.Value);
    builder.append(number.Suffix);
  }

  CSSOMString FormatCSSNumberValue(const SerializableNumber &number) noexcept
  {
    CSSOMString builder;
    FormatCSSNumberValue(builder, number);
    return builder;
  }
}
