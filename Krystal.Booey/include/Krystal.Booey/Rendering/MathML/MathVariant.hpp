#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  // These are the mathvariant values from the MathML3 recommendation.
  // The special value none means that no explicit mathvariant value has been specified.
  // Note that the numeral values are important for the computation performed in the mathVariantMapCodePoint
  // function, do not change them!
  // In MathML Core only the italic mathvariant is used to implement `text-transform: math-auto`, other values
  // are deprecated.
  enum class MathVariant : uint8
  {
    None = 0,
    Normal = 1,
    Bold = 2,
    Italic = 3,
    BoldItalic = 4,
    Script = 5,
    BoldScript = 6,
    Fraktur = 7,
    DoubleStruck = 8,
    BoldFraktur = 9,
    SansSerif = 10,
    BoldSansSerif = 11,
    SansSerifItalic = 12,
    SansSerifBoldItalic = 13,
    Monospace = 14,
    Initial = 15,
    Tailed = 16,
    Looped = 17,
    Stretched = 18,
  };

  /// @brief The code point @p codePoint maps to in the mathematical alphanumeric symbols of @p mathvariant,
  /// or @p codePoint if it has none.
  KRYS_NODISCARD char32 MathVariantMapCodePoint(char32 codePoint, MathVariant mathvariant) noexcept;
}
