#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridFormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutChildIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>

namespace krys::boo::layout
{
  GridFormattingContext::GridFormattingContext(const ElementBox &gridBox, LayoutState &layoutState) noexcept
      : _gridBox(gridBox), _globalLayoutState(layoutState), _integrationUtils(layoutState)
  {
  }

  UnplacedGridItems GridFormattingContext::ConstructUnplacedGridItems() const noexcept
  {
    struct GridItem
    {
      const ElementBox *LayoutBox;
      int Order;
    };

    List<GridItem> gridItems;
    for (const auto &gridItem : ChildrenOfType<ElementBox>(_gridBox))
    {
      if (gridItem.IsOutOfFlowPositioned())
      {
        continue;
      }

      gridItems.push_back({&gridItem, gridItem.Style().GetOrder().Value});
    }

    std::ranges::stable_sort(gridItems, {}, &GridItem::Order);

    UnplacedGridItems unplacedGridItems;
    for (const auto &gridItem : gridItems)
    {
      const auto &gridItemStyle = gridItem.LayoutBox->Style();

      const auto &gridItemColumnStart = gridItemStyle.GridItemColumnStart();
      const auto &gridItemColumnEnd = gridItemStyle.GridItemColumnEnd();
      const auto &gridItemRowStart = gridItemStyle.GridItemRowStart();
      const auto &gridItemRowEnd = gridItemStyle.GridItemRowEnd();

      auto unplacedGridItem = UnplacedGridItem {*gridItem.LayoutBox, gridItemColumnStart, gridItemColumnEnd,
                                                gridItemRowStart, gridItemRowEnd};

      // Check if this item is fully explicitly positioned
      auto fullyExplicitlyPositionedItem = gridItemColumnStart.IsExplicit() && gridItemColumnEnd.IsExplicit()
                                           && gridItemRowStart.IsExplicit() && gridItemRowEnd.IsExplicit();

      // FIXME: support definite row/column positioning
      // We should place items with definite row or column positions
      // but currently we only support fully explicitly positioned items.
      // See: https://www.w3.org/TR/css-grid-1/#auto-placement-algo
      if (fullyExplicitlyPositionedItem)
      {
        unplacedGridItems.NonAutoPositionedItems.push_back(krys::move(unplacedGridItem));
      }
      else if (unplacedGridItem.HasDefiniteRowPosition())
      {
        unplacedGridItems.DefiniteRowPositionedItems.push_back(krys::move(unplacedGridItem));
      }
      else
      {
        unplacedGridItems.AutoPositionedItems.push_back(krys::move(unplacedGridItem));
      }
    }
    return unplacedGridItems;
  }

  void GridFormattingContext::Layout(GridLayoutConstraints layoutConstraints) noexcept
  {
    auto unplacedGridItems = ConstructUnplacedGridItems();
    auto [usedTrackSizes, gridItemRects] = GridLayout {*this}.Layout(layoutConstraints, unplacedGridItems);

    // Grid layout positions each item within its containing block which is the grid area.
    // Here we translate it to the coordinate space of the grid.
    auto mapGridItemLocationsToGrid = [&]
    {
      // Compute gap values for columns and rows.
      // For now, we handle fixed gaps only (not percentages or calc).
      const auto &gridStyle = Root().Style();

      auto columnGap = GridLayoutUtils::ComputeGapValue(gridStyle.ColumnGap());
      auto rowGap = GridLayoutUtils::ComputeGapValue(gridStyle.RowGap());

      for (auto &gridItemRect : gridItemRects)
      {
        const auto &lineNumbersForGridArea = gridItemRect.LineNumbersForGridArea;
        auto columnPosition = GridLayoutUtils::ComputeGridLinePosition(lineNumbersForGridArea.ColumnStartLine,
                                                                       usedTrackSizes.ColumnSizes, columnGap);
        auto rowPosition = GridLayoutUtils::ComputeGridLinePosition(lineNumbersForGridArea.RowStartLine,
                                                                    usedTrackSizes.RowSizes, rowGap);

        gridItemRect.BorderBoxRect.MoveBy({columnPosition, rowPosition});
      }
    };
    mapGridItemLocationsToGrid();
    SetGridItemGeometries(gridItemRects);
  }

