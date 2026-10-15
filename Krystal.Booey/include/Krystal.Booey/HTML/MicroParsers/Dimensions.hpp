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

  /// @brief Parses the lengths of presentational attributes (`width`, `height`, `hspace`, ...).
  /// @note This is the legacy behaviour browsers share rather than the spec's rules for parsing dimension
  /// values: leading whitespace, digits with an optional fraction, then an optional `%`; anything after is
  /// ignored.
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

    KRYS_NODISCARD static Maybe<HTMLDimensionParsingResult>
      ParseHTMLDimensionNumber(dom::DOMStringView data) noexcept
    {
      if (data.empty())
      {
        return {};
      }

      size_t position = 0;
      while (position < data.size() && text::IsASCIIWhitespace(data[position]))
      {
        ++position;
      }
      if (position == data.size())
      {
        return {};
      }

      auto start = position;
      while (position < data.size() && text::IsASCIIDigit(data[position]))
      {
        ++position;
      }
      if (start == position)
      {
        return {};
      }

      if (position < data.size() && data[position] == '.')
      {
        ++position;
        while (position < data.size() && text::IsASCIIDigit(data[position]))
        {
          ++position;
        }
      }

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

      HTMLDimension dimension {.Number = result->Number, .DimensionType = HTMLDimension::Type::Pixel};
      if (result->ParsedLength < dimensionString.size() && dimensionString[result->ParsedLength] == '%')
      {
        dimension.DimensionType = HTMLDimension::Type::Percentage;
      }
      return dimension;
    }
  };
}
