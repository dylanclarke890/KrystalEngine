#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::style
{
  class Extractor;
}

namespace krys::boo::css
{
  class StyleProperties;

  enum class PropertyId : uint16;

  struct SerialisationContext;

  KRYS_NODISCARD CSSOMString SerialiseShorthandValue(const SerialisationContext &context,
                                                     const Style::Extractor &extractor,
                                                     PropertyId shorthandId) noexcept;

  KRYS_NODISCARD CSSOMString SerialiseShorthandValue(const SerialisationContext &context,
                                                     const StyleProperties &properties,
                                                     PropertyId shorthandId) noexcept;
}