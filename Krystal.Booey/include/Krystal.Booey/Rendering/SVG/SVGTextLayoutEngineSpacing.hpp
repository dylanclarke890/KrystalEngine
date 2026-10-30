#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class FontCascade;

  /// @brief Helper used by SVGTextLayoutEngine to handle 'letter-spacing' and 'word-spacing'.
  class SVGTextLayoutEngineSpacing
  {
    KRYS_NON_COPYABLE(SVGTextLayoutEngineSpacing);

  private:
    const FontCascade &_font;
    char32 _lastCharacter {0};

  public:
    explicit SVGTextLayoutEngineSpacing(const FontCascade &) noexcept;

    KRYS_NODISCARD float CalculateCSSSpacing(char32) noexcept;
  };
}
