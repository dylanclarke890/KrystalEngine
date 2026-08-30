#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
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
      ConsumeUnresolvedGridTemplateAreasRow(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSGridLineNamesValue>
      ConsumeGridLineNames(CSSTokenRange &tokens, CSSPropertyParserState &state,
                           AllowEmpty allowEmpty = AllowEmpty::No) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridLine(CSSTokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTrackSize(CSSTokenRange &tokens,
                                                         CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTrackList(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         TrackListType = TrackListType::GridAuto) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTemplatesRowsOrColumns(CSSTokenRange &tokens,
                                                                      CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridTemplateAreas(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeGridAutoFlow(CSSTokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept;
  }
}