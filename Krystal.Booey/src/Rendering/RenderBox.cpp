#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLLegendElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderDeprecatedFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    // The side tables of the boxes that have an override, keyed by address. Entries are removed when the box
    // is destroyed.
    template <typename T>
    using OverrideMap = HashMap<const RenderBox *, T>;

    KRYS_NODISCARD OverrideMap<LayoutUnit> &OverridingLogicalHeightMap() noexcept
    {
      static OverrideMap<LayoutUnit> map;
      return map;
    }
    KRYS_NODISCARD OverrideMap<LayoutUnit> &OverridingLogicalWidthMap() noexcept
    {
      static OverrideMap<LayoutUnit> map;
      return map;
    }
    KRYS_NODISCARD OverrideMap<style::PreferredSize> &
      OverridingLogicalHeightMapForFlexBasisComputation() noexcept
    {
      static OverrideMap<style::PreferredSize> map;
      return map;
    }
    KRYS_NODISCARD OverrideMap<style::PreferredSize> &
      OverridingLogicalWidthMapForFlexBasisComputation() noexcept
    {
      static OverrideMap<style::PreferredSize> map;
      return map;
    }
    // FIXME: We should store these based on physical direction.
    KRYS_NODISCARD OverrideMap<RenderBox::GridAreaSize> &GridAreaContentLogicalHeightMap() noexcept
    {
      static OverrideMap<RenderBox::GridAreaSize> map;
      return map;
    }
    KRYS_NODISCARD OverrideMap<RenderBox::GridAreaSize> &GridAreaContentLogicalWidthMap() noexcept
    {
      static OverrideMap<RenderBox::GridAreaSize> map;
      return map;
    }

    template <typename T>
    KRYS_NODISCARD Maybe<T> Find(const OverrideMap<T> &map, const RenderBox *box) noexcept
    {
      auto it = map.find(box);
      return it != map.end() ? Maybe<T> {it->second} : null;
    }

    constexpr uint32 BackgroundObscurationTestMaxDepth = 4;

    bool s_hadNonVisibleOverflow = false;
  }

  RenderBox::RenderBox(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                       TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBoxModelObject(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsBox,
                             typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBox());
  }

  RenderBox::RenderBox(Type type, dom::Document &document, RenderStyle &&style,
                       EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBoxModelObject(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsBox,
                             typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBox());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBox::~RenderBox() noexcept = default;

  void RenderBox::WillBeDestroyed() noexcept
  {
    // FIXME: Stop autoscrolling, and unregister the box's scroll snap positions, container query box,
    // anchors, position-try box and anchor scroll adjusters, once there are any.

    // The weak maps upstream forget the box by themselves.
    OverridingLogicalHeightMap().erase(this);
    OverridingLogicalWidthMap().erase(this);
    OverridingLogicalHeightMapForFlexBasisComputation().erase(this);
    OverridingLogicalWidthMapForFlexBasisComputation().erase(this);
    GridAreaContentLogicalHeightMap().erase(this);
    GridAreaContentLogicalWidthMap().erase(this);
    RenderBlock::RemovePercentHeightDescendant(*this);

    RenderBoxModelObject::WillBeDestroyed();
  }

  void RenderBox::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    s_hadNonVisibleOverflow = HasNonVisibleOverflow();

    const RenderStyle *oldStyle = HasInitializedStyle() ? &Style() : nullptr;
    if (oldStyle)
    {
      // FIXME: Repaint the root contents (the background of the root element or the body element could
      // propagate up to the canvas), once there's painting.

      // When a layout hint happens and an object's position style changes, we have to do a layout
      // to dirty the render tree using the old position value now.
      if (diff == style::DifferenceResult::Layout && Parent()
          && oldStyle->GetPosition() != newStyle.GetPosition())
      {
        if (!oldStyle->HasOutOfFlowPosition() && newStyle.HasOutOfFlowPosition())
        {
          // We are about to go out of flow. Before that takes place, we need to mark the
          // current containing block chain for preferred widths recalculation.
          SetNeedsLayoutAndPreferredWidthsUpdate();
          // FIXME: Clear the flex container's cached sizes of the item, once there's flex layout.
          if (dom::IsInTopLayerOrBackdrop(Style(), GetElement()))
          {
            // Since top layer's containing block is driven by the associated element's state (see
            // Element::isInTopLayerOrBackdrop) and this state is set before styleWillChange call, dirtying
            // ancestors starting from _this_ fails to mark the current ancestor chain properly.
            Parent()->SetChildNeedsLayout();
          }
        }
        else
        {
          ScheduleLayout(MarkContainingBlocksForLayout());
        }

        if (oldStyle->GetPosition() != PositionType::Static && newStyle.HasOutOfFlowPosition())
        {
          Parent()->SetChildNeedsLayout();
        }
        // FIXME: Remove the box from its block's float list, once there are floating objects.
      }
    }

    // FIXME: Scroll snap positions, container query boxes, position-try boxes and anchor-positioned boxes,
    // once there are any.

    RenderBoxModelObject::StyleWillChange(diff, newStyle);
  }

  void RenderBox::InvalidateAncestorBackgroundObscurationStatus() noexcept
  {
    auto *parentToInvalidate = Parent();
    for (uint32 i = 0; i < BackgroundObscurationTestMaxDepth && parentToInvalidate; ++i)
    {
      parentToInvalidate->InvalidateBackgroundObscurationStatus();
      parentToInvalidate = parentToInvalidate->Parent();
    }
  }

  void RenderBox::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    // Horizontal writing mode definition is updated in RenderBoxModelObject::updateFromStyle,
    // (as part of the RenderBoxModelObject::styleDidChange call below). So, we can safely cache the
    // horizontal writing mode value before style change here.
    bool oldHorizontalWritingMode = IsHorizontalWritingMode();

    RenderBoxModelObject::StyleDidChange(diff, oldStyle);

    const RenderStyle &newStyle = Style();
    if (NeedsLayout() && oldStyle)
    {
      RenderBlock::RemovePercentHeightDescendant(*this);

      // Normally we can do optimized positioning layout for absolute/fixed positioned objects. There is one
      // special case, however, which is when the positioned object's margin-before is changed. In this case
      // the parent has to get a layout in order to run margin collapsing to determine the new static
      // position.
      if (IsOutOfFlowPositioned() && newStyle.HasStaticBlockPosition(IsHorizontalWritingMode())
          && oldStyle->MarginBefore() != newStyle.MarginBefore() && Parent()
          && !Parent()->NormalChildNeedsLayout())
      {
        Parent()->SetChildNeedsLayout();
      }
    }

    if (RenderBlock::HasPercentHeightContainerMap() && FirstChild()
        && oldHorizontalWritingMode != IsHorizontalWritingMode())
    {
      RenderBlock::ClearPercentHeightDescendantsFrom(*this);
    }

    // FIXME: The scrollable area's scroll position, scrollbar placement, width and colour, once there are
    // layers.

    // Our opaqueness might have changed without triggering layout.
    if (diff >= style::DifferenceResult::Repaint && diff <= style::DifferenceResult::RepaintLayer)
    {
      InvalidateAncestorBackgroundObscurationStatus();
    }

    // FIXME: The view's scrollbar overlay style and the compositor, the shape outside and the grid position,
    // once there are any.

    // Changing the position from/to absolute can potentially create/remove flex/grid items, as absolutely
    // positioned children of a flex/grid box are out-of-flow, and thus, not flex/grid items. This means that
    // we need to clear any override content size set by our container, because it would likely be incorrect
    // after the style change.
    if (IsOutOfFlowPositioned() && Parent()
        && Parent()->Style().IsDisplayFlexibleBoxIncludingDeprecatedOrGridFormattingContextBox())
    {
      ClearOverridingSize();
    }

    if (oldStyle && oldStyle->HasOutOfFlowPosition() != Style().HasOutOfFlowPosition())
    {
      ClearGridAreaContentSize();
      if (auto *containingBlock = ContainingBlock(); containingBlock && oldStyle->HasOutOfFlowPosition())
      {
        // When going from out-of-flow to inflow, the containing block gains new descendant content and its
        // preferred width becomes invalid.
        containingBlock->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }

    // FIXME: Anchor scroll adjusters and the anchor dependencies of scrollers, once there's anchor
    // positioning.
  }

  void RenderBox::UpdateFromStyle() noexcept
  {
    RenderBoxModelObject::UpdateFromStyle();

    const auto &styleToUse = Style();
    bool isDocElementRenderer = IsDocumentElementRenderer();
    bool isViewObject = IsRenderView();

    // The root and the RenderView always paint their backgrounds/borders.
    if (isDocElementRenderer || isViewObject)
    {
      SetHasVisibleBoxDecorations(true);
    }

    SetFloating(!IsOutOfFlowPositioned() && styleToUse.IsFloating());

    // We also handle <body> and <html>, whose overflow applies to the viewport.
    if (!(EffectiveOverflowX() == Overflow::Visible && EffectiveOverflowY() == Overflow::Visible)
        && !isDocElementRenderer && IsRenderBlock())
    {
      bool boxHasNonVisibleOverflow = true;
      if (IsBody())
      {
        // Overflow on the body can propagate to the viewport under the following conditions.
        // (1) The root element is <html>.
        // (2) We are the primary <body> (can be checked by looking at document.body).
        // (3) The root element has visible overflow.
        // (4) No containment is set either on the body or on the html document element.
        auto &document = GetDocument();
        auto documentElement = document.DocumentElement();
        auto *documentElementRenderer = documentElement->Renderer();
        auto *htmlElement = DynamicDowncast<html::HTMLElement>(documentElement.get());
        if (htmlElement && Is<html::HTMLHtmlElement>(*htmlElement) && document.Body().get() == GetElement()
            && documentElementRenderer && documentElementRenderer->EffectiveOverflowX() == Overflow::Visible
            && styleToUse.UsedContain().IsNone() && documentElementRenderer->Style().UsedContain().IsNone())
        {
          boxHasNonVisibleOverflow = false;
        }
      }
      // Check for overflow clip.
      // It's sufficient to just check one direction, since it's illegal to have visible on only one overflow
      // value.
      if (boxHasNonVisibleOverflow)
      {
        // FIXME: Repaint the overflow that's erased, once there's painting.
        SetHasNonVisibleOverflow();
      }
    }

    SetHasTransformRelatedProperty(ComputeHasTransformRelatedProperty(styleToUse));
    // FIXME: Reflections (`-webkit-box-reflect` was dropped).
  }

  bool RenderBox::ComputeHasTransformRelatedProperty(const RenderStyle &styleToUse) const noexcept
  {
    if (styleToUse.HasTransformRelatedProperty())
    {
      return true;
    }
    // FIXME: The rest only applies with `css3DTransformBackfaceVisibilityInteroperabilityEnabled`, a preview
    // setting that's off by default.
    return false;
  }

#pragma region Frame rect and boxes

  LayoutUnit RenderBox::LogicalLeft() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? X() : Y();
  }

  LayoutUnit RenderBox::LogicalRight() const noexcept
  {
    return LogicalLeft() + LogicalWidth();
  }

  LayoutUnit RenderBox::LogicalTop() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? Y() : X();
  }

  LayoutUnit RenderBox::LogicalBottom() const noexcept
  {
    return LogicalTop() + LogicalHeight();
  }

  LayoutUnit RenderBox::LogicalWidth() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? Width() : Height();
  }

  LayoutUnit RenderBox::LogicalHeight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? Height() : Width();
  }

  void RenderBox::SetLogicalLeft(LayoutUnit left) noexcept
  {
    if (Style().GetWritingMode().IsHorizontal())
    {
      SetX(left);
    }
    else
    {
      SetY(left);
    }
  }

  void RenderBox::SetLogicalTop(LayoutUnit top) noexcept
  {
    if (Style().GetWritingMode().IsHorizontal())
    {
      SetY(top);
    }
    else
    {
      SetX(top);
    }
  }

  void RenderBox::SetLogicalLocation(LayoutPoint location) noexcept
  {
    SetLocation(Style().GetWritingMode().IsHorizontal() ? location : location.TransposedPoint());
  }

  void RenderBox::SetLogicalWidth(LayoutUnit size) noexcept
  {
    if (Style().GetWritingMode().IsHorizontal())
    {
      SetWidth(size);
    }
    else
    {
      SetHeight(size);
    }
  }

  void RenderBox::SetLogicalHeight(LayoutUnit size) noexcept
  {
    if (Style().GetWritingMode().IsHorizontal())
    {
      SetHeight(size);
    }
    else
    {
      SetWidth(size);
    }
  }

  void RenderBox::SetLogicalSize(LayoutSize size) noexcept
  {
    SetSize(Style().GetWritingMode().IsHorizontal() ? size : size.TransposedSize());
  }

  LayoutSize RenderBox::LogicalSize() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? _frameRect.Size() : _frameRect.Size().TransposedSize();
  }

  LayoutRect RenderBox::MarginBoxRect() const noexcept
  {
    auto zoomFactor = Style().UsedZoomForLength();
    auto left = ResolveLengthPercentageUsingContainerLogicalWidth(Style().MarginLeft(), zoomFactor);
    auto right = ResolveLengthPercentageUsingContainerLogicalWidth(Style().MarginRight(), zoomFactor);
    auto top = ResolveLengthPercentageUsingContainerLogicalWidth(Style().MarginTop(), zoomFactor);
    auto bottom = ResolveLengthPercentageUsingContainerLogicalWidth(Style().MarginBottom(), zoomFactor);
    return {-left, -top, Size().Width() + left + right, Size().Height() + top + bottom};
  }

  LayoutRect RenderBox::ContentBoxRect() const noexcept
  {
    auto verticalScrollbarWidth = 0_lu;
    auto horizontalScrollbarHeight = 0_lu;
    auto leftScrollbarSpace = 0_lu;
    auto topScrollbarSpace = 0_lu;

    if (HasNonVisibleOverflow())
    {
      verticalScrollbarWidth = LayoutUnit(VerticalScrollbarWidth());
      horizontalScrollbarHeight = LayoutUnit(HorizontalScrollbarHeight());

      bool bothEdgeScrollbarGutters = Style().GetScrollbarGutter().IsStableBothEdges();

      if (Style().ShouldPlaceVerticalScrollbarOnLeft() || bothEdgeScrollbarGutters)
      {
        leftScrollbarSpace = verticalScrollbarWidth;
      }
      // FIXME: It's wrong that scrollbar-gutter: both-edges affects height: webkit.org/b/266938
      if (bothEdgeScrollbarGutters)
      {
        topScrollbarSpace = horizontalScrollbarHeight;
      }
    }

    auto padding = Padding();
    auto borderWidths = BorderWidths();
    auto location = LayoutPoint {borderWidths.Left() + padding.Left() + leftScrollbarSpace,
                                 borderWidths.Top() + padding.Top() + topScrollbarSpace};

    auto paddingBoxWidth =
      std::max(0_lu, Width() - borderWidths.Left() - borderWidths.Right() - verticalScrollbarWidth);
    auto paddingBoxHeight =
      std::max(0_lu, Height() - borderWidths.Top() - borderWidths.Bottom() - horizontalScrollbarHeight);

    auto width = std::max(0_lu, paddingBoxWidth - padding.Left() - padding.Right() - leftScrollbarSpace);
    auto height = std::max(0_lu, paddingBoxHeight - padding.Top() - padding.Bottom() - topScrollbarSpace);

    return {location, LayoutSize {width, height}};
  }

  LayoutPoint RenderBox::ContentBoxLocation() const noexcept
  {
    LayoutUnit verticalScrollbarSpace =
      (Style().ShouldPlaceVerticalScrollbarOnLeft() || Style().GetScrollbarGutter().IsStableBothEdges())
        ? LayoutUnit(VerticalScrollbarWidth())
        : 0_lu;
    LayoutUnit horizontalScrollbarSpace =
      Style().GetScrollbarGutter().IsStableBothEdges() ? LayoutUnit(HorizontalScrollbarHeight()) : 0_lu;
    return {BorderLeft() + PaddingLeft() + verticalScrollbarSpace,
            BorderTop() + PaddingTop() + horizontalScrollbarSpace};
  }

  LayoutRect RenderBox::ComputedCSSContentBoxRect() const noexcept
  {
    auto stableBothEdges = Style().GetScrollbarGutter().IsStableBothEdges();
    return LayoutRect(BorderLeft() + ComputedCSSPaddingLeft(), BorderTop() + ComputedCSSPaddingTop(),
                      PaddingBoxWidth() - ComputedCSSPaddingLeft() - ComputedCSSPaddingRight()
                        - (stableBothEdges ? LayoutUnit(VerticalScrollbarWidth()) : 0_lu),
                      PaddingBoxHeight() - ComputedCSSPaddingTop() - ComputedCSSPaddingBottom()
                        - (stableBothEdges ? LayoutUnit(HorizontalScrollbarHeight()) : 0_lu));
  }

  LayoutPoint RenderBox::TopLeftLocation() const noexcept
  {
    // This is inlined for speed, since it is used by updateLayerPosition() during scrolling.
    auto *view = GetDocument().View();
    if (!view || !view->HasFlippedBlockRenderers())
    {
      return Location();
    }
    return TopLeftLocationWithFlipping();
  }

  LayoutSize RenderBox::TopLeftLocationOffset() const noexcept
  {
    auto location = TopLeftLocation();
    return {location.X(), location.Y()};
  }

  LayoutPoint RenderBox::TopLeftLocationWithFlipping() const noexcept
  {
    auto *containerBlock = ContainingBlock();
    if (!containerBlock || containerBlock == this)
    {
      return Location();
    }
    return containerBlock->FlipForWritingModeForChild(*this, Location());
  }

  LayoutSize RenderBox::ContentBoxSize() const noexcept
  {
    return {ContentBoxWidth(), ContentBoxHeight()};
  }

  LayoutUnit RenderBox::ContentBoxWidth() const noexcept
  {
    return std::max(
      0_lu,
      PaddingBoxWidth() - PaddingLeft() - PaddingRight()
        - (Style().GetScrollbarGutter().IsStableBothEdges() ? LayoutUnit(VerticalScrollbarWidth()) : 0_lu));
  }

  LayoutUnit RenderBox::ContentBoxHeight() const noexcept
  {
    return std::max(0_lu, PaddingBoxHeight() - PaddingTop() - PaddingBottom()
                            - (Style().GetScrollbarGutter().IsStableBothEdges()
                                 ? LayoutUnit(HorizontalScrollbarHeight())
                                 : 0_lu));
  }

  LayoutSize RenderBox::ContentBoxLogicalSize() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ContentBoxSize() : ContentBoxSize().TransposedSize();
  }

  LayoutUnit RenderBox::ContentBoxLogicalWidth() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ContentBoxWidth() : ContentBoxHeight();
  }

  LayoutUnit RenderBox::ContentBoxLogicalHeight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ContentBoxHeight() : ContentBoxWidth();
  }

  LayoutUnit RenderBox::ContentBoxLogicalWidth(LayoutUnit overridingBorderBoxWidth) const noexcept
  {
    return std::max(
      LayoutUnit(),
      overridingBorderBoxWidth - BorderAndPaddingLogicalWidth() - LayoutUnit(ScrollbarLogicalWidth())
        - (Style().GetScrollbarGutter().IsStableBothEdges() ? LayoutUnit(ScrollbarLogicalWidth()) : 0_lu));
  }

  LayoutUnit RenderBox::ContentBoxLogicalHeight(LayoutUnit overridingBorderBoxHeight) const noexcept
  {
    return std::max(
      0_lu,
      overridingBorderBoxHeight - BorderAndPaddingLogicalHeight() - LayoutUnit(ScrollbarLogicalHeight())
        - (Style().GetScrollbarGutter().IsStableBothEdges() ? LayoutUnit(ScrollbarLogicalHeight()) : 0_lu));
  }

  LayoutUnit RenderBox::PaddingBoxWidth() const noexcept
  {
    return std::max(0_lu, Width() - BorderLeft() - BorderRight() - LayoutUnit(VerticalScrollbarWidth()));
  }

  LayoutUnit RenderBox::PaddingBoxHeight() const noexcept
  {
    return std::max(0_lu, Height() - BorderTop() - BorderBottom() - LayoutUnit(HorizontalScrollbarHeight()));
  }

  LayoutRect RenderBox::PaddingBoxRect() const noexcept
  {
    auto offsetForScrollbar = 0_lu;
    auto verticalScrollbarWidth = 0_lu;
    auto horizontalScrollbarHeight = 0_lu;
    if (HasNonVisibleOverflow())
    {
      verticalScrollbarWidth = LayoutUnit(VerticalScrollbarWidth());
      offsetForScrollbar = Style().ShouldPlaceVerticalScrollbarOnLeft() ? verticalScrollbarWidth : 0_lu;
      horizontalScrollbarHeight = LayoutUnit(HorizontalScrollbarHeight());
    }

    auto borderWidths = BorderWidths();
    return LayoutRect {borderWidths.Left() + offsetForScrollbar, borderWidths.Top(),
                       Width() - borderWidths.Left() - borderWidths.Right() - verticalScrollbarWidth,
                       Height() - borderWidths.Top() - borderWidths.Bottom() - horizontalScrollbarHeight};
  }

  LayoutRect RenderBox::PaddingBoxRectIncludingScrollbar() const noexcept
  {
    auto borderWidths = BorderWidths();
    return LayoutRect(borderWidths.Left(), borderWidths.Top(),
                      Width() - borderWidths.Left() - borderWidths.Right(),
                      Height() - borderWidths.Top() - borderWidths.Bottom());
  }

  LayoutUnit RenderBox::ClientLeft() const noexcept
  {
    return BorderLeft();
  }

  LayoutUnit RenderBox::ClientTop() const noexcept
  {
    return BorderTop();
  }

  // More IE extensions.  clientWidth and clientHeight represent the interior of an object
  // excluding border and scrollbar.
  LayoutUnit RenderBox::ClientWidth() const noexcept
  {
    return PaddingBoxWidth();
  }

  LayoutUnit RenderBox::ClientHeight() const noexcept
  {
    return PaddingBoxHeight();
  }

  LayoutUnit RenderBox::ClientLogicalWidth() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ClientWidth() : ClientHeight();
  }

  LayoutUnit RenderBox::ClientLogicalHeight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ClientHeight() : ClientWidth();
  }

  LayoutUnit RenderBox::ClientLogicalBottom() const noexcept
  {
    return BorderBefore() + ClientLogicalHeight();
  }

  LayoutRect RenderBox::ClientBoxRect() const noexcept
  {
    return LayoutRect(ClientLeft(), ClientTop(), ClientWidth(), ClientHeight());
  }

  int32 RenderBox::ScrollWidth() const noexcept
  {
    // FIXME: The layer's scroll width when the box has potentially scrollable overflow, once there are
    // layers. For objects with visible overflow, this matches IE.
    if (Style().GetWritingMode().IsLogicalLeftInlineStart())
    {
      // FIXME: This should use snappedIntSize() instead with absolute coordinates.
      return RoundToInt(std::max(ClientWidth(), LayoutOverflowRect().MaxX() - BorderLeft()));
    }
    return RoundToInt(ClientWidth() - std::min<LayoutUnit>(0_lu, LayoutOverflowRect().X() - BorderLeft()));
  }

  int32 RenderBox::ScrollHeight() const noexcept
  {
    // FIXME: The layer's scroll height when the box has potentially scrollable overflow, once there are
    // layers. For objects with visible overflow, this matches IE.
    // FIXME: Need to work right with writing modes.
    // FIXME: This should use snappedIntSize() instead with absolute coordinates.
    return RoundToInt(std::max(ClientHeight(), LayoutOverflowRect().MaxY() - BorderTop()));
  }

  LayoutUnit RenderBox::OffsetLeft() const noexcept
  {
    return AdjustedPositionRelativeToOffsetParent(TopLeftLocation()).X();
  }

  LayoutUnit RenderBox::OffsetTop() const noexcept
  {
    return AdjustedPositionRelativeToOffsetParent(TopLeftLocation()).Y();
  }

