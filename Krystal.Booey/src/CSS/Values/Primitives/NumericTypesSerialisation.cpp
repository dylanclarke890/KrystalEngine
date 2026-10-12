#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <charconv>
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

    /// @brief Like printf("%.6f"), with trailing zeros (and a trailing decimal point) removed, and "-0" as "0".
    void AppendFormattedCSSNumber(CSSOMString &builder, double number) noexcept
    {
      Array<char, 512uz> buffer;
      auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), number, std::chars_format::fixed, 6);
      krys_debug_assert(result.ec == std::errc {});

      std::string_view formatted(buffer.data(), result.ptr);
      if (formatted.find('.') != std::string_view::npos)
      {
        while (formatted.ends_with('0'))
        {
          formatted.remove_suffix(1);
        }
        if (formatted.ends_with('.'))
        {
          formatted.remove_suffix(1);
        }
      }

      if (formatted == "-0")
      {
        formatted = "0";
      }

      builder.append(formatted.begin(), formatted.end());
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

    AppendFormattedCSSNumber(builder, number.Value);
    builder.append(number.Suffix);
  }

  CSSOMString FormatCSSNumberValue(const SerializableNumber &number) noexcept
  {
    CSSOMString builder;
    FormatCSSNumberValue(builder, number);
    return builder;
  }
}
