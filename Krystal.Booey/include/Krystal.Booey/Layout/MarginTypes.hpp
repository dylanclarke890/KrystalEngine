#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  struct ComputedVerticalMargin
  {
    Maybe<LayoutUnit> Before;
    Maybe<LayoutUnit> After;
  };

  struct UsedVerticalMargin
  {
    struct NonCollapsedValues
    {
      LayoutUnit Before;
      LayoutUnit After;
    };
    NonCollapsedValues NonCollapsed;

    struct CollapsedValues
    {
      Maybe<LayoutUnit> Before;
      Maybe<LayoutUnit> After;
      bool IsCollapsedThrough {false};
    };
    CollapsedValues Collapsed;

    // FIXME: This structure might need to change to indicate that the cached value is not necessarily the
    // same as the box's computed margin value. This only matters in case of collapse through margins when
    // they collapse into another sibling box. <div style="margin: 1px"></div><div style="margin:
    // 10px"></div> <- the second div's before/after marings collapse through and the same time they
    // collapse into the first div. When the parent computes its before margin, it should see the second
    // div's collapsed through margin as the value to collapse width (adjoining margin value). So while the
    // first div's before margin is not 10px, the cached value is 10px so that when we compute the parent's
    // margin we just need to check the first inflow child's cached margin values.
    struct PositiveAndNegativePair
    {
      struct Values
      {
        KRYS_NODISCARD bool IsNonZero() const noexcept
        {
          return Positive.value_or(0_lu) || Negative.value_or(0_lu);
        }

        Maybe<LayoutUnit> Positive;
        Maybe<LayoutUnit> Negative;
        bool IsQuirk {false};
      };
      Values Before;
      Values After;
    };
    PositiveAndNegativePair PositiveAndNegativeValues;
  };

  KRYS_NODISCARD inline LayoutUnit MarginBefore(const UsedVerticalMargin &usedVerticalMargin) noexcept
  {
    return usedVerticalMargin.Collapsed.Before.value_or(usedVerticalMargin.NonCollapsed.Before);
  }

  KRYS_NODISCARD inline LayoutUnit MarginAfter(const UsedVerticalMargin &usedVerticalMargin) noexcept
  {
    if (usedVerticalMargin.Collapsed.IsCollapsedThrough)
    {
      return 0_lu;
    }
    return usedVerticalMargin.Collapsed.After.value_or(usedVerticalMargin.NonCollapsed.After);
  }

  struct ComputedHorizontalMargin
  {
    Maybe<LayoutUnit> Start;
    Maybe<LayoutUnit> End;
  };

  struct UsedHorizontalMargin
  {
    LayoutUnit Start;
    LayoutUnit End;
  };

  struct PrecomputedMarginBefore
  {
    KRYS_NODISCARD LayoutUnit UsedValue() const noexcept
    {
      return CollapsedValue.value_or(NonCollapsedValue);
    }

    LayoutUnit NonCollapsedValue;
    Maybe<LayoutUnit> CollapsedValue;
    UsedVerticalMargin::PositiveAndNegativePair::Values PositiveAndNegativeMarginBefore;
  };
}
