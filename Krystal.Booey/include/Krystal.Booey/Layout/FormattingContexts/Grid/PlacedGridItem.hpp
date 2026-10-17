#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridTypes.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MaximumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  class UnplacedGridItem;

  /// @brief A grid item placed in the grid, with the computed sizes and alignment its layout needs.
  class PlacedGridItem
  {
  public:
    struct ComputedSizes
    {
      style::PreferredSize PreferredSize;
      style::MinimumSize MinimumSize;
      style::MaximumSize MaximumSize;
      style::MarginEdge MarginStart;
      style::MarginEdge MarginEnd;
    };

  private:
    const ElementBox *_layoutBox;
    ComputedSizes _inlineAxisSizes;
    ComputedSizes _blockAxisSizes;
    StyleSelfAlignmentData _inlineAxisAlignment;
    StyleSelfAlignmentData _blockAxisAlignment;
    style::ZoomFactor _usedZoom {1.0f, 1.0f};
    GridAreaLines _gridAreaLines;

  public:
    PlacedGridItem(const UnplacedGridItem &unplacedGridItem, GridAreaLines gridAreaLines,
                   const ComputedSizes &inlineAxisSizes, const ComputedSizes &blockAxisSizes,
                   const StyleSelfAlignmentData &inlineAxisAlignment,
                   const StyleSelfAlignmentData &blockAxisAlignment,
                   const style::ZoomFactor &usedZoom) noexcept;

    KRYS_NODISCARD const ComputedSizes &InlineAxisSizes() const noexcept
    {
      return _inlineAxisSizes;
    }
    KRYS_NODISCARD const ComputedSizes &BlockAxisSizes() const noexcept
    {
      return _blockAxisSizes;
    }

    KRYS_NODISCARD size_t ColumnStartLine() const noexcept
    {
      return _gridAreaLines.ColumnStartLine;
    }
    KRYS_NODISCARD size_t ColumnEndLine() const noexcept
    {
      return _gridAreaLines.ColumnEndLine;
    }
    KRYS_NODISCARD size_t RowStartLine() const noexcept
    {
      return _gridAreaLines.RowStartLine;
    }
    KRYS_NODISCARD size_t RowEndLine() const noexcept
    {
      return _gridAreaLines.RowEndLine;
    }

    KRYS_NODISCARD const ElementBox &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }

    KRYS_NODISCARD const StyleSelfAlignmentData &InlineAxisAlignment() const noexcept
    {
      return _inlineAxisAlignment;
    }
    KRYS_NODISCARD const StyleSelfAlignmentData &BlockAxisAlignment() const noexcept
    {
      return _blockAxisAlignment;
    }

    // FIXME: Add support for grid item's with preferred aspect ratios.
    KRYS_NODISCARD bool HasPreferredAspectRatio() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsReplacedElement() const noexcept
    {
      return _layoutBox->IsReplacedBox();
    }

    KRYS_NODISCARD const GridAreaLines &GetGridAreaLines() const noexcept
    {
      return _gridAreaLines;
    }
    KRYS_NODISCARD const style::ZoomFactor &UsedZoom() const noexcept
    {
      return _usedZoom;
    }
  };
}
