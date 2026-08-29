#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSParserMode.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct AllowAnchor : StronglyTypedBool<AllowAnchor>
  {
    using Base::Base;
  };

  struct AllowAnchorSize : StronglyTypedBool<AllowAnchorSize>
  {
    using Base::Base;
  };

  struct AllowUnitlessZero : StronglyTypedBool<AllowUnitlessZero>
  {
    using Base::Base;
  };

  struct CSSPropertyParserOptions
  {
    AllowAnchor AnchorPolicy {false};

    AllowAnchorSize AnchorSizePolicy {false};

    /// @brief Generally, unitless zero is forbidden for <angle> values, but in a few legacy cases, it is
    /// it can be allowed.
    /// @see https://drafts.csswg.org/css-values-4/#angles
    AllowUnitlessZero UnitlessZeroAngle {false};

    /// @brief Generally, unitless zero is allowed for <length> values, but in a few cases, when it is
    /// ambiguous with a <number> production, it can be forbidden.
    /// @see https://drafts.csswg.org/css-values-4/#lengths
    AllowUnitlessZero UnitlessZeroLength {true};

    Maybe<CSSParserMode> OverrideParserMode {Null};
  };
}