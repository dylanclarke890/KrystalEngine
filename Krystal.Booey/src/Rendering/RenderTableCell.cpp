#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableCellElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/BackgroundPainter.hpp"
#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "Krystal.Booey/Rendering/LayoutScope.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  namespace
  {
    // The largest rowspan, which rowspan=0 stands for.
    // https://html.spec.whatwg.org/#attr-tdth-rowspan
    constexpr uint32 MaxRowspan = 65'534;

    KRYS_NODISCARD CollapsedBorderValue MakeCollapsedBorder(const RenderElement &renderer,
                                                            const BorderValue &border,
                                                            IncludeBorderColor includeColor,
                                                            css::PropertyId colorProperty,
                                                            BorderPrecedence precedence) noexcept
    {
      const auto &style = renderer.Style();
      return CollapsedBorderValue(border,
                                  includeColor == IncludeBorderColor::Yes
                                    ? style.VisitedDependentColorWithColorFilter(colorProperty)
                                    : Color(),
                                  precedence, style.UsedZoomForLength());
    }

    KRYS_NODISCARD css::PropertyId ColorProperty(IncludeBorderColor includeColor,
                                                 css::PropertyId logicalProperty,
                                                 WritingMode writingMode) noexcept
    {
      return includeColor == IncludeBorderColor::Yes
               ? css::Property::ResolveDirectionAwareProperty(logicalProperty, writingMode)
               : css::PropertyId::Invalid;
    }

    inline void MarkCellDirtyWhenCollapsedBorderChanges(RenderTableCell *cell) noexcept
    {
      if (!cell)
      {
        return;
      }
      cell->SetNeedsLayoutAndPreferredWidthsUpdate();
    }

    // The following rules apply for resolving conflicts and figuring out which border
    // to use.
    // (1) Borders with the 'border-style' of 'hidden' take precedence over all other conflicting
    // borders. Any border with this value suppresses all borders at this location.
    // (2) Borders with a style of 'none' have the lowest priority. Only if the border properties of all
    // the elements meeting at this edge are 'none' will the border be omitted (but note that 'none' is
    // the default value for the border style.)
    // (3) If none of the styles are 'hidden' and at least one of them is not 'none', then narrow borders
    // are discarded in favor of wider ones. If several have the same 'border-width' then styles are preferred
    // in this order: 'double', 'solid', 'dashed', 'dotted', 'ridge', 'outset', 'groove', and the lowest:
    // 'inset'. (4) If border styles differ only in color, then a style set on a cell wins over one on a row,
    // which wins over a row group, column, column group and, lastly, table. It is undefined which color
    // is used when two elements of the same type disagree.
    KRYS_NODISCARD bool CompareBorders(const CollapsedBorderValue &border1,
                                       const CollapsedBorderValue &border2) noexcept
    {
      // Sanity check the values passed in. The null border have lowest priority.
      if (!border2.Exists())
      {
        return false;
      }
      if (!border1.Exists())
      {
        return true;
      }

      // Rule #1 above.
      if (border1.Style() == BorderStyle::Hidden)
      {
        return false;
      }
      if (border2.Style() == BorderStyle::Hidden)
      {
        return true;
      }

      // Rule #2 above.  A style of 'none' has lowest priority and always loses to any other border.
      if (border2.Style() == BorderStyle::None)
      {
        return false;
      }
      if (border1.Style() == BorderStyle::None)
      {
        return true;
      }

      // The first part of rule #3 above. Wider borders win.
      if (border1.Width() != border2.Width())
      {
        return border1.Width() < border2.Width();
      }

      // The borders have equal width.  Sort by border style.
      if (border1.Style() != border2.Style())
      {
        return border1.Style() < border2.Style();
      }

      // The border have the same width and style.  Rely on precedence (cell over row over row group, etc.)
      return border1.Precedence() < border2.Precedence();
    }

    KRYS_NODISCARD CollapsedBorderValue ChooseBorder(const CollapsedBorderValue &border1,
                                                     const CollapsedBorderValue &border2) noexcept
    {
      const auto &border = CompareBorders(border1, border2) ? border2 : border1;
      return border.Style() == BorderStyle::Hidden ? CollapsedBorderValue() : border;
    }
  }

  RenderTableCell::RenderTableCell(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TableCell, element, krys::move(style))
  {
    // We only update the flags when notified of DOM changes in ColSpanOrRowSpanChanged()
    // so we need to set their initial values here in case something asks for ColSpan()/RowSpan() before then.
    UpdateColAndRowSpanFlags();
    krys_debug_assert(IsRenderTableCell());
  }

  RenderTableCell::RenderTableCell(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TableCell, document, krys::move(style))
  {
    krys_debug_assert(IsRenderTableCell());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableCell::~RenderTableCell() noexcept = default;

  LayoutRect RenderTableCell::FrameRectForStickyPositioning() const noexcept
  {
    // RenderTableCell has the RenderTableRow as the container, but is positioned relatively to the
    // RenderTableSection. The sticky positioning algorithm assumes that elements are positioned relatively to
    // their container, so we correct for that here.
    krys_debug_assert(ParentBox());
    auto returnValue = FrameRect();
    returnValue.Move(-ParentBox()->LocationOffset());
    return returnValue;
  }

  RenderTableCell *RenderTableCell::NextCell() const noexcept
  {
    return Downcast<RenderTableCell>(NextSibling());
  }

  RenderTableCell *RenderTableCell::PreviousCell() const noexcept
  {
    return Downcast<RenderTableCell>(PreviousSibling());
  }

  RenderTableRow *RenderTableCell::Row() const noexcept
  {
    return Downcast<RenderTableRow>(static_cast<RenderObject *>(Parent()));
  }

  RenderTableSection *RenderTableCell::Section() const noexcept
  {
    auto *row = Row();
    if (!row)
    {
      return nullptr;
    }
    return Downcast<RenderTableSection>(row->Parent());
  }

  RenderTable *RenderTableCell::Table() const noexcept
  {
    auto *section = Section();
    if (!section)
    {
      return nullptr;
    }
    return Downcast<RenderTable>(section->Parent());
  }

  uint32 RenderTableCell::RowIndex() const noexcept
  {
    // This function shouldn't be called on a detached cell.
    krys_debug_assert(Row());
    return Row()->RowIndex();
  }

  WritingMode RenderTableCell::TableWritingMode() const noexcept
  {
    return Table()->GetWritingMode();
  }

  auto RenderTableCell::LocalRectsForRepaint(RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    // If the table grid is dirty, we cannot get reliable information about adjoining cells,
    // so we ignore outside borders. This should not be a problem because it means that
    // the table is going to recalculate the grid, relayout and repaint its current rect, which
    // includes any outside borders of this cell.
    if (!Table()->CollapseBorders() || Table()->NeedsSectionRecalc())
    {
      return RenderBlockFlow::LocalRectsForRepaint(repaintOutlineBounds);
    }

    bool flippedInline = TableWritingMode().IsInlineFlipped();
    LayoutUnit outlineSize {Style().OutlineSize()};
    LayoutUnit left = std::max(BorderHalfLeft(true), outlineSize);
    LayoutUnit right = std::max(BorderHalfRight(true), outlineSize);
    LayoutUnit top = std::max(BorderHalfTop(true), outlineSize);
    LayoutUnit bottom = std::max(BorderHalfBottom(true), outlineSize);
    if ((left && !flippedInline) || (right && flippedInline))
    {
      if (RenderTableCell *before = Table()->CellBefore(this))
      {
        top = std::max(top, before->BorderHalfTop(true));
        bottom = std::max(bottom, before->BorderHalfBottom(true));
      }
    }
    if ((left && flippedInline) || (right && !flippedInline))
    {
      if (RenderTableCell *after = Table()->CellAfter(this))
      {
        top = std::max(top, after->BorderHalfTop(true));
        bottom = std::max(bottom, after->BorderHalfBottom(true));
      }
    }
    if (top)
    {
      if (RenderTableCell *above = Table()->CellAbove(this))
      {
        left = std::max(left, above->BorderHalfLeft(true));
        right = std::max(right, above->BorderHalfRight(true));
      }
    }
    if (bottom)
    {
      if (RenderTableCell *below = Table()->CellBelow(this))
      {
        left = std::max(left, below->BorderHalfLeft(true));
        right = std::max(right, below->BorderHalfRight(true));
      }
    }

    LayoutPoint location(std::max<LayoutUnit>(left, -VisualOverflowRect().X()),
                         std::max<LayoutUnit>(top, -VisualOverflowRect().Y()));
    auto overflowRect = LayoutRect(-location.X(), -location.Y(),
                                   location.X() + std::max(Width() + right, VisualOverflowRect().MaxX()),
                                   location.Y() + std::max(Height() + bottom, VisualOverflowRect().MaxY()));

    // FIXME: layoutDelta needs to be applied in parts before/after transforms and
    // repaint containers. https://bugs.webkit.org/show_bug.cgi?id=23308
    overflowRect.Move(View().FrameView().LayoutContext().LayoutDelta());

    auto rects = RepaintRects {overflowRect};
    if (repaintOutlineBounds == RepaintOutlineBounds::Yes)
    {
      rects.OutlineBoundsRect = LocalOutlineBoundsRepaintRect();
    }

    return rects;
  }

  auto RenderTableCell::ComputeVisibleRectsInContainer(const RepaintRects &rects,
                                                       const RenderLayerModelObject *container,
                                                       VisibleRectContext context) const noexcept
    -> Maybe<RepaintRects>
  {
    if (container == this)
    {
      return rects;
    }

    auto adjustedRects = rects;
    if ((!View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled() || container
         || context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
        && Parent())
    {
      // Rows are in the same coordinate space, so don't add their offset in.
      adjustedRects.MoveBy(-ParentBox()->Location());
    }

    return RenderBlockFlow::ComputeVisibleRectsInContainer(adjustedRects, container, context);
  }

  void RenderTableCell::WillBeRemovedFromTree() noexcept
  {
    RenderBlockFlow::WillBeRemovedFromTree();
    if (!Table() || !Section())
    {
      return;
    }

    auto *section = Section();
    Table()->InvalidateCollapsedBorders();
    section->RemoveCachedCollapsedBorders(*this);
    section->SetNeedsCellRecalc();
  }

  const html::HTMLTableCellElement *RenderTableCell::TableCellElement() const noexcept
  {
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(GetElement());
    return htmlElement ? DynamicDowncast<html::HTMLTableCellElement>(*htmlElement) : nullptr;
  }

  uint32 RenderTableCell::ParseColSpanFromDOM() const noexcept
  {
    krys_debug_assert(GetElement());
    if (auto *cell = TableCellElement())
    {
      return std::min<uint32>(cell->ColSpan(), MaxColumnIndex);
    }
    // FIXME: MathML table cells (mtd), once there's MathML.
    return 1;
  }

  uint32 RenderTableCell::ParseRowSpanFromDOM() const noexcept
  {
    krys_debug_assert(GetElement());
    if (auto *cell = TableCellElement())
    {
      // When rowspan=0, the HTML spec says it should apply to the full remaining rows.
      // https://html.spec.whatwg.org/multipage/tables.html#attr-tdth-rowspan
      // > For this attribute, the value zero means that the cell is
      // > to span all the remaining rows in the row group.
      auto rowSpan = cell->RowSpan();
      return std::min<uint32>(rowSpan ? rowSpan : MaxRowspan, RenderTableRow::MaxRowIndex);
    }
    // FIXME: MathML table cells (mtd), once there's MathML.
    return 1;
  }

  void RenderTableCell::UpdateColAndRowSpanFlags() noexcept
  {
    // The vast majority of table cells do not have a colspan or rowspan,
    // so we keep a bool to know if we need to bother reading from the DOM.
    _hasColSpan = GetElement() && ParseColSpanFromDOM() != 1;
    _hasRowSpan = GetElement() && ParseRowSpanFromDOM() != 1;
  }

  void RenderTableCell::ColSpanOrRowSpanChanged() noexcept
  {
    krys_debug_assert(GetElement());

    UpdateColAndRowSpanFlags();

    // FIXME: I suspect that we could return early here if !_hasColSpan && !_hasRowSpan.

    SetNeedsLayoutAndPreferredWidthsUpdate();
    if (Parent() && Section())
    {
      Section()->SetNeedsCellRecalc();
    }
  }

  Pair<style::PreferredSize, style::ZoomFactor> RenderTableCell::StyleOrColLogicalWidth() const noexcept
  {
    const auto &style = Style();
    const auto &styleWidth = style.LogicalWidth();
    if (!styleWidth.IsAuto())
    {
      return {styleWidth, style.UsedZoomForLength()};
    }

    if (auto *firstColumn = Table()->ColElement(Col()))
    {
      // LogicalWidthFromColumns will return a zoomed size so we return a zoom
      // factor of 1.0 to avoid double zooming.
      return {LogicalWidthFromColumns(firstColumn, styleWidth),
              style::ZoomFactor {1.0f, style.DeviceScaleFactor()}};
    }

    return {styleWidth, style.UsedZoomForLength()};
  }

  style::PreferredSize
    RenderTableCell::LogicalWidthFromColumns(RenderTableCol *firstColForThisCell,
                                             const style::PreferredSize &widthFromStyle) const noexcept
  {
    krys_debug_assert(firstColForThisCell && firstColForThisCell == Table()->ColElement(Col()));
    auto *tableCol = firstColForThisCell;

    auto colSpanCount = ColSpan();
    LayoutUnit colWidthSum;
    for (uint32 i = 1; i <= colSpanCount; i++)
    {
      const auto &colWidth = tableCol->Style().LogicalWidth();

      // Percentage value should be returned only for colSpan == 1.
      // Otherwise we return original width for the cell.
      auto fixedColWidth = colWidth.TryFixed();
      if (!fixedColWidth)
      {
        if (colSpanCount > 1)
        {
          return widthFromStyle;
        }
        return colWidth;
      }

      colWidthSum += style::Evaluate<LayoutUnit>(*fixedColWidth, tableCol->Style().UsedZoomForLength());
      tableCol = tableCol->NextColumn();
      // If no next <col> tag found for the span we just return what we have for now.
      if (!tableCol)
      {
        break;
      }
    }

    // Column widths specified on <col> apply to the border box of the cell, see bug 8126.
    // FIXME: Why is border/padding ignored in the negative width case?
    if (colWidthSum > 0)
    {
      return style::PreferredSize::Fixed {
        std::max(0_lu, colWidthSum - BorderAndPaddingLogicalWidth()).ToFloat()};
    }
    return style::PreferredSize::Fixed {colWidthSum.ToFloat()};
  }

  void RenderTableCell::ComputePreferredLogicalWidths() noexcept
  {
    // The child cells rely on the grids up in the sections to do their ComputePreferredLogicalWidths work.
    // Normally the sections are set up early, as table cells are added, but relayout can cause the cells to
    // be freed, leaving stale pointers in the sections' grids.  We must refresh those grids before the child
    // cells try to use them.
    Table()->RecalcSectionsIfNeeded();

    // We don't want the preferred width from children to be affected by any
    // notional height on the cell, such as can happen when a percent sized image
    // scales up its width to match the available height. Setting a zero override
    // height prevents this from happening.
    auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight();
    if (overridingLogicalHeight)
    {
      SetOverridingBorderBoxLogicalHeight(LayoutUnit {});
    }
    RenderBlockFlow::ComputePreferredLogicalWidths();
    if (overridingLogicalHeight)
    {
      SetOverridingBorderBoxLogicalHeight(*overridingLogicalHeight);
    }

    if (!GetElement() || !Style().AutoWrap() || !GetElement()->HasAttribute(u8"nowrap"))
    {
      return;
    }

    auto [logicalWidth, usedZoom] = StyleOrColLogicalWidth();
    if (auto fixedLogicalWidth = logicalWidth.TryFixed())
    {
      // Nowrap is set, but we didn't actually use it because of the
      // fixed width set on the cell. Even so, it is a WinIE/Moz trait
      // to make the minwidth of the cell into the fixed width. They do this
      // even in strict mode, so do not make this a quirk. Affected the top
      // of hiptop.com.
      _minPreferredLogicalWidth =
        std::max(style::Evaluate<LayoutUnit>(*fixedLogicalWidth, usedZoom), _minPreferredLogicalWidth);
    }
  }

  bool RenderTableCell::ComputeIntrinsicPadding(LayoutUnit heightConstraint) noexcept
  {
    auto oldIntrinsicPaddingBefore = LayoutUnit {};
    auto oldIntrinsicPaddingAfter = LayoutUnit {};
    // LogicalHeight() here means orginal border box height (i.e. no column stretching for orthogonal box).
    auto borderBoxLogicalHeight = LayoutUnit {};
    if (!IsOrthogonal())
    {
      oldIntrinsicPaddingBefore = IntrinsicPaddingBefore();
      oldIntrinsicPaddingAfter = IntrinsicPaddingAfter();
      borderBoxLogicalHeight = LogicalHeight();
    }
    else
    {
      krys_debug_assert(_orthogonalCellContentIntrinsicHeight.has_value());
      borderBoxLogicalHeight = _orthogonalCellContentIntrinsicHeight.value_or(0_lu)
                               + RenderBlockFlow::PaddingBefore() + RenderBlockFlow::PaddingAfter()
                               + BorderBefore() + BorderAfter();
    }

    auto borderBoxLogicalHeightWithoutIntrinsicPadding =
      borderBoxLogicalHeight - (oldIntrinsicPaddingBefore + oldIntrinsicPaddingAfter);
    auto intrinsicPaddingBefore = oldIntrinsicPaddingBefore;

    auto alignment = Style().GetVerticalAlign();
    if (auto alignContent = Style().GetAlignContent(); !alignContent.IsNormal())
    {
      auto resolveAlignedContent = alignContent.Resolve();
      // align-content overrides vertical-align
      if (resolveAlignedContent.Position() == ContentPosition::Baseline)
      {
        alignment = css::keywords::Baseline {};
      }
      else if (resolveAlignedContent.IsCentered())
      {
        alignment = css::keywords::Middle {};
      }
      else if (resolveAlignedContent.IsStartward())
      {
        alignment = css::keywords::Top {};
      }
      else if (resolveAlignedContent.IsEndward())
      {
        alignment = css::keywords::Bottom {};
      }
    }

    auto applyStandard = [&]
    {
      auto baseline = CellBaselinePosition();
      if (!FirstInFlowChild() && baseline == BorderAndPaddingBefore())
      {
        // If baseline equals BorderAndPaddingBefore(), there is no real content.
        intrinsicPaddingBefore = 0_lu;
        return;
      }

      auto needsIntrinsicPadding = baseline > BorderAndPaddingBefore() || !borderBoxLogicalHeight;
      if (needsIntrinsicPadding)
      {
        intrinsicPaddingBefore = Section()->RowBaseline(RowIndex()) - (baseline - oldIntrinsicPaddingBefore);
      }
    };

    alignment.SwitchOn(
      [&](const css::keywords::Sub &) { applyStandard(); }, [&](const css::keywords::Super &)
      { applyStandard(); }, [&](const css::keywords::TextTop &) { applyStandard(); },
      [&](const css::keywords::TextBottom &) { applyStandard(); },
      [&](const css::keywords::Baseline &) { applyStandard(); },
      [&](const css::keywords::Top &)
      {
        // Do nothing.
      },
      [&](const css::keywords::Middle &)
      { intrinsicPaddingBefore = (heightConstraint - borderBoxLogicalHeightWithoutIntrinsicPadding) / 2; },
      [&](const css::keywords::Bottom &)
      { intrinsicPaddingBefore = heightConstraint - borderBoxLogicalHeightWithoutIntrinsicPadding; },
      [&](const css::keywords::WebkitBaselineMiddle &)
      {
        // Do nothing.
      },
      [&](const style::VerticalAlign::Length &) { applyStandard(); });

    auto intrinsicPaddingAfter =
      heightConstraint - borderBoxLogicalHeightWithoutIntrinsicPadding - intrinsicPaddingBefore;
    SetIntrinsicPaddingBefore(intrinsicPaddingBefore);
    SetIntrinsicPaddingAfter(intrinsicPaddingAfter);

    return intrinsicPaddingBefore != oldIntrinsicPaddingBefore
           || intrinsicPaddingAfter != oldIntrinsicPaddingAfter;
  }

  RenderBox::LogicalExtentComputedValues
    RenderTableCell::ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept
  {
    if (IsOrthogonal())
    {
      // Note that at this point, ContentBoxLogicalHeight means content height.
      _orthogonalCellContentIntrinsicHeight = ContentBoxLogicalHeight();
    }
    return RenderBlockFlow::ComputeLogicalHeight(logicalHeight, logicalTop);
  }

  void RenderTableCell::UpdateLogicalWidth() noexcept
  {
    if (IsComputingPreferredSize())
    {
      // While table layout sets the final logical width for cells, in case of
      // preferred width computation for orthogonal content, we have
      // to follow normal layout flow to be able to compute logical height.
      RenderBlockFlow::UpdateLogicalWidth();
      return;
    }

    if (auto logicalWidth = OverridingBorderBoxLogicalWidth())
    {
      SetLogicalWidth(*logicalWidth);
    }
  }

  LayoutUnit RenderTableCell::LogicalHeightForRowSizing() const noexcept
  {
    auto logicalSizeForRowSizing = [&]
    {
      if (!IsOrthogonal())
      {
        return LogicalHeight() - (IntrinsicPaddingBefore() + IntrinsicPaddingAfter());
      }
      LogicalExtentComputedValues values;
      ComputeLogicalWidth(values);
      return values.Extent;
    };

    // FIXME: This function does too much work, and is very hot during table layout!
    auto usedLogicalSize = logicalSizeForRowSizing();
    const auto &specifiedSize = !IsOrthogonal() ? Style().LogicalHeight() : Style().LogicalWidth();
    if (!specifiedSize.IsSpecified())
    {
      return usedLogicalSize;
    }

    auto computedLogicaSize = style::Evaluate<LayoutUnit>(specifiedSize, 0_lu, Style().UsedZoomForLength());
    // In strict mode, box-sizing: content-box do the right thing and actually add in the border and padding.
    // Call ComputedCSSPadding* directly to avoid including implicitPadding.
    if (!GetDocument().InQuirksMode() && Style().GetBoxSizing() != BoxSizing::BorderBox)
    {
      computedLogicaSize +=
        ComputedCSSPaddingBefore() + ComputedCSSPaddingAfter() + BorderBefore() + BorderAfter();
    }
    return std::max(computedLogicaSize, usedLogicalSize);
  }

  void RenderTableCell::SetCellLogicalWidth(LayoutUnit logicalWidthInTableDirection) noexcept
  {
    auto logicalSizeInTableDirection = !IsOrthogonal() ? LogicalWidth() : LogicalHeight();
    if (logicalWidthInTableDirection == logicalSizeInTableDirection)
    {
      return;
    }

    SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    SetCellWidthChanged(true);

    if (!IsOrthogonal())
    {
      SetLogicalWidth(logicalWidthInTableDirection);
      return;
    }

    SetLogicalHeight(logicalWidthInTableDirection);
    // As table layout drives the size of table cells, we have to prevent regular layout flow from
    // overriding this height value. This is similar to how RenderTableCell handles logical widths by
    // overriding UpdateLogicalWidth.
    SetOverridingBorderBoxLogicalHeight(logicalWidthInTableDirection);
  }

  void RenderTableCell::Layout() noexcept
  {
    auto oldCellBaseline = CellBaselinePosition().ToInt();
    auto scope = LayoutScope {*this};

    LayoutBlock(CellWidthChanged() ? RelayoutChildren::Yes : RelayoutChildren::No);

    // If we have replaced content, the intrinsic height of our content may have changed since the last time
    // we laid out. If that's the case the intrinsic padding we used for layout (the padding required to push
    // the contents of the cell down to the row's baseline) is included in our new height and baseline and
    // makes both of them wrong. So if our content's intrinsic height has changed push the new content up into
    // the intrinsic padding and relayout so that the rest of table and row layout can use the correct
    // baseline and height for this cell.
    if (IsBaselineAligned() && Section()->RowBaseline(RowIndex())
        && CellBaselinePosition() > Section()->RowBaseline(RowIndex()))
    {
      auto newIntrinsicPaddingBefore =
        std::max(0_lu, IntrinsicPaddingBefore()
                         - std::max(0_lu, CellBaselinePosition() - LayoutUnit(oldCellBaseline)));
      SetIntrinsicPaddingBefore(newIntrinsicPaddingBefore);
      SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      LayoutBlock(CellWidthChanged() ? RelayoutChildren::Yes : RelayoutChildren::No);
    }
    InvalidateHasEmptyCollapsedBorders();

    // FIXME: This value isn't the intrinsic content logical height, but we need
    // to update the value as its used by flexbox layout. crbug.com/367324
    CacheIntrinsicContentLogicalHeightForFlexItem(ContentBoxLogicalHeight());

    SetCellWidthChanged(false);
  }

  RectEdges<LayoutUnit> RenderTableCell::Padding() const noexcept
  {
    auto top = ComputedCSSPaddingTop();
    auto right = ComputedCSSPaddingRight();
    auto bottom = ComputedCSSPaddingBottom();
    auto left = ComputedCSSPaddingLeft();

    if (IsHorizontalWritingMode())
    {
      bool isTopToBottom = GetWritingMode().IsBlockTopToBottom();
      top += isTopToBottom ? IntrinsicPaddingBefore() : IntrinsicPaddingAfter();
      bottom += isTopToBottom ? IntrinsicPaddingAfter() : IntrinsicPaddingBefore();
    }
    else
    {
      bool isLeftToRight = GetWritingMode().IsBlockLeftToRight();
      left += isLeftToRight ? IntrinsicPaddingBefore() : IntrinsicPaddingAfter();
      right += isLeftToRight ? IntrinsicPaddingAfter() : IntrinsicPaddingBefore();
    }

    return {top, right, bottom, left};
  }

  LayoutUnit RenderTableCell::PaddingTop() const noexcept
  {
    auto result = ComputedCSSPaddingTop();
    if (!IsHorizontalWritingMode())
    {
      return result;
    }
    return result
           + (GetWritingMode().IsBlockTopToBottom() ? IntrinsicPaddingBefore() : IntrinsicPaddingAfter());
  }

  LayoutUnit RenderTableCell::PaddingBottom() const noexcept
  {
    auto result = ComputedCSSPaddingBottom();
    if (!IsHorizontalWritingMode())
    {
      return result;
    }
    return result
           + (GetWritingMode().IsBlockTopToBottom() ? IntrinsicPaddingAfter() : IntrinsicPaddingBefore());
  }

  LayoutUnit RenderTableCell::PaddingLeft() const noexcept
  {
    auto result = ComputedCSSPaddingLeft();
    if (IsHorizontalWritingMode())
    {
      return result;
    }
    return result
           + (GetWritingMode().IsBlockLeftToRight() ? IntrinsicPaddingBefore() : IntrinsicPaddingAfter());
  }

  LayoutUnit RenderTableCell::PaddingRight() const noexcept
  {
    auto result = ComputedCSSPaddingRight();
    if (IsHorizontalWritingMode())
    {
      return result;
    }
    return result
           + (GetWritingMode().IsBlockLeftToRight() ? IntrinsicPaddingAfter() : IntrinsicPaddingBefore());
  }

  LayoutUnit RenderTableCell::PaddingBefore() const noexcept
  {
    return ComputedCSSPaddingBefore() + IntrinsicPaddingBefore();
  }

  LayoutUnit RenderTableCell::PaddingAfter() const noexcept
  {
    return ComputedCSSPaddingAfter() + IntrinsicPaddingAfter();
  }

  void RenderTableCell::SetOverridingLogicalHeightFromRowHeight(LayoutUnit rowHeight) noexcept
  {
    ClearIntrinsicPadding();
    SetOverridingBorderBoxLogicalHeight(rowHeight);
  }

  LayoutUnit RenderTableCell::MinLogicalWidthForColumnSizing() noexcept
  {
    if (!IsOrthogonal())
    {
      return RenderBlockFlow::MinPreferredLogicalWidth();
    }

    auto computingPreferredSize = SetForScope {_isComputingPreferredSize, true};
    SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    LayoutIfNeeded();
    krys_debug_assert(_orthogonalCellContentIntrinsicHeight.has_value());
    return std::max(LogicalHeight(), _orthogonalCellContentIntrinsicHeight.value_or(0_lu));
  }

  LayoutUnit RenderTableCell::MaxLogicalWidthForColumnSizing() noexcept
  {
    if (!IsOrthogonal())
    {
      return RenderBlockFlow::MaxPreferredLogicalWidth();
    }

    auto computingPreferredSize = SetForScope {_isComputingPreferredSize, true};
    SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    LayoutIfNeeded();
    krys_debug_assert(_orthogonalCellContentIntrinsicHeight.has_value());
    return std::max(LogicalHeight(), _orthogonalCellContentIntrinsicHeight.value_or(0_lu));
  }

  LayoutUnit RenderTableCell::CellBaselinePosition() const noexcept
  {
    // <http://www.w3.org/TR/2007/CR-CSS21-20070719/tables.html#height-layout>: The baseline of a cell is the
    // baseline of the first in-flow line box in the cell, or the first in-flow table-row in the cell,
    // whichever comes first. If there is no such line box or table-row, the baseline is the bottom of content
    // edge of the cell box.
    if (!IsOrthogonal())
    {
      return FirstLineBaseline().value_or(BorderAndPaddingBefore() + ContentBoxLogicalHeight());
    }
    return {};
  }

  bool RenderTableCell::IsBaselineAligned() const noexcept
  {
    if (auto alignContent = Style().GetAlignContent(); !alignContent.IsNormal())
    {
      return alignContent.IsFirstBaseline();
    }

    const auto &verticalAlign = Style().GetVerticalAlign();
    return verticalAlign.IsBaseline() || verticalAlign.IsTextBottom() || verticalAlign.IsTextTop()
           || verticalAlign.IsSuper() || verticalAlign.IsSub() || verticalAlign.IsLength();
  }

  bool RenderTableCell::IsOrthogonal() const noexcept
  {
    if (auto *row = Row())
    {
      return GetWritingMode().IsOrthogonal(row->GetWritingMode());
    }
    krys_debug_assert(false);
    return false;
  }

  void RenderTableCell::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    krys_debug_assert(Style().Display() == DisplayType::TableCell);
    krys_debug_assert(!Row() || Row()->RowIndexWasSet());

    RenderBlockFlow::StyleDidChange(diff, oldStyle);

    if (Parent() && Section() && oldStyle && Style().Height() != oldStyle->Height())
    {
      Section()->RowLogicalHeightChanged(RowIndex());
    }

    // Our intrinsic padding pushes us down to align with the baseline of other cells on the row. If our
    // vertical-align has changed then so will the padding needed to align with other cells - clear it so we
    // can recalculate it from scratch.
    if (oldStyle
        && (Style().GetVerticalAlign() != oldStyle->GetVerticalAlign()
            || Style().GetAlignContent() != oldStyle->GetAlignContent()))
    {
      ClearIntrinsicPadding();
    }

    if (auto *table = Table(); table && oldStyle)
    {
      table->InvalidateCollapsedBordersAfterStyleChangeIfNeeded(*oldStyle, Style(), this);
      if (table->CollapseBorders() && diff == style::DifferenceResult::Layout)
      {
        MarkCellDirtyWhenCollapsedBorderChanges(table->CellBelow(this));
        MarkCellDirtyWhenCollapsedBorderChanges(table->CellAbove(this));
        MarkCellDirtyWhenCollapsedBorderChanges(table->CellBefore(this));
        MarkCellDirtyWhenCollapsedBorderChanges(table->CellAfter(this));
      }
    }
  }

  bool RenderTableCell::HasStartBorderAdjoiningTable() const noexcept
  {
    return !Col();
  }

  bool RenderTableCell::HasEndBorderAdjoiningTable() const noexcept
  {
    return Table()->ColToEffCol(Col() + ColSpan() - 1) == Table()->NumEffCols() - 1;
  }

  CollapsedBorderValue RenderTableCell::EmptyBorder() const noexcept
  {
    return CollapsedBorderValue(BorderValue(), Color(), BorderPrecedence::Cell, Style().UsedZoomForLength());
  }

  void RenderTableCell::SetHasEmptyCollapsedBorder(CollapsedBorderSide side, bool empty) const noexcept
  {
    switch (side)
    {
      case CollapsedBorderSide::After:  _hasEmptyCollapsedAfterBorder = empty; break;
      case CollapsedBorderSide::Before: _hasEmptyCollapsedBeforeBorder = empty; break;
      case CollapsedBorderSide::Start:  _hasEmptyCollapsedStartBorder = empty; break;
      case CollapsedBorderSide::End:    _hasEmptyCollapsedEndBorder = empty; break;
    }
    if (empty)
    {
      Table()->SetCollapsedEmptyBorderIsPresent();
    }
  }

  CollapsedBorderValue RenderTableCell::CollapsedStartBorder(IncludeBorderColor includeColor) const noexcept
  {
    if (!Table() || !Section())
    {
      return EmptyBorder();
    }

    if (_hasEmptyCollapsedStartBorder)
    {
      return EmptyBorder();
    }

    if (Table()->CollapsedBordersAreValid())
    {
      return Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Start);
    }

    auto result = ComputeCollapsedStartBorder(includeColor);
    SetHasEmptyCollapsedBorder(CollapsedBorderSide::Start, !result.Width());
    if (includeColor == IncludeBorderColor::Yes && !_hasEmptyCollapsedStartBorder)
    {
      Section()->SetCachedCollapsedBorder(*this, CollapsedBorderSide::Start, result);
    }
    return result;
  }

  CollapsedBorderValue
    RenderTableCell::ComputeCollapsedStartBorder(IncludeBorderColor includeColor) const noexcept
  {
    // For the start border, we need to check, in order of precedence:
    // (1) Our start border.
    auto startColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderInlineStartColor, TableWritingMode());
    auto endColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderInlineEndColor, TableWritingMode());
    auto result = MakeCollapsedBorder(*this, Style().BorderStart(TableWritingMode()), includeColor,
                                      startColorProperty, BorderPrecedence::Cell);

    auto *table = Table();
    if (!table)
    {
      return result;
    }

    // (2) The end border of the preceding cell.
    auto *cellBefore = table->CellBefore(this);
    if (cellBefore)
    {
      auto cellBeforeAdjoiningBorder =
        MakeCollapsedBorder(*cellBefore, cellBefore->BorderAdjoiningCellAfter(*this), includeColor,
                            endColorProperty, BorderPrecedence::Cell);
      // |result| should be the 2nd argument as |cellBefore| should win in case of equality per CSS 2.1
      // (Border conflict resolution, point 4).
      result = ChooseBorder(cellBeforeAdjoiningBorder, result);
      if (!result.Exists())
      {
        return result;
      }
    }

    bool startBorderAdjoinsTable = HasStartBorderAdjoiningTable();
    if (startBorderAdjoinsTable)
    {
      // (3) Our row's start border.
      result =
        ChooseBorder(result, MakeCollapsedBorder(*Row(), Row()->BorderAdjoiningStartCell(*this), includeColor,
                                                 startColorProperty, BorderPrecedence::Row));
      if (!result.Exists())
      {
        return result;
      }

      // (4) Our row group's start border.
      result = ChooseBorder(
        result, MakeCollapsedBorder(*Section(), Section()->BorderAdjoiningStartCell(*this), includeColor,
                                    startColorProperty, BorderPrecedence::RowGroup));
      if (!result.Exists())
      {
        return result;
      }
    }

    // (5) Our column and column group's start borders.
    bool startColEdge = false;
    bool endColEdge = false;
    if (auto *colElt = table->ColElement(Col(), &startColEdge, &endColEdge))
    {
      if (colElt->IsTableColumnGroup() && startColEdge)
      {
        // The |colElt| is a column group and is also the first colgroup (in case of spanned colgroups).
        result = ChooseBorder(result, MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellStartBorder(),
                                                          includeColor, startColorProperty,
                                                          BorderPrecedence::ColumnGroup));
        if (!result.Exists())
        {
          return result;
        }
      }
      else if (!colElt->IsTableColumnGroup())
      {
        // We first consider the |colElt| and irrespective of whether it is a spanned col or not, we apply
        // its start border. This is as per HTML5 which states that: "For the purposes of the CSS table model,
        // the col element is expected to be treated as if it was present as many times as its span attribute
        // specifies".
        result = ChooseBorder(result, MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellStartBorder(),
                                                          includeColor, startColorProperty,
                                                          BorderPrecedence::Column));
        if (!result.Exists())
        {
          return result;
        }
        // Next, apply the start border of the enclosing colgroup but only if it is adjacent to the cell's
        // edge.
        if (auto *enclosingColumnGroup = colElt->EnclosingColumnGroupIfAdjacentBefore())
        {
          result = ChooseBorder(
            result,
            MakeCollapsedBorder(*enclosingColumnGroup, enclosingColumnGroup->BorderAdjoiningCellStartBorder(),
                                includeColor, startColorProperty, BorderPrecedence::ColumnGroup));
          if (!result.Exists())
          {
            return result;
          }
        }
      }
    }

    // (6) The end border of the preceding column.
    if (cellBefore)
    {
      if (auto *colElt = table->ColElement(Col() - 1, &startColEdge, &endColEdge))
      {
        if (colElt->IsTableColumnGroup() && endColEdge)
        {
          // The element is a colgroup and is also the last colgroup (in case of spanned colgroups).
          result =
            ChooseBorder(MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellAfter(*this), includeColor,
                                             endColorProperty, BorderPrecedence::ColumnGroup),
                         result);
          if (!result.Exists())
          {
            return result;
          }
        }
        else if (colElt->IsTableColumn())
        {
          // Resolve the collapsing border against the col's border ignoring any 'span' as per HTML5.
          result = ChooseBorder(MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellAfter(*this),
                                                    includeColor, endColorProperty, BorderPrecedence::Column),
                                result);
          if (!result.Exists())
          {
            return result;
          }
          // Next, if the previous col has a parent colgroup then its end border should be applied
          // but only if it is adjacent to the cell's edge.
          if (auto *enclosingColumnGroup = colElt->EnclosingColumnGroupIfAdjacentAfter())
          {
            result = ChooseBorder(
              MakeCollapsedBorder(*enclosingColumnGroup, enclosingColumnGroup->BorderAdjoiningCellEndBorder(),
                                  includeColor, endColorProperty, BorderPrecedence::ColumnGroup),
              result);
            if (!result.Exists())
            {
              return result;
            }
          }
        }
      }
    }

    if (startBorderAdjoinsTable)
    {
      // (7) The table's start border.
      result = ChooseBorder(result, MakeCollapsedBorder(*table, table->Style().BorderStart(), includeColor,
                                                        startColorProperty, BorderPrecedence::Table));
      if (!result.Exists())
      {
        return result;
      }
    }

    return result;
  }

  CollapsedBorderValue RenderTableCell::CollapsedEndBorder(IncludeBorderColor includeColor) const noexcept
  {
    if (!Table() || !Section())
    {
      return EmptyBorder();
    }

    if (_hasEmptyCollapsedEndBorder)
    {
      return EmptyBorder();
    }

    if (Table()->CollapsedBordersAreValid())
    {
      return Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::End);
    }

    auto result = ComputeCollapsedEndBorder(includeColor);
    SetHasEmptyCollapsedBorder(CollapsedBorderSide::End, !result.Width());
    if (includeColor == IncludeBorderColor::Yes && !_hasEmptyCollapsedEndBorder)
    {
      Section()->SetCachedCollapsedBorder(*this, CollapsedBorderSide::End, result);
    }
    return result;
  }

  CollapsedBorderValue
    RenderTableCell::ComputeCollapsedEndBorder(IncludeBorderColor includeColor) const noexcept
  {
    // For end border, we need to check, in order of precedence:
    // (1) Our end border.
    auto startColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderInlineStartColor, TableWritingMode());
    auto endColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderInlineEndColor, TableWritingMode());
    auto result = MakeCollapsedBorder(*this, Style().BorderEnd(TableWritingMode()), includeColor,
                                      endColorProperty, BorderPrecedence::Cell);

    auto *table = Table();
    if (!table)
    {
      return result;
    }

    // Note: We have to use the effective column information instead of whether we have a cell after as a
    // table doesn't have to be regular (any row can have less cells than the total cell count).
    bool isEndColumn = table->ColToEffCol(Col() + ColSpan() - 1) == table->NumEffCols() - 1;

    // (2) The start border of the following cell.
    if (!isEndColumn)
    {
      if (auto *cellAfter = table->CellAfter(this))
      {
        auto cellAfterAdjoiningBorder =
          MakeCollapsedBorder(*cellAfter, cellAfter->BorderAdjoiningCellBefore(*this), includeColor,
                              startColorProperty, BorderPrecedence::Cell);
        result = ChooseBorder(result, cellAfterAdjoiningBorder);
        if (!result.Exists())
        {
          return result;
        }
      }
    }

    bool endBorderAdjoinsTable = HasEndBorderAdjoiningTable();
    if (endBorderAdjoinsTable)
    {
      // (3) Our row's end border.
      result =
        ChooseBorder(result, MakeCollapsedBorder(*Row(), Row()->BorderAdjoiningEndCell(*this), includeColor,
                                                 endColorProperty, BorderPrecedence::Row));
      if (!result.Exists())
      {
        return result;
      }

      // (4) Our row group's end border.
      result =
        ChooseBorder(result, MakeCollapsedBorder(*Section(), Section()->BorderAdjoiningEndCell(*this),
                                                 includeColor, endColorProperty, BorderPrecedence::RowGroup));
      if (!result.Exists())
      {
        return result;
      }
    }

    // (5) Our column and column group's end borders.
    bool startColEdge = false;
    bool endColEdge = false;
    if (auto *colElt = table->ColElement(Col() + ColSpan() - 1, &startColEdge, &endColEdge))
    {
      if (colElt->IsTableColumnGroup() && endColEdge)
      {
        // The element is a colgroup and is also the last colgroup (in case of spanned colgroups).
        result = ChooseBorder(result, MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellEndBorder(),
                                                          includeColor, endColorProperty,
                                                          BorderPrecedence::ColumnGroup));
        if (!result.Exists())
        {
          return result;
        }
      }
      else if (!colElt->IsTableColumnGroup())
      {
        // First apply the end border of the column irrespective of whether it is spanned or not. This is as
        // per HTML5 which states that: "For the purposes of the CSS table model, the col element is expected
        // to be treated as if it was present as many times as its span attribute specifies".
        result =
          ChooseBorder(result, MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellEndBorder(),
                                                   includeColor, endColorProperty, BorderPrecedence::Column));
        if (!result.Exists())
        {
          return result;
        }
        // Next, if it has a parent colgroup then we apply its end border but only if it is adjacent to the
        // cell.
        if (auto *enclosingColumnGroup = colElt->EnclosingColumnGroupIfAdjacentAfter())
        {
          result = ChooseBorder(
            result,
            MakeCollapsedBorder(*enclosingColumnGroup, enclosingColumnGroup->BorderAdjoiningCellEndBorder(),
                                includeColor, endColorProperty, BorderPrecedence::ColumnGroup));
          if (!result.Exists())
          {
            return result;
          }
        }
      }
    }

    // (6) The start border of the next column.
    if (!isEndColumn)
    {
      if (auto *colElt = table->ColElement(Col() + ColSpan(), &startColEdge, &endColEdge))
      {
        if (colElt->IsTableColumnGroup() && startColEdge)
        {
          // This case is a colgroup without any col, we only compute it if it is adjacent to the cell's edge.
          result = ChooseBorder(result, MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellBefore(*this),
                                                            includeColor, startColorProperty,
                                                            BorderPrecedence::ColumnGroup));
          if (!result.Exists())
          {
            return result;
          }
        }
        else if (colElt->IsTableColumn())
        {
          // Resolve the collapsing border against the col's border ignoring any 'span' as per HTML5.
          result = ChooseBorder(result, MakeCollapsedBorder(*colElt, colElt->BorderAdjoiningCellBefore(*this),
                                                            includeColor, startColorProperty,
                                                            BorderPrecedence::Column));
          if (!result.Exists())
          {
            return result;
          }
          // If we have a parent colgroup, resolve the border only if it is adjacent to the cell.
          if (auto *enclosingColumnGroup = colElt->EnclosingColumnGroupIfAdjacentBefore())
          {
            result = ChooseBorder(
              result, MakeCollapsedBorder(*enclosingColumnGroup,
                                          enclosingColumnGroup->BorderAdjoiningCellStartBorder(),
                                          includeColor, startColorProperty, BorderPrecedence::ColumnGroup));
            if (!result.Exists())
            {
              return result;
            }
          }
        }
      }
    }

    if (endBorderAdjoinsTable)
    {
      // (7) The table's end border.
      result = ChooseBorder(result, MakeCollapsedBorder(*table, table->Style().BorderEnd(), includeColor,
                                                        endColorProperty, BorderPrecedence::Table));
      if (!result.Exists())
      {
        return result;
      }
    }

    return result;
  }

  CollapsedBorderValue RenderTableCell::CollapsedBeforeBorder(IncludeBorderColor includeColor) const noexcept
  {
    if (!Table() || !Section())
    {
      return EmptyBorder();
    }

    if (_hasEmptyCollapsedBeforeBorder)
    {
      return EmptyBorder();
    }

    if (Table()->CollapsedBordersAreValid())
    {
      return Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Before);
    }

    auto result = ComputeCollapsedBeforeBorder(includeColor);
    SetHasEmptyCollapsedBorder(CollapsedBorderSide::Before, !result.Width());
    if (includeColor == IncludeBorderColor::Yes && !_hasEmptyCollapsedBeforeBorder)
    {
      Section()->SetCachedCollapsedBorder(*this, CollapsedBorderSide::Before, result);
    }
    return result;
  }

  CollapsedBorderValue
    RenderTableCell::ComputeCollapsedBeforeBorder(IncludeBorderColor includeColor) const noexcept
  {
    // For before border, we need to check, in order of precedence:
    // (1) Our before border.
    auto tableWritingMode = TableWritingMode();
    auto beforeColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderBlockStartColor, tableWritingMode);
    auto afterColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderBlockEndColor, tableWritingMode);
    auto result = MakeCollapsedBorder(*this, Style().BorderBefore(tableWritingMode), includeColor,
                                      beforeColorProperty, BorderPrecedence::Cell);

    auto *table = Table();
    if (!table)
    {
      return result;
    }

    auto *previousCell = table->CellAbove(this);
    if (previousCell)
    {
      // (2) A before cell's after border.
      result =
        ChooseBorder(MakeCollapsedBorder(*previousCell, previousCell->Style().BorderAfter(tableWritingMode),
                                         includeColor, afterColorProperty, BorderPrecedence::Cell),
                     result);
      if (!result.Exists())
      {
        return result;
      }
    }

    // (3) Our row's before border.
    result =
      ChooseBorder(result, MakeCollapsedBorder(*Parent(), Parent()->Style().BorderBefore(tableWritingMode),
                                               includeColor, beforeColorProperty, BorderPrecedence::Row));
    if (!result.Exists())
    {
      return result;
    }

    // (4) The previous row's after border.
    if (previousCell)
    {
      RenderTableRow *previousRow = nullptr;
      if (previousCell->Section() == Section())
      {
        previousRow = DynamicDowncast<RenderTableRow>(Parent()->PreviousSibling());
      }
      else
      {
        previousRow = previousCell->Section()->LastRow();
      }

      if (previousRow)
      {
        result =
          ChooseBorder(MakeCollapsedBorder(*previousRow, previousRow->Style().BorderAfter(tableWritingMode),
                                           includeColor, afterColorProperty, BorderPrecedence::Row),
                       result);
        if (!result.Exists())
        {
          return result;
        }
      }
    }

    // Now check row groups.
    auto *currSection = Section();
    if (!RowIndex())
    {
      // (5) Our row group's before border.
      result = ChooseBorder(
        result, MakeCollapsedBorder(*currSection, currSection->Style().BorderBefore(tableWritingMode),
                                    includeColor, beforeColorProperty, BorderPrecedence::RowGroup));
      if (!result.Exists())
      {
        return result;
      }

      // (6) Previous row group's after border.
      currSection = table->SectionAbove(currSection, SkipEmptySections::Yes);
      if (currSection)
      {
        result =
          ChooseBorder(MakeCollapsedBorder(*currSection, currSection->Style().BorderAfter(tableWritingMode),
                                           includeColor, afterColorProperty, BorderPrecedence::RowGroup),
                       result);
        if (!result.Exists())
        {
          return result;
        }
      }
    }

    if (!currSection)
    {
      // (8) Our column and column group's before borders.
      if (auto *colElt = table->ColElement(Col()))
      {
        result = ChooseBorder(
          result, MakeCollapsedBorder(*colElt, colElt->Style().BorderBefore(tableWritingMode), includeColor,
                                      beforeColorProperty, BorderPrecedence::Column));
        if (!result.Exists())
        {
          return result;
        }
        if (auto *enclosingColumnGroup = colElt->EnclosingColumnGroup())
        {
          result = ChooseBorder(
            result, MakeCollapsedBorder(*enclosingColumnGroup,
                                        enclosingColumnGroup->Style().BorderBefore(tableWritingMode),
                                        includeColor, beforeColorProperty, BorderPrecedence::ColumnGroup));
          if (!result.Exists())
          {
            return result;
          }
        }
      }

      // (9) The table's before border.
      result = ChooseBorder(result, MakeCollapsedBorder(*table, table->Style().BorderBefore(), includeColor,
                                                        beforeColorProperty, BorderPrecedence::Table));
      if (!result.Exists())
      {
        return result;
      }
    }

    return result;
  }

  CollapsedBorderValue RenderTableCell::CollapsedAfterBorder(IncludeBorderColor includeColor) const noexcept
  {
    if (!Table() || !Section())
    {
      return EmptyBorder();
    }

    if (_hasEmptyCollapsedAfterBorder)
    {
      return EmptyBorder();
    }

    if (Table()->CollapsedBordersAreValid())
    {
      return Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::After);
    }

    auto result = ComputeCollapsedAfterBorder(includeColor);
    SetHasEmptyCollapsedBorder(CollapsedBorderSide::After, !result.Width());
    if (includeColor == IncludeBorderColor::Yes && !_hasEmptyCollapsedAfterBorder)
    {
      Section()->SetCachedCollapsedBorder(*this, CollapsedBorderSide::After, result);
    }
    return result;
  }

  CollapsedBorderValue
    RenderTableCell::ComputeCollapsedAfterBorder(IncludeBorderColor includeColor) const noexcept
  {
    // For after border, we need to check, in order of precedence:
    // (1) Our after border.
    auto tableWritingMode = TableWritingMode();
    auto beforeColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderBlockStartColor, tableWritingMode);
    auto afterColorProperty =
      ColorProperty(includeColor, css::PropertyId::BorderBlockEndColor, tableWritingMode);
    auto result = MakeCollapsedBorder(*this, Style().BorderAfter(tableWritingMode), includeColor,
                                      afterColorProperty, BorderPrecedence::Cell);

    auto *table = Table();
    if (!table)
    {
      return result;
    }

    auto *nextCell = table->CellBelow(this);
    if (nextCell)
    {
      // (2) An after cell's before border.
      result =
        ChooseBorder(result, MakeCollapsedBorder(*nextCell, nextCell->Style().BorderBefore(tableWritingMode),
                                                 includeColor, beforeColorProperty, BorderPrecedence::Cell));
      if (!result.Exists())
      {
        return result;
      }
    }

    // (3) Our row's after border. (FIXME: Deal with rowspan!)
    result =
      ChooseBorder(result, MakeCollapsedBorder(*Parent(), Parent()->Style().BorderAfter(tableWritingMode),
                                               includeColor, afterColorProperty, BorderPrecedence::Row));
    if (!result.Exists())
    {
      return result;
    }

    // (4) The next row's before border.
    if (nextCell)
    {
      result =
        ChooseBorder(result, MakeCollapsedBorder(*nextCell->Parent(),
                                                 nextCell->Parent()->Style().BorderBefore(tableWritingMode),
                                                 includeColor, beforeColorProperty, BorderPrecedence::Row));
      if (!result.Exists())
      {
        return result;
      }
    }

    // Now check row groups.
    auto *currSection = Section();
    if (RowIndex() + RowSpan() >= currSection->NumRows())
    {
      // (5) Our row group's after border.
      result = ChooseBorder(
        result, MakeCollapsedBorder(*currSection, currSection->Style().BorderAfter(tableWritingMode),
                                    includeColor, afterColorProperty, BorderPrecedence::RowGroup));
      if (!result.Exists())
      {
        return result;
      }

      // (6) Following row group's before border.
      currSection = table->SectionBelow(currSection, SkipEmptySections::Yes);
      if (currSection)
      {
        result = ChooseBorder(
          result, MakeCollapsedBorder(*currSection, currSection->Style().BorderBefore(tableWritingMode),
                                      includeColor, beforeColorProperty, BorderPrecedence::RowGroup));
        if (!result.Exists())
        {
          return result;
        }
      }
    }

    if (!currSection)
    {
      // (8) Our column and column group's after borders.
      if (auto *colElt = table->ColElement(Col()))
      {
        result = ChooseBorder(
          result, MakeCollapsedBorder(*colElt, colElt->Style().BorderAfter(tableWritingMode), includeColor,
                                      afterColorProperty, BorderPrecedence::Column));
        if (!result.Exists())
        {
          return result;
        }
        if (auto *enclosingColumnGroup = colElt->EnclosingColumnGroup())
        {
          result = ChooseBorder(
            result, MakeCollapsedBorder(*enclosingColumnGroup,
                                        enclosingColumnGroup->Style().BorderAfter(tableWritingMode),
                                        includeColor, afterColorProperty, BorderPrecedence::ColumnGroup));
          if (!result.Exists())
          {
            return result;
          }
        }
      }

      // (9) The table's after border.
      result =
        ChooseBorder(result, MakeCollapsedBorder(*table, table->Style().BorderAfter(tableWritingMode),
                                                 includeColor, afterColorProperty, BorderPrecedence::Table));
      if (!result.Exists())
      {
        return result;
      }
    }

    return result;
  }

  RectEdges<LayoutUnit> RenderTableCell::BorderWidths() const noexcept
  {
    auto *table = Table();
    if (!table || !table->CollapseBorders())
    {
      return RenderBlockFlow::BorderWidths();
    }

    return {BorderHalfTop(false), BorderHalfRight(false), BorderHalfBottom(false), BorderHalfLeft(false)};
  }

  LayoutUnit RenderTableCell::BorderLeft() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderLeft();
    }
    return table->CollapseBorders() ? BorderHalfLeft(false) : RenderBlockFlow::BorderLeft();
  }

  LayoutUnit RenderTableCell::BorderRight() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderRight();
    }
    return table->CollapseBorders() ? BorderHalfRight(false) : RenderBlockFlow::BorderRight();
  }

  LayoutUnit RenderTableCell::BorderTop() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderTop();
    }
    return table->CollapseBorders() ? BorderHalfTop(false) : RenderBlockFlow::BorderTop();
  }

  LayoutUnit RenderTableCell::BorderBottom() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderBottom();
    }
    return table->CollapseBorders() ? BorderHalfBottom(false) : RenderBlockFlow::BorderBottom();
  }

  // FIXME: https://bugs.webkit.org/show_bug.cgi?id=46191, make the collapsed border drawing
  // work with different block flow values instead of being hard-coded to top-to-bottom.
  LayoutUnit RenderTableCell::BorderStart() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderStart();
    }
    return table->CollapseBorders() ? BorderHalfStart(false) : RenderBlockFlow::BorderStart();
  }

  LayoutUnit RenderTableCell::BorderEnd() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderEnd();
    }
    return table->CollapseBorders() ? BorderHalfEnd(false) : RenderBlockFlow::BorderEnd();
  }

  LayoutUnit RenderTableCell::BorderBefore() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderBefore();
    }
    return table->CollapseBorders() ? BorderHalfBefore(false) : RenderBlockFlow::BorderBefore();
  }

  LayoutUnit RenderTableCell::BorderAfter() const noexcept
  {
    auto *table = Table();
    if (!table)
    {
      return RenderBlockFlow::BorderAfter();
    }
    return table->CollapseBorders() ? BorderHalfAfter(false) : RenderBlockFlow::BorderAfter();
  }

  LayoutUnit RenderTableCell::BorderHalfLeft(bool outer) const noexcept
  {
    auto writingMode = TableWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? BorderHalfStart(outer) : BorderHalfEnd(outer);
    }
    return writingMode.IsBlockLeftToRight() ? BorderHalfBefore(outer) : BorderHalfAfter(outer);
  }

  LayoutUnit RenderTableCell::BorderHalfRight(bool outer) const noexcept
  {
    auto writingMode = TableWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight() ? BorderHalfEnd(outer) : BorderHalfStart(outer);
    }
    return writingMode.IsBlockLeftToRight() ? BorderHalfAfter(outer) : BorderHalfBefore(outer);
  }

  LayoutUnit RenderTableCell::BorderHalfTop(bool outer) const noexcept
  {
    auto writingMode = TableWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? BorderHalfBefore(outer) : BorderHalfAfter(outer);
    }
    return writingMode.IsInlineTopToBottom() ? BorderHalfStart(outer) : BorderHalfEnd(outer);
  }

  LayoutUnit RenderTableCell::BorderHalfBottom(bool outer) const noexcept
  {
    auto writingMode = TableWritingMode();
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom() ? BorderHalfAfter(outer) : BorderHalfBefore(outer);
    }
    return writingMode.IsInlineTopToBottom() ? BorderHalfEnd(outer) : BorderHalfStart(outer);
  }

  LayoutUnit RenderTableCell::BorderHalfStart(bool outer) const noexcept
  {
    auto border = CollapsedStartBorder(IncludeBorderColor::No);
    if (border.Exists())
    {
      return CollapsedBorderValue::AdjustedCollapsedBorderWidth(
        border.Width().ToFloat(), GetDocument().DeviceScaleFactor(),
        !(TableWritingMode().IsInlineFlipped() ^ outer));
    }
    return 0_lu;
  }

  LayoutUnit RenderTableCell::BorderHalfEnd(bool outer) const noexcept
  {
    auto border = CollapsedEndBorder(IncludeBorderColor::No);
    if (border.Exists())
    {
      return CollapsedBorderValue::AdjustedCollapsedBorderWidth(border.Width().ToFloat(),
                                                                GetDocument().DeviceScaleFactor(),
                                                                TableWritingMode().IsInlineFlipped() ^ outer);
    }
    return 0_lu;
  }

  LayoutUnit RenderTableCell::BorderHalfBefore(bool outer) const noexcept
  {
    auto border = CollapsedBeforeBorder(IncludeBorderColor::No);
    if (border.Exists())
    {
      return CollapsedBorderValue::AdjustedCollapsedBorderWidth(
        border.Width().ToFloat(), GetDocument().DeviceScaleFactor(),
        !(TableWritingMode().IsBlockFlipped() ^ outer));
    }
    return 0_lu;
  }

  LayoutUnit RenderTableCell::BorderHalfAfter(bool outer) const noexcept
  {
    auto border = CollapsedAfterBorder(IncludeBorderColor::No);
    if (border.Exists())
    {
      return CollapsedBorderValue::AdjustedCollapsedBorderWidth(border.Width().ToFloat(),
                                                                GetDocument().DeviceScaleFactor(),
                                                                TableWritingMode().IsBlockFlipped() ^ outer);
    }
    return 0_lu;
  }

  void RenderTableCell::ScrollbarsChanged(bool horizontalScrollbarChanged,
                                          bool verticalScrollbarChanged) noexcept
  {
    auto scrollbarHeight = LayoutUnit(ScrollbarLogicalHeight());
    if (!scrollbarHeight)
    {
      return; // Not sure if we should be doing something when a scrollbar goes away or not.
    }

    // We only care if the scrollbar that affects our intrinsic padding has been added.
    if ((IsHorizontalWritingMode() && !horizontalScrollbarChanged)
        || (!IsHorizontalWritingMode() && !verticalScrollbarChanged))
    {
      return;
    }

    // Shrink our intrinsic padding as much as possible to accommodate the scrollbar.
    if ((Style().GetVerticalAlign().IsMiddle() && Style().GetAlignContent().IsNormal())
        || Style().GetAlignContent().Resolve().IsCentered())
    {
      auto totalHeight = LogicalHeight();
      auto heightWithoutIntrinsicPadding = totalHeight - IntrinsicPaddingBefore() - IntrinsicPaddingAfter();
      totalHeight -= scrollbarHeight;
      auto newBeforePadding = (totalHeight - heightWithoutIntrinsicPadding) / 2;
      auto newAfterPadding = totalHeight - heightWithoutIntrinsicPadding - newBeforePadding;
      SetIntrinsicPaddingBefore(newBeforePadding);
      SetIntrinsicPaddingAfter(newAfterPadding);
    }
    else
    {
      SetIntrinsicPaddingAfter(IntrinsicPaddingAfter() - scrollbarHeight);
    }
  }

  bool RenderTableCell::HasLineIfEmpty() const noexcept
  {
    // FIXME: Editable cells have a line even when empty (hasEditableStyle), once there's editing.
    return RenderBlock::HasLineIfEmpty();
  }

  const BorderValue &
    RenderTableCell::BorderAdjoiningCellAfter(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
    krys_debug_assert(Table()->CellBefore(&cell) == this);
    return Style().BorderEnd(TableWritingMode());
  }

  const BorderValue &
    RenderTableCell::BorderAdjoiningCellBefore(KRYS_MAYBE_UNUSED const RenderTableCell &cell) const noexcept
  {
    krys_debug_assert(Table()->CellAfter(&cell) == this);
    return Style().BorderStart(TableWritingMode());
  }

  const BorderValue &RenderTableCell::BorderAdjoiningTableEnd() const noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(IsFirstOrLastCellInRow());
