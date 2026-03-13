#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/Predicates.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>

namespace Krys::Text::EncodingTable
{
  template <typename TIndexCodePoint = index32_code_point,
            typename TIndex = remove_cvref_t<decltype(std::declval<TIndexCodePoint>()[0])>>
  KRYS_NODISCARD constexpr inline Maybe<std::size_t>
    GenericCodePointToIndex(Span<const TIndexCodePoint> indexCodePointMap,
                            std::uint_least32_t lookupCodePoint) noexcept
  {
    const TIndex lookupCode = static_cast<TIndex>(lookupCodePoint);

    auto first = ::std::ranges::cbegin(indexCodePointMap);
    auto last = ::std::ranges::cend(indexCodePointMap);

    const auto predicate = [&lookupCode](const TIndexCodePoint &value)
    {
      return lookupCode == value[1];
    };

    auto it = std::find_if(first, last, predicate);
    if (it == last)
    {
      return std::nullopt;
    }

    const auto &indexAndCodepoint = *it;
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }

  template <typename TIndexCodePoint = index32_code_point,
            typename TIndex = remove_cvref_t<decltype(std::declval<TIndexCodePoint>()[0])>>
  KRYS_NODISCARD constexpr inline Maybe<std::uint_least32_t>
    GenericIndexToCodePoint(Span<const TIndexCodePoint> indexCodePointMap,
                            std::size_t lookupIndexPointer) noexcept
  {
    TIndex lookupIndex = static_cast<TIndex>(lookupIndexPointer);

    auto first = ::std::ranges::cbegin(indexCodePointMap);
    auto last = ::std::ranges::cend(indexCodePointMap);
    auto it =
      Krys::Ranges::lower_bound(first, last, lookupIndex, &LessThanIndexTarget<TIndexCodePoint, TIndex>);

    if (it.Current == it.Last)
    {
      return std::nullopt;
    }

    const TIndexCodePoint &indexAndCodepoint = *it.Current;
    if (indexAndCodepoint[0] != lookupIndex)
    {
      return std::nullopt;
    }

    return static_cast<std::uint_least32_t>(indexAndCodepoint[1]);
  }
}
