#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSRatio;
  struct CSSPropertyParserState;
  struct ColorInterpolationMethod;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <color-interpolation-method> consuming (raw)
    KRYS_NODISCARD Maybe<ColorInterpolationMethod>
      ConsumeColorInterpolationMethod(TokenRange &tokens, CSSPropertyParserState &state) noexcept;
  }
}