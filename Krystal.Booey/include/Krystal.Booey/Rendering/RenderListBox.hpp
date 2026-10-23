#pragma once

#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class HTMLOptGroupElement;
  class HTMLOptionElement;
  class HTMLSelectElement;
}

namespace krys::boo
{
  /// @brief The renderer of a select drawn as a list box: it draws its own items, and is its own scrollable
  /// area, scrolling by whole items.
  class RenderListBox final : public RenderBlockFlow, public ScrollableArea
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderListBox);

  public:
    // Both bases can be pointed to, so each has its own count: a checked pointer to the list box counts as a
    // renderer, and deleting it checks both.
    using CanMakeCheckedPtr<RenderObject, false>::AddRefChecked;
    using CanMakeCheckedPtr<RenderObject, false>::SubRefChecked;

    KRYS_NODISCARD uint32 GetRefCountChecked() const noexcept
    {
      return CanMakeCheckedPtr<RenderObject, false>::GetRefCountChecked()
             + CanMakeCheckedPtr<ScrollableArea, false>::GetRefCountChecked();
    }

    void SetDidBeginCheckedPtrDeletion() noexcept
    {
      CanMakeCheckedPtr<RenderObject, false>::SetDidBeginCheckedPtrDeletion();
      CanMakeCheckedPtr<ScrollableArea, false>::SetDidBeginCheckedPtrDeletion();
    }

  private:
    bool _optionsChanged {true};
    bool _scrollToRevealSelectionAfterLayout {false};
    bool _inAutoscroll {false};
    int32 _optionsLogicalWidth {0};

    RefPtr<Scrollbar> _scrollbar;

    // Note: This is based on item index rather than a pixel offset.
    ScrollPosition _scrollPosition;

    Maybe<int32> _indexOfFirstVisibleItemInsidePaddingBeforeArea;
    Maybe<int32> _indexOfFirstVisibleItemInsidePaddingAfterArea;

  public:
    RenderListBox(html::HTMLSelectElement &element, RenderStyle &&style) noexcept;
    ~RenderListBox() noexcept override;

    KRYS_NODISCARD html::HTMLSelectElement &SelectElement() const noexcept;

    void SelectionChanged() noexcept;

    void SetOptionsChanged(bool changed) noexcept
    {
      _optionsChanged = changed;
    }

    KRYS_NODISCARD int32 ListIndexAtOffset(const LayoutSize &offset) const noexcept;
    KRYS_NODISCARD LayoutRect ItemBoundingBoxRect(const LayoutPoint &additionalOffset,
                                                  int32 index) const noexcept;
    KRYS_NODISCARD Maybe<LayoutRect>
      LocalBoundsOfOption(const html::HTMLOptionElement &option) const noexcept;
    KRYS_NODISCARD Maybe<LayoutRect>
      LocalBoundsOfOptGroup(const html::HTMLOptGroupElement &optGroup) const noexcept;

    bool ScrollToRevealElementAtListIndex(int32 index) noexcept;
    KRYS_NODISCARD bool ListIndexIsVisible(int32 index) const noexcept;

    /// @return The new index, or -1 if no scroll occurred.
    int32 ScrollToward(const IntPoint &destination) noexcept;

    /// @brief How many items tall the list box is.
    KRYS_NODISCARD uint32 Size() const noexcept;

    bool Scroll(ScrollDirection direction, ScrollGranularity granularity, uint32 stepCount = 1,
                dom::Element **stopElement = nullptr) noexcept override;

    void ScrollDidEnd() noexcept final;

    KRYS_NODISCARD Scrollbar *VerticalScrollbar() const noexcept final;
    KRYS_NODISCARD Scrollbar *HorizontalScrollbar() const noexcept final;

    KRYS_NODISCARD ScrollPosition GetScrollPosition() const noexcept final;

    KRYS_NODISCARD bool IsHorizontalWritingMode() const noexcept final
    {
      return RenderBlockFlow::IsHorizontalWritingMode();
    }

  private:
    void WillBeDestroyed() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderListBox";
    }

    void UpdateFromElement() noexcept override;

    KRYS_NODISCARD bool HasControlClip() const noexcept override
    {
      return true;
    }

    void PaintObject(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept override;
    KRYS_NODISCARD LayoutRect ControlClipRect(const LayoutPoint &additionalOffset) const noexcept override;
    bool IsPointInOverflowControl(HitTestResult &result, const LayoutPoint &locationInContainer,
                                  const LayoutPoint &accumulatedOffset) noexcept override;
    bool LogicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity, uint32 stepCount = 1,
                       dom::Element **stopElement = nullptr) noexcept override;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;
    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;
    void ComputePreferredLogicalWidths() noexcept override;
    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

    void Layout() noexcept override;

    KRYS_NODISCARD bool CanBeProgramaticallyScrolled() const noexcept override
    {
      return true;
    }

    void Autoscroll(const IntPoint &positionInWindow) noexcept override;
    void StopAutoscroll() noexcept override;

    KRYS_NODISCARD int32 VerticalScrollbarWidth() const noexcept override;
    KRYS_NODISCARD int32 HorizontalScrollbarHeight() const noexcept override;
    KRYS_NODISCARD int32 ScrollLeft() const noexcept override;
    KRYS_NODISCARD int32 ScrollTop() const noexcept override;
    KRYS_NODISCARD int32 ScrollWidth() const noexcept override;
    KRYS_NODISCARD int32 ScrollHeight() const noexcept override;
    void SetScrollLeft(int32 newLeft, const ScrollPositionChangeOptions &options =
                                        ScrollPositionChangeOptions::CreateProgrammatic()) noexcept override;
    void SetScrollTop(int32 newTop, const ScrollPositionChangeOptions &options =
                                      ScrollPositionChangeOptions::CreateProgrammatic()) noexcept override;

    KRYS_NODISCARD int32 LogicalScrollTop() const noexcept;
    void SetLogicalScrollTop(int32 newLogicalScrollTop) noexcept;

    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;

