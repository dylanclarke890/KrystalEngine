#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class StyleProperties;

  enum class PropertyId : uint16;

  struct SerialisationContext;

  // FIXME: Add the style::Extractor overload (computed style serialisation) once style::Extractor is ported.

  /// @brief Serialises the shorthand `shorthandId` from its longhands in `properties`.
  /// @return The serialisation, or `null` if the longhands can't be represented by the shorthand.
  KRYS_NODISCARD Maybe<CSSOMString> SerialiseShorthandValue(const SerialisationContext &context,
                                                            const StyleProperties &properties,
                                                            PropertyId shorthandId) noexcept;
}
