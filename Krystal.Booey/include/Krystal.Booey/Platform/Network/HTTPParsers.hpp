#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <chrono>

namespace krys::boo
{
  /// @brief The time an HTTP-date names (`Last-Modified`, `Date`): an IMF-fixdate
  /// (`Sun, 06 Nov 1994 08:49:37 GMT`), or the obsolete RFC 850 (`Sunday, 06-Nov-94 08:49:37 GMT`) or asctime
  /// (`Sun Nov  6 08:49:37 1994`) forms. None for anything else.
  /// @see https://httpwg.org/specs/rfc9110.html#http.date
  KRYS_NODISCARD Maybe<std::chrono::sys_seconds> ParseHTTPDate(utf8_stringview value) noexcept;

  /// @brief Whether `X-Content-Type-Options` @p value says `nosniff`: its first value is `nosniff`.
  /// @see https://fetch.spec.whatwg.org/#determine-nosniff
  KRYS_NODISCARD bool IsNosniffContentTypeOptions(utf8_stringview value) noexcept;
}
