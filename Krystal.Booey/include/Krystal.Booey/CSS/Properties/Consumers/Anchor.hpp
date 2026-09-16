#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    enum class ValueType
    {
      Specified,
      Computed
    };

    // Take two keywords that make up a <position-area> and build a Value that
    // minimizes the serialization of position-area. The minimization done depends on
    // the context where the value is used.
    //
    // For all contexts:
    // * If one keyword is explicit about its axis, and the other keyword is span-all, only keep the first
    // keyword.
    // * If returning a pair of keywords, order the block/X axis keyword before the inline/Y axis keyword.
    //
    // For context where the computed value is used:
    // * If one keyword is explicitly on the block/inline axis, and the other keyword is explicitly on the
    // opposite axis or axisless, remove the explicit-ness. (e.g "block-start inline-end" becomes "start end")
    //
    // Returns null if the keywords aren't valid/compatible. Otherwise, return a
    // PrimitiveValue or ValuePair depending on if the keywords can be collapsed.
    KRYS_NODISCARD RefPtr<Value> ValueForPositionArea(ValueId dimension1, ValueId dimension2,
                                                         ValueType type) noexcept;

    /// @grammar <'position-area'>
    /// @see https://drafts.csswg.org/css-anchor-position-1/#propdef-position-area
    KRYS_NODISCARD RefPtr<Value> ConsumePositionArea(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;
  }
}