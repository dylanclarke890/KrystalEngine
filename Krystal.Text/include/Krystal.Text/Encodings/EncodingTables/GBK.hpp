#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include "Krystal.Text/Encodings/EncodingTables/GB18030.tables.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/GenericLookup.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/Predicates.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/TableTypes.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>

namespace Krys::Text::EncodingTable
{
  KRYS_NODISCARD constexpr inline std::optional<std::uint_least32_t>
    gbk_index_to_code_point(std::size_t lookupIndexPointer) noexcept
  {
    return GenericIndexToCodePoint(Span<const index32_code_point>(GB18030RangesIndexCodePointMap),
                                   lookupIndexPointer);
  }

  KRYS_NODISCARD constexpr inline std::optional<std::size_t>
    gbk_code_point_to_index(std::uint_least32_t lookupCodePoint) noexcept
  {
    return GenericCodePointToIndex(Span<const index32_code_point>(GB18030RangesIndexCodePointMap),
                                   lookupCodePoint);
  }
}
