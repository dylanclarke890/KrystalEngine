#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/Predicates.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <algorithm>
#include <ranges>

namespace Krys::Text::EncodingTable
{
  template <char32 TInvalidCodePoint = 0xFFFFu, typename TTable>
  KRYS_NODISCARD constexpr Maybe<uint32> SingleByteHighIndexToCodePoint(TTable &table,
                                                                        size_t lookupIndexPointer) noexcept
  {
    auto lookupIndex = static_cast<const decltype(*std::ranges::cbegin(table)[0])>(lookupIndexPointer);
    auto begin = std::ranges::cbegin(table);
    auto end = std::ranges::cend(table);
    auto it = std::lower_bound(begin, end, lookupIndex, &LessThanIndexTarget<decltype(*begin)>);

    if (it == end)
    {
      return std::nullopt;
    }

    auto &indexAndCodepoint = *it;
    if (indexAndCodepoint[0] != lookupIndex)
    {
      return std::nullopt;
    }

    if constexpr (TInvalidCodePoint != 0)
    {
      if (indexAndCodepoint[0] == TInvalidCodePoint)
      {
        return std::nullopt;
      }
    }

    return static_cast<uint32>(indexAndCodepoint[1]);
  }

  template <char32 TInvalidCodePoint = 0xFFFFu, typename TTable>
  KRYS_NODISCARD constexpr inline Maybe<size_t>
    SingleByteHighCodePointToIndex(const TTable &table, uint32 lookupCodePoint) noexcept
  {
    auto begin = std::ranges::cbegin(table);
    auto end = std::ranges::cend(table);
    auto it = std::find_if(begin, end, [&](const auto &value) { return lookupCodePoint == value[1]; });
    if (it == end)
    {
      return std::nullopt;
    }

    auto &indexAndCodepoint = *it;
    if constexpr (TInvalidCodePoint != 0)
    {
      if (indexAndCodepoint[0] == TInvalidCodePoint)
      {
        return std::nullopt;
      }
    }

    return static_cast<size_t>(indexAndCodepoint[0]);
  }
}
