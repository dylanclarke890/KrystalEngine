#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSRatio;
  struct PropertyParserState;
  struct ColorInterpolationMethod;

  namespace PropertyParserHelpers
  {
    // MARK: <color-interpolation-method> consuming (raw)
    KRYS_NODISCARD Maybe<ColorInterpolationMethod>
      ConsumeColorInterpolationMethod(TokenRange &tokens, PropertyParserState &state) noexcept;
  }
}