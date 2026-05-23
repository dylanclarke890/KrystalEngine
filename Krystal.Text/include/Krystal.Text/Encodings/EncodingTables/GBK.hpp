#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/Encodings/EncodingTables/GB18030.hpp"
#include "Krystal.Text/Encodings/EncodingTables/Utils/GenericLookup.hpp"

namespace Krys::Text::EncodingTable
{
  KRYS_NODISCARD constexpr Maybe<uint32> GBKIndexToCodePoint(size_t lookupIndexPointer) noexcept
  {
    return GenericIndexToCodePoint(Span<const index32_code_point>(GB18030RangesIndexCodePointMap),
                                   lookupIndexPointer);
  }

  KRYS_NODISCARD constexpr Maybe<size_t> GBKCodePointToIndex(uint32 lookupCodePoint) noexcept
  {
    return GenericCodePointToIndex(Span<const index32_code_point>(GB18030RangesIndexCodePointMap),
                                   lookupCodePoint);
  }
}
