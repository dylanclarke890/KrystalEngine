#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;
  using GridNamedAreaMapRow = SmallList<CSSOMString, 8uz>;
  class CSSGridLineNamesValue;

  namespace CSSPropertyParserHelpers
  {
    // https://drafts.csswg.org/css-grid/

    enum class AllowEmpty : bool
    {
      No,
      Yes
    };

    enum TrackListType : uint8
    {
      GridTemplate,
      GridTemplateNoRepeat,
      GridAuto
    };

    KRYS_NODISCARD bool IsGridBreadthIdent(CSSValueId) noexcept;

    // Parses a single <string> token from a <'grid-template-areas'> production.
    KRYS_NODISCARD Maybe<GridNamedAreaMapRow>
      ConsumeUnresolvedGridTemplateAreasRow(TokenRange &tokens, CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSGridLineNamesValue>
      ConsumeGridLineNames(TokenRange &tokens, CSSPropertyParserState &state,
                           AllowEmpty allowEmpty = AllowEmpty::No) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridLine(TokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTrackSize(TokenRange &tokens,
                                                         CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTrackList(TokenRange &tokens, CSSPropertyParserState &state,
                                                         TrackListType = TrackListType::GridAuto) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTemplatesRowsOrColumns(TokenRange &tokens,
                                                                      CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTemplateAreas(TokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridAutoFlow(TokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;
  }
}