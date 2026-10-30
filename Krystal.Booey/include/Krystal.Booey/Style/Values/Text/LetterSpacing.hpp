#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'letter-spacing'> = normal | <length-percentage>
  // NOTE: Computed value resolves `normal` to 0px.
  // https://drafts.csswg.org/css-text-4/#propdef-letter-spacing
  struct LetterSpacing : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>>
  {
    using Base::Base;

    LetterSpacing(css::keywords::Normal) noexcept : Base {Fixed {0}}
    {
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<LetterSpacing>
  {
    KRYS_NODISCARD LetterSpacing operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::LetterSpacing);
