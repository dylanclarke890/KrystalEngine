/*
 * Copyright (C) 2002, 2003 The Karbon Developers
 * Copyright (C) 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2013-2024 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
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

  /// @brief The number `string` is, after any leading whitespace; 0 if it isn't one.
  KRYS_NODISCARD float ToFloatOrZero(dom::DOMStringView string) noexcept;
  /// @brief The number @p string spells, after any leading whitespace; null if anything else follows it.
  KRYS_NODISCARD Maybe<double> ToDouble(dom::DOMStringView string) noexcept;

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
