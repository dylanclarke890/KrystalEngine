#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  struct ColorInterpolationMethod;
}

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <color-interpolation-method> consuming (raw)
    KRYS_NODISCARD Maybe<ColorInterpolationMethod>
      ConsumeColorInterpolationMethod(TokenRange &tokens, PropertyParserState &state) noexcept;
  }
}
