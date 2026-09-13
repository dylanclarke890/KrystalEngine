#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPosition.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <position> | <bg-position>
    // https://drafts.csswg.org/css-values/#position

    // MARK: <position> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePosition(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept;

    // MARK: <position-x> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePositionX(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;

    // MARK: <position-y> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePositionY(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;

    // MARK: <position> (unresolved)
    KRYS_NODISCARD Maybe<CSSPosition> ConsumePositionUnresolved(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept;

    // MARK: <bg-position> (unresolved)
    KRYS_NODISCARD Maybe<CSSPosition>
      ConsumeBackgroundPositionUnresolved(TokenRange &tokens, PropertyParserState &state) noexcept;

    // MARK: <position-x> (unresolved)
    KRYS_NODISCARD Maybe<CSSPositionX> ConsumePositionXUnresolved(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept;

    // MARK: <position-y> (unresolved)
    KRYS_NODISCARD Maybe<CSSPositionY> ConsumePositionYUnresolved(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept;

    // MARK: Subset / Special case parsers.

    // NOTE: This is only used by the `<-webkit-radial-gradient()>` and `<transform-origin>` parsers.
    KRYS_NODISCARD Maybe<CSSPosition>
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