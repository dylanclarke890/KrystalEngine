#include "Krystal.Booey/Layout/Integration/Grid/LayoutIntegrationGridLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridFormattingContext.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxGeometryUpdater.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutTree/FormattingContextBoxIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Grid.hpp"
#include "Krystal.Booey/Rendering/OrderIterator.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout_integration
{
  namespace
  {
    KRYS_NODISCARD bool IsSupportedTrackList(const style::GridTemplateList &gridTemplateList) noexcept
    {
      if (gridTemplateList.Tracks.empty())
      {
        return false;
      }

      for (const auto &trackListEntry : gridTemplateList.Tracks)
      {
        auto isSupported = SwitchOn(
          trackListEntry,
          [](const style::GridTrackSize &trackSize)
          {
            // Since a GridTrackSize type of Breadth sets the MinTrackBreadth and
            // MaxTrackBreadth to the same value we only need to check one.
            return trackSize.IsBreadth() && trackSize.MinTrackBreadth().IsLength()
                   && trackSize.MinTrackBreadth().Length().IsFixed();
          },
          [](const List<css::CSSOMString> &names) { return names.empty(); },
          [](const style::GridTrackEntryRepeat &) { return false; },
          [](const style::GridTrackEntryAutoRepeat &) { return false; },
          [](const style::GridTrackEntrySubgrid &) { return false; });
        if (!isSupported)
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD bool IsSupportedLine(const style::GridPosition &position, size_t linesCount) noexcept
    {
      return position.IsExplicit() && position.NamedGridLine().empty() && position.ExplicitPosition() > 0
             && position.ExplicitPosition() <= static_cast<int>(linesCount);
    }

    // Only items that are placed at explicit unnamed lines of the explicit grid and span a single track are
    // supported.
    KRYS_NODISCARD bool IsSupportedPlacement(const style::GridPosition &start, const style::GridPosition &end,
                                             size_t linesCount) noexcept
    {
      if (!IsSupportedLine(start, linesCount) || !IsSupportedLine(end, linesCount))
      {
        return false;
      }
      return std::abs(end.ExplicitPosition() - start.ExplicitPosition()) == 1;
    }

    // Items have a fixed size in both axes, so these all place the item at the start of its grid area.
    KRYS_NODISCARD bool IsSupportedSelfAlignment(const StyleSelfAlignmentData &alignment) noexcept
    {
      switch (alignment.Position())
      {
        case ItemPosition::Start:
        case ItemPosition::FlexStart:
        case ItemPosition::SelfStart:
        case ItemPosition::Normal:
        case ItemPosition::Stretch:   return true;
        default:                      return false;
      }
    }

    KRYS_NODISCARD layout::GridLayoutConstraints
      ConstraintsForGridContent(const layout::ElementBox &gridContainer) noexcept
    {
      auto &gridContainerRenderer = Downcast<RenderGrid>(*gridContainer.RendererForIntegration());

      auto availableInlineSpace = [&]
      {
        if (auto overridingWidth = gridContainerRenderer.OverridingBorderBoxLogicalWidth())
        {
          return gridContainerRenderer.ContentBoxLogicalWidth(*overridingWidth);
        }
        return gridContainerRenderer.ContentBoxLogicalWidth();
      }();
      auto availableBlockSpace = gridContainerRenderer.AvailableLogicalHeightForContentBox();

      return {availableInlineSpace, availableBlockSpace};
    }
  }

  GridLayout::GridLayout(RenderGrid &renderGrid) noexcept
      : _gridBox(ShareCheckedPtr(BoxTreeUpdater {renderGrid}.Build().get())),
        _layoutState(WeakPtr<layout::LayoutState>::WithRef(renderGrid.View().GetLayoutState()))
  {
  }

  GridLayout::~GridLayout() noexcept
  {
    auto &renderer = GridBoxRenderer();
    _gridBox = nullptr;

    BoxTreeUpdater {renderer}.TearDown();
  }

  bool GridLayout::CanUseFor(const RenderGrid &renderGrid) noexcept
  {
    const auto &renderGridStyle = renderGrid.Style();

    if (!renderGridStyle.Width().IsFixed() || !renderGridStyle.Height().IsFixed())
    {
      return false;
    }

    if (renderGridStyle.Display() != DisplayType::Grid)
    {
      // An inline grid needs a baseline.
      return false;
    }

    if (!renderGridStyle.GetWritingMode().IsHorizontal())
    {
      return false;
    }

    if (!renderGridStyle.GetMarginTrim().IsNone())
    {
      return false;
    }

    if (!renderGridStyle.IsOverflowVisible())
    {
      return false;
    }

    if (!renderGrid.FirstInFlowChild())
    {
      return false;
    }

    // Only grid-auto-flow: row and row dense are supported.
    if (renderGridStyle.GetGridAutoFlow().IsColumn())
    {
      return false;
    }

    // Only fixed-length gaps are supported.
    for (const auto *gap : {&renderGridStyle.RowGap(), &renderGridStyle.ColumnGap()})
    {
      if (!gap->IsNormal() && !gap->TryFixed())
      {
        return false;
      }
    }

    if (renderGrid.IsOutOfFlowPositioned())
    {
      return false;
    }

    if (!renderGridStyle.GetGridTemplateAreas().IsNone())
    {
      return false;
    }

    const auto &gridTemplateColumns = renderGridStyle.GridTemplateColumns();
    const auto &gridTemplateRows = renderGridStyle.GridTemplateRows();
    if (!IsSupportedTrackList(gridTemplateColumns) || !IsSupportedTrackList(gridTemplateRows))
    {
      return false;
    }

    if (renderGridStyle.UsedContain().Contains(style::ContainValue::Size))
    {
      return false;
    }

    auto linesFromGridTemplateColumnsCount = gridTemplateColumns.Sizes.size() + 1;
    auto linesFromGridTemplateRowsCount = gridTemplateRows.Sizes.size() + 1;
    for (const auto &gridItem : ChildrenOfType<RenderBox>(renderGrid))
    {
      if (!gridItem.IsRenderBlockFlow())
      {
        return false;
      }

      const auto &gridItemStyle = gridItem.Style();

      if (!gridItemStyle.Width().IsFixed() || !gridItemStyle.Height().IsFixed())
      {
        return false;
      }

      if (auto fixedMinWidth = gridItemStyle.MinWidth().TryFixed();
          fixedMinWidth && fixedMinWidth->UnresolvedValue())
      {
        return false;
      }

      if (!gridItemStyle.MaxWidth().IsNone())
      {
        return false;
      }

      if (auto fixedMinHeight = gridItemStyle.MinHeight().TryFixed();
          fixedMinHeight && fixedMinHeight->UnresolvedValue())
      {
        return false;
      }

      if (!gridItemStyle.MaxHeight().IsNone())
      {
        return false;
      }

      if (gridItemStyle.HasBorder())
      {
        return false;
      }

      if (gridItemStyle.GetPaddingBox().AnyOf([](const auto &paddingEdge)
                                              { return !paddingEdge.IsPossiblyZero(); }))
      {
        return false;
      }

      if (gridItemStyle.GetMarginBox().AnyOf([](const auto &marginEdge)
                                             { return !marginEdge.IsPossiblyZero(); }))
      {
        return false;
      }

      // Normal and legacy (justify-items' initial value) behave as normal for grid items.
      auto normal = StyleSelfAlignmentData {ItemPosition::Normal};
      auto usedJustifySelf = !gridItemStyle.GetJustifySelf().IsAuto()
                               ? gridItemStyle.GetJustifySelf().Resolve(normal)
                               : renderGridStyle.GetJustifyItems().Resolve(normal);
      if (!IsSupportedSelfAlignment(usedJustifySelf))
      {
        return false;
      }

      auto usedAlignSelf = !gridItemStyle.GetAlignSelf().IsAuto()
                             ? gridItemStyle.GetAlignSelf().Resolve(normal)
                             : renderGridStyle.GetAlignItems().Resolve(normal);
      if (!IsSupportedSelfAlignment(usedAlignSelf))
      {
        return false;
      }

      if (!IsSupportedPlacement(gridItemStyle.GridItemColumnStart(), gridItemStyle.GridItemColumnEnd(),
                                linesFromGridTemplateColumnsCount))
      {
        return false;
      }

      if (!IsSupportedPlacement(gridItemStyle.GridItemRowStart(), gridItemStyle.GridItemRowEnd(),
                                linesFromGridTemplateRowsCount))
      {
        return false;
      }

      if (gridItemStyle.GetWritingMode().IsVertical())
      {
        return false;
      }

      if (gridItem.IsOutOfFlowPositioned())
      {
        return false;
      }

      if (gridItemStyle.HasAspectRatio())
      {
        return false;
      }

      if (!gridItemStyle.IsOverflowVisible())
      {
        return false;
      }

      if (gridItemStyle.UsedContain().Contains(style::ContainValue::Size))
      {
        return false;
      }
    }
    return true;
  }

  void GridLayout::UpdateFormattingContextGeometries() noexcept
  {
    auto boxGeometryUpdater = BoxGeometryUpdater {GetLayoutState(), GridBox()};
    boxGeometryUpdater.SetFormattingContextRootGeometry(
      GridBoxRenderer().ContainingBlock()->ContentBoxLogicalWidth());
    boxGeometryUpdater.SetFormattingContextContentGeometry(
      GetLayoutState().GeometryForBox(GridBox()).ContentBoxWidth(), {});
  }

  void GridLayout::UpdateGridItemRenderers() noexcept
  {
    for (auto &layoutBox : layout::FormattingContextBoxes(GridBox()))
    {
      auto &renderer = Downcast<RenderBox>(*layoutBox.RendererForIntegration());
      const auto &gridItemGeometry = GetLayoutState().GeometryForBox(layoutBox);
      auto borderBoxRect = layout::BoxGeometry::BorderBoxRect(gridItemGeometry);

      renderer.SetLocation(borderBoxRect.TopLeft());
      renderer.SetWidth(borderBoxRect.Width());
      renderer.SetHeight(borderBoxRect.Height());

      renderer.SetMarginBefore(gridItemGeometry.MarginBefore());
      renderer.SetMarginAfter(gridItemGeometry.MarginAfter());
      renderer.SetMarginStart(gridItemGeometry.MarginStart());
      renderer.SetMarginEnd(gridItemGeometry.MarginEnd());
    }
  }

  void GridLayout::UpdateFormattingContextRootRenderer() noexcept
  {
    auto &currentGrid = GridBoxRenderer().CurrentGrid();
    currentGrid.SetNeedsItemsPlacement(false);
    OrderIteratorPopulator orderIteratorPopulator(currentGrid.GetOrderIterator());

    for (auto &layoutBox : layout::FormattingContextBoxes(GridBox()))
    {
      orderIteratorPopulator.CollectChild(Downcast<RenderBox>(*layoutBox.RendererForIntegration()));
    }
  }

  void GridLayout::Layout() noexcept
  {
    layout::GridFormattingContext {GridBox(), GetLayoutState()}.Layout(ConstraintsForGridContent(GridBox()));
    UpdateGridItemRenderers();
    UpdateFormattingContextRootRenderer();
  }

  const RenderGrid &GridLayout::GridBoxRenderer() const noexcept
  {
    return Downcast<RenderGrid>(*_gridBox->RendererForIntegration());
  }

  RenderGrid &GridLayout::GridBoxRenderer() noexcept
  {
    return Downcast<RenderGrid>(*_gridBox->RendererForIntegration());
  }
}