#endif
    return Style().BorderEnd(TableWritingMode());
  }

  const BorderValue &RenderTableCell::BorderAdjoiningTableStart() const noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(IsFirstOrLastCellInRow());
#endif
    return Style().BorderStart(TableWritingMode());
  }

#if KRYS_DEBUG
  bool RenderTableCell::IsFirstOrLastCellInRow() const noexcept
  {
    return !Table()->CellAfter(this) || !Table()->CellBefore(this);
  }
#endif

  LayoutSize RenderTableCell::OffsetFromContainer(const RenderElement &container, const LayoutPoint &point,
                                                  bool *offsetDependsOnPoint) const noexcept
  {
    krys_debug_assert(&container == Container());

    auto offset = RenderBlockFlow::OffsetFromContainer(container, point, offsetDependsOnPoint);
    if (auto *containerOfRow = container.Container(); containerOfRow && Parent())
    {
      offset -= ParentBox()->OffsetFromContainer(*containerOfRow, point);
    }

    return offset;
  }

  CollapsedBorderValue RenderTableCell::CachedCollapsedLeftBorder(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight()
               ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Start)
               : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::End);
    }
    return writingMode.IsBlockLeftToRight()
             ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Before)
             : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::After);
  }

  CollapsedBorderValue RenderTableCell::CachedCollapsedRightBorder(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsInlineLeftToRight()
               ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::End)
               : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Start);
    }
    return writingMode.IsBlockLeftToRight()
             ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::After)
             : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Before);
  }

  CollapsedBorderValue RenderTableCell::CachedCollapsedTopBorder(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom()
               ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Before)
               : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::After);
    }
    return writingMode.IsInlineTopToBottom()
             ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Start)
             : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::End);
  }

  CollapsedBorderValue RenderTableCell::CachedCollapsedBottomBorder(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return writingMode.IsBlockTopToBottom()
               ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::After)
               : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Before);
    }
    return writingMode.IsInlineTopToBottom()
             ? Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::End)
             : Section()->CachedCollapsedBorder(*this, CollapsedBorderSide::Start);
  }

  void RenderTableCell::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(paintInfo.Phase != PaintPhase::CollapsedTableBorders);
    RenderBlockFlow::Paint(paintInfo, paintOffset);
  }

  namespace
  {
    struct CollapsedBorder
    {
      CollapsedBorderValue BorderValue;
      BoxSide Side {BoxSide::Top};
      bool ShouldPaint {false};
      LayoutUnit X1;
      LayoutUnit Y1;
      LayoutUnit X2;
      LayoutUnit Y2;
      BorderStyle Style {BorderStyle::None};
    };

    class CollapsedBorders
    {
    public:
      void AddBorder(const CollapsedBorderValue &borderValue, BoxSide borderSide, bool shouldPaint,
                     LayoutUnit x1, LayoutUnit y1, LayoutUnit x2, LayoutUnit y2,
                     BorderStyle borderStyle) noexcept
      {
        if (borderValue.Exists() && shouldPaint)
        {
          _borders[_count].BorderValue = borderValue;
          _borders[_count].Side = borderSide;
          _borders[_count].ShouldPaint = shouldPaint;
          _borders[_count].X1 = x1;
          _borders[_count].X2 = x2;
          _borders[_count].Y1 = y1;
          _borders[_count].Y2 = y2;
          _borders[_count].Style = borderStyle;
          _count++;
        }
      }

      KRYS_NODISCARD CollapsedBorder *NextBorder() noexcept
      {
        for (uint32 i = 0; i < _count; i++)
        {
          if (_borders[i].BorderValue.Exists() && _borders[i].ShouldPaint)
          {
            _borders[i].ShouldPaint = false;
            return &_borders[i];
          }
        }

        return nullptr;
      }

    private:
      Array<CollapsedBorder, 4> _borders {};
      uint32 _count {0};
    };

    void AddBorderStyle(List<CollapsedBorderValue> &borderValues, CollapsedBorderValue borderValue) noexcept
    {
      if (!borderValue.Exists())
      {
        return;
      }
      size_t count = borderValues.size();
      for (size_t i = 0; i < count; ++i)
      {
        if (borderValues[i].IsSameIgnoringColor(borderValue))
        {
          return;
        }
      }
      borderValues.push_back(borderValue);
    }
  }

  void RenderTableCell::CollectBorderValues(List<CollapsedBorderValue> &borderValues) const noexcept
  {
    AddBorderStyle(borderValues, CollapsedStartBorder());
    AddBorderStyle(borderValues, CollapsedEndBorder());
    AddBorderStyle(borderValues, CollapsedBeforeBorder());
    AddBorderStyle(borderValues, CollapsedAfterBorder());
  }

  void RenderTableCell::SortBorderValues(List<CollapsedBorderValue> &borderValues) noexcept
  {
    std::ranges::sort(borderValues, CompareBorders);
  }

  void RenderTableCell::PaintCollapsedBorders(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(paintInfo.Phase == PaintPhase::CollapsedTableBorders);

    if (!paintInfo.ShouldPaintWithinRoot(*this) || Style().UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    LayoutRect localRepaintRect = paintInfo.Rect;
    LayoutRect paintRect = LayoutRect(paintOffset + Location(), FrameRect().Size());
    if (paintRect.Y() - Table()->OuterBorderTop() >= localRepaintRect.MaxY())
    {
      return;
    }

    if (paintRect.MaxY() + Table()->OuterBorderBottom() <= localRepaintRect.Y())
    {
      return;
    }

    GraphicsContext &graphicsContext = paintInfo.Context();
    if (!Table()->CurrentBorderValue() || graphicsContext.PaintingDisabled())
    {
      return;
    }

    WritingMode writingMode = TableWritingMode();
    CollapsedBorderValue leftVal = CachedCollapsedLeftBorder(writingMode);
    CollapsedBorderValue rightVal = CachedCollapsedRightBorder(writingMode);
    CollapsedBorderValue topVal = CachedCollapsedTopBorder(writingMode);
    CollapsedBorderValue bottomVal = CachedCollapsedBottomBorder(writingMode);

    // Adjust our x/y/width/height so that we paint the collapsed borders at the correct location.
    LayoutUnit topWidth = topVal.Width();
    LayoutUnit bottomWidth = bottomVal.Width();
    LayoutUnit leftWidth = leftVal.Width();
    LayoutUnit rightWidth = rightVal.Width();

    float deviceScaleFactor = GetDocument().DeviceScaleFactor();
    LayoutUnit leftHalfCollapsedBorder =
      CollapsedBorderValue::AdjustedCollapsedBorderWidth(leftWidth, deviceScaleFactor, false);
    LayoutUnit topHalfCollapsedBorder =
      CollapsedBorderValue::AdjustedCollapsedBorderWidth(topWidth, deviceScaleFactor, false);
    LayoutUnit rightHalfCollapsedBorder =
      CollapsedBorderValue::AdjustedCollapsedBorderWidth(rightWidth, deviceScaleFactor, true);
    LayoutUnit bottomHalfCollapsedBorder =
      CollapsedBorderValue::AdjustedCollapsedBorderWidth(bottomWidth, deviceScaleFactor, true);

    LayoutRect borderRect =
      LayoutRect(paintRect.X() - leftHalfCollapsedBorder, paintRect.Y() - topHalfCollapsedBorder,
                 paintRect.Width() + leftHalfCollapsedBorder + rightHalfCollapsedBorder,
                 paintRect.Height() + topHalfCollapsedBorder + bottomHalfCollapsedBorder);

    BorderStyle topStyle = CollapsedBorderStyle(topVal.Style());
    BorderStyle bottomStyle = CollapsedBorderStyle(bottomVal.Style());
    BorderStyle leftStyle = CollapsedBorderStyle(leftVal.Style());
    BorderStyle rightStyle = CollapsedBorderStyle(rightVal.Style());

    bool renderTop = topStyle > BorderStyle::Hidden && !topVal.IsTransparent()
                     && FloorToDevicePixel(topWidth, deviceScaleFactor);
    bool renderBottom = bottomStyle > BorderStyle::Hidden && !bottomVal.IsTransparent()
                        && FloorToDevicePixel(bottomWidth, deviceScaleFactor);
    bool renderLeft = leftStyle > BorderStyle::Hidden && !leftVal.IsTransparent()
                      && FloorToDevicePixel(leftWidth, deviceScaleFactor);
    bool renderRight = rightStyle > BorderStyle::Hidden && !rightVal.IsTransparent()
                       && FloorToDevicePixel(rightWidth, deviceScaleFactor);

    // We never paint diagonals at the joins.  We simply let the border with the highest
    // precedence paint on top of borders with lower precedence.
    CollapsedBorders borders;
    borders.AddBorder(topVal, BoxSide::Top, renderTop, borderRect.X(), borderRect.Y(), borderRect.MaxX(),
                      borderRect.Y() + topWidth, topStyle);
    borders.AddBorder(bottomVal, BoxSide::Bottom, renderBottom, borderRect.X(),
                      borderRect.MaxY() - bottomWidth, borderRect.MaxX(), borderRect.MaxY(), bottomStyle);
    borders.AddBorder(leftVal, BoxSide::Left, renderLeft, borderRect.X(), borderRect.Y(),
                      borderRect.X() + leftWidth, borderRect.MaxY(), leftStyle);
    borders.AddBorder(rightVal, BoxSide::Right, renderRight, borderRect.MaxX() - rightWidth, borderRect.Y(),
                      borderRect.MaxX(), borderRect.MaxY(), rightStyle);

    bool antialias = BorderPainter::ShouldAntialiasLines(graphicsContext);

    for (CollapsedBorder *border = borders.NextBorder(); border; border = borders.NextBorder())
    {
      if (border->BorderValue.IsSameIgnoringColor(*Table()->CurrentBorderValue()))
      {
        BorderPainter::DrawLineForBoxSide(
          graphicsContext, GetDocument(),
          FloatRect(LayoutRect(LayoutPoint(border->X1, border->Y1), LayoutPoint(border->X2, border->Y2))),
          border->Side, border->BorderValue.GetColor(), border->Style, 0, 0, antialias);
      }
    }
  }

  KRYS_NODISCARD static LayoutRect BackgroundRectForRow(const RenderBox &tableRow,
                                                        const RenderTable &table) noexcept
  {
    LayoutRect rect = tableRow.FrameRect();
    if (!table.CollapseBorders())
    {
      // Row frameRects include unwanted hSpacing on both inline ends.
      auto hSpacing = table.HBorderSpacing();
      LayoutUnit vSpacing = 0_lu;
      if (table.GetWritingMode().IsHorizontal())
      {
        rect.Contract({vSpacing, hSpacing, vSpacing, hSpacing});
      }
      else
      {
        rect.Contract({hSpacing, vSpacing, hSpacing, vSpacing});
      }
    }
    return rect;
  }

  KRYS_NODISCARD static LayoutRect BackgroundRectForSection(const RenderTableSection &tableSection,
                                                            const RenderTable &table) noexcept
  {
    LayoutRect rect = {{}, tableSection.Size()};
    if (!table.CollapseBorders())
    {
      auto hSpacing = table.HBorderSpacing();
      auto vSpacing = table.VBorderSpacing();
      // All sections' size()s include unwanted vSpacing at the block-end
      // position. The first section's size() includes additional unwanted
      // vSpacing at the block-start position. All sections' size()s include
      // unwanted hSpacing on both inline ends.
      auto beforeBlockSpacing = &tableSection == table.TopSection() ? vSpacing : 0_lu;
      if (table.GetWritingMode().IsHorizontal())
      {
        rect.Contract({beforeBlockSpacing, hSpacing, vSpacing, hSpacing});
      }
      else if (table.GetWritingMode().IsBlockFlipped())
      {
        rect.Contract({hSpacing, beforeBlockSpacing, hSpacing, vSpacing});
      }
      else
      {
        rect.Contract({hSpacing, vSpacing, hSpacing, beforeBlockSpacing});
      }
    }
    return rect;
  }

  void RenderTableCell::PaintBackgroundsBehindCell(PaintInfo &paintInfo, LayoutPoint paintOffset,
                                                   RenderBox *backgroundObject,
                                                   LayoutPoint backgroundPaintOffset) noexcept
  {
    krys_debug_assert(backgroundObject);

    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return;
    }

    if (Style().UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    RenderTable *tableElt = Table();
    if (!tableElt->CollapseBorders() && Style().EmptyCells() == EmptyCell::Hide && !FirstChild())
    {
      return;
    }

    const auto &backgroundStyle = backgroundObject->Style();
    auto &bgLayers = backgroundStyle.GetBackgroundLayers();

    auto color = backgroundStyle.VisitedDependentColor(css::PropertyId::BackgroundColor);
    if (!style::HasImageInAnyLayer(bgLayers) && !color.IsVisible())
    {
      return;
    }

    color = backgroundStyle.ColorByApplyingColorFilter(color);

    LayoutPoint adjustedPaintOffset = paintOffset;
    if (backgroundObject != this)
    {
      adjustedPaintOffset.MoveBy(Location());
    }

    // Background images attached to the row or row group must span the row
    // or row group. Draw them at the backgroundObject's dimensions, but
    // clipped to this cell.
    // FIXME: This should also apply to columns and column groups.
    bool paintBackgroundObject = backgroundObject != this && style::HasImageInAnyLayer(bgLayers)
                                 && !Is<RenderTableCol>(backgroundObject);
    // We have to clip here because the background would paint
    // on top of the borders otherwise. This only matters for cells and rows.
    bool shouldClip =
      paintBackgroundObject
      || (backgroundObject->HasLayer() && (backgroundObject == this || backgroundObject == Parent())
          && tableElt->CollapseBorders());
    GraphicsContextStateSaver stateSaver(paintInfo.Context(), shouldClip);
    if (paintBackgroundObject)
    {
      paintInfo.Context().Clip(FloatRect(LayoutRect {adjustedPaintOffset, Size()}));
    }
    else if (shouldClip)
    {
      LayoutRect clipRect(adjustedPaintOffset.X() + BorderLeft(), adjustedPaintOffset.Y() + BorderTop(),
                          Width() - BorderLeft() - BorderRight(), Height() - BorderTop() - BorderBottom());
      paintInfo.Context().Clip(FloatRect(clipRect));
    }
    LayoutRect fillRect;
    if (paintBackgroundObject)
    {
      if (auto *tableSectionRenderer = DynamicDowncast<RenderTableSection>(backgroundObject))
      {
        fillRect = BackgroundRectForSection(*tableSectionRenderer, *tableElt);
      }
      else
      {
        fillRect = BackgroundRectForRow(*backgroundObject, *tableElt);
      }
      fillRect.MoveBy(backgroundPaintOffset);
    }
    else
    {
      fillRect = LayoutRect {adjustedPaintOffset, Size()};
    }
    auto compositeOp = GetDocument().CompositeOperatorForBackgroundColor(color, *this);
    BackgroundPainter painter {*this, paintInfo};
    if (backgroundObject != this)
    {
      painter.SetOverrideClip(FillBox::BorderBox);
      painter.SetOverrideOrigin(FillBox::BorderBox);
    }
    painter.PaintFillLayers(color, bgLayers, fillRect, BleedAvoidance::None, compositeOp, backgroundObject);
  }

  void RenderTableCell::PaintBoxDecorations(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return;
    }

    RenderTable *table = Table();
    if (!table->CollapseBorders() && Style().EmptyCells() == EmptyCell::Hide && !FirstChild())
    {
      return;
    }

    LayoutRect paintRect = LayoutRect(paintOffset, FrameRect().Size());
    AdjustBorderBoxRectForPainting(paintRect);

    BackgroundPainter backgroundPainter {*this, paintInfo};
    backgroundPainter.PaintBoxShadow(paintRect, Style(), style::ShadowStyle::Normal);

    // Paint our cell background.
    PaintBackgroundsBehindCell(paintInfo, paintOffset, this, paintOffset);

    backgroundPainter.PaintBoxShadow(paintRect, Style(), style::ShadowStyle::Inset);

    if (!Style().HasBorder() || table->CollapseBorders())
    {
      return;
    }

    BorderPainter borderPainter {*this, paintInfo};
    borderPainter.PaintBorder(paintRect, Style());
  }
}