#pragma endregion

#pragma region Tree

  RenderBox *RenderBox::ParentBox() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Parent()))
    {
      return box;
    }

    krys_debug_assert(!Parent());
    return nullptr;
  }

  RenderBox *RenderBox::FirstChildBox() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(FirstChild()))
    {
      return box;
    }

    krys_debug_assert(!FirstChild());
    return nullptr;
  }

  RenderBox *RenderBox::FirstInFlowChildBox() const noexcept
  {
    return DynamicDowncast<RenderBox>(FirstInFlowChild());
  }

  RenderBox *RenderBox::LastChildBox() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(LastChild()))
    {
      return box;
    }

    krys_debug_assert(!LastChild());
    return nullptr;
  }

  RenderBox *RenderBox::LastInFlowChildBox() const noexcept
  {
    return DynamicDowncast<RenderBox>(LastInFlowChild());
  }

  RenderBox *RenderBox::PreviousSiblingBox() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(PreviousSibling()))
    {
      return box;
    }

    krys_debug_assert(!PreviousSibling());
    return nullptr;
  }

  RenderBox *RenderBox::PreviousInFlowSiblingBox() const noexcept
  {
    for (auto *curr = PreviousSiblingBox(); curr; curr = curr->PreviousSiblingBox())
    {
      if (!curr->IsFloatingOrOutOfFlowPositioned())
      {
        return curr;
      }
    }
    return nullptr;
  }

  RenderBox *RenderBox::NextSiblingBox() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(NextSibling()))
    {
      return box;
    }

    krys_debug_assert(!NextSibling());
    return nullptr;
  }

  RenderBox *RenderBox::NextInFlowSiblingBox() const noexcept
  {
    for (auto *curr = NextSiblingBox(); curr; curr = curr->NextSiblingBox())
    {
      if (!curr->IsFloatingOrOutOfFlowPositioned())
      {
        return curr;
      }
    }
    return nullptr;
  }

  bool RenderBox::IsColumnSpanner() const noexcept
  {
    return Style().GetColumnSpan() == ColumnSpan::All;
  }

#pragma endregion

#pragma region Margins

  LayoutUnit RenderBox::MarginLogicalLeft() const noexcept
  {
    return MarginLogicalLeft(Style().GetWritingMode());
  }

  LayoutUnit RenderBox::MarginLogicalRight() const noexcept
  {
    return MarginLogicalRight(Style().GetWritingMode());
  }

  LayoutUnit RenderBox::MarginBoxLogicalHeight(WritingMode writingMode) const noexcept
  {
    return writingMode.IsHorizontal() ? MarginBoxRect().Height() : MarginBoxRect().Width();
  }

  void RenderBox::SetMarginBefore(LayoutUnit value) noexcept
  {
    SetMarginBefore(value, Style().GetWritingMode());
  }

  void RenderBox::SetMarginAfter(LayoutUnit value) noexcept
  {
    SetMarginAfter(value, Style().GetWritingMode());
  }

  void RenderBox::SetMarginStart(LayoutUnit value) noexcept
  {
    SetMarginStart(value, Style().GetWritingMode());
  }

  void RenderBox::SetMarginEnd(LayoutUnit value) noexcept
  {
    SetMarginEnd(value, Style().GetWritingMode());
  }

  bool RenderBox::ShouldTrimChildMargin(style::MarginTrimSide type, const RenderBox &child) const noexcept
  {
    return Style().GetMarginTrim().Contains(type) && IsChildEligibleForMarginTrim(type, child);
  }

  void RenderBox::MarkMarginAsTrimmed(style::MarginTrimSide newTrimmedMargin) noexcept
  {
    EnsureRareData().TrimmedMargins.Add(newTrimmedMargin);
  }

  void RenderBox::ClearTrimmedMarginsMarkings() noexcept
  {
    krys_debug_assert(HasRareData());
    EnsureRareData().TrimmedMargins = {};
  }

  bool RenderBox::HasTrimmedMargin(Maybe<style::MarginTrimSide> marginTrimType) const noexcept
  {
    if (!IsInFlow())
    {
      return false;
    }
    if (!HasRareData())
    {
      return false;
    }
    return marginTrimType ? GetRareData().TrimmedMargins.Contains(*marginTrimType)
                          : !GetRareData().TrimmedMargins.IsEmpty();
  }

  template <typename Function>
  LayoutUnit RenderBox::ComputeOrTrimInlineMargin(const RenderBlock &containingBlock,
                                                  style::MarginTrimSide marginSide,
                                                  const Function &computeInlineMargin) const noexcept
  {
    if (containingBlock.ShouldTrimChildMargin(marginSide, *this))
    {
      // FIXME(255434): This should be set when the margin is being trimmed
      // within the context of its layout system (block, flex, grid) and should not
      // be done at this level within RenderBox. We should be able to leave the
      // trimming responsibility to each of those contexts and not need to
      // do any of it here (trimming the margin and setting the rare data bit)
      if (IsGridItem()
          && (marginSide == style::MarginTrimSide::InlineStart
              || marginSide == style::MarginTrimSide::InlineEnd))
      {
        const_cast<RenderBox &>(*this).MarkMarginAsTrimmed(marginSide);
      }
      return 0_lu;
    }
    return computeInlineMargin();
  }

  void RenderBox::ComputeInlineDirectionMargins(const RenderBlock &containingBlock, LayoutUnit containerWidth,
                                                Maybe<LayoutUnit> availableSpaceAdjustedWithFloats,
                                                LayoutUnit childWidth, LayoutUnit &marginStart,
                                                LayoutUnit &marginEnd) const noexcept
  {
    const auto &containingBlockStyle = containingBlock.Style();
    auto marginStartLength = Style().MarginStart(containingBlockStyle.GetWritingMode());
    auto marginEndLength = Style().MarginEnd(containingBlockStyle.GetWritingMode());

    const auto zoomFactor = Style().UsedZoomForLength();

    if (IsFloating())
    {
      marginStart = style::EvaluateMinimum<LayoutUnit>(marginStartLength, containerWidth, zoomFactor);
      marginEnd = style::EvaluateMinimum<LayoutUnit>(marginEndLength, containerWidth, zoomFactor);
      return;
    }

    if (IsInline())
    {
      // Inline blocks/tables don't have their margins increased.
      marginStart = ComputeOrTrimInlineMargin(
        containingBlock, style::MarginTrimSide::InlineStart,
        [&] { return style::EvaluateMinimum<LayoutUnit>(marginStartLength, containerWidth, zoomFactor); });
      marginEnd = ComputeOrTrimInlineMargin(
        containingBlock, style::MarginTrimSide::InlineStart,
        [&] { return style::EvaluateMinimum<LayoutUnit>(marginEndLength, containerWidth, zoomFactor); });
      return;
    }

    if (Is<RenderFlexibleBox>(containingBlock))
    {
      // We need to let flexbox handle the margin adjustment - otherwise, flexbox
      // will think we're wider than we actually are and calculate line sizes
      // wrong. See also http://dev.w3.org/csswg/css-flexbox/#auto-margins
      if (marginStartLength.IsAuto())
      {
        marginStartLength = style::MarginEdge::Fixed {0};
      }
      if (marginEndLength.IsAuto())
      {
        marginEndLength = style::MarginEdge::Fixed {0};
      }
    }

    if (auto *grid = DynamicDowncast<RenderGrid>(containingBlock); grid && grid->IsComputingTrackSizes())
    {
      if (marginStartLength.IsAuto())
      {
        marginStartLength = style::MarginEdge::Fixed {0};
      }
      if (marginEndLength.IsAuto())
      {
        marginEndLength = style::MarginEdge::Fixed {0};
      }
    }

    auto handleMarginAuto = [&]
    {
      auto containerWidthForMarginAuto = availableSpaceAdjustedWithFloats.value_or(containerWidth);
      // Case One: The object is being centered in the containing block's available logical width.
      auto marginAutoCenter =
        marginStartLength.IsAuto() && marginEndLength.IsAuto() && childWidth < containerWidthForMarginAuto;
      auto alignModeCenter = containingBlock.Style().GetTextAlign() == style::TextAlign::WebKitCenter
                             && !marginStartLength.IsAuto() && !marginEndLength.IsAuto();
      if (marginAutoCenter || alignModeCenter)
      {
        // Other browsers center the margin box for align=center elements so we match them here.
        marginStart = ComputeOrTrimInlineMargin(
          containingBlock, style::MarginTrimSide::InlineStart,
          [&]
          {
            auto marginStartWidth =
              style::EvaluateMinimum<LayoutUnit>(marginStartLength, containerWidthForMarginAuto, zoomFactor);
            auto marginEndWidth =
              style::EvaluateMinimum<LayoutUnit>(marginEndLength, containerWidthForMarginAuto, zoomFactor);
            auto centeredMarginBoxStart = std::max<LayoutUnit>(
              0_lu, (containerWidthForMarginAuto - childWidth - marginStartWidth - marginEndWidth) / 2);
            return centeredMarginBoxStart + marginStartWidth;
          });
        marginEnd = ComputeOrTrimInlineMargin(
          containingBlock, style::MarginTrimSide::InlineEnd,
          [&]
          {
            auto marginEndWidth =
              style::EvaluateMinimum<LayoutUnit>(marginEndLength, containerWidthForMarginAuto, zoomFactor);
            return containerWidthForMarginAuto - childWidth - marginStart + marginEndWidth;
          });
        return true;
      }

      // Case Two: The object is being pushed to the start of the containing block's available logical width.
      if (marginEndLength.IsAuto() && childWidth < containerWidthForMarginAuto)
      {
        marginStart = style::Evaluate<LayoutUnit>(marginStartLength, containerWidthForMarginAuto, zoomFactor);
        marginEnd = containerWidthForMarginAuto - childWidth - marginStart;
        return true;
      }

      // Case Three: The object is being pushed to the end of the containing block's available logical width.
      auto pushToEndFromTextAlign =
        !marginEndLength.IsAuto()
        && ((!containingBlockStyle.GetWritingMode().IsBidiLTR()
             && containingBlockStyle.GetTextAlign() == style::TextAlign::WebKitLeft)
            || (containingBlockStyle.GetWritingMode().IsBidiLTR()
                && containingBlockStyle.GetTextAlign() == style::TextAlign::WebKitRight));
      if ((marginStartLength.IsAuto() || pushToEndFromTextAlign) && childWidth < containerWidthForMarginAuto)
      {
        marginEnd = ComputeOrTrimInlineMargin(
          containingBlock, style::MarginTrimSide::InlineEnd, [&]
          { return style::Evaluate<LayoutUnit>(marginEndLength, containerWidthForMarginAuto, zoomFactor); });
        marginStart =
          ComputeOrTrimInlineMargin(containingBlock, style::MarginTrimSide::InlineStart,
                                    [&] { return containerWidthForMarginAuto - childWidth - marginEnd; });
        return true;
      }
      return false;
    };
    if (handleMarginAuto())
    {
      return;
    }

    // Case Four: Either no auto margins, or our width is >= the container width (css2.1, 10.3.3). In that
    // case auto margins will just turn into 0.
    marginStart = ComputeOrTrimInlineMargin(
      containingBlock, style::MarginTrimSide::InlineStart,
      [&] { return style::EvaluateMinimum<LayoutUnit>(marginStartLength, containerWidth, zoomFactor); });
    marginEnd = ComputeOrTrimInlineMargin(
      containingBlock, style::MarginTrimSide::InlineEnd,
      [&] { return style::EvaluateMinimum<LayoutUnit>(marginEndLength, containerWidth, zoomFactor); });
  }

  void RenderBox::ComputeBlockDirectionMargins(const RenderBlock &containingBlock, LayoutUnit &marginBefore,
                                               LayoutUnit &marginAfter) const noexcept
  {
    // First assert that we're not calling this method on box types that don't support margins.
    krys_debug_assert(!IsRenderTableCell());
    krys_debug_assert(!IsRenderTableRow());
    krys_debug_assert(!IsRenderTableSection());
    krys_debug_assert(!IsRenderTableCol());

    // Margins are calculated with respect to the logical width of
    // the containing block (8.3)
    LayoutUnit cw = ContainingBlockLogicalWidthForContent();
    marginBefore =
      ConstrainBlockMarginInAvailableSpaceOrTrim(containingBlock, cw, style::MarginTrimSide::BlockStart);
    marginAfter =
      ConstrainBlockMarginInAvailableSpaceOrTrim(containingBlock, cw, style::MarginTrimSide::BlockEnd);
  }

  void RenderBox::ComputeAndSetBlockDirectionMargins(const RenderBlock &containingBlock) noexcept
  {
    LayoutUnit marginBefore;
    LayoutUnit marginAfter;
    ComputeBlockDirectionMargins(containingBlock, marginBefore, marginAfter);
    containingBlock.SetMarginBeforeForChild(*this, marginBefore);
    containingBlock.SetMarginAfterForChild(*this, marginAfter);
  }

  LayoutUnit
    RenderBox::ConstrainBlockMarginInAvailableSpaceOrTrim(const RenderBox &containingBlock,
                                                          LayoutUnit availableSpace,
                                                          style::MarginTrimSide marginSide) const noexcept
  {
    krys_debug_assert(marginSide == style::MarginTrimSide::BlockStart
                      || marginSide == style::MarginTrimSide::BlockEnd);
    if (containingBlock.ShouldTrimChildMargin(marginSide, *this))
    {
      // FIXME(255434): This should be set when the margin is being trimmed
      // within the context of its layout system (block, flex, grid) and should not
      // be done at this level within RenderBox. We should be able to leave the
      // trimming responsibility to each of those contexts and not need to
      // do any of it here (trimming the margin and setting the rare data bit)
      if (IsGridItem())
      {
        const_cast<RenderBox &>(*this).MarkMarginAsTrimmed(marginSide);
      }
      return 0_lu;
    }

    return marginSide == style::MarginTrimSide::BlockStart
             ? style::EvaluateMinimum<LayoutUnit>(
                 Style().MarginBefore(containingBlock.Style().GetWritingMode()), availableSpace,
                 Style().UsedZoomForLength())
             : style::EvaluateMinimum<LayoutUnit>(
                 Style().MarginAfter(containingBlock.Style().GetWritingMode()), availableSpace,
                 Style().UsedZoomForLength());
  }

#pragma endregion

