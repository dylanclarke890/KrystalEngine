#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSRatio;
  struct CSSPropertyParserState;
  struct ColorInterpolationMethod;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <color-interpolation-method> consuming (raw)
    KRYS_NODISCARD Maybe<ColorInterpolationMethod>
      ConsumeColorInterpolationMethod(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;
  }
}