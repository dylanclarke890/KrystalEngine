/*
 * Copyright (C) 2006-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *               2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollAnimator.hpp"
#include "Krystal.Booey/Platform/Scrolling/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Text/UnicodeBidi.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo
{
  namespace
  {
    constexpr int32 ItemBlockSpacing = 1;

    constexpr int32 OptionsSpacingInlineStart = 2;

    // Default size when the multiple attribute is present but size attribute is absent.
    constexpr int32 DefaultSize = 4;

    KRYS_NODISCARD FontCascade Bolder(dom::Document &document, const FontCascade &font) noexcept
    {
      auto description = font.GetFontDescription();
      description.SetWeight(description.BolderWeight());
      FontCascade result(krys::move(description), font);
      result.Update(ShareRefPtr<FontSelector>(&document.GetFontSelector()));
      return result;
    }

    KRYS_NODISCARD LayoutSize ItemOffsetForAlignment(const TextRun &textRun, const RenderStyle &elementStyle,
                                                     const RenderStyle *itemStyle,
                                                     const FontCascade &itemFont,
                                                     LayoutRect itemBoundingBox) noexcept
    {
      auto actualAlignment = itemStyle->GetTextAlign();
      // FIXME: Firefox doesn't respect TextAlign::Justify. Should we?
      // FIXME: Handle TextAlign::End here
      if (actualAlignment == style::TextAlign::Start || actualAlignment == style::TextAlign::Justify)
      {
        actualAlignment = itemStyle->GetWritingMode().IsLogicalLeftInlineStart() ? style::TextAlign::Left
                                                                                 : style::TextAlign::Right;
      }

      bool isHorizontalWritingMode = elementStyle.GetWritingMode().IsHorizontal();

      auto itemBoundingBoxLogicalWidth =
        isHorizontalWritingMode ? itemBoundingBox.Width() : itemBoundingBox.Height();
      auto itemBoundingBoxLogicalHeight =
        isHorizontalWritingMode ? itemBoundingBox.Height() : itemBoundingBox.Width();
      auto offset = LayoutSize(0_lu, LayoutUnit(itemFont.MetricsOfPrimaryFont().IntAscent()));
      if (actualAlignment == style::TextAlign::Right || actualAlignment == style::TextAlign::WebKitRight)
      {
        float textWidth = itemFont.Width(textRun);
        offset.SetWidth(itemBoundingBoxLogicalWidth - LayoutUnit(textWidth)
                        - LayoutUnit(OptionsSpacingInlineStart));
      }
      else if (actualAlignment == style::TextAlign::Center
               || actualAlignment == style::TextAlign::WebKitCenter)
      {
        float textWidth = itemFont.Width(textRun);
        offset.SetWidth((itemBoundingBoxLogicalWidth - LayoutUnit(textWidth)) / 2);
      }
      else
      {
        offset.SetWidth(LayoutUnit(OptionsSpacingInlineStart));
      }

      if (elementStyle.GetWritingMode().IsLineOverLeft())
      {
        offset.SetWidth(offset.Width() + LayoutUnit(itemFont.Width(textRun)));
        offset.SetHeight(itemBoundingBoxLogicalHeight - offset.Height());
      }

      if (!isHorizontalWritingMode)
      {
        return LayoutSize {-offset.Height(), offset.Width()};
      }

      return offset;
    }
  }

  RenderListBox::RenderListBox(html::HTMLSelectElement &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::ListBox, element, krys::move(style))
  {
    View().FrameView().AddScrollableArea(*this);
  }

  // Do not add any code in below destructor. Add it to WillBeDestroyed() instead.
  RenderListBox::~RenderListBox() noexcept = default;

  void RenderListBox::WillBeDestroyed() noexcept
  {
    (void)View().FrameView().RemoveScrollableArea(*this);
    DestroyScrollbar();
    RenderBlockFlow::WillBeDestroyed();
  }

  html::HTMLSelectElement &RenderListBox::SelectElement() const noexcept
  {
    return Downcast<html::HTMLSelectElement>(NodeForNonAnonymous());
  }

  void RenderListBox::UpdateFromElement() noexcept
  {
    if (!_optionsChanged)
    {
      return;
    }

    float logicalWidth = 0;
    auto &normalFont = Style().GetFontCascade();
    Maybe<FontCascade> boldFont;
    for (auto &element : SelectElement().ListItems())
    {
      utf8_string text;
      const FontCascade *font = &normalFont;
      if (auto *optionElement = DynamicDowncast<html::HTMLOptionElement>(element.get()))
      {
        text = optionElement->TextIndentedToRespectGroupLabel();
      }
      else if (auto *optGroupElement = DynamicDowncast<html::HTMLOptGroupElement>(element.get()))
      {
        text = optGroupElement->GroupLabelText();
        if (!boldFont)
        {
          boldFont = Bolder(GetDocument(), normalFont);
        }
        font = &*boldFont;
      }
      if (text.empty())
      {
        continue;
      }
      text = ApplyTextTransform(Style(), krys::move(text));
      auto textRun = ConstructTextRun(text, Style(), ExpansionBehavior::AllowRightOnly());
      logicalWidth = std::max(logicalWidth, font->Width(textRun));
    }
    // FIXME: Is ceiling right here, or should we be doing some kind of rounding instead?
    _optionsLogicalWidth = static_cast<int32>(std::ceil(logicalWidth));
    _optionsChanged = false;

    SetHasScrollbar(ScrollbarOrientationForWritingMode());

    SetNeedsLayoutAndPreferredWidthsUpdate();
  }

  void RenderListBox::SelectionChanged() noexcept
  {
    Repaint();
    if (!_inAutoscroll)
    {
      if (_optionsChanged || NeedsLayout())
      {
        _scrollToRevealSelectionAfterLayout = true;
      }
      else
      {
        ScrollToRevealSelection();
      }
    }

    if (auto *cache = GetDocument().ExistingAXObjectCache())
    {
      cache->DeferSelectedChildrenChangedIfNeeded(SelectElement());
    }
  }

  void RenderListBox::Layout() noexcept
  {
    RenderBlockFlow::Layout();

    if (_scrollbar)
    {
      int32 maximumOffset = MaximumLogicalScrollOffset();
      int32 listHeight = ListLogicalHeight().ToInt();
      _scrollbar->SetEnabled(maximumOffset > 0);
      // A line is an item, and a page the items in view less one, so the keys go item by item.
      int32 itemHeight = ItemLogicalHeight().ToInt();
      _scrollbar->SetSteps(itemHeight, std::max(1, NumVisibleItems() - 1) * itemHeight);
      _scrollbar->SetProportion(listHeight - maximumOffset, listHeight);

      // A flipped list starts at the far end, so its positions run from minus the maximum offset to 0.
      IntPoint scrollOrigin;
      if (GetWritingMode().IsBlockFlipped())
      {
        scrollOrigin = IntPoint(0, maximumOffset);
        if (_scrollbar->Orientation() == ScrollbarOrientation::Horizontal)
        {
          scrollOrigin = scrollOrigin.TransposedPoint();
        }
      }
      SetScrollOrigin(scrollOrigin);
      _scrollbar->OffsetDidChange();

      // Fewer or shorter items leave less to scroll.
      if (LogicalScrollOffset() > maximumOffset)
      {
        ScrollToLogicalOffset(maximumOffset);
      }
    }

    if (_scrollToRevealSelectionAfterLayout)
    {
      LayoutStateDisabler layoutStateDisabler(View().FrameView().LayoutContext());
      ScrollToRevealSelection();
    }
  }

  void RenderListBox::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlockFlow::StyleDidChange(diff, oldStyle);

    if (oldStyle && oldStyle->GetWritingMode() != Style().GetWritingMode())
    {
      if (_scrollbar)
      {
        SetHasScrollbar(ScrollbarOrientationForWritingMode());
      }
    }
  }

  void RenderListBox::ScrollToRevealSelection() noexcept
  {
    _scrollToRevealSelectionAfterLayout = false;

    int32 firstIndex = SelectElement().ActiveSelectionStartListIndex();
    if (firstIndex >= 0 && !ListIndexIsVisible(SelectElement().ActiveSelectionEndListIndex()))
    {
      (void)ScrollToRevealElementAtListIndex(firstIndex);
    }
  }

  void RenderListBox::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                    LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto logicalWidth = ExplicitIntrinsicInnerLogicalWidth())
      {
        maxLogicalWidth = *logicalWidth;
      }
      else
      {
        maxLogicalWidth = LayoutUnit(2 * OptionsSpacingInlineStart);
      }
    }
    else
    {
      maxLogicalWidth = LayoutUnit(2 * OptionsSpacingInlineStart + _optionsLogicalWidth);
    }

    if (_scrollbar)
    {
      maxLogicalWidth +=
        LayoutUnit(_scrollbar->Orientation() == ScrollbarOrientation::Vertical ? _scrollbar->Width()
                                                                               : _scrollbar->Height());
    }

    auto &logicalWidth = Style().LogicalWidth();
    if (logicalWidth.IsCalculated())
    {
      minLogicalWidth =
        std::max(0_lu, style::Evaluate<LayoutUnit>(logicalWidth, 0_lu, Style().UsedZoomForLength()));
    }
    else if (!logicalWidth.IsPercent())
    {
      minLogicalWidth = maxLogicalWidth;
    }
  }

  void RenderListBox::ComputePreferredLogicalWidths() noexcept
  {
    _minPreferredLogicalWidth = 0_lu;
    _maxPreferredLogicalWidth = 0_lu;

    if (auto fixedLogicalWidth = Style().LogicalWidth().TryFixed();
        fixedLogicalWidth && fixedLogicalWidth->IsPositive())
    {
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength())});
    }
    else
    {
      ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
    }

    RenderBox::ComputePreferredLogicalWidths(Style().LogicalMinWidth(), Style().LogicalMaxWidth(),
                                             GetWritingMode().IsHorizontal()
                                               ? HorizontalBorderAndPaddingExtent()
                                               : VerticalBorderAndPaddingExtent());

    ClearNeedsPreferredWidthsUpdate();
  }

  uint32 RenderListBox::Size() const noexcept
  {
    if (Style().GetFieldSizing() == FieldSizing::Content)
    {
      return static_cast<uint32>(NumItems());
    }

    uint32 specifiedSize = SelectElement().Size();
    if (specifiedSize >= 1)
    {
      return specifiedSize;
    }

    return DefaultSize;
  }

  int32 RenderListBox::NumVisibleItems() const noexcept
  {
    // Only count fully visible rows. But don't return 0 even if only part of a row shows.
    return std::max<int32>(
      1, ((ContentBoxLogicalHeight() + LayoutUnit(ItemBlockSpacing)) / ItemLogicalHeight()).ToInt());
  }

  int32 RenderListBox::NumItems() const noexcept
  {
    return static_cast<int32>(SelectElement().ListItems().size());
  }

  LayoutUnit RenderListBox::ListLogicalHeight() const noexcept
  {
    return ItemLogicalHeight() * NumItems() - LayoutUnit(ItemBlockSpacing);
  }

  RenderBox::LogicalExtentComputedValues
    RenderListBox::ComputeLogicalHeight(LayoutUnit, LayoutUnit logicalTop) const noexcept
  {
    LayoutUnit logicalHeight =
      ItemLogicalHeight() * static_cast<int32>(Size()) - LayoutUnit(ItemBlockSpacing);

    if (ShouldApplySizeContainment())
    {
      if (auto explicitIntrinsicHeight = ExplicitIntrinsicInnerLogicalHeight())
      {
        logicalHeight = *explicitIntrinsicHeight;
      }
    }

    CacheIntrinsicContentLogicalHeightForFlexItem(logicalHeight);
    logicalHeight +=
      GetWritingMode().IsHorizontal() ? VerticalBorderAndPaddingExtent() : HorizontalBorderAndPaddingExtent();
    return RenderBox::ComputeLogicalHeight(logicalHeight, logicalTop);
  }

  LayoutRect RenderListBox::ItemBoundingBoxRect(const LayoutPoint &additionalOffset,
                                                int32 index) const noexcept
  {
    LayoutUnit x = additionalOffset.X() + BorderLeft() + PaddingLeft();
    LayoutUnit y = additionalOffset.Y() + BorderTop() + PaddingTop();

    if (auto *vBar = VerticalScrollbar(); vBar && ShouldPlaceVerticalScrollbarOnLeft())
    {
      x += LayoutUnit(vBar->OccupiedWidth());
    }

    auto itemOffset = ItemLogicalHeight() * index - LayoutUnit(LogicalScrollOffset());
    if (GetWritingMode().IsBlockFlipped())
    {
      itemOffset = ContentBoxLogicalHeight() - ItemLogicalHeight() - itemOffset;
    }

    if (GetWritingMode().IsVertical())
    {
      return LayoutRect(x + itemOffset, y, ItemLogicalHeight(), ContentBoxHeight());
    }

    return LayoutRect(x, y + itemOffset, ContentBoxWidth(), ItemLogicalHeight());
  }

  Maybe<int32> RenderListBox::OptionRowIndex(const html::HTMLOptionElement &optionElement) const noexcept
  {
    // We can't use optionElement.Index(), because it doesn't account for optgroup items.
    int32 rowIndex = 0;
    for (auto &item : SelectElement().ListItems())
    {
      if (item.get() == &optionElement)
      {
        return rowIndex;
      }

      ++rowIndex;
    }

    return {};
  }

  Maybe<LayoutRect>
    RenderListBox::LocalBoundsOfOption(const html::HTMLOptionElement &optionElement) const noexcept
  {
    auto rowIndex = OptionRowIndex(optionElement);
    if (!rowIndex)
    {
      return {};
    }

    return ItemBoundingBoxRect({}, *rowIndex);
  }

  Maybe<LayoutRect>
    RenderListBox::LocalBoundsOfOptGroup(const html::HTMLOptGroupElement &optGroupElement) const noexcept
  {
    if (optGroupElement.OwnerSelectElement() != &SelectElement())
    {
      return {};
    }

    Maybe<LayoutRect> boundingBox;
    int32 rowIndex = 0;

    for (auto &item : SelectElement().ListItems())
    {
      if (Is<html::HTMLOptGroupElement>(*item))
      {
        if (item.get() == &optGroupElement)
        {
          boundingBox = ItemBoundingBoxRect({}, rowIndex);
        }
      }
      else if (Is<html::HTMLOptionElement>(*item) && boundingBox)
      {
        if (item->ParentNode() != &optGroupElement)
        {
          break;
        }

        boundingBox->SetHeight(boundingBox->Height() + ItemBoundingBoxRect({}, rowIndex).Height());
      }
      ++rowIndex;
    }

    return boundingBox;
  }

  void RenderListBox::PaintItem(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                const PaintFunction &paintFunction) noexcept
  {
    // The items any part of which shows in the content box or the padding around it, which clips them.
    LayoutUnit offset(LogicalScrollOffset());
    int32 firstIndex = std::max(0, ItemIndexAtLogicalOffset(offset - PaddingBefore()));
    int32 endIndex =
      std::min(NumItems(), ItemIndexAtLogicalOffset(offset + ContentBoxLogicalHeight() + PaddingAfter()) + 1);
    for (int32 i = firstIndex; i < endIndex; ++i)
    {
      paintFunction(paintInfo, paintOffset, i);
    }
  }

  void RenderListBox::PaintObject(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (Style().UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    if (paintInfo.Phase == PaintPhase::Foreground)
    {
      PaintItem(paintInfo, paintOffset,
                [this](PaintInfo &info, const LayoutPoint &offset, int32 listItemIndex)
                { PaintItemForeground(info, offset, listItemIndex); });
    }

    // Paint the children.
    RenderBlockFlow::PaintObject(paintInfo, paintOffset);

    switch (paintInfo.Phase)
    {
      // Depending on whether we have overlay scrollbars they get rendered in the foreground or background
      // phases.
      case PaintPhase::Foreground:
        if (_scrollbar && _scrollbar->IsOverlayScrollbar())
        {
          PaintScrollbar(paintInfo, paintOffset, *_scrollbar);
        }
        break;
      case PaintPhase::BlockBackground:
        if (_scrollbar && !_scrollbar->IsOverlayScrollbar())
        {
          PaintScrollbar(paintInfo, paintOffset, *_scrollbar);
        }
        break;
      case PaintPhase::ChildBlockBackground:
      case PaintPhase::ChildBlockBackgrounds:
        PaintItem(paintInfo, paintOffset,
                  [this](PaintInfo &info, const LayoutPoint &offset, int32 listItemIndex)
                  { PaintItemBackground(info, offset, listItemIndex); });
        break;
      default: break;
    }
  }

  bool RenderListBox::UseDarkAppearanceForScrollbars() const noexcept
  {
    return UseDarkAppearance();
  }

  void RenderListBox::PaintScrollbar(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                     Scrollbar &scrollbar) noexcept
  {
    auto scrollRect = RectForScrollbar(scrollbar);
    scrollRect.MoveBy(paintOffset);
    scrollbar.SetFrameRect(SnappedIntRect(scrollRect));
    scrollbar.Paint(paintInfo.Context(), SnappedIntRect(paintInfo.Rect));
  }

  void RenderListBox::PaintItemForeground(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                          int32 listIndex) noexcept
  {
    auto &listItems = SelectElement().ListItems();
    RefPtr<html::HTMLElement> listItemElement = ShareRefPtr(listItems[static_cast<size_t>(listIndex)].get());

    auto *itemStyle = listItemElement->ComputedStyle();
    if (!itemStyle)
    {
      return;
    }

    if (itemStyle->UsedVisibility() == Visibility::Hidden)
    {
      return;
    }

    utf8_string itemText;
    auto *optionElement = DynamicDowncast<html::HTMLOptionElement>(*listItemElement);
    auto *optGroupElement = DynamicDowncast<html::HTMLOptGroupElement>(*listItemElement);
    if (optionElement)
    {
      itemText = optionElement->TextIndentedToRespectGroupLabel();
    }
    else if (optGroupElement)
    {
      itemText = optGroupElement->GroupLabelText();
    }
    else
    {
      return;
    }
    itemText = ApplyTextTransform(Style(), krys::move(itemText));

    auto &theme = RenderTheme::Singleton();
    Color textColor = itemStyle->ColorResolvingCurrentColor(css::PropertyId::Color);
    if (optionElement && optionElement->Selected())
    {
      if (GetDocument().Selection().IsFocusedAndActive()
          && GetDocument().FocusedElement() == &SelectElement())
      {
        textColor = theme.ActiveListBoxSelectionForegroundColor(StyleColorOptions());
      }
      // Honor the foreground color for disabled items
      else if (!listItemElement->IsDisabledFormControl() && !SelectElement().IsDisabledFormControl())
      {
        textColor = theme.InactiveListBoxSelectionForegroundColor(StyleColorOptions());
      }
    }

    auto &context = paintInfo.Context();
    GraphicsContextStateSaver stateSaver(context);

    context.SetFillColor(textColor);

    TextRun textRun(itemText, 0, 0, ExpansionBehavior::AllowRightOnly(),
                    itemStyle->GetWritingMode().BidiDirection(), IsOverride(itemStyle->GetUnicodeBidi()),
                    true);
    FontCascade itemFont = Style().GetFontCascade();
    if (optGroupElement)
    {
      itemFont = Bolder(GetDocument(), itemFont);
    }
    LayoutRect r = ItemBoundingBoxRect(paintOffset, listIndex);
    r.Move(ItemOffsetForAlignment(textRun, Style(), itemStyle, itemFont, r));

    bool isHorizontalWritingMode = GetWritingMode().IsHorizontal();
    if (!isHorizontalWritingMode)
    {
      auto rotationOrigin = RoundedIntPoint(r.MaxXMinYCorner());
      context.Translate(FloatPoint(rotationOrigin));
      if (GetWritingMode().IsLineOverLeft())
      {
        context.Rotate(-std::numbers::pi_v<float> / 2);
      }
      else
      {
        context.Rotate(std::numbers::pi_v<float> / 2);
      }
      context.Translate(-FloatPoint(rotationOrigin));
    }

    // Draw the item text
    context.DrawBidiText(
      itemFont, textRun,
      FloatPoint(RoundedIntPoint(isHorizontalWritingMode ? r.Location() : r.MaxXMinYCorner())));
  }

  void RenderListBox::PaintItemBackground(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                          int32 listIndex) noexcept
  {
    auto &listItems = SelectElement().ListItems();
    RefPtr<html::HTMLElement> listItemElement = ShareRefPtr(listItems[static_cast<size_t>(listIndex)].get());
    auto *itemStyle = listItemElement->ComputedStyle();
    if (!itemStyle)
    {
      return;
    }

    auto &theme = RenderTheme::Singleton();
    Color backColor;
    if (auto *option = DynamicDowncast<html::HTMLOptionElement>(*listItemElement);
        option && option->Selected())
    {
      if (GetDocument().Selection().IsFocusedAndActive()
          && GetDocument().FocusedElement() == &SelectElement())
      {
        backColor = theme.ActiveListBoxSelectionBackgroundColor(StyleColorOptions());
      }
      else
      {
        backColor = theme.InactiveListBoxSelectionBackgroundColor(StyleColorOptions());
      }
    }
    else
    {
      backColor = itemStyle->ColorResolvingCurrentColor(css::PropertyId::BackgroundColor);
    }

    // Draw the background for this list box item
    if (itemStyle->UsedVisibility() == Visibility::Hidden)
    {
      return;
    }

    LayoutRect itemRect = ItemBoundingBoxRect(paintOffset, listIndex);
    itemRect.Intersect(ControlClipRect(paintOffset));
    paintInfo.Context().FillRect(FloatRect(SnappedIntRect(itemRect)), backColor);
  }

  bool RenderListBox::IsPointInOverflowControl(HitTestResult &result, const LayoutPoint &locationInContainer,
                                               const LayoutPoint &accumulatedOffset) noexcept
  {
    auto *activeScrollbar = VerticalScrollbar();
    if (!activeScrollbar)
    {
      activeScrollbar = HorizontalScrollbar();
    }

    if (!activeScrollbar || !activeScrollbar->ShouldParticipateInHitTesting())
    {
      return false;
    }

    auto scrollbarRect = RectForScrollbar(*activeScrollbar);
    scrollbarRect.MoveBy(accumulatedOffset);

    if (!scrollbarRect.Contains(locationInContainer))
    {
      return false;
    }

    result.SetScrollbar(ShareRefPtr(activeScrollbar));
    return true;
  }

  int32 RenderListBox::ListIndexAtOffset(const LayoutSize &offset) const noexcept
  {
    if (!NumItems())
    {
      return -1;
    }

    int32 scrollbarHeight = 0;
    if (auto *hBar = HorizontalScrollbar())
    {
      scrollbarHeight = hBar->Height();
    }

    if (offset.Height() < BorderTop()
        || offset.Height() > Height() - BorderBottom() - LayoutUnit(scrollbarHeight))
    {
      return -1;
    }

    int32 scrollbarWidth = 0;
    if (auto *vBar = VerticalScrollbar())
    {
      scrollbarWidth = vBar->Width();
    }

    if (ShouldPlaceVerticalScrollbarOnLeft()
        && (offset.Width() < BorderLeft() + PaddingLeft() + LayoutUnit(scrollbarWidth)
            || offset.Width() > Width() - BorderRight() - PaddingRight()))
    {
      return -1;
    }
    if (!ShouldPlaceVerticalScrollbarOnLeft()
        && (offset.Width() < BorderLeft() + PaddingLeft()
            || offset.Width() > Width() - BorderRight() - PaddingRight() - LayoutUnit(scrollbarWidth)))
    {
      return -1;
    }

    auto offsetLogicalHeight = GetWritingMode().IsHorizontal() ? offset.Height() : offset.Width();

    // How far into the content box, from its start, then into the list.
    auto offsetInContent = GetWritingMode().IsBlockFlipped()
                             ? LogicalHeight() - BorderAndPaddingBefore() - offsetLogicalHeight
                             : offsetLogicalHeight - BorderAndPaddingBefore();
    int32 index = ItemIndexAtLogicalOffset(offsetInContent + LayoutUnit(LogicalScrollOffset()));
    return index >= 0 && index < NumItems() ? index : -1;
  }

  int32 RenderListBox::ScrollToward(const IntPoint &destination) noexcept
  {
    // FIXME: This doesn't work correctly with transforms.
    FloatPoint absPos = LocalToAbsolute();
    IntSize positionOffset = RoundedIntSize(FloatPoint(destination) - absPos);
    int32 positionOffsetLogicalHeight =
      GetWritingMode().IsHorizontal() ? positionOffset.Height() : positionOffset.Width();

    // The items fully in view: past them, the next one either way is revealed.
    int32 firstIndex = FirstFullyVisibleIndex(LogicalScrollOffset());
    int32 endIndex = EndOfFullyVisibleIndexes(LogicalScrollOffset());

    if (GetWritingMode().IsBlockFlipped())
    {
      if (positionOffsetLogicalHeight < BorderAndPaddingAfter() && ScrollToRevealElementAtListIndex(endIndex))
      {
        return endIndex - 1;
      }

      if (positionOffsetLogicalHeight > LogicalHeight() - BorderAndPaddingBefore()
          && ScrollToRevealElementAtListIndex(firstIndex - 1))
      {
        return firstIndex - 1;
      }
    }
    else
    {
      if (positionOffsetLogicalHeight < BorderAndPaddingBefore()
          && ScrollToRevealElementAtListIndex(firstIndex - 1))
      {
        return firstIndex - 1;
      }

      if (positionOffsetLogicalHeight > LogicalHeight() - BorderAndPaddingAfter()
          && ScrollToRevealElementAtListIndex(endIndex))
      {
        return endIndex - 1;
      }
    }

    return ListIndexAtOffset(LayoutSize(positionOffset));
  }

  void RenderListBox::PanScroll(const IntPoint &source, const IntPoint &mousePosition) noexcept
  {
    // FIXME: This does not support vertical writing mode or flipped block directions.
    constexpr int32 maxSpeed = 20;
    constexpr int32 iconRadius = 7;
    constexpr int32 speedReducer = 4;

    // FIXME: This doesn't work correctly with transforms.
    FloatPoint absOffset = LocalToAbsolute();

    // If the point is too far from the center we limit the speed.
    int32 yDelta = std::clamp(mousePosition.Y() - source.Y(), -maxSpeed, maxSpeed);

    // At the center we leave the space for the icon.
    if (std::abs(yDelta) < iconRadius)
    {
      return;
    }

    if (yDelta > 0)
    {
      absOffset.Move(0, static_cast<float>(ListLogicalHeight()));
    }
    else
    {
      --yDelta;
    }

    // Let's attenuate the speed.
    yDelta /= speedReducer;

    // Only scrolling, where WebKit also selects the option it reaches, which no press has asked for.
    (void)ScrollToward(IntPoint(0, static_cast<int32>(absOffset.Y()) + yDelta));
  }

  void RenderListBox::Autoscroll(const IntPoint &) noexcept
  {
    auto *frame = GetDocument().Frame();
    if (!frame)
    {
      return;
    }
    auto lastKnownMousePosition = frame->GetEventHandler().LastKnownMousePosition();
    if (!lastKnownMousePosition)
    {
      return;
    }
    IntPoint pos = FlooredIntPoint(View().FrameView().WindowToContents(*lastKnownMousePosition));

    int32 endIndex = ScrollToward(pos);
    if (SelectElement().IsDisabledFormControl())
    {
      return;
    }

    if (endIndex >= 0)
    {
      _inAutoscroll = true;

      if (!SelectElement().Multiple())
      {
        SelectElement().SetActiveSelectionAnchorIndex(endIndex);
      }

      SelectElement().SetActiveSelectionEndIndex(endIndex);
      SelectElement().UpdateListBoxSelection(!SelectElement().Multiple());
      _inAutoscroll = false;
    }
  }

  void RenderListBox::StopAutoscroll() noexcept
  {
    if (SelectElement().IsDisabledFormControl())
    {
      return;
    }

    SelectElement().ListBoxOnChange();
  }

  bool RenderListBox::ScrollToRevealElementAtListIndex(int32 index, ScrollIsAnimated animated) noexcept
  {
    // Judged by where a smooth scroll is going, so keys pressed in quick succession go on from there.
    int32 targetOffset = TargetLogicalScrollOffset();
    if (index < 0 || index >= NumItems() || ListIndexIsVisible(index, targetOffset))
    {
      return false;
    }

    // To the item's start if it's before what's in view, else to its end.
    auto itemStart = ItemLogicalHeight() * index;
    auto newOffset =
      index < FirstFullyVisibleIndex(targetOffset)
        ? itemStart
        : itemStart + ItemLogicalHeight() - LayoutUnit(ItemBlockSpacing) - ContentBoxLogicalHeight();
    ScrollToLogicalOffset(newOffset.Ceil(),
                          ScrollPositionChangeOptions::CreateProgrammaticWithOptions(animated));
    return true;
  }

  bool RenderListBox::ListIndexIsVisible(int32 index) const noexcept
  {
    return ListIndexIsVisible(index, LogicalScrollOffset());
  }

  bool RenderListBox::ListIndexIsVisible(int32 index, int32 offset) const noexcept
  {
    if (index < 0 || index >= NumItems())
    {
      return false;
    }

    LayoutUnit scrollOffset(offset);
    auto itemStart = ItemLogicalHeight() * index;
    auto itemEnd = itemStart + ItemLogicalHeight() - LayoutUnit(ItemBlockSpacing);
    return itemStart >= scrollOffset - PaddingBefore()
           && itemEnd <= scrollOffset + ContentBoxLogicalHeight() + PaddingAfter();
  }

  bool RenderListBox::Scroll(ScrollDirection direction, ScrollGranularity granularity, uint32 stepCount,
                             dom::Element **, ScrollIsAnimated animated) noexcept
  {
    return ScrollableArea::Scroll(direction, granularity, stepCount, animated);
  }

  bool RenderListBox::LogicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity,
                                    uint32 stepCount, dom::Element **, ScrollIsAnimated animated) noexcept
  {
    return ScrollableArea::Scroll(
      LogicalToPhysical(direction, GetWritingMode().IsHorizontal(), GetWritingMode().IsBlockFlipped()),
      granularity, stepCount, animated);
  }

  int32 RenderListBox::LogicalScrollOffset() const noexcept
  {
    auto scrollPosition = GetScrollPosition();
    return std::abs(GetWritingMode().IsHorizontal() ? scrollPosition.Y() : scrollPosition.X());
  }

  int32 RenderListBox::TargetLogicalScrollOffset() const noexcept
  {
    auto *animator = ExistingScrollAnimator();
    if (!animator || GetScrollAnimationStatus() != ScrollAnimationStatus::Animating)
    {
      return LogicalScrollOffset();
    }

    auto position = ScrollPositionFromOffset(RoundedIntPoint(animator->DestinationOffset()));
    return std::abs(GetWritingMode().IsHorizontal() ? position.Y() : position.X());
  }

  int32 RenderListBox::MaximumLogicalScrollOffset() const noexcept
  {
    return std::max(0, (ListLogicalHeight() - ContentBoxLogicalHeight()).Ceil());
  }

  int32 RenderListBox::FirstFullyVisibleIndex(int32 offset) const noexcept
  {
    return (LayoutUnit(offset) / ItemLogicalHeight()).Ceil();
  }

  int32 RenderListBox::EndOfFullyVisibleIndexes(int32 offset) const noexcept
  {
    // The last item has no spacing after it.
    return std::min(
      NumItems(),
      ((LayoutUnit(offset) + ContentBoxLogicalHeight() + LayoutUnit(ItemBlockSpacing)) / ItemLogicalHeight())
        .Floor());
  }

  int32 RenderListBox::ItemIndexAtLogicalOffset(LayoutUnit offset) const noexcept
  {
    return (offset / ItemLogicalHeight()).Floor();
  }

  ScrollPosition RenderListBox::GetScrollPosition() const noexcept
  {
    return _scrollPosition;
  }

  ScrollPosition RenderListBox::MinimumScrollPosition() const noexcept
  {
    return ScrollPositionFromOffset(ScrollOffset());
  }

  ScrollPosition RenderListBox::MaximumScrollPosition() const noexcept
  {
    auto maximumScrollOffset = ScrollOffset(0, MaximumLogicalScrollOffset());
    if (!GetWritingMode().IsHorizontal())
    {
      maximumScrollOffset = maximumScrollOffset.TransposedPoint();
    }
    return ScrollPositionFromOffset(maximumScrollOffset);
  }

  void RenderListBox::SetScrollOffset(const ScrollOffset &offset) noexcept
  {
    ScrollTo(ScrollPositionFromOffset(offset));
  }

  void RenderListBox::ScrollTo(const ScrollPosition &position) noexcept
  {
    if (position == _scrollPosition)
    {
      return;
    }

    _scrollPosition = position;

    Repaint();
    SetIsAwaitingScrollend(true);
    GetDocument().AddPendingScrollEventTarget(SelectElement(), dom::ScrollEventType::Scroll);
  }

  LayoutUnit RenderListBox::ItemLogicalHeight() const noexcept
  {
    return LayoutUnit(Style().MetricsOfPrimaryFont().IntHeight() + ItemBlockSpacing);
  }

  int32 RenderListBox::VerticalScrollbarWidth() const noexcept
  {
    if (auto *vBar = VerticalScrollbar())
    {
      return vBar->OccupiedWidth();
    }

    return 0;
  }

  int32 RenderListBox::HorizontalScrollbarHeight() const noexcept
  {
    if (auto *hBar = HorizontalScrollbar())
    {
      return hBar->OccupiedHeight();
    }

    return 0;
  }

  Scrollbar *RenderListBox::VerticalScrollbar() const noexcept
  {
    if (_scrollbar && _scrollbar->Orientation() == ScrollbarOrientation::Vertical)
    {
      return _scrollbar.get();
    }

    return nullptr;
  }

  Scrollbar *RenderListBox::HorizontalScrollbar() const noexcept
  {
    if (_scrollbar && _scrollbar->Orientation() == ScrollbarOrientation::Horizontal)
    {
      return _scrollbar.get();
    }

    return nullptr;
  }

  ScrollbarOrientation RenderListBox::ScrollbarOrientationForWritingMode() const noexcept
  {
    if (GetWritingMode().IsHorizontal())
    {
      return ScrollbarOrientation::Vertical;
    }
    return ScrollbarOrientation::Horizontal;
  }

  // FIXME: We ignore padding in the vertical direction as far as these values are concerned, since that's how
  // the control currently paints.
  int32 RenderListBox::ScrollWidth() const noexcept
  {
    if (GetWritingMode().IsHorizontal())
    {
      return RoundToInt(ClientWidth());
    }

    return RoundToInt(std::max(ClientWidth(), ListLogicalHeight()));
  }

  int32 RenderListBox::ScrollHeight() const noexcept
  {
    if (GetWritingMode().IsHorizontal())
    {
      return RoundToInt(std::max(ClientHeight(), ListLogicalHeight()));
    }

    return RoundToInt(ClientHeight());
  }

  int32 RenderListBox::ScrollLeft() const noexcept
  {
    if (GetWritingMode().IsHorizontal())
    {
      return 0;
    }
    return LogicalScrollTop();
  }

  void RenderListBox::SetScrollLeft(int32 newLeft, const ScrollPositionChangeOptions &options) noexcept
  {
    if (GetWritingMode().IsHorizontal())
    {
      return;
    }

    SetLogicalScrollTop(newLeft, options);
  }

  int32 RenderListBox::ScrollTop() const noexcept
  {
    if (GetWritingMode().IsHorizontal())
    {
      return LogicalScrollTop();
    }
    return 0;
  }

  void RenderListBox::SetScrollTop(int32 newTop, const ScrollPositionChangeOptions &options) noexcept
  {
    if (!GetWritingMode().IsHorizontal())
    {
      return;
    }

    SetLogicalScrollTop(newTop, options);
  }

  int32 RenderListBox::LogicalScrollTop() const noexcept
  {
    auto scrollPosition = GetScrollPosition();
    return GetWritingMode().IsHorizontal() ? scrollPosition.Y() : scrollPosition.X();
  }

  void RenderListBox::SetScrollPosition(const ScrollPosition &position,
                                        const ScrollPositionChangeOptions &options) noexcept
  {
    SetLogicalScrollTop(GetWritingMode().IsHorizontal() ? position.Y() : position.X(), options);
  }

  void RenderListBox::ScrollToLogicalOffset(int32 offset, const ScrollPositionChangeOptions &options) noexcept
  {
    offset = std::clamp(offset, 0, MaximumLogicalScrollOffset());
    int32 logicalPosition = GetWritingMode().IsBlockFlipped() ? -offset : offset;
    auto position = GetWritingMode().IsHorizontal() ? ScrollPosition(0, logicalPosition)
                                                    : ScrollPosition(logicalPosition, 0);

    // A smooth scroll is retargeted by another, and stopped by any other.
    if (options.Animated == ScrollIsAnimated::Yes)
    {
      ScrollToPositionWithAnimation(position, options);
      return;
    }

    auto previousScrollType = CurrentScrollType();
    SetCurrentScrollType(options.Type);
    (void)ScrollToPositionWithoutAnimation(position);
    SetCurrentScrollType(previousScrollType);
  }

  void RenderListBox::SetLogicalScrollTop(int32 newLogicalScrollTop,
                                          const ScrollPositionChangeOptions &options) noexcept
  {
    // A flipped list's positions are negative.
    ScrollToLogicalOffset(GetWritingMode().IsBlockFlipped() ? -newLogicalScrollTop : newLogicalScrollTop,
                          options);
  }

  bool RenderListBox::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                  const HitTestLocation &locationInContainer,
                                  const LayoutPoint &accumulatedOffset, HitTestAction hitTestAction) noexcept
  {
    if (!RenderBlockFlow::NodeAtPoint(request, result, locationInContainer, accumulatedOffset, hitTestAction))
    {
      return false;
    }

    auto &listItems = SelectElement().ListItems();
    int32 size = NumItems();
    LayoutPoint adjustedLocation = accumulatedOffset + Location();

    for (int32 i = 0; i < size; ++i)
    {
      if (!ItemBoundingBoxRect(adjustedLocation, i).Contains(locationInContainer.Point()))
      {
        continue;
      }
      if (auto *node = listItems[static_cast<size_t>(i)].get())
      {
        result.SetInnerNode(node);
        if (!result.InnerNonSharedNode())
        {
          result.SetInnerNonSharedNode(node);
        }
        result.SetLocalPoint(locationInContainer.Point() - ToLayoutSize(adjustedLocation));
        break;
      }
    }

    return true;
  }

  LayoutRect RenderListBox::ControlClipRect(const LayoutPoint &additionalOffset) const noexcept
  {
    // Clip against the padding box, to give <option>s and overlay scrollbar some extra space to get painted.
    LayoutRect clipRect = PaddingBoxRect();
    clipRect.MoveBy(additionalOffset);
    return clipRect;
  }

  LayoutRect RenderListBox::RectForScrollbar(const Scrollbar &scrollbar) const noexcept
  {
    LayoutUnit left, top, width, height;

    if (scrollbar.Orientation() == ScrollbarOrientation::Vertical)
    {
      left = ShouldPlaceVerticalScrollbarOnLeft() ? BorderLeft()
                                                  : Width() - BorderRight() - LayoutUnit(scrollbar.Width());
      top = BorderTop();
      width = LayoutUnit(scrollbar.Width());
      height = Height() - VerticalBorderExtent();
    }
    else
    {
      left = BorderLeft();
      top = Height() - BorderBottom() - LayoutUnit(scrollbar.Height());
      width = Width() - HorizontalBorderExtent();
      height = LayoutUnit(scrollbar.Height());
    }

    return LayoutRect {left, top, width, height};
  }

  void RenderListBox::RegisterForAnimatedScroll() noexcept
  {
    View().FrameView().AddScrollableAreaForAnimatedScroll(*this);
  }

  void RenderListBox::InvalidateScrollbar(Scrollbar &scrollbar, const IntRect &rect) noexcept
  {
    auto scrollRect = rect;
    auto scrollbarLocation = RectForScrollbar(scrollbar).Location();
    scrollRect.Move(scrollbarLocation.X().ToInt(), scrollbarLocation.Y().ToInt());
    RepaintRectangle(LayoutRect(scrollRect));
  }

  IntPoint RenderListBox::ConvertFromContainingViewToScrollbar(const Scrollbar &scrollbar,
                                                               const IntPoint &parentPoint) const noexcept
  {
    // From the view's coordinates into page ("absolute") coordinates, then the renderer's.
    auto &view = View().FrameView();
    IntPoint absolutePoint = parentPoint + IntSize(view.ScrollX(), view.ScrollY());
    IntPoint point =
      RoundedIntPoint(AbsoluteToLocal(FloatPoint(absolutePoint), {MapCoordinatesMode::UseTransforms}));
    auto scrollbarLocation = RectForScrollbar(scrollbar).Location();
    point.Move(-scrollbarLocation.X().ToInt(), -scrollbarLocation.Y().ToInt());
    return point;
  }

  IntSize RenderListBox::ContentsSize() const noexcept
  {
    // What's in view and as much again as the list scrolls, so the scrollable area's maximum is the list
    // box's.
    auto size = VisibleSize();
    if (GetWritingMode().IsHorizontal())
    {
      size.Expand(0, MaximumLogicalScrollOffset());
    }
    else
    {
      size.Expand(MaximumLogicalScrollOffset(), 0);
    }
    return size;
  }

  ScrollableArea *RenderListBox::EnclosingScrollableArea() const noexcept
  {
    auto *layer = EnclosingLayer();
    if (!layer)
    {
      return nullptr;
    }

    auto *enclosingScrollableLayer = layer->EnclosingScrollableLayer(ExcludeSelf, CrossFrameBoundaries::No);
    if (!enclosingScrollableLayer)
    {
      return nullptr;
    }

    return enclosingScrollableLayer->GetScrollableArea();
  }

  bool RenderListBox::IsScrollableOrRubberbandable() const noexcept
  {
    return _scrollbar != nullptr;
  }

  void RenderListBox::ScrollDidEnd() noexcept
  {
    if (!IsAwaitingScrollend())
    {
      return;
    }
    SetIsAwaitingScrollend(false);
    GetDocument().AddPendingScrollEventTarget(SelectElement(), dom::ScrollEventType::Scrollend);
  }

  Ref<Scrollbar> RenderListBox::CreateScrollbar(ScrollbarOrientation orientation) noexcept
  {
    auto widget = Scrollbar::CreateNativeScrollbar(
      *this, orientation, RenderTheme::Singleton().ScrollbarWidthStyleForPart(StyleAppearance::Listbox));
    DidAddScrollbar(widget.get(), orientation);
    return widget;
  }

  void RenderListBox::DestroyScrollbar() noexcept
  {
    if (!_scrollbar)
    {
      return;
    }

    WillRemoveScrollbar(*_scrollbar, _scrollbar->Orientation());
    _scrollbar = nullptr;
  }

  void RenderListBox::SetHasScrollbar(ScrollbarOrientation orientation) noexcept
  {
    if (VerticalScrollbar() && orientation == ScrollbarOrientation::Vertical)
    {
      return;
    }

    if (HorizontalScrollbar() && orientation == ScrollbarOrientation::Horizontal)
    {
      return;
    }

    DestroyScrollbar();
    _scrollbar = CreateScrollbar(orientation);
    _scrollbar->StyleChanged();
  }
}