#pragma region Layout and sizing

  void RenderBox::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    RenderObject *child = FirstChild();
    if (!child)
    {
      ClearNeedsLayout();
      return;
    }

    // FIXME: The layout state, once there is one.
    while (child)
    {
      if (child->NeedsLayout())
      {
        Downcast<RenderElement>(*child).Layout();
      }
      krys_debug_assert(!child->NeedsLayout());
      child = child->NextSibling();
    }
    InvalidateBackgroundObscurationStatus();
    ClearNeedsLayout();
  }

  bool RenderBox::ShouldResetLogicalHeightBeforeLayout() const noexcept
  {
    auto *flexBoxParent = DynamicDowncast<RenderFlexibleBox>(Parent());
    return flexBoxParent && flexBoxParent->ShouldResetFlexItemLogicalHeightBeforeLayout();
  }

  void RenderBox::ResetLogicalHeightBeforeLayoutIfNeeded() noexcept
  {
    if (ShouldResetLogicalHeightBeforeLayout())
    {
      SetLogicalHeight(0_lu);
    }
  }

  LayoutUnit RenderBox::MinPreferredLogicalWidth() const noexcept
  {
    if (NeedsPreferredLogicalWidthsUpdate())
    {
      SetLayoutNeededForbiddenScope layoutForbiddenScope(*this);
      const_cast<RenderBox &>(*this).ComputePreferredLogicalWidths();
    }
    return _minPreferredLogicalWidth;
  }

  LayoutUnit RenderBox::MaxPreferredLogicalWidth() const noexcept
  {
    if (NeedsPreferredLogicalWidthsUpdate())
    {
      SetLayoutNeededForbiddenScope layoutForbiddenScope(*this);
      const_cast<RenderBox &>(*this).ComputePreferredLogicalWidths();
    }
    return _maxPreferredLogicalWidth;
  }

  void RenderBox::SetOverridingBorderBoxLogicalHeight(LayoutUnit height) noexcept
  {
    OverridingLogicalHeightMap()[this] = height;
  }

  void RenderBox::SetOverridingBorderBoxLogicalWidth(LayoutUnit width) noexcept
  {
    OverridingLogicalWidthMap()[this] = width;
  }

  void RenderBox::ClearOverridingBorderBoxLogicalHeight() noexcept
  {
    OverridingLogicalHeightMap().erase(this);
  }

  void RenderBox::ClearOverridingBorderBoxLogicalWidth() noexcept
  {
    OverridingLogicalWidthMap().erase(this);
  }

  void RenderBox::ClearOverridingSize() noexcept
  {
    ClearOverridingBorderBoxLogicalHeight();
    ClearOverridingBorderBoxLogicalWidth();
  }

  Maybe<LayoutUnit> RenderBox::OverridingBorderBoxLogicalWidth() const noexcept
  {
    return Find(OverridingLogicalWidthMap(), this);
  }

  Maybe<LayoutUnit> RenderBox::OverridingBorderBoxLogicalHeight() const noexcept
  {
    return Find(OverridingLogicalHeightMap(), this);
  }

  Maybe<RenderBox::GridAreaSize> RenderBox::GridAreaContentWidth(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return GridAreaContentLogicalWidth();
    }
    return GridAreaContentLogicalHeight();
  }

  Maybe<RenderBox::GridAreaSize> RenderBox::GridAreaContentHeight(WritingMode writingMode) const noexcept
  {
    if (writingMode.IsHorizontal())
    {
      return GridAreaContentLogicalHeight();
    }
    return GridAreaContentLogicalWidth();
  }

  Maybe<RenderBox::GridAreaSize> RenderBox::GridAreaContentLogicalWidth() const noexcept
  {
    return Find(GridAreaContentLogicalWidthMap(), this);
  }

  Maybe<RenderBox::GridAreaSize> RenderBox::GridAreaContentLogicalHeight() const noexcept
  {
    return Find(GridAreaContentLogicalHeightMap(), this);
  }

  void RenderBox::SetGridAreaContentLogicalWidth(GridAreaSize logicalWidth) noexcept
  {
    GridAreaContentLogicalWidthMap()[this] = logicalWidth;
  }

  void RenderBox::SetGridAreaContentLogicalHeight(GridAreaSize logicalHeight) noexcept
  {
    GridAreaContentLogicalHeightMap()[this] = logicalHeight;
  }

  void RenderBox::ClearGridAreaContentSize() noexcept
  {
    GridAreaContentLogicalWidthMap().erase(this);
    ClearGridAreaContentLogicalHeight();
  }

  void RenderBox::ClearGridAreaContentLogicalHeight() noexcept
  {
    GridAreaContentLogicalHeightMap().erase(this);
  }

  Maybe<style::PreferredSize> RenderBox::OverridingLogicalHeightForFlexBasisComputation() const noexcept
  {
    return Find(OverridingLogicalHeightMapForFlexBasisComputation(), this);
  }

  void RenderBox::SetOverridingBorderBoxLogicalHeightForFlexBasisComputation(
    const style::PreferredSize &logicalHeight) noexcept
  {
    OverridingLogicalHeightMapForFlexBasisComputation().insert_or_assign(this, logicalHeight);
  }

  void RenderBox::ClearOverridingLogicalHeightForFlexBasisComputation() noexcept
  {
    OverridingLogicalHeightMapForFlexBasisComputation().erase(this);
  }

  Maybe<style::PreferredSize> RenderBox::OverridingLogicalWidthForFlexBasisComputation() const noexcept
  {
    return Find(OverridingLogicalWidthMapForFlexBasisComputation(), this);
  }

  void RenderBox::SetOverridingBorderBoxLogicalWidthForFlexBasisComputation(
    const style::PreferredSize &logicalWidth) noexcept
  {
    OverridingLogicalWidthMapForFlexBasisComputation().insert_or_assign(this, logicalWidth);
  }

  void RenderBox::ClearOverridingLogicalWidthForFlexBasisComputation() noexcept
  {
    OverridingLogicalWidthMapForFlexBasisComputation().erase(this);
  }

  LayoutUnit
    RenderBox::AdjustBorderBoxLogicalWidthForBoxSizing(LayoutUnit computedLogicalWidth) const noexcept
  {
    auto bordersAndPadding = BorderAndPaddingLogicalWidth();
    if (Style().GetBoxSizing() == BoxSizing::ContentBox)
    {
      return computedLogicalWidth + bordersAndPadding;
    }
    return std::max(computedLogicalWidth, bordersAndPadding);
  }

  LayoutUnit RenderBox::AdjustBorderBoxLogicalHeightForBoxSizing(LayoutUnit height) const noexcept
  {
    LayoutUnit bordersPlusPadding = BorderAndPaddingLogicalHeight();
    if (Style().GetBoxSizing() == BoxSizing::ContentBox)
    {
      return height + bordersPlusPadding;
    }
    return std::max(height, bordersPlusPadding);
  }

  LayoutUnit
    RenderBox::AdjustContentBoxLogicalWidthForBoxSizing(LayoutUnit computedLogicalWidth) const noexcept
  {
    if (Style().GetBoxSizing() == BoxSizing::ContentBox)
    {
      return std::max(0_lu, computedLogicalWidth);
    }
    return std::max(0_lu, computedLogicalWidth - BorderAndPaddingLogicalWidth());
  }

  LayoutUnit RenderBox::AdjustContentBoxLogicalHeightForBoxSizing(Maybe<LayoutUnit> height) const noexcept
  {
    if (!height)
    {
      return 0_lu;
    }
    LayoutUnit result = *height;
    if (Style().GetBoxSizing() == BoxSizing::BorderBox)
    {
      result -= BorderAndPaddingLogicalHeight();
    }
    return std::max(0_lu, result);
  }

  LayoutUnit RenderBox::AdjustIntrinsicLogicalHeightForBoxSizing(LayoutUnit height) const noexcept
  {
    if (Style().GetBoxSizing() == BoxSizing::BorderBox)
    {
      return height + BorderAndPaddingLogicalHeight();
    }
    return height;
  }

  void RenderBox::ConstrainLogicalMinMaxSizesByAspectRatio(LayoutUnit &computedMinSize,
                                                           LayoutUnit &computedMaxSize,
                                                           LayoutUnit computedSize,
                                                           MinimumSizeIsAutomaticContentBased minimumSizeType,
                                                           ConstrainDimension dimension) const noexcept
  {
    // TODO: Here we use isSpecified() to present the definite value. This is not quite correct, for the
    // definite value should also include a size of the initial containing block and the “stretch-fit” sizing
    // of non-replaced blocks if they have definite values. See https://www.w3.org/TR/css-sizing-3/#definite
    const RenderStyle &styleToUse = Style();
    krys_debug_assert(styleToUse.HasAspectRatio() || IsRenderReplacedWithIntrinsicRatio());
    auto logicalSize =
      dimension == ConstrainDimension::Width ? styleToUse.LogicalWidth() : styleToUse.LogicalHeight();
    // https://www.w3.org/TR/css-sizing-4/#aspect-ratio-minimum
    if (minimumSizeType == MinimumSizeIsAutomaticContentBased::Yes)
    {
      // Only use Automatic Content-based Minimum Sizes in the ratio-dependent axis.
      if (logicalSize.IsSpecified())
      {
        computedMinSize = std::min(computedMinSize, computedSize);
      }
      computedMinSize = std::min(computedMinSize, computedMaxSize);
    }

    if (logicalSize.IsSpecified())
    {
      return;
    }

    // Sizing constraints in either axis (the origin axis) should be transferred through the preferred aspect
    // ratio. See https://www.w3.org/TR/css-sizing-4/#aspect-ratio-size-transfers
    bool shouldCheckTransferredMinSize = dimension == ConstrainDimension::Width
                                           ? !styleToUse.LogicalMinWidth().IsSpecified()
                                           : !styleToUse.LogicalMinHeight().IsSpecified();
    bool shouldCheckTransferredMaxSize = dimension == ConstrainDimension::Width
                                           ? !styleToUse.LogicalMaxWidth().IsSpecified()
                                           : !styleToUse.LogicalMaxHeight().IsSpecified();
    if (!shouldCheckTransferredMaxSize && !shouldCheckTransferredMinSize)
    {
      return;
    }

    auto [transferredLogicalMinSize, transferredLogicalMaxSize] =
      dimension == ConstrainDimension::Width ? ComputeMinMaxLogicalWidthFromAspectRatio()
                                             : ComputeMinMaxLogicalHeightFromAspectRatio();
    if (shouldCheckTransferredMaxSize && transferredLogicalMaxSize != LayoutUnit::Max())
    {
      // The transferred max size should be floored by the definite minimum size.
      if (!shouldCheckTransferredMinSize && minimumSizeType == MinimumSizeIsAutomaticContentBased::No)
      {
        transferredLogicalMaxSize = std::max(transferredLogicalMaxSize, computedMinSize);
      }
      computedMaxSize = std::min(computedMaxSize, transferredLogicalMaxSize);
      if (minimumSizeType == MinimumSizeIsAutomaticContentBased::Yes)
      {
        computedMinSize = std::min(computedMinSize, computedMaxSize);
      }
    }

    if (shouldCheckTransferredMinSize && transferredLogicalMinSize > LayoutUnit())
    {
      // The transferred min size should be capped by the definite maximum size.
      if (!shouldCheckTransferredMaxSize)
      {
        transferredLogicalMinSize = std::min(transferredLogicalMinSize, computedMaxSize);
      }
      computedMinSize = std::max(computedMinSize, transferredLogicalMinSize);
    }
  }

  LayoutUnit RenderBox::ConstrainLogicalWidthByMinMax(LayoutUnit logicalWidth, LayoutUnit availableWidth,
                                                      const RenderBlock &cb,
                                                      AllowIntrinsic allowIntrinsic) const noexcept
  {
    const auto &styleToUse = Style();
    auto computedMaxWidth = LayoutUnit::Max();
    if (!styleToUse.LogicalMaxWidth().IsNone()
        && (allowIntrinsic == AllowIntrinsic::Yes || !styleToUse.LogicalMaxWidth().IsIntrinsic()))
    {
      computedMaxWidth = ComputeLogicalWidthUsing(styleToUse.LogicalMaxWidth(), availableWidth, cb);
    }

    if (allowIntrinsic == AllowIntrinsic::No && styleToUse.LogicalMinWidth().IsIntrinsic())
    {
      return std::min(logicalWidth, computedMaxWidth);
    }

    auto logicalMinWidth = styleToUse.LogicalMinWidth();
    auto minimumSizeType = MinimumSizeIsAutomaticContentBased::No;
    LayoutUnit computedMinWidth;
    if (logicalMinWidth.IsAuto() && ShouldComputeLogicalWidthFromAspectRatio()
        && (styleToUse.LogicalWidth().IsAuto() || styleToUse.LogicalWidth().IsMinContent()
            || styleToUse.LogicalWidth().IsMaxContent())
        && !Is<RenderReplaced>(*this) && EffectiveOverflowInlineDirection() == Overflow::Visible)
    {
      // The automatic minimum size in the ratio-dependent axis is  its min-content size. See
      // https://www.w3.org/TR/css-sizing-4/#aspect-ratio-minimum
      logicalMinWidth = css::keywords::MinContent {};
      minimumSizeType = MinimumSizeIsAutomaticContentBased::Yes;
    }
    computedMinWidth = ComputeLogicalWidthUsing(logicalMinWidth, availableWidth, cb);

    if (styleToUse.HasAspectRatio())
    {
      ConstrainLogicalMinMaxSizesByAspectRatio(computedMinWidth, computedMaxWidth, logicalWidth,
                                               minimumSizeType, ConstrainDimension::Width);
    }

    logicalWidth = std::min(logicalWidth, computedMaxWidth);
    return std::max(logicalWidth, computedMinWidth);
  }

  LayoutUnit
    RenderBox::ConstrainLogicalHeightByMinMax(LayoutUnit logicalHeight,
                                              Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    const auto &styleToUse = Style();
    Maybe<LayoutUnit> computedLogicalMaxHeight;
    if (!styleToUse.LogicalMaxHeight().IsNone())
    {
      computedLogicalMaxHeight =
        ComputeLogicalHeightUsing(styleToUse.LogicalMaxHeight(), intrinsicContentHeight);
    }

    auto logicalMinHeight = styleToUse.LogicalMinHeight();
    auto minimumSizeType = MinimumSizeIsAutomaticContentBased::No;
    if (logicalMinHeight.IsAuto() && ShouldComputeLogicalHeightFromAspectRatio() && intrinsicContentHeight
        && !Is<RenderReplaced>(*this) && EffectiveOverflowBlockDirection() == Overflow::Visible)
    {
      auto heightFromAspectRatio =
        BlockSizeFromAspectRatio(BorderAndPaddingLogicalWidth(), BorderAndPaddingLogicalHeight(),
                                 Style().LogicalAspectRatio(), Style().BoxSizingForAspectRatio(),
                                 LogicalWidth(), Style().GetAspectRatio(), IsRenderReplaced())
        - BorderAndPaddingLogicalHeight();
      if (FirstChild())
      {
        heightFromAspectRatio = std::max(heightFromAspectRatio, *intrinsicContentHeight);
      }
      logicalMinHeight = style::MinimumSize::Fixed {heightFromAspectRatio.ToFloat()};
      minimumSizeType = MinimumSizeIsAutomaticContentBased::Yes;
    }
    if (logicalMinHeight.IsMinContent() || logicalMinHeight.IsMaxContent())
    {
      logicalMinHeight = css::keywords::Auto {};
    }
    auto computedLogicalMinHeight = ComputeLogicalHeightUsing(logicalMinHeight, intrinsicContentHeight);
    auto maxHeight = computedLogicalMaxHeight.value_or(LayoutUnit::Max());
    auto minHeight = computedLogicalMinHeight.value_or(LayoutUnit());
    if (styleToUse.HasAspectRatio())
    {
      ConstrainLogicalMinMaxSizesByAspectRatio(minHeight, maxHeight, logicalHeight, minimumSizeType,
                                               ConstrainDimension::Height);
    }
    logicalHeight = std::min(logicalHeight, maxHeight);
    return std::max(logicalHeight, minHeight);
  }

  LayoutUnit RenderBox::ConstrainContentBoxLogicalHeightByMinMax(
    LayoutUnit logicalHeight, Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    // If the min/max height and logical height are both percentages we take advantage of already knowing the
    // current resolved percentage height to avoid recursing up through our containing blocks again to
    // determine it.
    const auto &styleToUse = Style();

    auto percentageLogicalHeight = styleToUse.LogicalHeight().TryPercentage();

    const auto &logicalMaxHeight = styleToUse.LogicalMaxHeight();
    if (!logicalMaxHeight.IsNone())
    {
      if (auto percentageLogicalMaxHeight = logicalMaxHeight.TryPercentage();
          percentageLogicalMaxHeight && percentageLogicalHeight)
      {
        auto availableLogicalHeight = logicalHeight / percentageLogicalHeight->Value * 100;
        logicalHeight =
          std::min(logicalHeight, style::Evaluate<LayoutUnit>(*percentageLogicalMaxHeight,
                                                              LayoutUnit(availableLogicalHeight)));
      }
      else
      {
        if (auto computedContentLogicalMaxHeight =
              ComputeContentLogicalHeight(logicalMaxHeight, intrinsicContentHeight))
        {
          logicalHeight = std::min(logicalHeight, *computedContentLogicalMaxHeight);
        }
      }
    }

    const auto &logicalMinHeight = styleToUse.LogicalMinHeight();
    if (auto percentageLogicalMinHeight = logicalMinHeight.TryPercentage();
        percentageLogicalMinHeight && percentageLogicalHeight)
    {
      auto availableLogicalHeight = logicalHeight / percentageLogicalHeight->Value * 100;
      logicalHeight =
        std::max(logicalHeight, style::Evaluate<LayoutUnit>(*percentageLogicalMinHeight,
                                                            LayoutUnit(availableLogicalHeight)));
    }
    else
    {
      if (auto computedContentLogicalMinHeight =
            ComputeContentLogicalHeight(logicalMinHeight, intrinsicContentHeight))
      {
        logicalHeight = std::max(logicalHeight, *computedContentLogicalMinHeight);
      }
    }

    return logicalHeight;
  }

  LayoutUnit RenderBox::ShrinkLogicalWidthToAvoidFloats(LayoutUnit childMarginStart,
                                                        LayoutUnit childMarginEnd,
                                                        const RenderBlock &containingBlock) const noexcept
  {
    LayoutUnit logicalTopPosition = LogicalTop();
    LayoutUnit logicalHeight = containingBlock.LogicalHeightForChild(*this);
    LayoutUnit result = containingBlock.AvailableLogicalWidthForLine(logicalTopPosition, logicalHeight)
                        - childMarginStart - childMarginEnd;

    // We need to see if margins on either the start side or the end side can contain the floats in question.
    // If they can, then just using the line width is inaccurate. In the case where a float completely fits,
    // we don't need to use the line offset at all, but can instead push all the way to the content edge of
    // the containing block. In the case where the float doesn't fit, we can use the line offset, but we need
    // to grow it by the margin to reflect the fact that the margin was "consumed" by the float. Negative
    // margins aren't consumed by the float, and so we ignore them.
    if (childMarginStart > 0)
    {
      LayoutUnit startContentSide = containingBlock.StartOffsetForContent();
      LayoutUnit startContentSideWithMargin = startContentSide + childMarginStart;
      LayoutUnit startOffset = containingBlock.StartOffsetForLine(logicalTopPosition, logicalHeight);
      if (startOffset > startContentSideWithMargin)
      {
        result += childMarginStart;
      }
      else
      {
        result += startOffset - startContentSide;
      }
    }

    if (childMarginEnd > 0)
    {
      LayoutUnit endContentSide = containingBlock.EndOffsetForContent();
      LayoutUnit endContentSideWithMargin = endContentSide + childMarginEnd;
      LayoutUnit endOffset = containingBlock.EndOffsetForLine(logicalTopPosition, logicalHeight);
      if (endOffset > endContentSideWithMargin)
      {
        result += childMarginEnd;
      }
      else
      {
        result += endOffset - endContentSide;
      }
    }

    return result;
  }

  LayoutUnit RenderBox::ContainingBlockLogicalWidthForContent() const noexcept
  {
    auto *containingBlock = ContainingBlock();
    if (!containingBlock)
    {
      // Should not be called on detached renderer (e.g. during initial style setting).
      krys_debug_assert(false);
      return {};
    }

    // FIXME: The containing inline size of an out-of-flow box (PositionedLayoutConstraints), once there's
    // out-of-flow layout.

    if (IsGridItem())
    {
      krys_debug_assert(Is<RenderGrid>(containingBlock));
      if (auto gridAreaContentLogicalWidth = GridAreaContentLogicalWidth())
      {
        return gridAreaContentLogicalWidth->value_or(0_lu);
      }
    }

    return containingBlock->ContentBoxLogicalWidth();
  }

  LayoutUnit
    RenderBox::ContainingBlockLogicalHeightForContent(AvailableLogicalHeightType heightType) const noexcept
  {
    if (IsGridItem())
    {
      if (auto gridAreaContentLogicalHeight = GridAreaContentLogicalHeight();
          gridAreaContentLogicalHeight && *gridAreaContentLogicalHeight)
      {
        // FIXME: Containing block for a grid item is the grid area it's located in. We need to return
        // whatever height value we get from gridAreaContentLogicalHeight() here, including std::nullopt.
        return **gridAreaContentLogicalHeight;
      }
    }

    if (auto *containingBlock = ContainingBlock())
    {
      return containingBlock->AvailableLogicalHeight(heightType);
    }

    krys_debug_assert(false);
    return 0_lu;
  }

  LayoutUnit RenderBox::ContainingBlockAvailableLineWidth() const noexcept
  {
    return ContainingBlock()->AvailableLogicalWidthForLine(
      LogicalTop(), AvailableLogicalHeight(AvailableLogicalHeightType::IncludeMarginBorderPadding));
  }

  LayoutUnit RenderBox::PerpendicularContainingBlockLogicalHeight() const noexcept
  {
    if (IsGridItem())
    {
      if (auto gridAreaContentLogicalHeight = GridAreaContentLogicalHeight();
          gridAreaContentLogicalHeight && *gridAreaContentLogicalHeight)
      {
        return **gridAreaContentLogicalHeight;
      }
    }

    auto *containingBlock = ContainingBlock();
    if (auto overridingLogicalHeight = containingBlock->OverridingBorderBoxLogicalHeight())
    {
      return containingBlock->ContentBoxLogicalHeight(*overridingLogicalHeight);
    }

    const auto &containingBlockStyle = containingBlock->Style();
    const auto &logicalHeight = containingBlockStyle.LogicalHeight();

    // FIXME: For now just support fixed heights.  Eventually should support percentage heights as well.
    if (auto fixedLogicalHeight = logicalHeight.TryFixed())
    {
      // Use the content box logical height as specified by the style.
      return containingBlock->AdjustContentBoxLogicalHeightForBoxSizing(
        LayoutUnit {fixedLogicalHeight->ResolveZoom(containingBlockStyle.UsedZoomForLength())});
    }

    auto layoutSize = View().FrameView().GetLayoutSize();
    LayoutUnit fillFallbackExtent = containingBlockStyle.GetWritingMode().IsHorizontal()
                                      ? LayoutUnit(layoutSize.Height())
                                      : LayoutUnit(layoutSize.Width());
    LayoutUnit fillAvailableExtent =
      containingBlock->AvailableLogicalHeight(AvailableLogicalHeightType::ExcludeMarginBorderPadding);
    View().AddPercentHeightDescendant(const_cast<RenderBox &>(*this));
    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=158286 We also need to perform the same
    // percentHeightDescendant treatment to the element which dictates the return value for
    // containingBlock()->availableLogicalHeight() above.
    return std::min(fillAvailableExtent, fillFallbackExtent);
  }

  void RenderBox::UpdateLogicalWidth() noexcept
  {
    LogicalExtentComputedValues computedValues;
    ComputeLogicalWidth(computedValues);

    SetLogicalWidth(computedValues.Extent);
    SetLogicalLeft(computedValues.Position);
    SetMarginStart(computedValues.Margins.Start);
    SetMarginEnd(computedValues.Margins.End);
  }

  static LayoutUnit InlineSizeFromAspectRatio(LayoutUnit borderPaddingInlineSum,
                                              LayoutUnit borderPaddingBlockSum, double aspectRatioValue,
                                              BoxSizing boxSizing, LayoutUnit blockSize,
                                              const style::AspectRatio &aspectRatio,
                                              bool isRenderReplaced) noexcept
  {
    if (boxSizing == BoxSizing::BorderBox && aspectRatio.IsRatio() && !isRenderReplaced)
    {
      return std::max(borderPaddingInlineSum, LayoutUnit(blockSize * aspectRatioValue));
    }

    return LayoutUnit((blockSize - borderPaddingBlockSum) * aspectRatioValue) + borderPaddingInlineSum;
  }

  LayoutUnit RenderBox::BlockSizeFromAspectRatio(LayoutUnit borderPaddingInlineSum,
                                                 LayoutUnit borderPaddingBlockSum, double aspectRatioValue,
                                                 BoxSizing boxSizing, LayoutUnit inlineSize,
                                                 const style::AspectRatio &aspectRatio,
                                                 bool isRenderReplaced) noexcept
  {
    if (boxSizing == BoxSizing::BorderBox && aspectRatio.IsRatio() && !isRenderReplaced)
    {
      return std::max(borderPaddingBlockSum, LayoutUnit(inlineSize / aspectRatioValue));
    }
    return LayoutUnit((inlineSize - borderPaddingInlineSum) / aspectRatioValue) + borderPaddingBlockSum;
  }

  static bool ShouldMarginInlineEndContributeToScrollableOverflow(const RenderBox &renderer) noexcept
  {
    auto isSupportedContent = renderer.IsGridItem() || renderer.IsFlexItemIncludingDeprecated()
                              || (renderer.IsInFlow() && renderer.Parent()->IsBlockContainer());
    if (!isSupportedContent)
    {
      return false;
    }

    const auto &parentStyle = renderer.Parent()->Style();
    if (parentStyle.OverflowX() != Overflow::Visible && parentStyle.OverflowX() != Overflow::Clip)
    {
      return true;
    }
    return parentStyle.OverflowY() != Overflow::Visible && parentStyle.OverflowY() != Overflow::Clip;
  }

  void RenderBox::ComputeLogicalWidth(LogicalExtentComputedValues &computedValues) const noexcept
  {
    computedValues.Extent = LogicalWidth();
    computedValues.Position = LogicalLeft();
    computedValues.Margins.Start = MarginStart();
    computedValues.Margins.End = MarginEnd();

    if (IsOutOfFlowPositioned())
    {
      krys_debug_assert(!OverridingBorderBoxLogicalWidth());
      krys_debug_assert(!OverridingLogicalWidthForFlexBasisComputation());
      // FIXME: The logical width of out-of-flow positioned boxes (computeOutOfFlowPositionedLogicalWidth),
      // once there's out-of-flow layout.
      return;
    }

    // The parent box is flexing us, so it has increased or decreased our width. Use the width from the style
    // context.
    // FIXME: Account for block-flow in flexible boxes (webkit.org/b/46418)
    if (auto logicalWidth =
          Parent()->IsFlexibleBoxIncludingDeprecated() ? OverridingBorderBoxLogicalWidth() : null)
    {
      computedValues.Extent = *logicalWidth;
      return;
    }

    // FIXME: Stretching is the only reason why we don't want the box to be treated as a replaced element, so
    // we could perhaps refactor all this logic, not only for flex and grid since alignment is intended to be
    // applied to any block.
    auto treatAsReplaced = [&]
    {
      // FIXME: Account for block-flow in flexible boxes.
      // https://bugs.webkit.org/show_bug.cgi?id=46418
      // The legacy box properties were dropped, so a -webkit-box is always horizontal: replaced children stay
      // replaced.
      auto isReplaced = Is<RenderReplaced>(*this);
      if (!isReplaced)
      {
        return false;
      }
      return !IsGridItem() || !HasStretchedLogicalWidth();
    }();

    auto usedLogicalWidthLength = [&]() -> style::PreferredSize
    {
      if (auto overridingLogicalWidthLength = OverridingLogicalWidthForFlexBasisComputation())
      {
        return *overridingLogicalWidthLength;
      }
      return Style().LogicalWidth();
    }();

    auto containerLogicalWidth = std::max(0_lu, ContainingBlockLogicalWidthForContent());
    const auto &styleToUse = Style();
    if (IsInline() && Is<RenderReplaced>(*this))
    {
      // just calculate margins
      computedValues.Margins.Start = style::EvaluateMinimum<LayoutUnit>(
        styleToUse.MarginStart(), containerLogicalWidth, styleToUse.UsedZoomForLength());
      computedValues.Margins.End = style::EvaluateMinimum<LayoutUnit>(
        styleToUse.MarginEnd(), containerLogicalWidth, styleToUse.UsedZoomForLength());
      if (treatAsReplaced)
      {
        auto evaluatedWidth = Downcast<RenderReplaced>(*this).ComputeReplacedLogicalWidth();
        auto totalWidth = evaluatedWidth + BorderAndPaddingLogicalWidth();
        computedValues.Extent = std::max(totalWidth, MinPreferredLogicalWidth());
      }
      return;
    }

    const auto &containingBlock = *ContainingBlock();
    bool hasPerpendicularContainingBlock =
      containingBlock.IsHorizontalWritingMode() != IsHorizontalWritingMode();

    // Width calculations
    auto logicalWidth = [&]
    {
      if (auto overridingLogicalWidth = OverridingBorderBoxLogicalWidth())
      {
        return *overridingLogicalWidth;
      }
      if (treatAsReplaced)
      {
        return Downcast<RenderReplaced>(*this).ComputeReplacedLogicalWidth() + BorderAndPaddingLogicalWidth();
      }
      if (ShouldComputeLogicalWidthFromAspectRatio() && Style().LogicalWidth().IsAuto())
      {
        return ComputeLogicalWidthFromAspectRatio();
      }

      auto containerWidthInInlineDirection = !hasPerpendicularContainingBlock
                                               ? containerLogicalWidth
                                               : PerpendicularContainingBlockLogicalHeight();
      auto preferredWidth =
        ComputeLogicalWidthUsing(usedLogicalWidthLength, containerWidthInInlineDirection, containingBlock);
      return ConstrainLogicalWidthByMinMax(preferredWidth, containerWidthInInlineDirection, containingBlock);
    };
    computedValues.Extent = logicalWidth();

    // Margin calculations.
    if (hasPerpendicularContainingBlock || IsFloating() || IsInline())
    {
      computedValues.Margins.Start = ComputeOrTrimInlineMargin(
        containingBlock, style::MarginTrimSide::BlockStart,
        [&]
        {
          return style::EvaluateMinimum<LayoutUnit>(styleToUse.MarginStart(), containerLogicalWidth,
                                                    styleToUse.UsedZoomForLength());
        });
      computedValues.Margins.End = ComputeOrTrimInlineMargin(
        containingBlock, style::MarginTrimSide::BlockEnd,
        [&]
        {
          return style::EvaluateMinimum<LayoutUnit>(styleToUse.MarginEnd(), containerLogicalWidth,
                                                    styleToUse.UsedZoomForLength());
        });
    }
    else
    {
      auto containerLogicalWidthForAutoMargins = containerLogicalWidth;
      if (AvoidsFloats() && containingBlock.ContainsFloats())
      {
        containerLogicalWidthForAutoMargins = ContainingBlockAvailableLineWidth();
      }
      bool hasInvertedDirection =
        containingBlock.Style().GetWritingMode().IsInlineOpposing(Style().GetWritingMode());
      ComputeInlineDirectionMargins(
        containingBlock, containerLogicalWidth, containerLogicalWidthForAutoMargins, computedValues.Extent,
        hasInvertedDirection ? computedValues.Margins.End : computedValues.Margins.Start,
        hasInvertedDirection ? computedValues.Margins.Start : computedValues.Margins.End);
    }

    auto shouldIgnoreOverconstrainedMargin = [&]
    {
      if (IsGridItem() || IsFlexItemIncludingDeprecated())
      {
        return true;
      }
      // Is this replaced inline?
      if (IsFloating() || IsInline())
      {
        return true;
      }
      // FIXME: MathML blocks take the size of their content, once there's MathML.
      if (hasPerpendicularContainingBlock)
      {
        return true;
      }

      if (ShouldMarginInlineEndContributeToScrollableOverflow(*this))
      {
        return true;
      }

      return !containerLogicalWidth
             || containerLogicalWidth
                  == (computedValues.Extent + computedValues.Margins.Start + computedValues.Margins.End);
    };
    if (!shouldIgnoreOverconstrainedMargin())
    {
      auto availableSpaceForMargin = containerLogicalWidth - computedValues.Extent;
      bool hasInvertedDirection =
        containingBlock.Style().GetWritingMode().IsInlineOpposing(Style().GetWritingMode());
      if (hasInvertedDirection)
      {
        computedValues.Margins.Start = availableSpaceForMargin - computedValues.Margins.End;
      }
      else
      {
        computedValues.Margins.End = availableSpaceForMargin - computedValues.Margins.Start;
      }
    }
  }

  LayoutUnit RenderBox::FillAvailableMeasure(LayoutUnit availableLogicalWidth) const noexcept
  {
    LayoutUnit marginStart;
    LayoutUnit marginEnd;
    return FillAvailableMeasure(availableLogicalWidth, marginStart, marginEnd);
  }

  LayoutUnit RenderBox::FillAvailableMeasure(LayoutUnit availableLogicalWidth, LayoutUnit &marginStart,
                                             LayoutUnit &marginEnd) const noexcept
  {
    auto *container = ContainingBlock();
    bool isOrthogonalElement = IsHorizontalWritingMode() != container->IsHorizontalWritingMode();
    const auto &marginStartLength = Style().MarginStart();
    const auto &marginEndLength = Style().MarginEnd();
    LayoutUnit availableSizeForResolvingMargin =
      isOrthogonalElement ? ContainingBlockLogicalWidthForContent() : availableLogicalWidth;
    marginStart = ComputeOrTrimInlineMargin(*container, style::MarginTrimSide::InlineStart,
                                            [&]
                                            {
                                              return style::EvaluateMinimum<LayoutUnit>(
                                                marginStartLength, availableSizeForResolvingMargin,
                                                Style().UsedZoomForLength());
                                            });
    marginEnd = ComputeOrTrimInlineMargin(*container, style::MarginTrimSide::InlineEnd,
                                          [&]
                                          {
                                            return style::EvaluateMinimum<LayoutUnit>(
                                              marginEndLength, availableSizeForResolvingMargin,
                                              Style().UsedZoomForLength());
                                          });
    return availableLogicalWidth - marginStart - marginEnd;
  }

  void RenderBox::ComputeIntrinsicKeywordLogicalWidths(IntrinsicKeyword keyword, LayoutUnit borderAndPadding,
                                                       LayoutUnit &minLogicalWidth,
                                                       LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (keyword == IntrinsicKeyword::MinIntrinsic)
    {
      ComputeIntrinsicKeywordLogicalWidths(minLogicalWidth, maxLogicalWidth);
      return;
    }
    if (ShouldComputeLogicalWidthFromAspectRatio())
    {
      minLogicalWidth = maxLogicalWidth = ComputeLogicalWidthFromAspectRatioInternal() - borderAndPadding;
      if (FirstChild())
      {
        LayoutUnit minChildrenLogicalWidth;
        LayoutUnit maxChildrenLogicalWidth;
        ComputeIntrinsicKeywordLogicalWidths(minChildrenLogicalWidth, maxChildrenLogicalWidth);
        minLogicalWidth = std::max(minLogicalWidth, minChildrenLogicalWidth);
        maxLogicalWidth = std::max(maxLogicalWidth, maxChildrenLogicalWidth);
      }
    }
    else
    {
      ComputeIntrinsicKeywordLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }
  }

  LayoutUnit RenderBox::ComputeIntrinsicLogicalWidthUsing(IntrinsicKeyword keyword,
                                                          LayoutUnit availableLogicalWidth,
                                                          LayoutUnit borderAndPadding) const noexcept
  {
    if (keyword == IntrinsicKeyword::WebkitFillAvailable)
    {
      return std::max(borderAndPadding, FillAvailableMeasure(availableLogicalWidth));
    }

    LayoutUnit minLogicalWidth;
    LayoutUnit maxLogicalWidth;
    ComputeIntrinsicKeywordLogicalWidths(keyword, borderAndPadding, minLogicalWidth, maxLogicalWidth);

    switch (keyword)
    {
      case IntrinsicKeyword::MaxContent:   return maxLogicalWidth + borderAndPadding;
      case IntrinsicKeyword::MinContent:
      case IntrinsicKeyword::MinIntrinsic: return minLogicalWidth + borderAndPadding;
      case IntrinsicKeyword::FitContent:
        return std::max(
          minLogicalWidth + borderAndPadding,
          std::min(maxLogicalWidth + borderAndPadding, FillAvailableMeasure(availableLogicalWidth)));
      case IntrinsicKeyword::WebkitFillAvailable: break;
    }
    krys_debug_assert(false);
    return 0_lu;
  }

  template <typename SizeType>
  LayoutUnit RenderBox::ComputeIntrinsicLogicalWidthUsingGeneric(const SizeType &logicalWidth,
                                                                 LayoutUnit availableLogicalWidth,
                                                                 LayoutUnit borderAndPadding) const noexcept
  {
    if (logicalWidth.IsFillAvailable())
    {
      return ComputeIntrinsicLogicalWidthUsing(IntrinsicKeyword::WebkitFillAvailable, availableLogicalWidth,
                                               borderAndPadding);
    }
    if (logicalWidth.IsMinIntrinsic())
    {
      return ComputeIntrinsicLogicalWidthUsing(IntrinsicKeyword::MinContent, availableLogicalWidth,
                                               borderAndPadding);
    }
    if (logicalWidth.IsMaxContent())
    {
      return ComputeIntrinsicLogicalWidthUsing(IntrinsicKeyword::MaxContent, availableLogicalWidth,
                                               borderAndPadding);
    }
    if (logicalWidth.IsMinContent())
    {
      return ComputeIntrinsicLogicalWidthUsing(IntrinsicKeyword::MinContent, availableLogicalWidth,
                                               borderAndPadding);
    }
    if (logicalWidth.IsFitContent())
    {
      return ComputeIntrinsicLogicalWidthUsing(IntrinsicKeyword::FitContent, availableLogicalWidth,
                                               borderAndPadding);
    }

    krys_debug_assert(false);
    return 0_lu;
  }

  LayoutUnit RenderBox::ComputeIntrinsicLogicalWidthUsing(const style::PreferredSize &logicalWidth,
                                                          LayoutUnit availableLogicalWidth,
                                                          LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, borderAndPadding);
  }

  LayoutUnit RenderBox::ComputeIntrinsicLogicalWidthUsing(const style::MinimumSize &logicalWidth,
                                                          LayoutUnit availableLogicalWidth,
                                                          LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, borderAndPadding);
  }

  LayoutUnit RenderBox::ComputeIntrinsicLogicalWidthUsing(const style::MaximumSize &logicalWidth,
                                                          LayoutUnit availableLogicalWidth,
                                                          LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, borderAndPadding);
  }

  LayoutUnit RenderBox::ComputeIntrinsicLogicalWidthUsing(const style::FlexBasis &logicalWidth,
                                                          LayoutUnit availableLogicalWidth,
                                                          LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, borderAndPadding);
  }

  template <typename SizeType>
  LayoutUnit RenderBox::ComputeLogicalWidthUsingGeneric(const SizeType &logicalWidth,
                                                        LayoutUnit availableLogicalWidth,
                                                        const RenderBlock &containingBlock) const noexcept
  {
    if constexpr (std::same_as<SizeType, style::MinimumSize>)
    {
      if (logicalWidth.IsAuto())
      {
        return BorderAndPaddingLogicalWidth();
      }
    }

    if (logicalWidth.IsSpecified())
    {
      // FIXME: If the containing block flow is perpendicular to our direction we need to use the available
      // logical height instead.
      return AdjustBorderBoxLogicalWidthForBoxSizing(
        style::Evaluate<LayoutUnit>(logicalWidth, availableLogicalWidth, Style().UsedZoomForLength()));
    }

    if (logicalWidth.IsIntrinsic() || logicalWidth.IsMinIntrinsic())
    {
      return ComputeIntrinsicLogicalWidthUsing(logicalWidth, availableLogicalWidth,
                                               BorderAndPaddingLogicalWidth());
    }

    LayoutUnit marginStart;
    LayoutUnit marginEnd;
    LayoutUnit logicalWidthResult = FillAvailableMeasure(availableLogicalWidth, marginStart, marginEnd);

    if (ShrinkToAvoidFloats() && containingBlock.ContainsFloats())
    {
      logicalWidthResult = std::min(logicalWidthResult,
                                    ShrinkLogicalWidthToAvoidFloats(marginStart, marginEnd, containingBlock));
    }

    if constexpr (std::same_as<SizeType, style::PreferredSize> || std::same_as<SizeType, style::FlexBasis>)
    {
      if (SizesPreferredLogicalWidthToFitContent())
      {
        return std::max(MinPreferredLogicalWidth(), std::min(MaxPreferredLogicalWidth(), logicalWidthResult));
      }
    }
    return logicalWidthResult;
  }

  LayoutUnit RenderBox::ComputeLogicalWidthUsing(const style::PreferredSize &logicalWidth,
                                                 LayoutUnit availableLogicalWidth,
                                                 const RenderBlock &containingBlock) const noexcept
  {
    return ComputeLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, containingBlock);
  }

  LayoutUnit RenderBox::ComputeLogicalWidthUsing(const style::MinimumSize &logicalWidth,
                                                 LayoutUnit availableLogicalWidth,
                                                 const RenderBlock &containingBlock) const noexcept
  {
    return ComputeLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, containingBlock);
  }

  LayoutUnit RenderBox::ComputeLogicalWidthUsing(const style::MaximumSize &logicalWidth,
                                                 LayoutUnit availableLogicalWidth,
                                                 const RenderBlock &containingBlock) const noexcept
  {
    return ComputeLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, containingBlock);
  }

  LayoutUnit RenderBox::ComputeLogicalWidthUsing(const style::FlexBasis &logicalWidth,
                                                 LayoutUnit availableLogicalWidth,
                                                 const RenderBlock &containingBlock) const noexcept
  {
    return ComputeLogicalWidthUsingGeneric(logicalWidth, availableLogicalWidth, containingBlock);
  }

  bool RenderBox::ColumnFlexItemHasStretchAlignment() const noexcept
  {
    // auto margins mean we don't stretch. Note that this function will only be
    // used for widths, so we don't have to check marginBefore/marginAfter.
    const auto &parentStyle = Parent()->Style();
    krys_debug_assert(parentStyle.IsColumnFlexDirection());
    if (Style().MarginStart().IsAuto() || Style().MarginEnd().IsAuto())
    {
      return false;
    }

    auto normalBehavior = ContainingBlock()->SelfAlignmentNormalBehavior();

    if (!Style().GetAlignSelf().IsAuto())
    {
      return Style().GetAlignSelf().Resolve(StyleSelfAlignmentData {normalBehavior}).Position()
             == ItemPosition::Stretch;
    }
    return parentStyle.GetAlignItems().Resolve(StyleSelfAlignmentData {normalBehavior}).Position()
           == ItemPosition::Stretch;
  }

  bool RenderBox::IsStretchingColumnFlexItem() const noexcept
  {
    // The legacy box properties were dropped, so a -webkit-box is always horizontal and never stretches its
    // children's widths.

    // We don't stretch multiline flexboxes because they need to apply line spacing (align-content) first.
    if (Is<RenderFlexibleBox>(*Parent()) && Parent()->Style().GetFlexWrap() == FlexWrap::NoWrap
        && Parent()->Style().IsColumnFlexDirection() && ColumnFlexItemHasStretchAlignment())
    {
      return true;
    }
    return false;
  }

  // FIXME: Can/Should we move this inside specific layout classes (flex. grid)? Can we refactor
  // columnFlexItemHasStretchAlignment logic?
  bool RenderBox::HasStretchedLogicalHeight() const noexcept
  {
    const auto &style = Style();
    if (!style.LogicalHeight().IsAuto() || style.MarginBefore().IsAuto() || style.MarginAfter().IsAuto())
    {
      return false;
    }
    RenderBlock *containingBlock = ContainingBlock();
    if (!containingBlock)
    {
      // We are evaluating align-self/justify-self, which default to 'normal' for the root element.
      // The 'normal' value behaves like 'start' except for Flexbox Items, which obviously should have a
      // container.
      return false;
    }
    if (containingBlock->IsHorizontalWritingMode() != IsHorizontalWritingMode())
    {
      if (auto *grid = DynamicDowncast<RenderGrid>(*this);
          grid && grid->IsSubgridInParentDirection(style::GridTrackSizingDirection::Columns))
      {
        return true;
      }

      auto normalBehavior = containingBlock->SelfAlignmentNormalBehavior(this);
      if (!style.GetJustifySelf().IsAuto())
      {
        return style.GetJustifySelf().Resolve(StyleSelfAlignmentData {normalBehavior}).Position()
               == ItemPosition::Stretch;
      }
      return containingBlock->Style()
               .GetJustifyItems()
               .Resolve(StyleSelfAlignmentData {normalBehavior})
               .Position()
             == ItemPosition::Stretch;
    }
    if (auto *grid = DynamicDowncast<RenderGrid>(*this);
        grid && grid->IsSubgridInParentDirection(style::GridTrackSizingDirection::Rows))
    {
      return true;
    }

    auto normalBehavior = containingBlock->SelfAlignmentNormalBehavior(this);
    if (!style.GetAlignSelf().IsAuto())
    {
      return style.GetAlignSelf().Resolve(StyleSelfAlignmentData {normalBehavior}).Position()
             == ItemPosition::Stretch;
    }
    return containingBlock->Style()
             .GetAlignItems()
             .Resolve(StyleSelfAlignmentData {normalBehavior})
             .Position()
           == ItemPosition::Stretch;
  }

  // FIXME: Can/Should we move this inside specific layout classes (flex. grid)? Can we refactor
  // columnFlexItemHasStretchAlignment logic?
  bool RenderBox::HasStretchedLogicalWidth(StretchingMode stretchingMode) const noexcept
  {
    const auto &style = Style();
    if (!style.LogicalWidth().IsAuto() || style.MarginStart().IsAuto() || style.MarginEnd().IsAuto())
    {
      return false;
    }
    RenderBlock *containingBlock = ContainingBlock();
    if (!containingBlock)
    {
      // We are evaluating align-self/justify-self, which default to 'normal' for the root element.
      // The 'normal' value behaves like 'start' except for Flexbox Items, which obviously should have a
      // container.
      return false;
    }
    auto normalItemPosition = stretchingMode == StretchingMode::Any
                                ? containingBlock->SelfAlignmentNormalBehavior(this)
                                : ItemPosition::Normal;
    if (containingBlock->IsHorizontalWritingMode() != IsHorizontalWritingMode())
    {
      if (auto *grid = DynamicDowncast<RenderGrid>(*this);
          grid && grid->IsSubgridInParentDirection(style::GridTrackSizingDirection::Rows))
      {
        return true;
      }

      if (!style.GetAlignSelf().IsAuto())
      {
        return style.GetAlignSelf().Resolve(StyleSelfAlignmentData {normalItemPosition}).Position()
               == ItemPosition::Stretch;
      }
      return containingBlock->Style()
               .GetAlignItems()
               .Resolve(StyleSelfAlignmentData {normalItemPosition})
               .Position()
             == ItemPosition::Stretch;
    }
    if (auto *grid = DynamicDowncast<RenderGrid>(*this);
        grid && grid->IsSubgridInParentDirection(style::GridTrackSizingDirection::Columns))
    {
      return true;
    }

    if (!style.GetJustifySelf().IsAuto())
    {
      return style.GetJustifySelf().Resolve(StyleSelfAlignmentData {normalItemPosition}).Position()
             == ItemPosition::Stretch;
    }
    return containingBlock->Style()
             .GetJustifyItems()
             .Resolve(StyleSelfAlignmentData {normalItemPosition})
             .Position()
           == ItemPosition::Stretch;
  }

  bool RenderBox::SizesPreferredLogicalWidthToFitContent() const noexcept
  {
    // Marquees in WinIE are like a mixture of blocks and inline-blocks.  They size as though they're blocks,
    // but they allow text to sit on the same line as the marquee.
    if (IsFloating() || (IsNonReplacedAtomicInlineLevelBox() && !IsHTMLMarquee()))
    {
      return true;
    }

    if (IsGridItem())
    {
      // FIXME: The masonry logic should not be living in RenderBox; it should ideally live in RenderGrid.
      // This is a temporary solution to prevent regressions.
      // FIXME: Orthogonal grid items in masonry columns (GridLayoutFunctions::isOrthogonalGridItem), once
      // there's grid layout.
      auto *renderGrid = Downcast<RenderGrid>(Parent());
      return renderGrid->IsMasonry(style::GridTrackSizingDirection::Columns) || !HasStretchedLogicalWidth();
    }

    // This code may look a bit strange.  Basically width:intrinsic should clamp the size when testing both
    // min-width and width.  max-width is only clamped if it is also intrinsic.
    const auto &logicalWidth = Style().LogicalWidth();
    if (logicalWidth.IsIntrinsicKeyword())
    {
      return true;
    }

    // Children of a horizontal marquee do not fill the container by default.
    // FIXME: Need to deal with MarqueeDirection::Auto value properly. It could be vertical.
    // FIXME: Think about block-flow here.  Need to find out how marquee direction relates to
    // block-flow (as well as how marquee overflow should relate to block flow).
    // https://bugs.webkit.org/show_bug.cgi?id=46472
    // The marquee properties were dropped, so the direction is always `auto`.
    if (Parent()->IsHTMLMarquee())
    {
      return true;
    }

    // FIXME: MathML blocks take the size of their content, not of their container, once there's MathML.

    // Flexible box items should shrink wrap, so we lay them out at their intrinsic widths.
    // In the case of columns that have a stretch alignment, we layout at the stretched size
    // to avoid an extra layout when applying alignment.
    if (Is<RenderFlexibleBox>(*Parent()))
    {
      // For multiline columns, we need to apply align-content first, so we can't stretch now.
      if (!Parent()->Style().IsColumnFlexDirection() || Parent()->Style().GetFlexWrap() != FlexWrap::NoWrap)
      {
        return true;
      }
      if (!ColumnFlexItemHasStretchAlignment())
      {
        return true;
      }
    }

    // Flexible horizontal boxes lay out children at their intrinsic widths.  Also vertical boxes
    // that don't stretch their kids lay out their children at their intrinsic widths.
    // FIXME: Think about block-flow here.
    // https://bugs.webkit.org/show_bug.cgi?id=46473
    // The legacy box properties were dropped, so a -webkit-box is always horizontal.
    if (Parent()->IsRenderDeprecatedFlexibleBox())
    {
      return true;
    }

    // Button, input, select, textarea, and legend treat width value of 'auto' as 'intrinsic' unless it's in a
    // stretching column flexbox.
    // FIXME: Think about block-flow here.
    // https://bugs.webkit.org/show_bug.cgi?id=46473
    if (logicalWidth.IsAuto() && !IsStretchingColumnFlexItem())
    {
      if (auto *element = DynamicDowncast<html::HTMLElement>(GetElement());
          element
          && (Is<html::HTMLInputElement>(*element) || Is<html::HTMLSelectElement>(*element)
              || Is<html::HTMLButtonElement>(*element) || Is<html::HTMLTextAreaElement>(*element)
              || Is<html::HTMLLegendElement>(*element)))
      {
        return true;
      }
    }

    if (IsHorizontalWritingMode() != ContainingBlock()->IsHorizontalWritingMode())
    {
      return true;
    }

    return false;
  }

  void RenderBox::OverrideLogicalHeightForSizeContainment() noexcept
  {
    LayoutUnit intrinsicHeight;
    if (auto height = ExplicitIntrinsicInnerLogicalHeight())
    {
      intrinsicHeight = *height;
    }
    // FIXME: Menu lists keep their theme's height, once there are form controls.

    // We need the exact width of border and padding here, yet we can't use borderAndPadding* interfaces.
    // Because these interfaces evetually call borderAfter/Before, and RenderBlock::borderBefore
    // adds extra border to fieldset by adding intrinsicBorderForFieldset which is not needed here.
    auto borderAndPadding = RenderBox::BorderBefore() + RenderBox::PaddingBefore() + RenderBox::BorderAfter()
                            + RenderBox::PaddingAfter();
    SetLogicalHeight(intrinsicHeight + borderAndPadding + LayoutUnit(ScrollbarLogicalHeight()));
  }

  void RenderBox::UpdateLogicalHeight() noexcept
  {
    if (ShouldApplySizeContainment() && !IsRenderGrid())
    {
      OverrideLogicalHeightForSizeContainment();
    }

    // FIXME: Cache the intrinsic content height of a flex item
    // (cacheIntrinsicContentLogicalHeightForFlexItem), once there's flex layout.
    auto computedValues = ComputeLogicalHeight(LogicalHeight(), LogicalTop());
    SetLogicalHeight(computedValues.Extent);
    SetLogicalTop(computedValues.Position);
    SetMarginBefore(computedValues.Margins.Before);
    SetMarginAfter(computedValues.Margins.After);
  }

  static bool ShouldFlipBeforeAfterMargins(WritingMode containingBlockWritingMode,
                                           WritingMode childWritingMode) noexcept
  {
    krys_debug_assert(containingBlockWritingMode.IsOrthogonal(childWritingMode));
    auto childBlockFlowDirection = childWritingMode.BlockDirection();
    bool shouldFlip = false;
    switch (containingBlockWritingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom:
        shouldFlip = (childBlockFlowDirection == FlowDirection::RightToLeft);
        break;
      case FlowDirection::BottomToTop:
        shouldFlip = (childBlockFlowDirection == FlowDirection::RightToLeft);
        break;
      case FlowDirection::RightToLeft:
        shouldFlip = (childBlockFlowDirection == FlowDirection::BottomToTop);
        break;
      case FlowDirection::LeftToRight:
        shouldFlip = (childBlockFlowDirection == FlowDirection::BottomToTop);
        break;
    }

    if (containingBlockWritingMode.IsInlineFlipped())
    {
      shouldFlip = !shouldFlip;
    }

    return shouldFlip;
  }

  RenderBox::LogicalExtentComputedValues RenderBox::ComputeLogicalHeight(LayoutUnit logicalHeight,
                                                                         LayoutUnit logicalTop) const noexcept
  {
    LogicalExtentComputedValues computedValues;
    computedValues.Extent = logicalHeight;
    computedValues.Position = logicalTop;

    // Inline non-replaced elements do not support a height property.
    if (IsInline() && !IsBlockLevelReplacedOrAtomicInline())
    {
      return computedValues;
    }

    // FIXME: Let a table cell compute its preferred size, and use the logical width of orthogonal cells set
    // by table layout, once there's table layout.

    if (IsOutOfFlowPositioned())
    {
      // FIXME: The logical height of out-of-flow positioned boxes (computeOutOfFlowPositionedLogicalHeight),
      // once there's out-of-flow layout.
      return computedValues;
    }

    const auto &parent = *Parent();

    // Context here refers to the general term and not necessarily a formatting context
    // since, for example, we might compute the logical height from the aspect ratio.
    auto usedLogicalHeightFromContext = [&]() -> Maybe<LayoutUnit>
    {
      if (Is<RenderTable>(*this))
      {
        // Table as flex and grid item is special and needs table like handling.
        auto heightValue = logicalHeight;
        if (ShouldComputeLogicalHeightFromAspectRatio())
        {
          heightValue =
            BlockSizeFromAspectRatio(HorizontalBorderAndPaddingExtent(), VerticalBorderAndPaddingExtent(),
                                     Style().LogicalAspectRatio(), Style().BoxSizingForAspectRatio(),
                                     LogicalWidth(), Style().GetAspectRatio(), Is<RenderReplaced>(*this));
        }
        return heightValue;
      }

      if (Is<RenderFlexibleBox>(parent))
      {
        if (auto overridingLogicalHeight = OverridingLogicalHeightForFlexBasisComputation())
        {
          krys_debug_assert(!OverridingBorderBoxLogicalHeight());
          return null;
        }

        if (auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight())
        {
          return *overridingLogicalHeight;
        }

        if (auto *replaced = DynamicDowncast<RenderReplaced>(*this))
        {
          return replaced->ComputeReplacedLogicalHeight() + BorderAndPaddingLogicalHeight();
        }
        return null;
      }

      if (auto *deprecatedFlexBox = DynamicDowncast<RenderDeprecatedFlexibleBox>(parent))
      {
        if (auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight())
        {
          return *overridingLogicalHeight;
        }

        // The legacy box properties were dropped, so a -webkit-box is horizontal and stretches: its replaced
        // children aren't treated as replaced.

        // Block children of horizontal flexible boxes fill the height of the box.
        if (Style().LogicalHeight().IsAuto() && deprecatedFlexBox->IsStretchingChildren())
        {
          return deprecatedFlexBox->ContentBoxLogicalHeight() - MarginBefore() - MarginAfter();
        }

        return null;
      }
      if (Is<RenderGrid>(parent))
      {
        if (auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight())
        {
          return *overridingLogicalHeight;
        }

        if (auto *replaced = DynamicDowncast<RenderReplaced>(*this))
        {
          return replaced->ComputeReplacedLogicalHeight() + BorderAndPaddingLogicalHeight();
        }

        return null;
      }

      if (auto *replaced = DynamicDowncast<RenderReplaced>(*this))
      {
        return replaced->ComputeReplacedLogicalHeight() + BorderAndPaddingLogicalHeight();
      }

      return null;
    };

    auto computedLogicalHeight = [&]() -> style::PreferredSize
    {
      if (Is<RenderFlexibleBox>(parent))
      {
        if (auto overridingLogicalHeight = OverridingLogicalHeightForFlexBasisComputation())
        {
          krys_debug_assert(!OverridingBorderBoxLogicalHeight());
          return *overridingLogicalHeight;
        }
      }
      return Style().LogicalHeight();
    }();

    auto usedLogicalHeight = [&]
    {
      if (auto heightFromFormattingContext = usedLogicalHeightFromContext())
      {
        return *heightFromFormattingContext;
      }

      // Callers passing LayoutUnit::max() for logicalHeight means an indefinite height, so
      // translate this to a nullopt intrinsic height for further logical height computations.
      auto intrinsicHeight = logicalHeight != LayoutUnit::Max() ? Maybe<LayoutUnit> {logicalHeight} : null;
      if (ShouldComputeLogicalHeightFromAspectRatio())
      {
        if (intrinsicHeight && Style().GetBoxSizing() == BoxSizing::ContentBox)
        {
          *intrinsicHeight -= RenderBox::BorderBefore() + RenderBox::PaddingBefore()
                              + RenderBox::BorderAfter() + RenderBox::PaddingAfter();
        }
        auto heightFromAspectRatio =
          BlockSizeFromAspectRatio(HorizontalBorderAndPaddingExtent(), VerticalBorderAndPaddingExtent(),
                                   Style().LogicalAspectRatio(), Style().BoxSizingForAspectRatio(),
                                   LogicalWidth(), Style().GetAspectRatio(), Is<RenderReplaced>(*this));
        return ConstrainLogicalHeightByMinMax(heightFromAspectRatio, intrinsicHeight);
      }

      if (intrinsicHeight)
      {
        *intrinsicHeight -= BorderAndPaddingLogicalHeight();
      }
      auto mainOrPreferredHeight =
        ComputeLogicalHeightUsing(computedLogicalHeight, intrinsicHeight).value_or(computedValues.Extent);
      return ConstrainLogicalHeightByMinMax(mainOrPreferredHeight, intrinsicHeight);
    };
    computedValues.Extent = usedLogicalHeight();

    auto computeMargins = [&]
    {
      const auto &containingBlock = *ContainingBlock();
      bool hasPerpendicularContainingBlock =
        containingBlock.IsHorizontalWritingMode() != IsHorizontalWritingMode();
      bool shouldFlipBeforeAfter =
        hasPerpendicularContainingBlock
          ? ShouldFlipBeforeAfterMargins(containingBlock.Style().GetWritingMode(), Style().GetWritingMode())
          : containingBlock.Style().GetWritingMode().IsBlockOpposing(Style().GetWritingMode());
      auto marginBefore =
        shouldFlipBeforeAfter ? computedValues.Margins.After : computedValues.Margins.Before;
      auto marginAfter = shouldFlipBeforeAfter ? computedValues.Margins.Before : computedValues.Margins.After;

      if (hasPerpendicularContainingBlock)
      {
        ComputeInlineDirectionMargins(containingBlock, ContainingBlockLogicalWidthForContent(), null,
                                      computedValues.Extent, marginBefore, marginAfter);
      }
      else
      {
        ComputeBlockDirectionMargins(containingBlock, marginBefore, marginAfter);
      }
      computedValues.Margins.Before = shouldFlipBeforeAfter ? marginAfter : marginBefore;
      computedValues.Margins.After = shouldFlipBeforeAfter ? marginBefore : marginAfter;
    };
    computeMargins();

    // WinIE quirk: The <html> block always fills the entire canvas in quirks mode. The <body> always fills
    // the <html> block in quirks mode. Only apply this quirk if the block is normal flow and no height is
    // specified. When we're printing, we also need this quirk if the body or root has a percentage height
    // since we don't set a height in RenderView when we're printing. So without this quirk, the height has
    // nothing to be a percentage of, and it ends up being 0. That is bad.
    // FIXME: The paginated content when printing, once there's printing.
    if (StretchesToViewport())
    {
      auto margins = CollapsedMarginBefore() + CollapsedMarginAfter();
      auto visibleHeight = View().PageOrViewLogicalHeight();
      if (IsDocumentElementRenderer())
      {
        computedValues.Extent = std::max(computedValues.Extent, visibleHeight - margins);
      }
      else if (auto *parentBox = DynamicDowncast<RenderBox>(parent))
      {
        auto marginsBordersPadding = margins + parentBox->MarginBefore() + parentBox->MarginAfter()
                                     + parentBox->BorderAndPaddingLogicalHeight();
        computedValues.Extent = std::max(computedValues.Extent, visibleHeight - marginsBordersPadding);
      }
    }
    return computedValues;
  }

  LayoutUnit RenderBox::ComputeLogicalHeightWithoutLayout() const noexcept
  {
    // FIXME:: We should probably return something other than just
    // border + padding, but for now we have no good way to do anything else
    // without layout, so we just use that.
    auto estimatedHeight = BorderAndPaddingLogicalHeight();
    if (ShouldApplySizeContainment())
    {
      if (auto height = ExplicitIntrinsicInnerLogicalHeight())
      {
        estimatedHeight += *height + LayoutUnit(ScrollbarLogicalHeight());
      }
    }
    LogicalExtentComputedValues computedValues = ComputeLogicalHeight(estimatedHeight, 0_lu);
    return computedValues.Extent;
  }

  template <typename SizeType>
  Maybe<LayoutUnit>
    RenderBox::ComputeLogicalHeightUsingGeneric(const SizeType &logicalHeight,
                                                Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    if (auto *replaced = DynamicDowncast<RenderReplaced>(*this))
    {
      if constexpr (std::same_as<SizeType, style::MinimumSize>)
      {
        if (!replaced->ReplacedMinLogicalHeightComputesAsNone())
        {
          return replaced->ComputeReplacedLogicalHeightUsing(logicalHeight) + BorderAndPaddingLogicalHeight();
        }
      }
      else if constexpr (std::same_as<SizeType, style::MaximumSize>)
      {
        if (!replaced->ReplacedMaxLogicalHeightComputesAsNone())
        {
          return replaced->ComputeReplacedLogicalHeightUsing(logicalHeight) + BorderAndPaddingLogicalHeight();
        }
      }
      return null;
    }
    if (auto computedContentAndScrollbarLogicalHeight =
          ComputeContentAndScrollbarLogicalHeightUsing(logicalHeight, intrinsicContentHeight))
    {
      return AdjustBorderBoxLogicalHeightForBoxSizing(*computedContentAndScrollbarLogicalHeight);
    }
    return null;
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeLogicalHeightUsing(const style::PreferredSize &logicalHeight,
                                         Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    return ComputeLogicalHeightUsingGeneric(logicalHeight, intrinsicContentHeight);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeLogicalHeightUsing(const style::MinimumSize &logicalHeight,
                                         Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    return ComputeLogicalHeightUsingGeneric(logicalHeight, intrinsicContentHeight);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeLogicalHeightUsing(const style::MaximumSize &logicalHeight,
                                         Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    return ComputeLogicalHeightUsingGeneric(logicalHeight, intrinsicContentHeight);
  }

  template <typename SizeType>
  Maybe<LayoutUnit>
    RenderBox::ComputeContentLogicalHeightGeneric(const SizeType &logicalHeight,
                                                  Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    if (auto computedContentAndScrollbarLogicalHeight =
          ComputeContentAndScrollbarLogicalHeightUsing(logicalHeight, intrinsicContentHeight))
    {
      return std::max<LayoutUnit>(
        0_lu, AdjustContentBoxLogicalHeightForBoxSizing(*computedContentAndScrollbarLogicalHeight)
                - LayoutUnit(ScrollbarLogicalHeight()));
    }
    return null;
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeContentLogicalHeight(const style::PreferredSize &logicalHeight,
                                           Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    return ComputeContentLogicalHeightGeneric(logicalHeight, intrinsicContentHeight);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeContentLogicalHeight(const style::MinimumSize &logicalHeight,
                                           Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    return ComputeContentLogicalHeightGeneric(logicalHeight, intrinsicContentHeight);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeContentLogicalHeight(const style::MaximumSize &logicalHeight,
                                           Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    return ComputeContentLogicalHeightGeneric(logicalHeight, intrinsicContentHeight);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeContentLogicalHeight(const style::FlexBasis &logicalHeight,
                                           Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    return ComputeContentLogicalHeightGeneric(logicalHeight, intrinsicContentHeight);
  }

  static bool IsOrthogonal(const RenderBox &renderer, const RenderElement &ancestor) noexcept
  {
    return renderer.IsHorizontalWritingMode() != ancestor.IsHorizontalWritingMode();
  }

  template <typename SizeType>
  Maybe<LayoutUnit>
    RenderBox::ComputeIntrinsicLogicalContentHeightUsingGeneric(const SizeType &logicalHeight,
                                                                Maybe<LayoutUnit> intrinsicContentHeight,
                                                                LayoutUnit borderAndPadding) const noexcept
  {
    auto intrinsic = [&]() -> Maybe<LayoutUnit>
    {
      if (intrinsicContentHeight)
      {
        return AdjustIntrinsicLogicalHeightForBoxSizing(*intrinsicContentHeight);
      }
      return null;
    };

    auto minMaxContent = [&]() -> Maybe<LayoutUnit>
    {
      // FIXME: The CSS sizing spec is considering changing what min-content/max-content should resolve to.
      // If that happens, this code will have to change.
      // FIXME: Images with a fixed logical width and no aspect ratio use their intrinsic ratio, once there
      // are images.
      return intrinsic();
    };

    auto fillAvailable = [&]() -> Maybe<LayoutUnit>
    {
      return ContainingBlock()->AvailableLogicalHeight(AvailableLogicalHeightType::ExcludeMarginBorderPadding)
             - borderAndPadding;
    };

    if (logicalHeight.IsMinContent() || logicalHeight.IsMaxContent())
    {
      return minMaxContent();
    }
    if (logicalHeight.IsIntrinsic() || logicalHeight.IsMinIntrinsic() || logicalHeight.IsFitContent())
    {
      return intrinsic();
    }
    if (logicalHeight.IsFillAvailable())
    {
      return fillAvailable();
    }
    krys_debug_assert(false);
    return 0_lu;
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeIntrinsicLogicalContentHeightUsing(const style::PreferredSize &logicalHeight,
                                                         Maybe<LayoutUnit> intrinsicContentHeight,
                                                         LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalContentHeightUsingGeneric(logicalHeight, intrinsicContentHeight,
                                                            borderAndPadding);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeIntrinsicLogicalContentHeightUsing(const style::MinimumSize &logicalHeight,
                                                         Maybe<LayoutUnit> intrinsicContentHeight,
                                                         LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalContentHeightUsingGeneric(logicalHeight, intrinsicContentHeight,
                                                            borderAndPadding);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeIntrinsicLogicalContentHeightUsing(const style::MaximumSize &logicalHeight,
                                                         Maybe<LayoutUnit> intrinsicContentHeight,
                                                         LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalContentHeightUsingGeneric(logicalHeight, intrinsicContentHeight,
                                                            borderAndPadding);
  }

  Maybe<LayoutUnit>
    RenderBox::ComputeIntrinsicLogicalContentHeightUsing(const style::FlexBasis &logicalHeight,
                                                         Maybe<LayoutUnit> intrinsicContentHeight,
                                                         LayoutUnit borderAndPadding) const noexcept
  {
    return ComputeIntrinsicLogicalContentHeightUsingGeneric(logicalHeight, intrinsicContentHeight,
                                                            borderAndPadding);
  }

  template <typename SizeType>
  Maybe<LayoutUnit> RenderBox::ComputeContentAndScrollbarLogicalHeightUsing(
    const SizeType &logicalHeight, Maybe<LayoutUnit> intrinsicContentHeight) const noexcept
  {
    auto intrinsic = [&]
    {
      // FIXME: The CSS sizing spec is considering changing what min-content/max-content should resolve to.
      // If that happens, this code will have to change.
      return ComputeIntrinsicLogicalContentHeightUsing(logicalHeight, intrinsicContentHeight,
                                                       BorderAndPaddingLogicalHeight());
    };

    if (auto fixedLogicalHeight = logicalHeight.TryFixed())
    {
      return LayoutUnit {fixedLogicalHeight->ResolveZoom(Style().UsedZoomForLength())};
    }
    if (logicalHeight.IsPercentOrCalculated())
    {
      return ComputePercentageLogicalHeight(logicalHeight);
    }
    if (logicalHeight.IsMinContent() || logicalHeight.IsMaxContent() || logicalHeight.IsFitContent()
        || logicalHeight.IsFillAvailable() || logicalHeight.IsIntrinsic() || logicalHeight.IsMinIntrinsic())
    {
      return intrinsic();
    }
    if constexpr (std::same_as<SizeType, style::MinimumSize>)
    {
      if (logicalHeight.IsAuto())
      {
        if (intrinsicContentHeight && IsFlexItem()
            && Downcast<RenderFlexibleBox>(Parent())->ShouldApplyMinBlockSizeAutoForFlexItem(*this))
        {
          return AdjustIntrinsicLogicalHeightForBoxSizing(*intrinsicContentHeight);
        }
        return LayoutUnit {0};
      }
    }
    return null;
  }

  bool
    RenderBox::SkipContainingBlockForPercentHeightCalculation(const RenderBox &containingBlock,
                                                              bool isPerpendicularWritingMode) const noexcept
  {
    // Flow threads for multicol or paged overflow should be skipped. They are invisible to the DOM,
    // and percent heights of children should be resolved against the multicol or paged container.
    if (containingBlock.IsRenderFragmentedFlow() && !isPerpendicularWritingMode)
    {
      return true;
    }

    // Render view is not considered auto height.
    if (Is<RenderView>(containingBlock))
    {
      return false;
    }

    // If the writing mode of the containing block is orthogonal to ours, it means
    // that we shouldn't skip anything, since we're going to resolve the
    // percentage height against a containing block *width*.
    if (isPerpendicularWritingMode)
    {
      return false;
    }

    // Anonymous blocks should not impede percentage resolution on a child.
    // Examples of such anonymous blocks are blocks wrapped around inlines that
    // have block siblings (from the CSS spec) and multicol flow threads (an
    // implementation detail). Another implementation detail, ruby runs, create
    // anonymous inline-blocks, so skip those too. All other types of anonymous
    // objects, such as table-cells and flexboxes, will be treated as if they were
    // non-anonymous.
    if (containingBlock.IsAnonymousForPercentageResolution())
    {
      return containingBlock.Style().Display() == DisplayType::Block
             || containingBlock.Style().Display() == DisplayType::InlineBlock;
    }

    // For quirks mode, we skip most auto-height containing blocks when computing
    // percentages.
    return GetDocument().InQuirksMode() && !containingBlock.IsRenderTableCell()
           && !containingBlock.IsOutOfFlowPositioned() && !containingBlock.IsRenderGrid()
           && !containingBlock.IsFlexibleBoxIncludingDeprecated()
           && containingBlock.Style().LogicalHeight().IsAuto();
  }

  template <typename SizeType>
  Maybe<LayoutUnit> RenderBox::ComputePercentageLogicalHeightGeneric(
    const SizeType &logicalHeight, UpdatePercentageHeightDescendants updateDescendants) const noexcept
  {
    bool skippedAutoHeightContainingBlock = false;
    auto *containingBlock = ContainingBlock();
    const RenderBox *containingBlockChild = this;
    LayoutUnit rootMarginBorderPaddingHeight;
    bool isHorizontal = IsHorizontalWritingMode();
    while (containingBlock && !Is<RenderView>(*containingBlock)
           && SkipContainingBlockForPercentHeightCalculation(
             *containingBlock, isHorizontal != containingBlock->IsHorizontalWritingMode()))
    {
      if (containingBlock->IsBody() || containingBlock->IsDocumentElementRenderer())
      {
        rootMarginBorderPaddingHeight += containingBlock->MarginBefore() + containingBlock->MarginAfter()
                                         + containingBlock->BorderAndPaddingLogicalHeight();
      }
      skippedAutoHeightContainingBlock = true;
      containingBlockChild = containingBlock;
      containingBlock = containingBlock->ContainingBlock();
    }
    if (updateDescendants == UpdatePercentageHeightDescendants::Yes)
    {
      containingBlock->AddPercentHeightDescendant(const_cast<RenderBox &>(*this));
    }

    // FIXME: Percentages are treated as auto in an auto-sized view, once the view can be auto-sized.

    if (IsFlexItem() && View().FrameView().LayoutContext().IsPercentHeightResolveDisabledFor(*this))
    {
      return null;
    }

    auto isOrthogonal = isHorizontal != containingBlock->IsHorizontalWritingMode();
    auto overridingAvailableSize = Maybe<LayoutUnit> {};
    if (IsGridItem())
    {
      if (auto gridAreaSize = isOrthogonal ? GridAreaContentLogicalWidth() : GridAreaContentLogicalHeight())
      {
        if (!*gridAreaSize)
        {
          return null;
        }
        overridingAvailableSize = *gridAreaSize;
      }
    }
    if (auto *tableCell = DynamicDowncast<RenderTableCell>(*containingBlock); tableCell && !isOrthogonal)
    {
      if (skippedAutoHeightContainingBlock)
      {
        return null;
      }
      // Table cells violate what the CSS spec says to do with heights. Basically we
      // don't care if the cell specified a height or not. We just always make ourselves
      // be a percentage of the cell's current content height.
      auto tableCellLogicalHeight = tableCell->OverridingBorderBoxLogicalHeight();
      if (!tableCellLogicalHeight)
      {
        // FIXME: Scrolling overflow in a cell with a specified height, or a table with one, initially has no
        // size (tableCellShouldHaveZeroInitialSize), once there's table layout.
        return null;
      }
      // Note: can't use contentBoxLogicalHeight here on table cells due to intrinsic padding.
      overridingAvailableSize = *tableCellLogicalHeight - tableCell->ComputedCSSPaddingBefore()
                                - tableCell->ComputedCSSPaddingAfter() - tableCell->BorderLogicalHeight()
                                - LayoutUnit(tableCell->ScrollbarLogicalHeight());
    }

    auto availableHeight =
      !overridingAvailableSize
        ? (!isOrthogonal ? containingBlock->AvailableLogicalHeightForPercentageComputation()
                         : Maybe<LayoutUnit> {containingBlockChild->ContainingBlockLogicalWidthForContent()})
        : overridingAvailableSize;

    if (!availableHeight)
    {
      return null;
    }

    auto result =
      style::Evaluate<LayoutUnit>(logicalHeight,
                                  *availableHeight - rootMarginBorderPaddingHeight
                                    + (IsRenderTable() && IsOutOfFlowPositioned()
                                         ? containingBlock->PaddingBefore() + containingBlock->PaddingAfter()
                                         : 0_lu),
                                  Style().UsedZoomForLength());

    // |overridingLogicalHeight| is the maximum height made available by the
    // cell to its percent height children when we decide they can determine the
    // height of the cell. If the percent height child is box-sizing:content-box
    // then we must subtract the border and padding from the cell's
    // |availableHeight| (given by |overridingLogicalHeight|) to arrive
    // at the child's computed height.
    bool subtractBorderAndPadding =
      IsRenderTable()
      || (Is<RenderTableCell>(*containingBlock) && !skippedAutoHeightContainingBlock
          && containingBlock->OverridingBorderBoxLogicalHeight()
          && Style().GetBoxSizing() == BoxSizing::ContentBox);
    if (subtractBorderAndPadding)
    {
      result -= BorderAndPaddingLogicalHeight();
      return std::max(0_lu, result);
    }
    return result;
  }

  Maybe<LayoutUnit> RenderBox::ComputePercentageLogicalHeight(
    const style::PreferredSize &logicalHeight,
    UpdatePercentageHeightDescendants updateDescendants) const noexcept
  {
    return ComputePercentageLogicalHeightGeneric(logicalHeight, updateDescendants);
  }

  Maybe<LayoutUnit> RenderBox::ComputePercentageLogicalHeight(
    const style::MinimumSize &logicalHeight,
    UpdatePercentageHeightDescendants updateDescendants) const noexcept
  {
    return ComputePercentageLogicalHeightGeneric(logicalHeight, updateDescendants);
  }

  Maybe<LayoutUnit> RenderBox::ComputePercentageLogicalHeight(
    const style::MaximumSize &logicalHeight,
    UpdatePercentageHeightDescendants updateDescendants) const noexcept
  {
    return ComputePercentageLogicalHeightGeneric(logicalHeight, updateDescendants);
  }

  Maybe<LayoutUnit> RenderBox::ComputePercentageLogicalHeight(
    const style::FlexBasis &logicalHeight, UpdatePercentageHeightDescendants updateDescendants) const noexcept
  {
    return ComputePercentageLogicalHeightGeneric(logicalHeight, updateDescendants);
  }

  bool RenderBox::ShouldComputePreferredLogicalWidthsFromStyle() const noexcept
  {
    auto logicalWidth = OverridingLogicalWidthForFlexBasisComputation().value_or(Style().LogicalWidth());
    auto fixedLogicalWidth = logicalWidth.TryFixed();
    return fixedLogicalWidth && fixedLogicalWidth->IsPositiveOrZero()
           && !(IsDeprecatedFlexItem()
                && !static_cast<int32>(fixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength())));
  }

  void RenderBox::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    ComputePreferredLogicalWidths(Style().LogicalMinWidth(), Style().LogicalMaxWidth(),
                                  BorderAndPaddingLogicalWidth());
    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderBox::ComputePreferredLogicalWidths(const style::MinimumSize &minLogicalWidth,
                                                const style::MaximumSize &maxLogicalWidth,
                                                LayoutUnit borderAndPaddingLogicalWidth) noexcept
  {
    auto usedMaxLogicalWidth = [&]
    {
      // FIXME: We should be able to handle other values for the max logical width here.
      if (auto fixedMaxLogicalWidth = maxLogicalWidth.TryFixed())
      {
        return AdjustContentBoxLogicalWidthForBoxSizing(
          LayoutUnit {fixedMaxLogicalWidth->ResolveZoom(Style().UsedZoomForLength())});
      }

      if (maxLogicalWidth.IsMinContent())
      {
        if (!ShouldComputePreferredLogicalWidthsFromStyle())
        {
          return _minPreferredLogicalWidth;
        }

        return ComputeIntrinsicLogicalWidthUsing(maxLogicalWidth, ContentBoxLogicalWidth(), {});
      }

      return LayoutUnit::Max();
    }();

    auto usedMinLogicalWidth = [&]() -> LayoutUnit
    {
      // FIXME: We should be able to handle other values for the min logical width here.
      if (auto fixedMinLogicalWidth = minLogicalWidth.TryFixed();
          fixedMinLogicalWidth && fixedMinLogicalWidth->IsPositive())
      {
        return AdjustContentBoxLogicalWidthForBoxSizing(
          LayoutUnit {fixedMinLogicalWidth->ResolveZoom(Style().UsedZoomForLength())});
      }

      if (minLogicalWidth.IsMaxContent())
      {
        return _maxPreferredLogicalWidth;
      }

      return {};
    }();

    if (!Style().LogicalWidth().IsFixed() && ShouldComputeLogicalHeightFromAspectRatio())
    {
      auto [transferredMinLogicalWidth, transferredMaxLogicalWidth] =
        ComputeMinMaxLogicalWidthFromAspectRatio();
      transferredMinLogicalWidth = std::max(transferredMinLogicalWidth - borderAndPaddingLogicalWidth, 0_lu);
      transferredMaxLogicalWidth = std::max(transferredMaxLogicalWidth - borderAndPaddingLogicalWidth, 0_lu);
      _minPreferredLogicalWidth =
        std::clamp(_minPreferredLogicalWidth, transferredMinLogicalWidth, transferredMaxLogicalWidth);
      _maxPreferredLogicalWidth =
        std::clamp(_maxPreferredLogicalWidth, transferredMinLogicalWidth, transferredMaxLogicalWidth);
    }

    _maxPreferredLogicalWidth = std::min(_maxPreferredLogicalWidth, usedMaxLogicalWidth);
    _minPreferredLogicalWidth = std::min(_minPreferredLogicalWidth, usedMaxLogicalWidth);

    _maxPreferredLogicalWidth = std::max(_maxPreferredLogicalWidth, usedMinLogicalWidth);
    _minPreferredLogicalWidth = std::max(_minPreferredLogicalWidth, usedMinLogicalWidth);

    _minPreferredLogicalWidth += borderAndPaddingLogicalWidth;
    _maxPreferredLogicalWidth += borderAndPaddingLogicalWidth;
  }

  LayoutUnit RenderBox::AvailableLogicalHeight(AvailableLogicalHeightType heightType) const noexcept
  {
    return ConstrainContentBoxLogicalHeightByMinMax(
      AvailableLogicalHeightUsing(Style().LogicalHeight(), heightType), null);
  }

  LayoutUnit RenderBox::AvailableLogicalHeightUsing(const style::PreferredSize &logicalHeight,
                                                    AvailableLogicalHeightType heightType) const noexcept
  {
    // We need to stop here, since we don't want to increase the height of the table
    // artificially.  We're going to rely on this cell getting expanded to some new
    // height, and then when we lay out again we'll use the calculation below.
    if (IsRenderTableCell() && (logicalHeight.IsAuto() || logicalHeight.IsPercentOrCalculated()))
    {
      if (auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight())
      {
        return *overridingLogicalHeight - ComputedCSSPaddingBefore() - ComputedCSSPaddingAfter()
               - BorderBefore() - BorderAfter() - LayoutUnit(ScrollbarLogicalHeight());
      }
      return LogicalHeight() - BorderAndPaddingLogicalHeight();
    }

    if (auto overridingLogicalHeight =
          IsFlexItem() ? Downcast<RenderFlexibleBox>(*Parent())
                           .UsedFlexItemOverridingLogicalHeightForPercentageResolution(*this)
                       : null)
    {
      return ContentBoxLogicalHeight(*overridingLogicalHeight);
    }

    if (ShouldComputeLogicalHeightFromAspectRatio())
    {
      auto borderAndPaddingLogicalHeight = BorderAndPaddingLogicalHeight();
      auto borderBoxLogicalHeight = BlockSizeFromAspectRatio(
        BorderAndPaddingLogicalWidth(), borderAndPaddingLogicalHeight, Style().LogicalAspectRatio(),
        Style().BoxSizingForAspectRatio(), LogicalWidth(), Style().GetAspectRatio(), IsRenderReplaced());
      if (heightType == AvailableLogicalHeightType::ExcludeMarginBorderPadding)
      {
        return borderBoxLogicalHeight - borderAndPaddingLogicalHeight;
      }
      return borderBoxLogicalHeight;
    }

    // FIXME: Percentages of out-of-flow positioned boxes resolve against their containing block's size
    // (PositionedLayoutConstraints), once there's out-of-flow layout.

    if (auto computedContentAndScrollbarLogicalHeight =
          ComputeContentAndScrollbarLogicalHeightUsing(logicalHeight, null))
    {
      return std::max<LayoutUnit>(
        0_lu, AdjustContentBoxLogicalHeightForBoxSizing(computedContentAndScrollbarLogicalHeight)
                - LayoutUnit(ScrollbarLogicalHeight()));
    }

    // Height of absolutely positioned, non-replaced elements section 5.3 rule 5
    // https://www.w3.org/TR/css-position-3/#abs-non-replaced-height
    if (auto *block = DynamicDowncast<RenderBlock>(*this);
        block && IsOutOfFlowPositioned() && Style().LogicalHeight().IsAuto()
        && !(Style().InsetBefore().IsAuto() || Style().InsetAfter().IsAuto()))
    {
      auto computedValues = block->ComputeLogicalHeight(block->LogicalHeight(), 0_lu);
      return computedValues.Extent - block->BorderAndPaddingLogicalHeight()
             - LayoutUnit(block->ScrollbarLogicalHeight());
    }

    LayoutUnit availableHeight = IsOrthogonal(*this, *ContainingBlock())
                                   ? ContainingBlockLogicalWidthForContent()
                                   : ContainingBlockLogicalHeightForContent(heightType);
    if (heightType == AvailableLogicalHeightType::ExcludeMarginBorderPadding)
    {
      // FIXME: Margin collapsing hasn't happened yet, so this incorrectly removes collapsed margins.
      availableHeight -= MarginBefore() + MarginAfter() + BorderAndPaddingLogicalHeight();
    }
    return availableHeight;
  }

  bool RenderBox::ShrinkToAvoidFloats() const noexcept
  {
    // Floating objects don't shrink. Objects that don't avoid floats don't shrink. Non-inline box type of
    // inline level elements don't shrink.
    if (IsInline() || IsFloating() || !AvoidsFloats())
    {
      return false;
    }

    // Only auto width objects can possibly shrink to avoid floats.
    return Style().Width().IsAuto();
  }

  bool RenderBox::AvoidsFloats() const noexcept
  {
    // FIXME: Form controls, once there are form controls, and MathML blocks, once there's MathML.
    if (Is<RenderReplaced>(*this) || IsLegend())
    {
      return true;
    }

    if (auto *renderBlock = DynamicDowncast<RenderBlock>(*this))
    {
      return renderBlock->CreatesNewFormattingContext();
    }

    return false;
  }

  bool RenderBox::PercentageLogicalHeightIsResolvable() const noexcept
  {
    // Do this to avoid duplicating all the logic that already exists when computing an actual percentage
    // height.
    return ComputePercentageLogicalHeight(style::PreferredSize {style::PreferredSize::Percentage {100}})
           != Maybe<LayoutUnit> {};
  }

  LayoutUnit RenderBox::LineHeight() const noexcept
  {
    auto shouldUseLineHeightFromStyle = [&]
    {
      if (Is<RenderBlock>(*this))
      {
        return true;
      }
      if (auto *listMarkerRenderer = DynamicDowncast<RenderListMarker>(*this))
      {
        return !listMarkerRenderer->IsImage();
      }
      return false;
    };
    if (shouldUseLineHeightFromStyle())
    {
      return LayoutUnit::FromFloatCeil(FirstLineStyle().ComputedLineHeight());
    }

    if (IsBlockLevelReplacedOrAtomicInline())
    {
      return MarginBefore() + LogicalHeight() + MarginAfter();
    }

    return {};
  }

  bool RenderBox::ShouldInvalidatePreferredWidths() const noexcept
  {
    return Style().PaddingStart().IsPercentOrCalculated() || Style().PaddingEnd().IsPercentOrCalculated()
           || (Style().HasAspectRatio()
               && (HasRelativeLogicalHeight() || (IsFlexItem() && HasStretchedLogicalHeight())));
  }

  bool RenderBox::HasRelativeDimensions() const noexcept
  {
    return Style().Height().IsPercentOrCalculated() || Style().Width().IsPercentOrCalculated()
           || Style().MaxHeight().IsPercentOrCalculated() || Style().MaxWidth().IsPercentOrCalculated()
           || Style().MinHeight().IsPercentOrCalculated() || Style().MinWidth().IsPercentOrCalculated();
  }

  bool RenderBox::HasRelativeLogicalHeight() const noexcept
  {
    return Style().LogicalHeight().IsPercentOrCalculated()
           || Style().LogicalMinHeight().IsPercentOrCalculated()
           || Style().LogicalMaxHeight().IsPercentOrCalculated();
  }

  bool RenderBox::HasRelativeLogicalWidth() const noexcept
  {
    return Style().LogicalWidth().IsPercentOrCalculated() || Style().LogicalMinWidth().IsPercentOrCalculated()
           || Style().LogicalMaxWidth().IsPercentOrCalculated();
  }

  bool RenderBox::StretchesToViewport() const noexcept
  {
    return GetDocument().InQuirksMode() && Style().LogicalHeight().IsAuto()
           && !IsFloatingOrOutOfFlowPositioned() && (IsDocumentElementRenderer() || IsBody())
           && !ShouldComputeLogicalHeightFromAspectRatio() && !IsInline();
  }

  LayoutUnit RenderBox::IntrinsicLogicalWidth() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? IntrinsicSize().Width() : IntrinsicSize().Height();
  }

  LayoutUnit RenderBox::IntrinsicLogicalHeight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? IntrinsicSize().Height() : IntrinsicSize().Width();
  }

  bool RenderBox::ShouldIgnoreLogicalMinMaxWidthSizes() const noexcept
  {
    if (!IsFlexItem())
    {
      return false;
    }
    if (auto *flexBox = DynamicDowncast<RenderFlexibleBox>(Parent()))
    {
      return flexBox->IsComputingFlexBaseSizes()
             && Style().GetWritingMode().IsHorizontal() == flexBox->IsHorizontalFlow();
    }
    krys_debug_assert(false);
    return false;
  }

  bool RenderBox::ShouldIgnoreLogicalMinMaxHeightSizes() const noexcept
  {
    if (!IsFlexItem())
    {
      return false;
    }
    if (auto *flexBox = DynamicDowncast<RenderFlexibleBox>(Parent()))
    {
      return flexBox->IsComputingFlexBaseSizes()
             && Style().GetWritingMode().IsHorizontal() != flexBox->IsHorizontalFlow();
    }
    krys_debug_assert(false);
    return false;
  }

  Maybe<LayoutUnit> RenderBox::ExplicitIntrinsicInnerWidth() const noexcept
  {
    krys_debug_assert(IsHorizontalWritingMode() ? ShouldApplySizeOrInlineSizeContainment()
                                                : ShouldApplySizeContainment());

    const auto &containIntrinsicWidth = Style().ContainIntrinsicWidth();
    if (containIntrinsicWidth.IsNone())
    {
      return null;
    }

    // FIXME: The last remembered size of an element whose contents are skipped (contain-intrinsic-size:
    // auto), once there's content-visibility.

    if (auto length = containIntrinsicWidth.TryLength())
    {
      return LayoutUnit {length->ResolveZoom(style::ZoomNeeded {})};
    }

    krys_debug_assert(containIntrinsicWidth.IsAutoAndNone());
    return null;
  }

  Maybe<LayoutUnit> RenderBox::ExplicitIntrinsicInnerHeight() const noexcept
  {
    krys_debug_assert(IsHorizontalWritingMode() ? ShouldApplySizeContainment()
                                                : ShouldApplySizeOrInlineSizeContainment());

    const auto &containIntrinsicHeight = Style().ContainIntrinsicHeight();
    if (containIntrinsicHeight.IsNone())
    {
      return null;
    }

    // FIXME: The last remembered size of an element whose contents are skipped (contain-intrinsic-size:
    // auto), once there's content-visibility.

    if (auto length = containIntrinsicHeight.TryLength())
    {
      return LayoutUnit {length->ResolveZoom(style::ZoomNeeded {})};
    }

    krys_debug_assert(containIntrinsicHeight.IsAutoAndNone());
    return null;
  }

  Maybe<LayoutUnit> RenderBox::ExplicitIntrinsicInnerLogicalWidth() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ExplicitIntrinsicInnerWidth()
                                                   : ExplicitIntrinsicInnerHeight();
  }

  Maybe<LayoutUnit> RenderBox::ExplicitIntrinsicInnerLogicalHeight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? ExplicitIntrinsicInnerHeight()
                                                   : ExplicitIntrinsicInnerWidth();
  }

  bool RenderBox::HasAutoHeightOrContainingBlockWithAutoHeight(
    UpdatePercentageHeightDescendants updatePercentageDescendants) const noexcept
  {
    const auto &logicalHeight = Style().LogicalHeight();
    auto *containingBlock = ContainingBlockForAutoHeightDetection(logicalHeight);

    if (updatePercentageDescendants == UpdatePercentageHeightDescendants::Yes
        && logicalHeight.IsPercentOrCalculated() && containingBlock)
    {
      containingBlock->AddPercentHeightDescendant(const_cast<RenderBox &>(*this));
    }

    if (IsFlexItem() && Downcast<RenderFlexibleBox>(*Parent()).CanUseFlexItemForPercentageResolution(*this))
    {
      return false;
    }

    if (IsGridItem())
    {
      if (auto containingBlockContentLogicalHeight = GridAreaContentLogicalHeight())
      {
        return !*containingBlockContentLogicalHeight;
      }
    }

    auto isOutOfFlowPositionedWithImplicitHeight =
      IsOutOfFlowPositioned() && !Style().InsetBefore().IsAuto() && !Style().InsetAfter().IsAuto();
    if (logicalHeight.IsAuto() && !isOutOfFlowPositionedWithImplicitHeight)
    {
      return true;
    }

    // We need the containing block to have a definite block-size in order to resolve the block-size of the
    // descendant, except when in quirks mode. Flexboxes follow strict behavior even in quirks mode, though.
    if (!containingBlock
        || (GetDocument().InQuirksMode() && !containingBlock->IsFlexibleBoxIncludingDeprecated()))
    {
      return false;
    }

    return !containingBlock->HasDefiniteLogicalHeight();
  }

  bool RenderBox::OverflowChangesMayAffectLayout() const noexcept
  {
    if (Style().OverflowY() != Overflow::Auto && Style().OverflowX() != Overflow::Auto)
    {
      return false;
    }

    // FIXME: Legacy scrollbar styles and the scrollbar theme's overlay scrollbars, once there are scrollbars.
    // Until then scrollbars take no space, as overlay scrollbars.
    return false;
  }

  bool RenderBox::HasIntrinsicAspectRatio() const noexcept
  {
    // FIXME: Images, video, canvas and view transition captures, once there are any.
    return false;
  }

#pragma endregion

#pragma region Scrollbars

  bool RenderBox::IncludeVerticalScrollbarSize() const noexcept
  {
    // FIXME: Whether the layer has overlay scrollbars, once there are layers.
    return HasNonVisibleOverflow() && HasLayer()
           && (Style().OverflowY() == Overflow::Scroll || Style().OverflowY() == Overflow::Auto
               || (Style().OverflowY() == Overflow::Hidden && !Style().GetScrollbarGutter().IsAuto()));
  }

  bool RenderBox::IncludeHorizontalScrollbarSize() const noexcept
  {
    // FIXME: Whether the layer has overlay scrollbars, once there are layers.
    return HasNonVisibleOverflow() && HasLayer()
           && (Style().OverflowX() == Overflow::Scroll || Style().OverflowX() == Overflow::Auto
               || (Style().OverflowX() == Overflow::Hidden && !Style().GetScrollbarGutter().IsAuto()));
  }

  int32 RenderBox::VerticalScrollbarWidth() const noexcept
  {
    // FIXME: The scrollable area's vertical scrollbar width, once there are layers.
    return 0;
  }

  int32 RenderBox::HorizontalScrollbarHeight() const noexcept
  {
    // FIXME: The scrollable area's horizontal scrollbar height, once there are layers.
    return 0;
  }

  int32 RenderBox::IntrinsicScrollbarLogicalWidthIncludingGutter() const noexcept
  {
    if (!HasNonVisibleOverflow())
    {
      return 0;
    }

    auto shouldIncludeScrollbarGutter =
      [](style::ScrollbarGutter gutter, bool hasVisibleOverflow, Overflow overflow)
    {
      return (overflow == Overflow::Auto && (!gutter.IsAuto() || hasVisibleOverflow))
             || (overflow == Overflow::Hidden && !gutter.IsAuto());
    };

    // FIXME: Overlay scrollbars (canUseOverlayScrollbars), once there are scrollbars.
    if (IsHorizontalWritingMode()
        && (Style().OverflowY() == Overflow::Scroll
            || shouldIncludeScrollbarGutter(Style().GetScrollbarGutter(), HasScrollableOverflowY(),
                                            Style().OverflowY())))
    {
      return Style().GetScrollbarGutter().IsStableBothEdges() ? VerticalScrollbarWidth() * 2
                                                              : VerticalScrollbarWidth();
    }

    if (!IsHorizontalWritingMode()
        && (Style().OverflowX() == Overflow::Scroll
            || shouldIncludeScrollbarGutter(Style().GetScrollbarGutter(), HasScrollableOverflowX(),
                                            Style().OverflowX())))
    {
      return Style().GetScrollbarGutter().IsStableBothEdges() ? HorizontalScrollbarHeight() * 2
                                                              : HorizontalScrollbarHeight();
    }

    return 0;
  }

  int32 RenderBox::ScrollbarLogicalWidth() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? VerticalScrollbarWidth() : HorizontalScrollbarHeight();
  }

  int32 RenderBox::ScrollbarLogicalHeight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? HorizontalScrollbarHeight() : VerticalScrollbarWidth();
  }

  bool RenderBox::ScrollsOverflow() const noexcept
  {
    return ScrollsOverflowX() || ScrollsOverflowY();
  }

  bool RenderBox::ScrollsOverflowX() const noexcept
  {
    return HasNonVisibleOverflow()
           && (Style().OverflowX() == Overflow::Scroll || Style().OverflowX() == Overflow::Auto);
  }

  bool RenderBox::ScrollsOverflowY() const noexcept
  {
    return HasNonVisibleOverflow()
           && (Style().OverflowY() == Overflow::Scroll || Style().OverflowY() == Overflow::Auto);
  }

  bool RenderBox::HasHorizontalOverflow() const noexcept
  {
    return ScrollWidth() != RoundToInt(PaddingBoxWidth());
  }

  bool RenderBox::HasVerticalOverflow() const noexcept
  {
    return ScrollHeight() != RoundToInt(PaddingBoxHeight());
  }

  bool RenderBox::HasScrollableOverflowX() const noexcept
  {
    return ScrollsOverflowX() && HasHorizontalOverflow();
  }

  bool RenderBox::HasScrollableOverflowY() const noexcept
  {
    return ScrollsOverflowY() && HasVerticalOverflow();
  }

  bool RenderBox::IsScrollContainerX() const noexcept
  {
    return Style().OverflowX() == Overflow::Scroll || Style().OverflowX() == Overflow::Hidden
           || Style().OverflowX() == Overflow::Auto;
  }

  bool RenderBox::IsScrollContainerY() const noexcept
  {
    return Style().OverflowY() == Overflow::Scroll || Style().OverflowY() == Overflow::Hidden
           || Style().OverflowY() == Overflow::Auto;
  }

#pragma endregion

#pragma region Overflow

  LayoutUnit RenderBox::LogicalLeftLayoutOverflow() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? LayoutOverflowRect().X() : LayoutOverflowRect().Y();
  }

  LayoutUnit RenderBox::LogicalRightLayoutOverflow() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? LayoutOverflowRect().MaxX()
                                                   : LayoutOverflowRect().MaxY();
  }

  LayoutUnit RenderBox::LogicalLeftVisualOverflow() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? VisualOverflowRect().X() : VisualOverflowRect().Y();
  }

  LayoutUnit RenderBox::LogicalRightVisualOverflow() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? VisualOverflowRect().MaxX()
                                                   : VisualOverflowRect().MaxY();
  }

  bool RenderBox::HasHorizontalLayoutOverflow() const noexcept
  {
    if (!_overflow)
    {
      return false;
    }

    auto layoutOverflowRect = _overflow->LayoutOverflowRect();
    auto paddingBoxRect = FlippedClientBoxRect();
    return layoutOverflowRect.X() < paddingBoxRect.X() || layoutOverflowRect.MaxX() > paddingBoxRect.MaxX();
  }

  bool RenderBox::HasVerticalLayoutOverflow() const noexcept
  {
    if (!_overflow)
    {
      return false;
    }

    auto layoutOverflowRect = _overflow->LayoutOverflowRect();
    auto paddingBoxRect = FlippedClientBoxRect();
    return layoutOverflowRect.Y() < paddingBoxRect.Y() || layoutOverflowRect.MaxY() > paddingBoxRect.MaxY();
  }

  void RenderBox::AddVisualEffectOverflow() noexcept
  {
    bool hasBoxShadow = Style().HasBoxShadow();
    bool hasBorderImageOutsets = Style().HasBorderImageOutsets();
    bool hasOutline = OutlineStyleForRepaint().HasOutlineInVisualOverflow();
    if (!hasBoxShadow && !hasBorderImageOutsets && !hasOutline)
    {
      return;
    }

    AddVisualOverflow(ApplyVisualEffectOverflow(BorderBoxRect()));

    // FIXME: The fragments' visual effect overflow, once there are fragmented flows.
  }

  static void ConvertOutsetsToOverflowCoordinates(LayoutBoxExtent &outsets, WritingMode writingMode) noexcept
  {
    switch (writingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom:
      case FlowDirection::LeftToRight: break;
      case FlowDirection::BottomToTop: std::swap(outsets.Top(), outsets.Bottom()); break;
      case FlowDirection::RightToLeft: std::swap(outsets.Left(), outsets.Right()); break;
    }
  }

  LayoutRect RenderBox::ApplyVisualEffectOverflow(const LayoutRect &borderBox) const noexcept
  {
    LayoutUnit overflowMinX = borderBox.X();
    LayoutUnit overflowMaxX = borderBox.MaxX();
    LayoutUnit overflowMinY = borderBox.Y();
    LayoutUnit overflowMaxY = borderBox.MaxY();

    // Compute box-shadow overflow first.
    if (Style().HasBoxShadow())
    {
      auto shadowOutsets = style::ShadowOutsetExtent(Style().GetBoxShadow(), Style().UsedZoomForLength());
      // Box-shadow extent's left and top are negative when extends to left and top, respectively, so negate
      // to convert to outsets.
      shadowOutsets.Left() = -shadowOutsets.Left();
      shadowOutsets.Top() = -shadowOutsets.Top();
      ConvertOutsetsToOverflowCoordinates(shadowOutsets, Style().GetWritingMode());

      overflowMinX = borderBox.X() - shadowOutsets.Left();
      overflowMaxX = borderBox.MaxX() + shadowOutsets.Right();
      overflowMinY = borderBox.Y() - shadowOutsets.Top();
      overflowMaxY = borderBox.MaxY() + shadowOutsets.Bottom();
    }

    // Now compute border-image-outset overflow.
    if (Style().HasBorderImageOutsets())
    {
      auto borderOutsets = Style().BorderImageOutsets();
      ConvertOutsetsToOverflowCoordinates(borderOutsets, Style().GetWritingMode());

      overflowMinX = std::min(overflowMinX, borderBox.X() - borderOutsets.Left());
      overflowMaxX = std::max(overflowMaxX, borderBox.MaxX() + borderOutsets.Right());
      overflowMinY = std::min(overflowMinY, borderBox.Y() - borderOutsets.Top());
      overflowMaxY = std::max(overflowMaxY, borderBox.MaxY() + borderOutsets.Bottom());
    }

    if (OutlineStyleForRepaint().HasOutlineInVisualOverflow())
    {
      LayoutUnit outlineSize {OutlineStyleForRepaint().OutlineSize()};
      overflowMinX = std::min(overflowMinX, borderBox.X() - outlineSize);
      overflowMaxX = std::max(overflowMaxX, borderBox.MaxX() + outlineSize);
      overflowMinY = std::min(overflowMinY, borderBox.Y() - outlineSize);
      overflowMaxY = std::max(overflowMaxY, borderBox.MaxY() + outlineSize);
    }
    // Add in the final overflow with shadows and outsets combined.
    return LayoutRect(overflowMinX, overflowMinY, overflowMaxX - overflowMinX, overflowMaxY - overflowMinY);
  }

  void RenderBox::AddOverflowFromInFlowChildren() noexcept
  {
    for (auto &child : ChildrenOfType<RenderBox>(*this))
    {
      if (!child.IsFloatingOrOutOfFlowPositioned())
      {
        AddOverflowFromContainedBox(child);
      }
    }
  }

  void RenderBox::AddOverflowFromContainedBox(const RenderBox &child) noexcept
  {
    krys_debug_assert(!child.IsFloating());
    AddOverflowWithRendererOffset(child, child.LocationOffset());
  }

  // 'offsetFromThis' is normally the renderer's position (RenderBox::location()).
  // However in case of a float box, it may belong to a different container
  // (but is intrusive to this container hence calling this function),
  // meaning that the renderer's position may not be the same as its actual offset from this container.
  void RenderBox::AddOverflowWithRendererOffset(const RenderBox &renderer, LayoutSize offsetFromThis) noexcept
  {
    // Never allow flow threads to propagate overflow up to a parent.
    if (renderer.IsRenderFragmentedFlow())
    {
      return;
    }

    // FIXME: The fragments' overflow, once there are fragmented flows.

    // Only propagate layout overflow from the child if the child isn't clipping its overflow.  If it is, then
    // its overflow is internal to it, and we don't care about it. layoutOverflowRectForPropagation takes care
    // of this and just propagates the border box rect instead.
    auto childLayoutOverflowRect = renderer.LayoutOverflowRectForPropagation(Style().GetWritingMode());
    childLayoutOverflowRect.Move(offsetFromThis);
    AddLayoutOverflow(childLayoutOverflowRect);

    auto ensurePaddingEndIsIncluded = [&]
    {
      if (!HasNonVisibleOverflow())
      {
        return;
      }

      // As per https://github.com/w3c/csswg-drafts/issues/3653 padding should contribute to the scrollable
      // overflow area.
      if (!PaddingEnd())
      {
        return;
      }

      // FIXME: Expand it to non-grid/flex cases when applicable.
      if (!Is<RenderGrid>(*this) && !Is<RenderFlexibleBox>(*this))
      {
        return;
      }

      if (renderer.IsOutOfFlowPositioned())
      {
        return;
      }

      // Note that we can't use childLayoutOverflowRect here as it already includes propagated overflow from
      // descendents.
      auto isMainAxisFlipped = [&]
      {
        auto isInlineFlipped = Style().GetWritingMode().IsInlineFlipped();
        if (auto *flexContainer = DynamicDowncast<RenderFlexibleBox>(*this))
        {
          auto isColumnOrRowReverseSameAsWrapReverse =
            flexContainer->IsColumnOrRowReverse() == flexContainer->IsWrapReverse();
          return isInlineFlipped == isColumnOrRowReverseSameAsWrapReverse;
        }
        return isInlineFlipped;
      }();
      auto childLogicalRight = [&]
      {
        if (!isMainAxisFlipped)
        {
          return (IsHorizontalWritingMode() ? offsetFromThis.Width() + renderer.Width()
                                            : offsetFromThis.Height() + renderer.Height())
                 + renderer.MarginEnd(Style().GetWritingMode());
        }
        return (IsHorizontalWritingMode() ? offsetFromThis.Width() : offsetFromThis.Height())
               - renderer.MarginEnd(Style().GetWritingMode());
      };

      auto layoutOverflowRect = LayoutOverflowRect();
      if (!isMainAxisFlipped)
      {
        auto layoutOverflowLogicalRightIncludingPaddingEnd = childLogicalRight() + PaddingEnd();
        if (IsHorizontalWritingMode())
        {
          layoutOverflowRect.ShiftMaxXEdgeTo(layoutOverflowLogicalRightIncludingPaddingEnd);
        }
        else
        {
          layoutOverflowRect.ShiftMaxYEdgeTo(layoutOverflowLogicalRightIncludingPaddingEnd);
        }
      }
      else
      {
        auto layoutOverflowLogicalRightIncludingPaddingEnd = childLogicalRight() - PaddingEnd();
        if (IsHorizontalWritingMode())
        {
          layoutOverflowRect.ShiftXEdgeTo(layoutOverflowLogicalRightIncludingPaddingEnd);
        }
        else
        {
          layoutOverflowRect.ShiftYEdgeTo(layoutOverflowLogicalRightIncludingPaddingEnd);
        }
      }
      AddLayoutOverflow(layoutOverflowRect);
    };
    ensurePaddingEndIsIncluded();

    if (PaintContainmentApplies())
    {
      return;
    }

    // Add in visual overflow from the child. Even if the child clips its overflow, it may still
    // have visual overflow of its own set from box shadows or reflections. It is unnecessary to propagate
    // this overflow if we are clipping our own overflow.
    if (HasPotentiallyScrollableOverflow())
    {
      return;
    }

    // Update our visual overflow in case the child spills out the block, but only if we were going to paint
    // the child block ourselves.
    // FIXME: Children with a self-painting layer paint themselves, once there are layers.
    auto childVisualOverflowRect = renderer.VisualOverflowRectForPropagation(Style().GetWritingMode());
    childVisualOverflowRect.Move(offsetFromThis);
    AddVisualOverflow(childVisualOverflowRect);
  }

  LayoutOptionalOutsets RenderBox::AllowedLayoutOverflow() const noexcept
  {
    LayoutOptionalOutsets allowance;

    // Overflow is in the block's coordinate space and thus is flipped
    // for horizontal-bt and vertical-rl writing modes. This means we can
    // treat horizontal-tb/bt as the same and vertical-lr/rl as the same.

    auto writingMode = Style().GetWritingMode();
    if (writingMode.IsHorizontal())
    {
      allowance.Top() = 0_lu;
      if (writingMode.IsInlineLeftToRight())
      {
        allowance.Left() = 0_lu;
      }
      else
      {
        allowance.Right() = 0_lu;
      }
    }
    else
    {
      allowance.Left() = 0_lu;
      if (writingMode.IsInlineTopToBottom())
      {
        allowance.Top() = 0_lu;
      }
      else
      {
        allowance.Bottom() = 0_lu;
      }
    }

    return allowance;
  }

  void RenderBox::AddLayoutOverflow(const LayoutRect &rect) noexcept
  {
    auto clientBox = FlippedClientBoxRect();
    if (clientBox.Contains(rect) || rect.IsEmpty())
    {
      return;
    }

    // For overflow clip objects, we don't want to propagate overflow into unreachable areas.
    LayoutRect overflowRect(rect);
    if (HasPotentiallyScrollableOverflow() || IsRenderView())
    {
      LayoutOptionalOutsets allowance = AllowedLayoutOverflow();
      // Non-negative values indicate a limit, let's apply them.
      if (allowance.Top())
      {
        overflowRect.ShiftYEdgeTo(std::max(overflowRect.Y(), clientBox.Y() - *allowance.Top()));
      }
      if (allowance.Bottom())
      {
        overflowRect.ShiftMaxYEdgeTo(std::min(overflowRect.MaxY(), clientBox.MaxY() + *allowance.Bottom()));
      }
      if (allowance.Left())
      {
        overflowRect.ShiftXEdgeTo(std::max(overflowRect.X(), clientBox.X() - *allowance.Left()));
      }
      if (allowance.Right())
      {
        overflowRect.ShiftMaxXEdgeTo(std::min(overflowRect.MaxX(), clientBox.MaxX() + *allowance.Right()));
      }

      // Now re-test with the adjusted rectangle and see if it has become unreachable or fully
      // contained.
      if (clientBox.Contains(overflowRect) || overflowRect.IsEmpty())
      {
        return;
      }
    }

    EnsureOverflow().AddLayoutOverflow(overflowRect);
  }

  void RenderBox::AddVisualOverflow(const LayoutRect &rect) noexcept
  {
    LayoutRect borderBox = BorderBoxRect();
    if (borderBox.Contains(rect) || rect.IsEmpty())
    {
      return;
    }

    EnsureOverflow().AddVisualOverflow(rect);
  }

  void RenderBox::ClearOverflow() noexcept
  {
    _overflow = nullptr;
    // FIXME: The fragments' overflow, once there are fragmented flows.
  }

  RenderOverflow &RenderBox::EnsureOverflow() noexcept
  {
    if (!_overflow)
    {
      _overflow = CreateUnique<RenderOverflow>(FlippedClientBoxRect(), BorderBoxRect());
    }

    return *_overflow;
  }

  LayoutRect RenderBox::LogicalVisualOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept
  {
    LayoutRect rect = VisualOverflowRectForPropagation(parentWritingMode);
    if (!parentWritingMode.IsHorizontal())
    {
      return rect.TransposedRect();
    }
    return rect;
  }

  LayoutRect RenderBox::VisualOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept
  {
    // If the writing modes of the child and parent match, then we don't have to
    // do anything fancy. Just return the result.
    LayoutRect rect = VisualOverflowRect();
    auto writingMode = Style().GetWritingMode();
    if (parentWritingMode.BlockDirection() == writingMode.BlockDirection())
    {
      return rect;
    }

    // We are putting ourselves into our parent's coordinate space.  If there is a flipped block mismatch
    // in a particular axis, then we have to flip the rect along that axis.
    if (writingMode.BlockDirection() == FlowDirection::RightToLeft
        || parentWritingMode.BlockDirection() == FlowDirection::RightToLeft)
    {
      rect.SetX(Width() - rect.MaxX());
    }
    else if (writingMode.BlockDirection() == FlowDirection::BottomToTop
             || parentWritingMode.BlockDirection() == FlowDirection::BottomToTop)
    {
      rect.SetY(Height() - rect.MaxY());
    }

    return rect;
  }

  LayoutRect RenderBox::LogicalLayoutOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept
  {
    LayoutRect rect = LayoutOverflowRectForPropagation(parentWritingMode);
    if (!parentWritingMode.IsHorizontal())
    {
      return rect.TransposedRect();
    }
    return rect;
  }

  LayoutRect RenderBox::LayoutOverflowRectForPropagation(WritingMode parentWritingMode) const noexcept
  {
    // Only propagate interior layout overflow if we don't completely clip it.
    auto rect = BorderBoxRect();
    // As per https://drafts.csswg.org/css-overflow-3/#scrollable, both flex and grid items margins' should
    // contribute to the scrollable overflow area.
    if (ShouldMarginInlineEndContributeToScrollableOverflow(*this))
    {
      auto marginEnd = std::max(0_lu, MarginEnd(parentWritingMode));
      if (parentWritingMode.IsHorizontal())
      {
        rect.SetWidth(rect.Width() + marginEnd);
      }
      else
      {
        rect.SetHeight(rect.Height() + marginEnd);
      }
    }

    if (!ShouldApplyLayoutContainment())
    {
      if (HasNonVisibleOverflow())
      {
        if (Style().OverflowX() == Overflow::Clip && Style().OverflowY() == Overflow::Visible)
        {
          LayoutRect clippedOverflowRect = LayoutOverflowRect();
          clippedOverflowRect.SetX(rect.X());
          clippedOverflowRect.SetWidth(rect.Width());
          rect.Unite(clippedOverflowRect);
        }
        else if (Style().OverflowY() == Overflow::Clip && Style().OverflowX() == Overflow::Visible)
        {
          LayoutRect clippedOverflowRect = LayoutOverflowRect();
          clippedOverflowRect.SetY(rect.Y());
          clippedOverflowRect.SetHeight(rect.Height());
          rect.Unite(clippedOverflowRect);
        }
      }
      else
      {
        rect.Unite(LayoutOverflowRect());
      }
    }

    // While a stickily positioned renderer is also inflow positioned, they stretch the overflow rect with
    // their inflow geometry (as opposed to the paint geometry) because they are not stationary.
    // FIXME: Transforms (the transform from the container), once there's painting.
    bool paintGeometryAffectsLayoutOverflow = IsInFlowPositioned() && !IsStickilyPositioned();
    if (paintGeometryAffectsLayoutOverflow)
    {
      // If we are relatively positioned or if we have a transform, then we have to convert
      // this rectangle into physical coordinates, apply relative positioning and transforms
      // to it, and then convert it back.
      // It ensures that the overflow rect tracks the paint geometry and not the inflow layout position.
      FlipForWritingMode(rect);

      rect.Move(OffsetForInFlowPosition());

      // Now we need to flip back.
      FlipForWritingMode(rect);
    }

    // If the writing modes of the child and parent match, then we don't have to
    // do anything fancy. Just return the result.
    auto writingMode = Style().GetWritingMode();
    if (parentWritingMode.BlockDirection() == writingMode.BlockDirection())
    {
      return rect;
    }

    // We are putting ourselves into our parent's coordinate space.  If there is a flipped block mismatch
    // in a particular axis, then we have to flip the rect along that axis.
    if (writingMode.BlockDirection() == FlowDirection::RightToLeft
        || parentWritingMode.BlockDirection() == FlowDirection::RightToLeft)
    {
      rect.SetX(Width() - rect.MaxX());
    }
    else if (writingMode.BlockDirection() == FlowDirection::BottomToTop
             || parentWritingMode.BlockDirection() == FlowDirection::BottomToTop)
    {
      rect.SetY(Height() - rect.MaxY());
    }

    return rect;
  }

  LayoutRect RenderBox::FlippedClientBoxRect() const noexcept
  {
    // Because of the special coordinate system used for overflow rectangles (not quite logical, not
    // quite physical), we need to flip the block progression coordinate in vertical-rl and
    // horizontal-bt writing modes. Apart from that, this method does the same as clientBoxRect().

    auto rect = PaddingBoxRectIncludingScrollbar();
    // Flip block progression axis if writing mode is vertical-rl or horizontal-bt.
    FlipForWritingMode(rect);
    if (HasNonVisibleOverflow())
    {
      // Subtract space occupied by scrollbars. They are at their physical edge in this coordinate
      // system, so order is important here: first flip, then subtract scrollbars.
      if (IsHorizontalWritingMode() && Style().ShouldPlaceVerticalScrollbarOnLeft())
      {
        rect.Move(LayoutUnit(VerticalScrollbarWidth()), 0_lu);
      }
      rect.Contract(LayoutUnit(VerticalScrollbarWidth()), LayoutUnit(HorizontalScrollbarHeight()));
    }
    return rect;
  }

