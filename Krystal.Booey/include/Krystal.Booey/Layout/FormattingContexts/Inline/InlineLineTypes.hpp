#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class Box;
  class InlineItem;

  using InlineBoxBoundaryTextSpacings = HashMap<size_t, float>;
  using TrimmableTextSpacings = InlineBoxBoundaryTextSpacings;

  struct TextSpacingContext
  {
    // InlineBoxBoundaryTextSpacings maps inline item indexes to spacing. It serves to identify start boxes
    // that mark spacing between boxes boundaries. If we have start box with an entry here, this means that we
    // have to add spacing between this start box and the previous box. We need to account for it during
    // layout and to reproduce such spacing among 2 boxes during display. Therefore, this serve for
    // identifying spacing between box boundaries such that we can both: add these spacing into layout/display
    // and trim this spacing during line breaking if needed.
    InlineBoxBoundaryTextSpacings InlineBoxBoundarySpacings;
    // TrimmableTextSpacings map inline item indexes to spacing. Text content can be split into multiple
    // different inline text items. This structure serves to identify spacing that was added to a text item
    // because of its adjacency to another text item. Ex: if we have 2 text items A and B, and the last
    // character of A and the first character of B require spacing to be added between them. This spacing is
    // always added as a leading spacing to B. However, during line breaking, if B gets placed on a different
    // line as A, they are no longer adjacent and we must trim such spacing. We don't use this map for
    // actually adding the spacing, just for trimming it. Therefore this maps serves only to identify
    // trimmable spacing between text runs. Maybe this can be called TrimmableTextSpacingBetweenTextItems or
    // something.
    TrimmableTextSpacings TrimmableSpacings;
  };

  enum class IsFirstFormattedLine : bool
  {
    No,
    Yes
  };

  enum class LineEndingTruncationPolicy : uint8
  {
    NoTruncation,
    WhenContentOverflowsInInlineDirection,
    WhenContentOverflowsInBlockDirection
  };

  struct ExpansionInfo
  {
    size_t OpportunityCount {0};
    List<size_t> OpportunityList;
    List<ExpansionBehavior> BehaviorList;
  };

  struct InlineItemPosition
  {
    size_t Index {0};
    size_t Offset {0}; // Note that this is offset relative to the start position of the InlineItem.

    KRYS_NODISCARD friend bool operator==(const InlineItemPosition &,
                                          const InlineItemPosition &) noexcept = default;
    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return Index || Offset;
    }
  };

  struct InlineItemRange
  {
    InlineItemRange(InlineItemPosition start, InlineItemPosition end) noexcept : Start(start), End(end)
    {
    }
    InlineItemRange(size_t startIndex, size_t endIndex) noexcept : Start {startIndex, {}}, End {endIndex, {}}
    {
    }
    InlineItemRange() noexcept = default;

    KRYS_NODISCARD size_t StartIndex() const noexcept
    {
      return Start.Index;
    }
    KRYS_NODISCARD size_t EndIndex() const noexcept
    {
      return End.Index;
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return StartIndex() == EndIndex() && Start.Offset == End.Offset;
    }

    InlineItemPosition Start;
    InlineItemPosition End;
  };

  struct PreviousLine
  {
    size_t LineIndex {0};
    // Content width measured during line breaking (avoid double-measuring).
    Maybe<InlineLayoutUnit> TrailingOverflowingContentWidth {};
    bool EndsWithLineBreak {false};
    TextDirection InlineBaseDirection {TextDirection::LTR};
    List<const Box *> SuspendedFloats;
  };
}
