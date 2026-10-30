#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'word-spacing'> = normal | <length-percentage>
  // NOTE: Computed value resolves `normal` to 0px.
  // https://drafts.csswg.org/css-text-4/#propdef-word-spacing
  struct WordSpacing : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>>
  {
    using Base::Base;

    WordSpacing(css::keywords::Normal) noexcept : Base {Fixed {0}}
    {
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<WordSpacing>
  {
    KRYS_NODISCARD WordSpacing operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::WordSpacing);
