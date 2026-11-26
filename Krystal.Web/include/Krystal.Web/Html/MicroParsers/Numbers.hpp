#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Html/MicroParsers/MicroParserResult.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"

namespace krys::web::html
{
  /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#numbers
  class Numbers
  {
  public:
    using position_variable = infra::StringAlgorithms::position_variable;

    /// @note Integers outside `int64`'s range are an `OutOfRange` error, whose value is the nearest of
    /// `int64`'s limits.
    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#rules-for-parsing-integers
    KRYS_NODISCARD static MicroParserResult<int64> ParseInteger(DOMStringView input) noexcept;

    /// @note Integers above `int64`'s maximum are an `OutOfRange` error, whose value is that maximum.
    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#rules-for-parsing-non-negative-integers
    KRYS_NODISCARD static MicroParserResult<uint64> ParseNonNegativeInteger(DOMStringView input) noexcept;

    /// @brief Parses @p input only if all of it is a valid non-negative integer: one or more ASCII digits.
    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#valid-non-negative-integer
    KRYS_NODISCARD static MicroParserResult<uint64>
      ParseValidNonNegativeInteger(DOMStringView input) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#valid-floating-point-number
    KRYS_NODISCARD static bool IsValidFloatingPointNumber(DOMStringView input) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#rules-for-parsing-floating-point-number-values
    KRYS_NODISCARD static MicroParserResult<double> ParseFloatingPoint(DOMStringView input) noexcept;

    /// @brief Parses @p input only if all of it is a valid floating-point number.
    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#valid-floating-point-number
    KRYS_NODISCARD static MicroParserResult<double> ParseValidFloatingPoint(DOMStringView input) noexcept;

    /// @brief The numbers in @p input, separated by ASCII whitespace, commas or semicolons, as an image map
    /// area's `coords` gives them: anything after a number up to the next separator is ignored, and a part
    /// that doesn't start with a number is zero.
    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#rules-for-parsing-a-list-of-floating-point-numbers
    KRYS_NODISCARD static List<double> ParseListOfFloatingPointNumbers(DOMStringView input) noexcept;
  };
}