#pragma endregion

#pragma region Writing mode flipping

  LayoutPoint RenderBox::FlipForWritingModeForChild(const RenderBox &child,
                                                    const LayoutPoint &point) const noexcept
  {
    if (!Style().GetWritingMode().IsBlockFlipped())
    {
      return point;
    }

    // The child is going to add in its x() and y(), so we have to make sure it ends up in
    // the right place.
    if (IsHorizontalWritingMode())
    {
      return LayoutPoint(point.X(), point.Y() + Height() - child.Height() - (2 * child.Y()));
    }
    return LayoutPoint(point.X() + Width() - child.Width() - (2 * child.X()), point.Y());
  }

  void RenderBox::FlipForWritingMode(LayoutRect &rect) const noexcept
  {
    if (!Style().GetWritingMode().IsBlockFlipped())
    {
      return;
    }

    if (IsHorizontalWritingMode())
    {
      rect.SetY(Height() - rect.MaxY());
    }
    else
    {
      rect.SetX(Width() - rect.MaxX());
    }
  }

  LayoutUnit RenderBox::FlipForWritingMode(LayoutUnit position) const noexcept
  {
    if (!Style().GetWritingMode().IsBlockFlipped())
    {
      return position;
    }
    return LogicalHeight() - position;
  }

  LayoutPoint RenderBox::FlipForWritingMode(const LayoutPoint &position) const noexcept
  {
    if (!Style().GetWritingMode().IsBlockFlipped())
    {
      return position;
    }
    return IsHorizontalWritingMode() ? LayoutPoint(position.X(), Height() - position.Y())
                                     : LayoutPoint(Width() - position.X(), position.Y());
  }

  LayoutSize RenderBox::FlipForWritingMode(const LayoutSize &offset) const noexcept
  {
    if (!Style().GetWritingMode().IsBlockFlipped())
    {
      return offset;
    }
    return IsHorizontalWritingMode() ? LayoutSize(offset.Width(), Height() - offset.Height())
                                     : LayoutSize(Width() - offset.Width(), offset.Height());
  }

