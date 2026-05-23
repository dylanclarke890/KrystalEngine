#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/Predicates.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <algorithm>
#include <ranges>

namespace Krys::Text::EncodingTable
{
  template <typename TIndexCodePoint,
            typename TIndex = remove_cvref_t<decltype(std::declval<TIndexCodePoint>()[0])>>
  KRYS_NODISCARD constexpr Maybe<size_t>
    GenericCodePointToIndex(Span<const TIndexCodePoint> indexCodePointMap, uint32 lookupCodePoint) noexcept
  {
    auto lookupCode = static_cast<const TIndex>(lookupCodePoint);
    auto begin = std::ranges::cbegin(indexCodePointMap);
    auto end = std::ranges::cend(indexCodePointMap);
    auto it = std::find_if(begin, end, [&](const auto &value) { return lookupCode == value[1]; });

    if (it == end)
    {
      return std::nullopt;
    }

    auto &indexAndCodepoint = *it;
    return static_cast<size_t>(indexAndCodepoint[0]);
  }

  template <typename TIndexCodePoint = index32_code_point,
            typename TIndex = remove_cvref_t<decltype(std::declval<TIndexCodePoint>()[0])>>
  KRYS_NODISCARD constexpr Maybe<uint32>
    GenericIndexToCodePoint(Span<const TIndexCodePoint> indexCodePointMap, size_t lookupIndexPointer) noexcept
  {
    auto lookupIndex = static_cast<TIndex>(lookupIndexPointer);
    auto begin = std::ranges::cbegin(indexCodePointMap);
    auto end = std::ranges::cend(indexCodePointMap);
    auto it = std::lower_bound(begin, end, lookupIndex, &LessThanIndexTarget<TIndexCodePoint, TIndex>);

    if (it == end)
    {
      return std::nullopt;
    }

    auto &indexAndCodepoint = *it;
    if (indexAndCodepoint[0] != lookupIndex)
    {
      return std::nullopt;
    }

    return static_cast<uint32>(indexAndCodepoint[1]);
  }
}
