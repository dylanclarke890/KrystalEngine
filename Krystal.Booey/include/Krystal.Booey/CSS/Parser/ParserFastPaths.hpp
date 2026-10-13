#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Color/ColorTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class Value;

  struct PropertyParserState;
  struct Range;
  struct ParserContext;

  class ParserFastPaths
  {
  public:
    /// @brief Parses simple values like '10px' or 'green', but makes no guarantees about handling any
    /// property completely.
    /// TODO: Not defined yet; it needs `PrimitiveValue`/`ValuePool` to link.
    KRYS_NODISCARD static RefPtr<Value> MaybeParseValue(PropertyId property, CSSOMStringView string,
                                                        PropertyParserState &state) noexcept;

    /// @brief Returns the allowed numeric value range for a length value if the property supports a single
    /// length value.
    /// FIXME: This should be generated using the codegen scripts.
    KRYS_NODISCARD static Maybe<Range>
      LengthValueRangeForPropertiesSupportingSimpleLengths(PropertyId id) noexcept;

    /// @brief Parses numeric and named colors.
    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseSimpleColor(CSSOMStringView string,
                                                               const ParserContext &context) noexcept;

    /// @brief Hex colors of length 3, 4, 6, or 8, without leading "#".
    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseHexColor(CSSOMStringView string) noexcept;

    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseNamedColor(CSSOMStringView string) noexcept;
  };
}
