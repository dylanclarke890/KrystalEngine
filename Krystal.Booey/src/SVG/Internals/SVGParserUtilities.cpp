#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>

namespace krys::boo::svg
{
  namespace
  {
    template <typename TFloat>
    KRYS_NODISCARD bool IsValidRange(const TFloat x) noexcept
    {
      constexpr TFloat max = Limits<TFloat>::max();
      return x >= -max && x <= max;
    }

    KRYS_NODISCARD bool StartsWithDigit(const dom::DOMStringView &buffer) noexcept
    {
      return !buffer.empty() && text::IsASCIIDigit(buffer.front());
    }

    // We use this generic parseNumber function to allow the Path parsing code to work
    // at a higher precision internally, without any unnecessary runtime cost or code
    // complexity.
    // FIXME: Can this be shared/replaced with number parsing in WTF?
    template <typename TFloat = float>
    KRYS_NODISCARD Maybe<TFloat>
      GenericParseNumber(dom::DOMStringView &buffer,
                         SuffixSkippingPolicy skip = SuffixSkippingPolicy::Skip) noexcept
    {
      // read the sign
      int sign = 1;
      if (!buffer.empty() && buffer.front() == '+')
      {
        buffer.remove_prefix(1);
      }
      else if (!buffer.empty() && buffer.front() == '-')
      {
        buffer.remove_prefix(1);
        sign = -1;
      }

      if (buffer.empty() || (!text::IsASCIIDigit(buffer.front()) && buffer.front() != '.'))
      {
        return {};
      }

      // read the integer part, build right-to-left
      auto spanDigitsStart = buffer;

      // Advance to first non-digit.
      while (StartsWithDigit(buffer))
      {
        buffer.remove_prefix(1);
      }

      TFloat integer = 0;
      size_t indexScanIntPart = spanDigitsStart.size() - buffer.size();
      if (indexScanIntPart > 0)
      {
        TFloat multiplier = 1;
        for (size_t i = indexScanIntPart; i > 0; --i)
        {
          integer += multiplier * static_cast<TFloat>(spanDigitsStart[i - 1] - '0');
          multiplier *= 10;
        }
        // Bail out early if this overflows.
        if (!IsValidRange(integer))
        {
          return {};
        }
      }

      // read the decimals
      TFloat decimal = 0;
      if (!buffer.empty() && buffer.front() == '.')
      {
        buffer.remove_prefix(1);

        // There must be a least one digit following the .
        if (!StartsWithDigit(buffer))
        {
          return {};
        }

        TFloat frac = 1;
        while (StartsWithDigit(buffer))
        {
          frac *= static_cast<TFloat>(0.1);
          decimal += (buffer.front() - '0') * frac;
          buffer.remove_prefix(1);
        }
      }

      // When we get here we should have consumed either a digit for the integer
      // part or a fractional part (with at least one digit after the '.'.)
      krys_debug_assert(spanDigitsStart.size() != buffer.size());

      TFloat number = 0;
      number = integer + decimal;
      number *= sign;

      // read the exponent part
      if (buffer.size() > 1 && (buffer[0] == 'e' || buffer[0] == 'E')
          && (buffer[1] != 'x' && buffer[1] != 'm'))
      {
        buffer.remove_prefix(1);

        // read the sign of the exponent
        bool exponentIsNegative = false;
        if (buffer.front() == '+')
        {
          buffer.remove_prefix(1);
        }
        else if (buffer.front() == '-')
        {
          buffer.remove_prefix(1);
          exponentIsNegative = true;
        }

        // There must be an exponent
        if (!StartsWithDigit(buffer))
        {
          return {};
        }

        TFloat exponent = 0;
        while (StartsWithDigit(buffer))
        {
          exponent *= static_cast<TFloat>(10);
          exponent += buffer.front() - '0';
          buffer.remove_prefix(1);
        }

        // FIXME: This is unnecessairly strict - the position of the decimal point
        // is not taken into account when limiting 'exponent'.
        if (exponentIsNegative)
        {
          exponent = -exponent;
        }

        // Fail if the exponent is greater than the largest positive power of ten
        // (that would yield a representable float).
        if (exponent > Limits<TFloat>::max_exponent10)
        {
          return {};
        }

        // If the exponent is smaller than smallest negative power of 10 (that
        // would yield a representable float), then rely on the pow()+rounding to
        // produce a reasonable result (likely zero.)
        if (exponent)
        {
          number *= static_cast<TFloat>(std::pow(10.0, exponent));
        }
      }

      // Don't return Infinity() or NaN().
      if (!IsValidRange(number))
      {
        return {};
      }

      if (skip == SuffixSkippingPolicy::Skip)
      {
        SkipOptionalSVGSpacesOrDelimiter(buffer);
      }

      return number;
    }
  }

