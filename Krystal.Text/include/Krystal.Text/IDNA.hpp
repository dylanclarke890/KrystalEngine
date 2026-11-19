#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::text
{
  /// @brief Converts a domain name to ASCII with UTS #46 processing, as the URL Standard's "domain to ASCII"
  /// does: it maps and validates each label (nontransitional, with CheckBidi and CheckJoiners) and encodes
  /// the non-ASCII ones as Punycode (`bücher.example` → `xn--bcher-kva.example`). The DNS length limits and
  /// the hyphen checks aren't applied.
  /// @return The ASCII domain name, or null if @p domain isn't valid.
  /// @see https://www.unicode.org/reports/tr46/#ToASCII
  KRYS_NODISCARD Maybe<utf8_string> DomainToASCII(utf8_stringview domain) noexcept;
}
