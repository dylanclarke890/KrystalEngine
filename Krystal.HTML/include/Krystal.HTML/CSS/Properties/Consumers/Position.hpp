#pragma once

#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPosition.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <position> | <bg-position>
    // https://drafts.csswg.org/css-values/#position

    // MARK: <position> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePosition(CSSTokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept;

    // MARK: <position-x> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePositionX(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;

    // MARK: <position-y> (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumePositionY(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;

    // MARK: <position> (unresolved)
    KRYS_NODISCARD Maybe<CSSPosition> ConsumePositionUnresolved(CSSTokenRange &tokens,
                                                                CSSPropertyParserState &state) noexcept;

    // MARK: <bg-position> (unresolved)
    KRYS_NODISCARD Maybe<CSSPosition>
      ConsumeBackgroundPositionUnresolved(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // MARK: <position-x> (unresolved)
    KRYS_NODISCARD Maybe<CSSPositionX> ConsumePositionXUnresolved(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state) noexcept;

    // MARK: <position-y> (unresolved)
    KRYS_NODISCARD Maybe<CSSPositionY> ConsumePositionYUnresolved(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state) noexcept;

    // MARK: Subset / Special case parsers.

    // NOTE: This is only used by the `<-webkit-radial-gradient()>` and `<transform-origin>` parsers.
    KRYS_NODISCARD Maybe<CSSPosition>
      ConsumeOneOrTwoComponentPositionUnresolved(CSSTokenRange &tokens,
                                                 CSSPropertyParserState &state) noexcept;

    // NOTE: This is only used by the `<horizontal-line-command>` parser
    KRYS_NODISCARD Maybe<TwoComponentPositionHorizontal>
      ConsumeTwoComponentPositionHorizontalUnresolved(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept;

    // NOTE: This is only used by the `<vertical-line-command>` parser
    KRYS_NODISCARD Maybe<TwoComponentPositionVertical>
      ConsumeTwoComponentPositionVerticalUnresolved(CSSTokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept;
  }
}