  PlacedGridItems GridFormattingContext::ConstructPlacedGridItems(const GridAreas &gridAreas) const noexcept
  {
    PlacedGridItems placedGridItems;
    placedGridItems.reserve(gridAreas.size());
    for (const auto &[unplacedGridItem, gridAreaLines] : gridAreas)
    {
      const auto &gridItemStyle = unplacedGridItem.LayoutBox().Style();

      // Normal and legacy (justify-items' initial value) behave as normal for grid items.
      auto normal = StyleSelfAlignmentData {ItemPosition::Normal};
      auto usedJustifySelf = [&]
      {
        if (const auto &gridItemJustifySelf = gridItemStyle.GetJustifySelf(); !gridItemJustifySelf.IsAuto())
        {
          return gridItemJustifySelf.Resolve(normal);
        }
        return Root().Style().GetJustifyItems().Resolve(normal);
      };

      auto usedAlignSelf = [&]
      {
        if (const auto &gridItemAlignSelf = gridItemStyle.GetAlignSelf(); !gridItemAlignSelf.IsAuto())
        {
          return gridItemAlignSelf.Resolve(normal);
        }
        return Root().Style().GetAlignItems().Resolve(normal);
      };

      auto inlineAxisSizes = PlacedGridItem::ComputedSizes {
        gridItemStyle.Width(), gridItemStyle.MinWidth(), gridItemStyle.MaxWidth(), gridItemStyle.MarginLeft(),
        gridItemStyle.MarginRight()};

      auto blockAxisSizes = PlacedGridItem::ComputedSizes {
        gridItemStyle.Height(), gridItemStyle.MinHeight(), gridItemStyle.MaxHeight(),
        gridItemStyle.MarginTop(), gridItemStyle.MarginBottom()};

      placedGridItems.emplace_back(unplacedGridItem, gridAreaLines, inlineAxisSizes, blockAxisSizes,
                                   usedJustifySelf(), usedAlignSelf(), gridItemStyle.UsedZoomForLength());
    }
    return placedGridItems;
  }

  const BoxGeometry &GridFormattingContext::GeometryForGridItem(const ElementBox &layoutBox) const noexcept
  {
    krys_debug_assert(layoutBox.IsGridItem());
    return GetLayoutState().GeometryForBox(layoutBox);
  }

  BoxGeometry &GridFormattingContext::GeometryForGridItem(const ElementBox &layoutBox) noexcept
  {
    krys_debug_assert(layoutBox.IsGridItem());
    return _globalLayoutState.EnsureGeometryForBox(layoutBox);
  }

  void GridFormattingContext::SetGridItemGeometries(const GridItemRects &gridItemRects) noexcept
  {
    for (const auto &gridItemRect : gridItemRects)
    {
      auto &boxGeometry = GeometryForGridItem(*gridItemRect.LayoutBox);
      const auto &gridItemBorderBox = gridItemRect.BorderBoxRect;

      const auto &margins = gridItemRect.Margins;
      boxGeometry.SetHorizontalMargin({margins.Left(), margins.Right()});
      boxGeometry.SetVerticalMargin({margins.Top(), margins.Bottom()});

      boxGeometry.SetTopLeft(gridItemBorderBox.Location());
      auto contentBoxInlineSize = gridItemBorderBox.Width() - boxGeometry.HorizontalBorderAndPadding();
      auto contentBoxBlockSize = gridItemBorderBox.Height() - boxGeometry.VerticalBorderAndPadding();

      boxGeometry.SetContentBoxSize({contentBoxInlineSize, contentBoxBlockSize});
    }
  }
}
