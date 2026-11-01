#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <charconv>
#include <cmath>

namespace krys::boo::html
{
  /// @brief A length from a presentational attribute: a number of pixels or a percentage.
  struct HTMLDimension
  {
    enum class Type : bool
    {
      Percentage,
      Pixel
    };

    double Number;
    Type DimensionType;
  };

  /// @brief Parses the lengths of presentational attributes (`width`, `height`, `hspace`, ...): leading
  /// whitespace, digits with an optional fraction, then an optional `%`; anything after is ignored.
  /// @see https://html.spec.whatwg.org/#percentages-and-dimensions
  class Dimensions
  {
    enum class IsMultiLength : bool
    {
      No,
      Yes
    };

  public:
    /// @see https://html.spec.whatwg.org/#rules-for-parsing-dimension-values
    KRYS_NODISCARD static Maybe<HTMLDimension> ParseHTMLDimension(dom::DOMStringView dimensionString) noexcept
    {
      return ParseHTMLDimensionInternal(dimensionString, IsMultiLength::No);
    }

    /// @brief As `ParseHTMLDimension`, but rejects relative lengths (`2*`).
    KRYS_NODISCARD static Maybe<HTMLDimension>
      ParseHTMLMultiLength(dom::DOMStringView multiLengthString) noexcept
    {
      return ParseHTMLDimensionInternal(multiLengthString, IsMultiLength::Yes);
    }

  private:
    struct HTMLDimensionParsingResult
    {
      double Number;
      size_t ParsedLength;
    };

    /// @brief Steps 1 to 7 of the rules for parsing dimension values, less the current dimension value: the
    /// value, and the position the current dimension value is given.
    KRYS_NODISCARD static Maybe<HTMLDimensionParsingResult>
      ParseHTMLDimensionNumber(dom::DOMStringView data) noexcept
    {
      // 1. Let input be the string being parsed.
      // 2. Let position be a position variable for input, initially pointing at the start of input.
      size_t position = 0;

      // 3. Skip ASCII whitespace within input given position.
      while (position < data.size() && text::IsASCIIWhitespace(data[position]))
      {
        ++position;
      }

      // 4. If position is past the end of input or the code point at position within input is not an ASCII
      //    digit, then return failure.
      if (position == data.size() || !text::IsASCIIDigit(data[position]))
      {
        return {};
      }

      // 5. Collect a sequence of code points that are ASCII digits from input given position, and interpret
      //    the resulting sequence as a base-ten integer. Let value be that number.
      auto start = position;
      while (position < data.size() && text::IsASCIIDigit(data[position]))
      {
        ++position;
      }

      // 6. If position is past the end of input, then return value as a length.
      // 7. If the code point at position within input is U+002E (.):
      if (position < data.size() && data[position] == '.')
      {
        // 7a. Advance position by 1.
        ++position;

        // 7b. If position is past the end of input or the code point at position within input is not an
        //     ASCII digit, then return the current dimension value with value, input, and position.
        // 7c. Let divisor have the value 1.
        // 7d. While true:
        // 7d.i. Multiply divisor by ten.
        // 7d.ii. Add the value of the code point at position within input, interpreted as a base-ten digit
        //        (0..9) and divided by divisor, to value.
        // 7d.iii. Advance position by 1.
        // 7d.iv. If position is past the end of input, then return value as a length.
        // 7d.v. If the code point at position within input is not an ASCII digit, then break.
        while (position < data.size() && text::IsASCIIDigit(data[position]))
        {
          ++position;
        }
      }

      // The digits and fraction are converted at once, rounding the exact value.
      const auto *first = reinterpret_cast<const char *>(data.data() + start);
      const auto *last = reinterpret_cast<const char *>(data.data() + position);
      double number {};
      auto result = std::from_chars(first, last, number, std::chars_format::fixed);
      if (result.ptr == first || !std::isfinite(number))
      {
        return {};
      }

      return HTMLDimensionParsingResult {.Number = number, .ParsedLength = position};
    }

    /// @see https://html.spec.whatwg.org/#current-dimension-value
    KRYS_NODISCARD static Maybe<HTMLDimension>
      ParseHTMLDimensionInternal(dom::DOMStringView dimensionString, IsMultiLength isMultiLength) noexcept
    {
      auto result = ParseHTMLDimensionNumber(dimensionString);
      if (!result)
      {
        return {};
      }

      // The relative_length is not supported, here to make sure number + * does not map to number
      if (isMultiLength == IsMultiLength::Yes && result->ParsedLength < dimensionString.size()
          && dimensionString[result->ParsedLength] == '*')
      {
        return {};
      }

      // 8. Return the current dimension value with value, input, and position.
      // The current dimension value:
      // 1. If position is past the end of input, then return value as a length.
      // 2. If the code point at position within input is U+0025 (%), then return value as a percentage.
      // 3. Return value as a length.
      HTMLDimension dimension {.Number = result->Number, .DimensionType = HTMLDimension::Type::Pixel};
      if (result->ParsedLength < dimensionString.size() && dimensionString[result->ParsedLength] == '%')
      {
        dimension.DimensionType = HTMLDimension::Type::Percentage;
      }
      return dimension;
    }
  };
}
