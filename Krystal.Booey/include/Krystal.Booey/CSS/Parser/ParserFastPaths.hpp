#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Core/Color/Color.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class Value;

  struct PropertyParserState;
  struct Range;
  struct ParserContext;

  class ParserFastPaths
  {
  public:
    KRYS_NODISCARD static Maybe<Value> MaybeParseValue(PropertyId property, CSSOMStringView string,
                                                       PropertyParserState &state) noexcept;

    /// @brief Returns the allowed numeric value range for a length value if the property supports a single
    /// length value.
    /// FIXME: This should be generated using the codegen scripts.
    KRYS_NODISCARD static Maybe<Range>
      LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId id) noexcept;

    /// @brief Parses numeric and named colors.
    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseSimpleColor(CSSOMStringView color,
                                                               const ParserContext &context) noexcept;

    /// @brief Hex colors of length 3, 4, 6, or 8, without leading "#".
    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseHexColor(CSSOMStringView color) noexcept;

    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseNamedColor(CSSOMStringView color) noexcept;
  };
}