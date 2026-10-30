#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  class Extractor;
}

namespace krys::boo::css
{
  class StyleProperties;

  enum class PropertyId : uint16;

  struct SerialisationContext;

  /// @brief Serialises the shorthand `shorthandId` from its longhands in `properties`.
  /// @return The serialisation, or `null` if the longhands can't be represented by the shorthand.
  KRYS_NODISCARD Maybe<CSSOMString> SerialiseShorthandValue(const SerialisationContext &context,
                                                            const StyleProperties &properties,
                                                            PropertyId shorthandId) noexcept;

  /// @brief Serialises the shorthand `shorthandId` from the computed values of its longhands.
  /// @return The serialisation, or an empty string if a longhand has no computed value.
  KRYS_NODISCARD CSSOMString SerialiseShorthandValue(const SerialisationContext &context,
                                                     const style::Extractor &extractor,
                                                     PropertyId shorthandId) noexcept;
}
