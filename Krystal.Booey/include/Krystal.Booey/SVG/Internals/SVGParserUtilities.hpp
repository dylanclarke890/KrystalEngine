#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <utility>

namespace krys::boo::svg
{
  // The parsers consume what they read from the front of `buffer`. SVG syntax is ASCII, so UTF-8 bytes can be
  // compared directly.

  enum class SuffixSkippingPolicy : uint8
  {
    DontSkip,
    Skip
  };

  KRYS_NODISCARD Maybe<float> ParseNumber(dom::DOMStringView &buffer,
                                          SuffixSkippingPolicy skip = SuffixSkippingPolicy::Skip) noexcept;

  /// @brief Parses the whole of `string` as a number.
  KRYS_NODISCARD Maybe<float>
    ParseNumberFromString(dom::DOMStringView string,
                          SuffixSkippingPolicy skip = SuffixSkippingPolicy::Skip) noexcept;

  KRYS_NODISCARD Maybe<std::pair<float, float>> ParseNumberOptionalNumber(dom::DOMStringView string) noexcept;

  KRYS_NODISCARD Maybe<bool> ParseArcFlag(dom::DOMStringView &buffer) noexcept;

  KRYS_NODISCARD Maybe<FloatPoint> ParsePoint(dom::DOMStringView string) noexcept;
  KRYS_NODISCARD Maybe<FloatRect> ParseRect(dom::DOMStringView string) noexcept;

  KRYS_NODISCARD Maybe<FloatPoint> ParseFloatPoint(dom::DOMStringView &buffer) noexcept;

  template <typename TChar>
  KRYS_NODISCARD constexpr bool IsSVGSpaceOrComma(TChar c) noexcept
  {
    return text::IsASCIIWhitespace(c) || c == ',';
  }

  constexpr bool SkipOptionalSVGSpaces(dom::DOMStringView &characters) noexcept
  {
    while (!characters.empty() && text::IsASCIIWhitespace(characters.front()))
    {
      characters.remove_prefix(1);
    }
    return !characters.empty();
  }

  constexpr bool SkipOptionalSVGSpacesOrDelimiter(dom::DOMStringView &characters,
                                                  char delimiter = ',') noexcept
  {
    if (characters.empty())
    {
      return false;
    }

    if (!text::IsASCIIWhitespace(characters.front()) && characters.front() != delimiter)
    {
      return false;
    }

    // There are only spaces in the remaining characters.
    if (!SkipOptionalSVGSpaces(characters))
    {
      return false;
    }

    if (characters.front() != delimiter)
    {
      return true;
    }

    // A delimiter is hit. Skip the following spaces also e.g. " , ".
    characters.remove_prefix(1);
    return SkipOptionalSVGSpaces(characters);
  }
}
