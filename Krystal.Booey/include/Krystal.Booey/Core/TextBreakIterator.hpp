#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief The number of extended grapheme clusters in @p string.
  /// @see https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
  KRYS_NODISCARD uint32 NumGraphemeClusters(utf8_stringview string) noexcept;
}