#pragma region ScrollableArea

    void SetScrollOffset(const ScrollOffset &offset) noexcept final;
    KRYS_NODISCARD ScrollPosition MinimumScrollPosition() const noexcept final;
    KRYS_NODISCARD ScrollPosition MaximumScrollPosition() const noexcept final;
    void InvalidateScrollbar(Scrollbar &scrollbar, const IntRect &rect) noexcept final;

    // We don't support resize on list boxes yet. If we did these would have to change.
    KRYS_NODISCARD bool IsScrollCornerVisible() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD IntRect ScrollCornerRect() const noexcept final
    {
      return IntRect();
    }

    KRYS_NODISCARD IntPoint ConvertFromContainingViewToScrollbar(
      const Scrollbar &scrollbar, const IntPoint &parentPoint) const noexcept final;
    KRYS_NODISCARD IntSize ContentsSize() const noexcept final;

    KRYS_NODISCARD IntSize VisibleSize() const noexcept final
    {
      return IntSize(Width().ToInt(), Height().ToInt());
    }

    KRYS_NODISCARD ScrollableArea *EnclosingScrollableArea() const noexcept final;
    KRYS_NODISCARD bool IsScrollableOrRubberbandable() const noexcept final;

    KRYS_NODISCARD bool ShouldPlaceVerticalScrollbarOnLeft() const noexcept final
    {
      return Style().ShouldPlaceVerticalScrollbarOnLeft();
    }

    KRYS_NODISCARD bool UseDarkAppearanceForScrollbars() const noexcept final;

#pragma endregion

    // NOTE: This should only be called by the overridden SetScrollOffset from ScrollableArea.
    void ScrollTo(const ScrollPosition &position) noexcept;
    void ScrollToPosition(int32 positionIndex) noexcept;

    using PaintFunction = Func<void(PaintInfo &, const LayoutPoint &, int32 listItemIndex)>;
    void PaintItem(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                   const PaintFunction &paintFunction) noexcept;

    void SetHasScrollbar(ScrollbarOrientation orientation) noexcept;
    KRYS_NODISCARD Ref<Scrollbar> CreateScrollbar(ScrollbarOrientation orientation) noexcept;
    void DestroyScrollbar() noexcept;

    KRYS_NODISCARD int32 MaximumNumberOfItemsThatFitInPaddingAfterArea() const noexcept;
    KRYS_NODISCARD int32 NumberOfVisibleItemsInPaddingBefore() const noexcept;
    KRYS_NODISCARD int32 NumberOfVisibleItemsInPaddingAfter() const noexcept;
    void ComputeFirstIndexesVisibleInPaddingBeforeAfterAreas() noexcept;
    KRYS_NODISCARD LayoutUnit ItemLogicalHeight() const noexcept;

    enum class ConsiderPadding : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD int32
      NumVisibleItems(ConsiderPadding considerPadding = ConsiderPadding::No) const noexcept;
    KRYS_NODISCARD int32 NumItems() const noexcept;
    KRYS_NODISCARD LayoutUnit ListLogicalHeight() const noexcept;
    KRYS_NODISCARD Maybe<int32> OptionRowIndex(const html::HTMLOptionElement &option) const noexcept;

    KRYS_NODISCARD LayoutRect RectForScrollbar(const Scrollbar &scrollbar) const noexcept;
    void PaintScrollbar(PaintInfo &paintInfo, const LayoutPoint &paintOffset, Scrollbar &scrollbar) noexcept;
    void PaintItemForeground(PaintInfo &paintInfo, const LayoutPoint &paintOffset, int32 listIndex) noexcept;
    void PaintItemBackground(PaintInfo &paintInfo, const LayoutPoint &paintOffset, int32 listIndex) noexcept;
    void ScrollToRevealSelection() noexcept;
    KRYS_NODISCARD ScrollbarOrientation ScrollbarOrientationForWritingMode() const noexcept;
    KRYS_NODISCARD int32 IndexOffset() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderListBox)

  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderListBox();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
