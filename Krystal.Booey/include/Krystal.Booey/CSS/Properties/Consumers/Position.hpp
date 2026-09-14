#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <position> | <bg-position>
    // https://drafts.csswg.org/css-values/#position

    // MARK: <position> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumePosition(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept;

    // MARK: <position-x> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumePositionX(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;

    // MARK: <position-y> (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumePositionY(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;

    // MARK: <position> (unresolved)
    KRYS_NODISCARD Maybe<Position> ConsumePositionUnresolved(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept;

    // MARK: <bg-position> (unresolved)
    KRYS_NODISCARD Maybe<Position>
      ConsumeBackgroundPositionUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <position-x> (unresolved)
    KRYS_NODISCARD Maybe<PositionX> ConsumePositionXUnresolved(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept;

    // MARK: <position-y> (unresolved)
    KRYS_NODISCARD Maybe<PositionY> ConsumePositionYUnresolved(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept;

    // MARK: Subset / Special case parsers.

    // NOTE: This is only used by the `<-webkit-radial-gradient()>` and `<transform-origin>` parsers.
    KRYS_NODISCARD Maybe<Position>
      ConsumeOneOrTwoComponentPositionUnresolved(TokenRange &tokens,
                                                 PropertyParserState &state) noexcept;

    // NOTE: This is only used by the `<horizontal-line-command>` parser
    KRYS_NODISCARD Maybe<TwoComponentPositionHorizontal>
      ConsumeTwoComponentPositionHorizontalUnresolved(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept;

    // NOTE: This is only used by the `<vertical-line-command>` parser
    KRYS_NODISCARD Maybe<TwoComponentPositionVertical>
      ConsumeTwoComponentPositionVerticalUnresolved(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept;
  }
}