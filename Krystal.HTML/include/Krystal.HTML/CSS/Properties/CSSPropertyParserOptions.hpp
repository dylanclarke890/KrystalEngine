#pragma once

#include "Krystal.HTML/CSS/Parser/Enums/CSSParserMode.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  struct AllowUnitlessZero : StronglyTypedBool<AllowUnitlessZero>
  {
    using Base::Base;
  };

  struct CSSPropertyParserOptions
  {
    Maybe<CSSParserMode> OverrideParserMode {Null};

    /// @brief Generally, unitless zero is forbidden for <angle> values, but in a few legacy cases, it is
    /// it can be allowed.
    /// @see https://drafts.csswg.org/css-values-4/#angles
    AllowUnitlessZero UnitlessZeroAngle {false};

    /// @brief Generally, unitless zero is allowed for <length> values, but in a few cases, when it is
    /// ambiguous with a <number> production, it can be forbidden.
    /// @see https://drafts.csswg.org/css-values-4/#lengths
    AllowUnitlessZero UnitlessZeroLength {true};
  };
}