  Maybe<float> ParseNumber(dom::DOMStringView &buffer, SuffixSkippingPolicy skip) noexcept
  {
    return GenericParseNumber(buffer, skip);
  }

  Maybe<float> ParseNumberFromString(dom::DOMStringView string, SuffixSkippingPolicy skip) noexcept
  {
    auto result = GenericParseNumber(string, skip);
    if (!string.empty())
    {
      return {};
    }
    return result;
  }

  // only used to parse largeArcFlag and sweepFlag which must be a "0" or "1"
  // and might not have any whitespace/comma after it
  Maybe<bool> ParseArcFlag(dom::DOMStringView &buffer) noexcept
  {
    if (buffer.empty())
    {
      return {};
    }

    const auto flagChar = buffer.front();
    buffer.remove_prefix(1);

    bool flag;
    if (flagChar == '0')
    {
      flag = false;
    }
    else if (flagChar == '1')
    {
      flag = true;
    }
    else
    {
      return {};
    }

    SkipOptionalSVGSpacesOrDelimiter(buffer);

    return flag;
  }

  Maybe<std::pair<float, float>> ParseNumberOptionalNumber(dom::DOMStringView string) noexcept
  {
    if (string.empty())
    {
      return {};
    }

    auto x = ParseNumber(string);
    if (!x)
    {
      return {};
    }

    if (string.empty())
    {
      return std::make_pair(*x, *x);
    }

    auto y = ParseNumber(string, SuffixSkippingPolicy::DontSkip);
    if (!y)
    {
      return {};
    }

    if (!string.empty())
    {
      return {};
    }

    return std::make_pair(*x, *y);
  }

  Maybe<FloatPoint> ParsePoint(dom::DOMStringView string) noexcept
  {
    if (string.empty())
    {
      return {};
    }

    if (!SkipOptionalSVGSpaces(string))
    {
      return {};
    }

    auto point = ParseFloatPoint(string);
    if (!point)
    {
      return {};
    }

    // Disallow anything except spaces at the end.
    SkipOptionalSVGSpaces(string);

    return point;
  }

  Maybe<FloatRect> ParseRect(dom::DOMStringView string) noexcept
  {
    SkipOptionalSVGSpaces(string);

    auto x = ParseNumber(string);
    if (!x)
    {
      return {};
    }
    auto y = ParseNumber(string);
    if (!y)
    {
      return {};
    }
    auto width = ParseNumber(string);
    if (!width)
    {
      return {};
    }
    auto height = ParseNumber(string, SuffixSkippingPolicy::DontSkip);
    if (!height)
    {
      return {};
    }

    return FloatRect {*x, *y, *width, *height};
  }

  Maybe<FloatPoint> ParseFloatPoint(dom::DOMStringView &buffer) noexcept
  {
    auto x = ParseNumber(buffer);
    if (!x)
    {
      return {};
    }

    auto y = ParseNumber(buffer);
    if (!y)
    {
      return {};
    }

    return FloatPoint {*x, *y};
  }
}
