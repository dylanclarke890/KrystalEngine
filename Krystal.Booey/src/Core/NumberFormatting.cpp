#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <charconv>
#include <cmath>
#include <string_view>

namespace krys::boo
{
  void FormatCSSNumber(utf8_string &builder, double number) noexcept
  {
    if (std::isnan(number))
    {
      builder.append(u8"NaN");
      return;
    }

    if (std::isinf(number))
    {
      builder.append(number < 0 ? u8"-infinity" : u8"infinity");
      return;
    }

    Array<char, 512uz> buffer;
    auto result =
      std::to_chars(buffer.data(), buffer.data() + buffer.size(), number, std::chars_format::fixed, 6);
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

    // If we've truncated the trailing zeros and a trailing decimal, we may have a -0. Remove the negative
    // sign in this case.
    if (formatted == "-0")
    {
      formatted.remove_prefix(1);
    }

    builder.append(formatted.begin(), formatted.end());
  }

  utf8_string FormatCSSNumber(double number) noexcept
  {
    utf8_string builder;
    FormatCSSNumber(builder, number);
    return builder;
  }
}