#pragma endregion

#pragma region Aspect ratio

  bool RenderBox::ShouldIgnoreAspectRatio() const noexcept
  {
    return !Style().HasAspectRatio() || IsTablePart();
  }

  static bool ShouldComputeLogicalWidthFromAspectRatioAndInsets(const RenderBox &renderer) noexcept
  {
    if (!renderer.IsOutOfFlowPositioned())
    {
      return false;
    }

    const auto &style = renderer.Style();
    if (!style.LogicalWidth().IsAuto())
    {
      // Not applicable for aspect ratio computation.
      return false;
    }
    // When both left and right are set, the out-of-flow positioned box is horizontally constrained and aspect
    // ratio for the logical width is not applicable.
    auto hasConstrainedWidth = (!style.InsetLogicalLeft().IsAuto() && !style.InsetLogicalRight().IsAuto())
                               || renderer.IntrinsicLogicalWidth();
    if (hasConstrainedWidth)
    {
      return false;
    }

    // When both top and bottom are set, the out-of-flow positioned box is vertically constrained and it can
    // be used as if it had a non-auto height value.
    auto hasConstrainedHeight = !style.InsetBefore().IsAuto() && !style.InsetAfter().IsAuto();
    if (!hasConstrainedHeight)
    {
      return false;
    }
    // FIXME: This could probably be omitted and let the callers handle the height check (as they seem to be
    // doing anyway).
    return style.LogicalHeight().IsAuto();
  }

  bool RenderBox::ShouldComputeLogicalHeightFromAspectRatio() const noexcept
  {
    if (ShouldIgnoreAspectRatio())
    {
      return false;
    }

    if (ShouldComputeLogicalWidthFromAspectRatioAndInsets(*this))
    {
      return false;
    }

    auto h = Style().LogicalHeight();
    return h.IsAuto() || h.IsIntrinsic()
           || (!IsOutOfFlowPositioned() && h.IsPercentOrCalculated()
               && !PercentageLogicalHeightIsResolvable());
  }

  bool RenderBox::ShouldComputeLogicalWidthFromAspectRatio() const noexcept
  {
    if (ShouldIgnoreAspectRatio())
    {
      return false;
    }

    if (IsGridItem())
    {
      if (Is<RenderReplaced>(*this))
      {
        if (HasStretchedLogicalWidth() && HasStretchedLogicalHeight())
        {
          return false;
        }
      }
      else if (HasStretchedLogicalWidth(StretchingMode::Explicit))
      {
        return false;
      }
      if (Style().LogicalWidth().IsPercentOrCalculated() && Parent()->Style().LogicalWidth().IsFixed())
      {
        return false;
      }
    }

    auto isResolvablePercentageHeight = [&]
    {
      return Style().LogicalHeight().IsPercentOrCalculated()
             && (IsOutOfFlowPositioned() || PercentageLogicalHeightIsResolvable());
    };
    return OverridingBorderBoxLogicalHeight() || ShouldComputeLogicalWidthFromAspectRatioAndInsets(*this)
           || Style().LogicalHeight().IsFixed() || isResolvablePercentageHeight();
  }

  LayoutUnit RenderBox::ComputeLogicalWidthFromAspectRatioInternal() const noexcept
  {
    krys_debug_assert(ShouldComputeLogicalWidthFromAspectRatio());
    auto computedValues = ComputeLogicalHeight(LogicalHeight(), LogicalTop());
    LayoutUnit logicalHeightforAspectRatio = computedValues.Extent;

    return InlineSizeFromAspectRatio(HorizontalBorderAndPaddingExtent(), VerticalBorderAndPaddingExtent(),
                                     Style().LogicalAspectRatio(), Style().BoxSizingForAspectRatio(),
                                     logicalHeightforAspectRatio, Style().GetAspectRatio(),
                                     IsRenderReplaced());
  }

  LayoutUnit RenderBox::ComputeLogicalWidthFromAspectRatio() const noexcept
  {
    auto logicalWidth = ComputeLogicalWidthFromAspectRatioInternal();
    LayoutUnit containerWidthInInlineDirection =
      std::max<LayoutUnit>(0_lu, ContainingBlockLogicalWidthForContent());
    return ConstrainLogicalWidthByMinMax(logicalWidth, containerWidthInInlineDirection, *ContainingBlock(),
                                         AllowIntrinsic::No);
  }

  bool RenderBox::IsRenderReplacedWithIntrinsicRatio() const noexcept
  {
    if (auto *replaced = DynamicDowncast<RenderReplaced>(*this))
    {
      return static_cast<bool>(replaced->ComputeIntrinsicAspectRatio());
    }
    return false;
  }

  Maybe<double> RenderBox::ResolveAspectRatio() const noexcept
  {
    if (auto *replacedElement = DynamicDowncast<RenderReplaced>(*this))
    {
      return replacedElement->ComputeIntrinsicAspectRatio();
    }
    if (Style().HasAspectRatio())
    {
      return Style().LogicalAspectRatio();
    }
    krys_debug_assert(false);
    return null;
  }

  Pair<LayoutUnit, LayoutUnit> RenderBox::ComputeMinMaxLogicalWidthFromAspectRatio() const noexcept
  {
    LayoutUnit transferredMinSize = LayoutUnit();
    LayoutUnit transferredMaxSize = LayoutUnit::Max();
    Maybe<double> aspectRatio = ResolveAspectRatio();
    if (!aspectRatio)
    {
      return {transferredMinSize, transferredMaxSize};
    }

    if (Style().LogicalMinHeight().IsSpecified())
    {
      if (LayoutUnit blockMinSize = ConstrainLogicalHeightByMinMax(LayoutUnit(), null);
          blockMinSize > LayoutUnit())
      {
        transferredMinSize = InlineSizeFromAspectRatio(
          BorderAndPaddingLogicalWidth(), BorderAndPaddingLogicalHeight(), *aspectRatio,
          Style().BoxSizingForAspectRatio(), blockMinSize, Style().GetAspectRatio(), IsRenderReplaced());
      }
    }
    if (Style().LogicalMaxHeight().IsSpecified())
    {
      if (LayoutUnit blockMaxSize = ConstrainLogicalHeightByMinMax(LayoutUnit::Max(), null);
          blockMaxSize != LayoutUnit::Max())
      {
        transferredMaxSize = InlineSizeFromAspectRatio(
          BorderAndPaddingLogicalWidth(), BorderAndPaddingLogicalHeight(), *aspectRatio,
          Style().BoxSizingForAspectRatio(), blockMaxSize, Style().GetAspectRatio(), IsRenderReplaced());
      }
    }
    // Spec says the transferred max size should be floored by the transferred min size
    transferredMaxSize = std::max(transferredMinSize, transferredMaxSize);
    return {transferredMinSize, transferredMaxSize};
  }

  Pair<LayoutUnit, LayoutUnit> RenderBox::ComputeMinMaxLogicalHeightFromAspectRatio() const noexcept
  {
    LayoutUnit transferredMinSize = LayoutUnit();
    LayoutUnit transferredMaxSize = LayoutUnit::Max();
    Maybe<double> aspectRatio = ResolveAspectRatio();
    if (!aspectRatio)
    {
      return {transferredMinSize, transferredMaxSize};
    }

    if (Style().LogicalMinWidth().IsSpecified())
    {
      if (LayoutUnit inlineMinSize = ComputeLogicalWidthUsing(
            Style().LogicalMinWidth(), ContainingBlockLogicalWidthForContent(), *ContainingBlock());
          inlineMinSize > LayoutUnit())
      {
        transferredMinSize = BlockSizeFromAspectRatio(
          BorderAndPaddingLogicalWidth(), BorderAndPaddingLogicalHeight(), *aspectRatio,
          Style().BoxSizingForAspectRatio(), inlineMinSize, Style().GetAspectRatio(), IsRenderReplaced());
      }
    }

    if (Style().LogicalMaxWidth().IsSpecified())
    {
      if (LayoutUnit inlineMaxSize = ComputeLogicalWidthUsing(
            Style().LogicalMaxWidth(), ContainingBlockLogicalWidthForContent(), *ContainingBlock());
          inlineMaxSize != LayoutUnit::Max())
      {
        transferredMaxSize = BlockSizeFromAspectRatio(
          BorderAndPaddingLogicalWidth(), BorderAndPaddingLogicalHeight(), *aspectRatio,
          Style().BoxSizingForAspectRatio(), inlineMaxSize, Style().GetAspectRatio(), IsRenderReplaced());
      }
    }
    // Spec says the transferred max size should be floored by the transferred min size
    transferredMaxSize = std::max(transferredMinSize, transferredMaxSize);
    return {transferredMinSize, transferredMaxSize};
  }

#pragma endregion

  const RenderBox &RenderBox::EnclosingClippingBoxForStickyPosition() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContainingBlockRangeSizeForPositioned(const RenderBoxModelObject &,
                                                              BoxAxis) const noexcept
  {
    krys_unreachable();
  }
}
