#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief A flex item with its geometry resolved along the flex container's main and cross axes.
  class LogicalFlexItem
  {
  public:
    struct MainAxisGeometry
    {
      KRYS_NODISCARD LayoutUnit Margin() const noexcept
      {
        return MarginStart.value_or(0_lu) + MarginEnd.value_or(0_lu);
      }

      Maybe<LayoutUnit> DefiniteFlexBasis;
      Maybe<LayoutUnit> Size;
      Maybe<LayoutUnit> MaximumSize;
      Maybe<LayoutUnit> MinimumSize;
      Maybe<LayoutUnit> MarginStart;
      Maybe<LayoutUnit> MarginEnd;
      LayoutUnit BorderAndPadding;
    };

    struct CrossAxisGeometry
    {
      KRYS_NODISCARD LayoutUnit Margin() const noexcept
      {
        return MarginStart.value_or(0_lu) + MarginEnd.value_or(0_lu);
      }
      KRYS_NODISCARD bool HasNonAutoMargins() const noexcept
      {
        return MarginStart && MarginEnd;
      }

      Maybe<LayoutUnit> DefiniteSize;
      LayoutUnit Ascent;
      LayoutUnit Descent;
      Maybe<LayoutUnit> MaximumSize;
      Maybe<LayoutUnit> MinimumSize;
      Maybe<LayoutUnit> MarginStart;
      Maybe<LayoutUnit> MarginEnd;
      LayoutUnit BorderAndPadding;
      bool HasSizeAuto {false};
    };

  private:
    CheckedPtr<const ElementBox> _layoutBox;
    MainAxisGeometry _mainAxisGeometry;
    CrossAxisGeometry _crossAxisGeometry;
    bool _hasAspectRatio {false};
    bool _isOrthogonal {false};

  public:
    LogicalFlexItem(const ElementBox &flexItem, const MainAxisGeometry &mainGeometry,
                    const CrossAxisGeometry &crossGeometry, bool hasAspectRatio, bool isOrthogonal) noexcept
        : _layoutBox(ShareCheckedPtr(&flexItem)), _mainAxisGeometry(mainGeometry),
          _crossAxisGeometry(crossGeometry), _hasAspectRatio(hasAspectRatio), _isOrthogonal(isOrthogonal)
    {
    }
    LogicalFlexItem() noexcept = default;

    KRYS_NODISCARD const MainAxisGeometry &MainAxis() const noexcept
    {
      return _mainAxisGeometry;
    }
    KRYS_NODISCARD const CrossAxisGeometry &CrossAxis() const noexcept
    {
      return _crossAxisGeometry;
    }

    KRYS_NODISCARD float GrowFactor() const noexcept
    {
      return Style().GetFlexGrow().Value;
    }
    KRYS_NODISCARD float ShrinkFactor() const noexcept
    {
      return Style().GetFlexShrink().Value;
    }

    KRYS_NODISCARD bool HasContentFlexBasis() const noexcept
    {
      return Style().GetFlexBasis().IsContent();
    }
    KRYS_NODISCARD bool HasAvailableSpaceDependentFlexBasis() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool HasAspectRatio() const noexcept
    {
      return _hasAspectRatio;
    }
    KRYS_NODISCARD bool IsOrthogonal() const noexcept
    {
      return _isOrthogonal;
    }
    KRYS_NODISCARD bool IsContentBoxBased() const noexcept
    {
      return Style().GetBoxSizing() == BoxSizing::ContentBox;
    }

    KRYS_NODISCARD const ElementBox &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }
    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return LayoutBox().Style();
    }
    KRYS_NODISCARD boo::WritingMode GetWritingMode() const noexcept
    {
      return Style().GetWritingMode();
    }
  };
}
