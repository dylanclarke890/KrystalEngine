#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/Predicates.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <cstddef>
#include <optional>

namespace Krys::Text::EncodingTable
{
  template <char32 TFailureCodePoint = 0xFFFFu, typename TTable>
  constexpr inline std::optional<std::uint_least32_t>
    SingleByteHighIndexToCodePoint(TTable &table, std::size_t lookupIndexPointer) noexcept
  {
    // standard lookup table
    auto first = ::std::ranges::cbegin(table);
    auto last = ::std::ranges::cend(table);

    using TIndexCodePoint = decltype(*first);
    using TIndex = decltype(*first[0]);
    const TIndex lookupIndex = static_cast<TIndex>(lookupIndexPointer);
    auto itAndLast =
      Krys::Ranges::lower_bound(first, last, lookupIndex, &LessThanIndexTarget<TIndexCodePoint>);

    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }
    const auto &indexAndCodepoint = *itAndLast.Current;
    if (indexAndCodepoint[0] != lookupIndex)
    {
      return std::nullopt;
    }
    if constexpr (TFailureCodePoint != 0)
    {
      if (indexAndCodepoint[0] == static_cast<TIndex>(TFailureCodePoint))
      {
        return std::nullopt;
      }
    }
    return static_cast<uint_least32_t>(indexAndCodepoint[1]);
  }

  template <char32 TFailureCodePoint = 0xFFFFu, typename TTable>
  constexpr inline std::optional<std::size_t>
    SingleByteHighCodePointToIndex(const TTable &table, std::uint_least32_t lookupCodePoint) noexcept
  {
    auto first = ::std::ranges::cbegin(table);
    auto last = ::std::ranges::cend(table);

    using TTableElement = remove_cvref_t<decltype(*first)>;
    auto predicate = [&lookupCodePoint](const TTableElement &value)
    {
      return lookupCodePoint == value[1];
    };
    auto itAndLast = Krys::Ranges::find_if(first, last, predicate);
    if (itAndLast.Current == itAndLast.Last)
    {
      return std::nullopt;
    }
    const TTableElement &indexAndCodepoint = *itAndLast.Current;
    if constexpr (TFailureCodePoint != 0)
    {
      if (indexAndCodepoint[0] == TFailureCodePoint)
      {
        return std::nullopt;
      }
    }
    return static_cast<std::size_t>(indexAndCodepoint[0]);
  }
}
