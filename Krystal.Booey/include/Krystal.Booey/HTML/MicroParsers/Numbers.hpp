#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/MicroParsers/MicroParserResult.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#numbers
  class Numbers
  {
  public:
    using position_variable = infra::StringAlgorithms::position_variable;

    /// @note Integers outside `int64`'s range are an `OutOfRange` error.
    /// @see https://html.spec.whatwg.org/#rules-for-parsing-integers
    KRYS_NODISCARD static MicroParserResult<int64> ParseInteger(dom::DOMStringView input) noexcept;

    /// @see https://html.spec.whatwg.org/#rules-for-parsing-non-negative-integers
    KRYS_NODISCARD static MicroParserResult<uint64>
      ParseNonNegativeInteger(dom::DOMStringView input) noexcept;

    /// @brief Parses @p input only if all of it is a valid non-negative integer: one or more ASCII digits.
    /// @see https://html.spec.whatwg.org/#valid-non-negative-integer
    KRYS_NODISCARD static MicroParserResult<uint64>
      ParseValidNonNegativeInteger(dom::DOMStringView input) noexcept;

    /// @see https://html.spec.whatwg.org/#valid-floating-point-number
    KRYS_NODISCARD static bool IsValidFloatingPointNumber(dom::DOMStringView input) noexcept;

    /// @see https://html.spec.whatwg.org/#rules-for-parsing-floating-point-number-values
    KRYS_NODISCARD static MicroParserResult<double> ParseFloatingPoint(dom::DOMStringView input) noexcept;

    /// @brief Parses @p input only if all of it is a valid floating-point number.
    /// @see https://html.spec.whatwg.org/#valid-floating-point-number
    KRYS_NODISCARD static MicroParserResult<double>
      ParseValidFloatingPoint(dom::DOMStringView input) noexcept;

    /// @brief The numbers in @p input, separated by ASCII whitespace, commas or semicolons, as an image map
    /// area's `coords` gives them: anything after a number up to the next separator is ignored, and a part
    /// that doesn't start with a number is zero.
    /// @see https://html.spec.whatwg.org/#rules-for-parsing-a-list-of-floating-point-numbers
    KRYS_NODISCARD static List<double> ParseListOfFloatingPointNumbers(dom::DOMStringView input) noexcept;
  };
}
