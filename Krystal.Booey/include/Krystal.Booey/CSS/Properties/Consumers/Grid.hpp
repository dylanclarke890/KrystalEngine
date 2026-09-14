#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;
  using GridNamedAreaMapRow = SmallList<CSSOMString, 8uz>;
  class GridLineNamesValue;

  namespace PropertyParserHelpers
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

    KRYS_NODISCARD bool IsGridBreadthIdent(ValueId) noexcept;

    // Parses a single <string> token from a <'grid-template-areas'> production.
    KRYS_NODISCARD Maybe<GridNamedAreaMapRow>
      ConsumeUnresolvedGridTemplateAreasRow(TokenRange &tokens, PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<GridLineNamesValue>
      ConsumeGridLineNames(TokenRange &tokens, PropertyParserState &state,
                           AllowEmpty allowEmpty = AllowEmpty::No) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeGridLine(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeGridTrackSize(TokenRange &tokens,
                                                         PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeGridTrackList(TokenRange &tokens, PropertyParserState &state,
                                                         TrackListType = TrackListType::GridAuto) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeGridTemplatesRowsOrColumns(TokenRange &tokens,
                                                                      PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeGridTemplateAreas(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeGridAutoFlow(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept;
  }
}