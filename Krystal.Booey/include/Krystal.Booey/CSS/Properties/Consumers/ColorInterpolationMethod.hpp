#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys
{
  struct ColourInterpolationMethod;
}

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <color-interpolation-method> consuming (raw)
    KRYS_NODISCARD Maybe<ColourInterpolationMethod>
      ConsumeColorInterpolationMethod(TokenRange &tokens, PropertyParserState &state) noexcept;
  }
}
