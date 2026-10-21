#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLLegendElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/BackgroundPainter.hpp"
#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/PositionedLayoutConstraints.hpp"
#include "Krystal.Booey/Rendering/RenderAncestorIterator.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderDeprecatedFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Style/Values/Masking/ClipPath.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollPadding.hpp"
#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"
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
    // FIXME: Stop autoscrolling, and unregister the box's scroll snap positions, anchors, position-try box
    // and anchor scroll adjusters, once there are any.
    if (HasInitializedStyle() && Style().GetContainerType() != ContainerType::Normal)
    {
      View().UnregisterContainerQueryBox(*this);
    }

    // The weak maps upstream forget the box by themselves.
    OverridingLogicalHeightMap().erase(this);
    OverridingLogicalWidthMap().erase(this);
    OverridingLogicalHeightMapForFlexBasisComputation().erase(this);
    OverridingLogicalWidthMapForFlexBasisComputation().erase(this);
    GridAreaContentLogicalHeightMap().erase(this);
    GridAreaContentLogicalWidthMap().erase(this);
    RenderBlock::RemovePercentHeightDescendant(*this);
    RenderBlock::RemoveOutOfFlowBox(*this);
    RemoveShapeOutsideInfo();

    RenderBoxModelObject::WillBeDestroyed();
  }

  namespace
  {
    KRYS_NODISCARD RenderBlockFlow *OutermostBlockContainingFloatingObject(RenderBox &box) noexcept
    {
      krys_debug_assert(box.IsFloating());
      RenderBlockFlow *parentBlock = nullptr;
      for (auto &ancestor : AncestorsOfType<RenderBlockFlow>(box))
      {
        if (!parentBlock || ancestor.ContainsFloat(box))
        {
          parentBlock = &ancestor;
        }
      }
      return parentBlock;
    }
  }

  void RenderBox::RemoveFloatingAndInvalidateForLayout() noexcept
  {
    krys_debug_assert(IsFloating());

    if (RenderTreeBeingDestroyed())
    {
      return;
    }

    if (auto *ancestor = OutermostBlockContainingFloatingObject(*this))
    {
      ancestor->MarkSiblingsWithFloatsForLayout(this);
      ancestor->MarkAllDescendantsWithFloatsForLayout(this, false);
    }
  }

  void RenderBox::RemoveFloatingOrOutOfFlowChildFromBlockLists() noexcept
  {
    krys_debug_assert(!RenderTreeBeingDestroyed());

    if (IsFloating())
    {
      return RemoveFloatingAndInvalidateForLayout();
    }

    if (IsOutOfFlowPositioned())
    {
      return RenderBlock::RemoveOutOfFlowBox(*this);
    }

    krys_debug_assert(false);
  }

  void RenderBox::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    s_hadNonVisibleOverflow = HasNonVisibleOverflow();

    const RenderStyle *oldStyle = HasInitializedStyle() ? &Style() : nullptr;
    if (oldStyle)
    {
      // The background of the root element or the body element could propagate up to
      // the canvas. Issue full repaint, when our style changes substantially.
      if (diff >= style::DifferenceResult::Repaint && (IsDocumentElementRenderer() || IsBody()))
      {
        View().RepaintRootContents();
        if (style::HasEntirelyFixedBackground(oldStyle->GetBackgroundLayers())
            != style::HasEntirelyFixedBackground(newStyle.GetBackgroundLayers()))
        {
          View().Compositor().RootLayerConfigurationChanged();
        }
      }

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
          if (auto *flexContainer = DynamicDowncast<RenderFlexibleBox>(Parent()))
          {
            flexContainer->ClearCachedFlexItemIntrinsicContentLogicalHeight(*this);
            flexContainer->ClearCachedMainSizeForFlexItem(*this);
          }
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
        if (IsFloating() && !IsOutOfFlowPositioned() && newStyle.HasOutOfFlowPosition())
        {
          RemoveFloatingOrOutOfFlowChildFromBlockLists();
        }
      }
    }

    // FIXME: Scroll snap positions, position-try boxes and anchor-positioned boxes, once there are any.
    if (newStyle.GetContainerType() != ContainerType::Normal)
    {
      View().RegisterContainerQueryBox(*this);
    }
    else if (oldStyle && oldStyle->GetContainerType() != ContainerType::Normal)
    {
      View().UnregisterContainerQueryBox(*this);
    }

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

    // FIXME: The view's scrollbar overlay style and the compositor, once there are any.

    if ((oldStyle && !oldStyle->GetShapeOutside().IsNone()) || !Style().GetShapeOutside().IsNone())
    {
      UpdateShapeOutsideInfoAfterStyleChange(Style(), oldStyle);
    }
    UpdateGridPositionAfterStyleChange(Style(), oldStyle);

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
        if (!s_hadNonVisibleOverflow && HasRenderOverflow())
        {
          // Erase the overflow.
          // Overflow changes have to result in immediate repaints of the entire layout overflow area because
          // repaints issued by removal of descendants get clipped using the updated style when they
          // shouldn't.
          IssueRepaint(VisualOverflowRect(), ClipRepaintToLayer::Yes, ForceRepaint::Yes);
          IssueRepaint(LayoutOverflowRect(), ClipRepaintToLayer::Yes, ForceRepaint::Yes);
        }
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
    if (HasPotentiallyScrollableOverflow() && Layer())
    {
      return Layer()->ScrollWidth();
    }
    // For objects with visible overflow, this matches IE.
    if (Style().GetWritingMode().IsLogicalLeftInlineStart())
    {
      // FIXME: This should use snappedIntSize() instead with absolute coordinates.
      return RoundToInt(std::max(ClientWidth(), LayoutOverflowRect().MaxX() - BorderLeft()));
    }
    return RoundToInt(ClientWidth() - std::min<LayoutUnit>(0_lu, LayoutOverflowRect().X() - BorderLeft()));
  }

  int32 RenderBox::ScrollHeight() const noexcept
  {
    if (HasPotentiallyScrollableOverflow() && Layer())
    {
      return Layer()->ScrollHeight();
    }
    // For objects with visible overflow, this matches IE.
    // FIXME: Need to work right with writing modes.
    // FIXME: This should use snappedIntSize() instead with absolute coordinates.
    return RoundToInt(std::max(ClientHeight(), LayoutOverflowRect().MaxY() - BorderTop()));
  }

  ScrollPosition RenderBox::GetScrollPosition() const noexcept
  {
    if (!HasPotentiallyScrollableOverflow())
    {
      return {0, 0};
    }

    krys_debug_assert(HasLayer());
    auto *scrollableArea = Layer()->GetScrollableArea();
    if (!scrollableArea)
    {
      return {0, 0};
    }

    return scrollableArea->GetScrollPosition();
  }

  void RenderBox::SetScrollLeft(int32 newLeft, const ScrollPositionChangeOptions &options) noexcept
  {
    if (!HasPotentiallyScrollableOverflow() || !Layer())
    {
      return;
    }
    auto *scrollableArea = Layer()->GetScrollableArea();
    krys_debug_assert(scrollableArea);
    // FIXME: The wheel event monitor (setupWheelEventMonitor), once there are wheel events.
    scrollableArea->ScrollToXPosition(newLeft, options);
  }

  void RenderBox::SetScrollTop(int32 newTop, const ScrollPositionChangeOptions &options) noexcept
  {
    if (!HasPotentiallyScrollableOverflow() || !Layer())
    {
      return;
    }
    auto *scrollableArea = Layer()->GetScrollableArea();
    krys_debug_assert(scrollableArea);
    // FIXME: The wheel event monitor (setupWheelEventMonitor), once there are wheel events.
    scrollableArea->ScrollToYPosition(newTop, options);
  }

  void RenderBox::SetScrollPosition(const ScrollPosition &position,
                                    const ScrollPositionChangeOptions &options) noexcept
  {
    if (!HasPotentiallyScrollableOverflow() || !Layer())
    {
      return;
    }
    auto *scrollableArea = Layer()->GetScrollableArea();
    krys_debug_assert(scrollableArea);
    // FIXME: The wheel event monitor (setupWheelEventMonitor), once there are wheel events.
    scrollableArea->SetScrollPosition(position, options);
  }

  void RenderBox::BoundingRects(List<LayoutRect> &rects, const LayoutPoint &accumulatedOffset) const noexcept
  {
    rects.push_back({accumulatedOffset, Size()});
  }

  void RenderBox::AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed) const noexcept
  {
    // FIXME: Boxes in a fragmented flow (absoluteQuadsForBox), once there are fragmented flows.
    auto localRect = FloatRect {0, 0, Width().ToFloat(), Height().ToFloat()};
    quads.push_back(LocalToAbsoluteQuad(localRect, MapCoordinatesMode::UseTransforms, wasFixed));
  }

  FloatQuad RenderBox::AbsoluteContentQuad() const noexcept
  {
    auto rect = ContentBoxRect();
    return LocalToAbsoluteQuad(FloatRect(rect));
  }

  void RenderBox::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                      TransformState &transformState, EnumSet<MapCoordinatesMode> mode,
                                      bool *wasFixed) const noexcept
  {
    if (ancestorContainer == this)
    {
      return;
    }

    if (!ancestorContainer && View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled())
    {
      auto *layoutState = View().FrameView().LayoutContext().LayoutState();
      auto offset = layoutState->PaintOffset() + LocationOffset();
      if (Style().HasInFlowPosition() && Layer())
      {
        offset += Layer()->OffsetForInFlowPosition();
      }
      transformState.Move(offset);
      return;
    }

    bool containerSkipped = false;
    auto *container = Container(ancestorContainer, containerSkipped);
    if (!container)
    {
      return;
    }

    bool isFixedPos = IsFixedPositioned();
    // If this box has a transform, it acts as a fixed position container for fixed descendants,
    // and may itself also be fixed position. So propagate 'fixed' up only if this box is fixed position.
    if (isFixedPos)
    {
      mode.Add(MapCoordinatesMode::IsFixed);
    }
    else if (mode.Contains(MapCoordinatesMode::IsFixed) && CanContainFixedPositionObjects())
    {
      mode.Remove(MapCoordinatesMode::IsFixed);
    }

    if (wasFixed)
    {
      *wasFixed = mode.Contains(MapCoordinatesMode::IsFixed);
    }

    auto containerOffset = OffsetFromContainer(*container, LayoutPoint(transformState.MappedPoint()));

    // FIXME: Remove the sticky position offset when sticky offsets are ignored (IgnoreStickyOffsets), once
    // there's sticky positioning.

    PushOntoTransformState(transformState, mode, ancestorContainer, container, containerOffset,
                           containerSkipped);
    if (containerSkipped)
    {
      return;
    }

    mode.Remove(MapCoordinatesMode::ApplyContainerFlip);

    container->MapLocalToContainer(ancestorContainer, transformState, mode, wasFixed);
  }

  const RenderElement *RenderBox::PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                         RenderGeometryMap &geometryMap) const noexcept
  {
    krys_debug_assert(ancestorToStopAt != this);

    bool ancestorSkipped;
    RenderElement *container = this->Container(ancestorToStopAt, ancestorSkipped);
    if (!container)
    {
      return nullptr;
    }

    PushOntoGeometryMap(geometryMap, ancestorToStopAt, container, ancestorSkipped);
    return ancestorSkipped ? ancestorToStopAt : container;
  }

  void RenderBox::MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                          TransformState &transformState) const noexcept
  {
    bool isFixedPos = IsFixedPositioned();
    if (isFixedPos)
    {
      mode.Add(MapCoordinatesMode::IsFixed);
    }
    else if (mode.Contains(MapCoordinatesMode::IsFixed) && CanContainFixedPositionObjects())
    {
      // If this box has a transform, it acts as a fixed position container for fixed descendants,
      // and may itself also be fixed position. So propagate 'fixed' up only if this box is fixed position.
      mode.Remove(MapCoordinatesMode::IsFixed);
    }

    RenderBoxModelObject::MapAbsoluteToLocalPoint(mode, transformState);
  }

  LayoutSize RenderBox::OffsetFromContainer(KRYS_MAYBE_UNUSED const RenderElement &container,
                                            const LayoutPoint &, bool *offsetDependsOnPoint) const noexcept
  {
    krys_debug_assert(&container == Container());

    LayoutSize offset;
    if (IsInFlowPositioned())
    {
      offset += OffsetForInFlowPosition();
    }

    if (!IsInline() || IsBlockLevelReplacedOrAtomicInline())
    {
      offset += TopLeftLocationOffset();
    }

    if (auto *boxContainer = DynamicDowncast<RenderBox>(container))
    {
      offset -= ToLayoutSize(LayoutPoint(boxContainer->GetScrollPosition()));
    }

    if (IsAbsolutelyPositioned() && container.IsInFlowPositioned())
    {
      if (auto *inlineContainer = DynamicDowncast<RenderInline>(container))
      {
        offset += inlineContainer->OffsetForInFlowPositionedInline(this);
      }
    }

    // FIXME: Fragmented flows make the offset depend on the point, once there are those.
    (void)offsetDependsOnPoint;

    return offset;
  }

  LayoutBoxExtent RenderBox::ScrollPaddingForViewportRect(const LayoutRect &viewportRect) const noexcept
  {
    return style::ExtentForRect(Style().GetScrollPaddingBox(), viewportRect, style::ZoomNeeded {});
  }

  int32 RenderBox::ScrollLeft() const noexcept
  {
    auto *scrollableArea = Layer() ? Layer()->GetScrollableArea() : nullptr;
    return (HasNonVisibleOverflow() && scrollableArea) ? scrollableArea->GetScrollPosition().X() : 0;
  }

  int32 RenderBox::ScrollTop() const noexcept
  {
    auto *scrollableArea = Layer() ? Layer()->GetScrollableArea() : nullptr;
    return (HasNonVisibleOverflow() && scrollableArea) ? scrollableArea->GetScrollPosition().Y() : 0;
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

  LayoutUnit RenderBox::AdjustBorderBoxLogicalWidthForBoxSizing(
    const style::Length<css::NonNegativeUnzoomed> &logicalWidth) const noexcept
  {
    return AdjustBorderBoxLogicalWidthForBoxSizing(
      style::Evaluate<LayoutUnit>(logicalWidth, Style().UsedZoomForLength()));
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

  LayoutUnit RenderBox::AdjustContentBoxLogicalWidthForBoxSizing(
    const style::Length<css::NonNegativeUnzoomed> &logicalWidth) const noexcept
  {
    return AdjustContentBoxLogicalWidthForBoxSizing(
      style::Evaluate<LayoutUnit>(logicalWidth, Style().UsedZoomForLength()));
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

    if (IsOutOfFlowPositioned())
    {
      PositionedLayoutConstraints constraints(*this, LogicalBoxAxis::Inline);
      return constraints.ContainingInlineSize();
    }

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
      // FIXME: This calculation is not patched for block-flow yet.
      // https://bugs.webkit.org/show_bug.cgi?id=46500
      ComputeOutOfFlowPositionedLogicalWidth(computedValues);
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
      auto *renderGrid = Downcast<RenderGrid>(Parent());
      return (renderGrid->AreMasonryColumns()
              && !GridLayoutFunctions::IsOrthogonalGridItem(*renderGrid, *this))
             || !HasStretchedLogicalWidth();
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

  void RenderBox::CacheIntrinsicContentLogicalHeightForFlexItem(LayoutUnit height) const noexcept
  {
    // FIXME: it should be enough with checking hasOverridingLogicalHeight() as this logic could be shared
    // by any layout system using overrides like grid or flex. However this causes a never ending sequence of
    // calls between layoutBlock() <-> relayoutToAvoidWidows().
    if (IsFloatingOrOutOfFlowPositioned())
    {
      return;
    }
    auto *flexibleBox = DynamicDowncast<RenderFlexibleBox>(Parent());
    if (!flexibleBox)
    {
      return;
    }
    if (OverridingBorderBoxLogicalHeight() || ShouldComputeLogicalHeightFromAspectRatio())
    {
      return;
    }
    flexibleBox->SetCachedFlexItemIntrinsicContentLogicalHeight(*this, height);
  }

  void RenderBox::UpdateLogicalHeight() noexcept
  {
    if (ShouldApplySizeContainment() && !IsRenderGrid())
    {
      OverrideLogicalHeightForSizeContainment();
    }

    CacheIntrinsicContentLogicalHeightForFlexItem(ContentBoxLogicalHeight());
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

    // Let's allow the table cell to compute its preferred size.
    if (auto *tableCell = DynamicDowncast<RenderTableCell>(*this);
        tableCell && !tableCell->IsComputingPreferredSize())
    {
      // Use the value set by table layout for orthogonal cells which in this case the logical width of the
      // cell from the table's point of view. see RenderTableCell::SetCellLogicalWidth.
      if (tableCell->IsOrthogonal())
      {
        computedValues.Extent = OverridingBorderBoxLogicalHeight().value_or(computedValues.Extent);
      }
      return computedValues;
    }

    if (IsOutOfFlowPositioned())
    {
      ComputeOutOfFlowPositionedLogicalHeight(computedValues);
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
      if (auto *renderImage = DynamicDowncast<RenderImage>(this))
      {
        auto computedFixedLogicalWidth = Style().LogicalWidth().TryFixed();
        if (computedFixedLogicalWidth && !Style().HasAspectRatio())
        {
          auto intrinsicRatio = renderImage->IntrinsicRatio();
          return ResolveHeightForRatio(
            BorderAndPaddingLogicalWidth(), BorderAndPaddingLogicalHeight(),
            LayoutUnit {computedFixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength())},
            intrinsicRatio.TransposedSize().AspectRatio(), BoxSizing::ContentBox);
        }
      }
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

  static bool TableCellShouldHaveZeroInitialSize(const RenderTableCell &tableCell, const RenderBox &child,
                                                 bool scrollsOverflowY) noexcept
  {
    // Normally we would let the cell size intrinsically, but scrolling overflow has to be
    // treated differently, since WinIE lets scrolled overflow fragments shrink as needed.
    // While we can't get all cases right, we can at least detect when the cell has a specified
    // height or when the table has a specified height. In these cases we want to initially have
    // no size and allow the flexing of the table or the cell to its specified height to cause us
    // to grow to fill the space. This could end up being wrong in some cases, but it is
    // preferable to the alternative (sizing intrinsically and making the row end up too big).
    if (!scrollsOverflowY)
    {
      return false;
    }
    if (tableCell.Style().LogicalHeight().IsAuto() && tableCell.Table()->Style().LogicalHeight().IsAuto())
    {
      return false;
    }
    if (child.IsBlockLevelReplacedOrAtomicInline())
    {
      return false;
    }
    // FIXME: Form controls other than fieldsets (HTMLFormControlElement), once there are those.
    return true;
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
        return TableCellShouldHaveZeroInitialSize(*tableCell, *this, ScrollsOverflowY()) ? Maybe(0_lu) : null;
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

    if (logicalHeight.IsPercentOrCalculated() && IsOutOfFlowPositioned() && !IsRenderFragmentedFlow())
    {
      PositionedLayoutConstraints constraints(*this, LogicalBoxAxis::Block);
      return AdjustContentBoxLogicalHeightForBoxSizing(style::Evaluate<LayoutUnit>(
        logicalHeight, constraints.ContainingSize(), Style().UsedZoomForLength()));
    }

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

    // The legacy scrollbar style (::-webkit-scrollbar) was dropped.
    return !ScrollbarTheme::Theme().UsesOverlayScrollbars();
  }

  bool RenderBox::HasIntrinsicAspectRatio() const noexcept
  {
    // FIXME: Video, canvas and view transition captures, once they have renderers.
    return IsBlockLevelReplacedOrAtomicInline() && IsRenderImage();
  }

#pragma endregion

#pragma region Scrollbars

  bool RenderBox::IncludeVerticalScrollbarSize() const noexcept
  {
    return HasNonVisibleOverflow() && Layer() && !Layer()->HasOverlayScrollbars()
           && (Style().OverflowY() == Overflow::Scroll || Style().OverflowY() == Overflow::Auto
               || (Style().OverflowY() == Overflow::Hidden && !Style().GetScrollbarGutter().IsAuto()));
  }

  bool RenderBox::IncludeHorizontalScrollbarSize() const noexcept
  {
    return HasNonVisibleOverflow() && Layer() && !Layer()->HasOverlayScrollbars()
           && (Style().OverflowX() == Overflow::Scroll || Style().OverflowX() == Overflow::Auto
               || (Style().OverflowX() == Overflow::Hidden && !Style().GetScrollbarGutter().IsAuto()));
  }

  bool RenderBox::CanUseOverlayScrollbars() const noexcept
  {
    // The legacy scrollbar style (::-webkit-scrollbar) was dropped.
    return ScrollbarTheme::Theme().UsesOverlayScrollbars();
  }

  bool RenderBox::HasAutoScrollbar(ScrollbarOrientation orientation) const noexcept
  {
    if (!HasNonVisibleOverflow())
    {
      return false;
    }

    auto isAutoOrScrollWithOverlayScrollbar = [&](Overflow overflow)
    {
      return overflow == Overflow::Auto || (overflow == Overflow::Scroll && CanUseOverlayScrollbars());
    };

    switch (orientation)
    {
      case ScrollbarOrientation::Horizontal: return isAutoOrScrollWithOverlayScrollbar(Style().OverflowX());
      case ScrollbarOrientation::Vertical:   return isAutoOrScrollWithOverlayScrollbar(Style().OverflowY());
    }
    return false;
  }

  bool RenderBox::HasAlwaysPresentScrollbar(ScrollbarOrientation orientation) const noexcept
  {
    if (!HasNonVisibleOverflow())
    {
      return false;
    }

    auto isAlwaysVisibleScrollbar = [&](Overflow overflow)
    {
      return overflow == Overflow::Scroll && !CanUseOverlayScrollbars();
    };

    switch (orientation)
    {
      case ScrollbarOrientation::Horizontal: return isAlwaysVisibleScrollbar(Style().OverflowX());
      case ScrollbarOrientation::Vertical:   return isAlwaysVisibleScrollbar(Style().OverflowY());
    }
    return false;
  }

  bool RenderBox::CanBeProgramaticallyScrolled() const noexcept
  {
    if (IsRenderView())
    {
      return true;
    }

    if (!HasPotentiallyScrollableOverflow())
    {
      return false;
    }

    if (HasScrollableOverflowX() || HasScrollableOverflowY())
    {
      return true;
    }

    // FIXME: Editable elements too (hasEditableStyle), once there's editing.
    return false;
  }

  bool RenderBox::ScrollLayer(ScrollDirection direction, ScrollGranularity granularity, uint32 stepCount,
                              dom::Element **stopElement) noexcept
  {
    auto *scrollableArea = Layer() ? Layer()->GetScrollableArea() : nullptr;
    if (scrollableArea && scrollableArea->Scroll(direction, granularity, stepCount))
    {
      if (stopElement)
      {
        *stopElement = GetElement();
      }

      return true;
    }

    return false;
  }

  bool RenderBox::Scroll(ScrollDirection direction, ScrollGranularity granularity, uint32 stepCount,
                         dom::Element **stopElement) noexcept
  {
    if (ScrollLayer(direction, granularity, stepCount, stopElement))
    {
      return true;
    }

    if (stopElement && *stopElement && *stopElement == GetElement())
    {
      return true;
    }

    // FIXME: Autoscrolling near the edge of a box (the start box and the wheel event's point), once there's
    // autoscrolling.
    RenderBlock *nextScrollBlock = ContainingBlock();

    if (nextScrollBlock && !nextScrollBlock->IsRenderView())
    {
      return nextScrollBlock->Scroll(direction, granularity, stepCount, stopElement);
    }

    return false;
  }

  bool RenderBox::LogicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity,
                                uint32 stepCount, dom::Element **stopElement) noexcept
  {
    if (auto *scrollableArea = Layer() ? Layer()->GetScrollableArea() : nullptr)
    {
      if (scrollableArea->Scroll(
            LogicalToPhysical(direction, IsHorizontalWritingMode(), GetWritingMode().IsBlockFlipped()),
            granularity, stepCount))
      {
        if (stopElement)
        {
          *stopElement = GetElement();
        }
        return true;
      }
    }

    if (stopElement && *stopElement && *stopElement == GetElement())
    {
      return true;
    }

    RenderBlock *b = ContainingBlock();
    if (b && !b->IsRenderView())
    {
      return b->LogicalScroll(direction, granularity, stepCount, stopElement);
    }
    return false;
  }

  bool RenderBox::CanBeScrolledAndHasScrollableArea() const noexcept
  {
    return CanBeProgramaticallyScrolled() && (HasHorizontalOverflow() || HasVerticalOverflow());
  }

  bool RenderBox::RequiresLayerWithScrollableArea() const noexcept
  {
    // FIXME: This is wrong; these boxes' layers should not need ScrollableAreas via RenderLayer.
    if (IsRenderView() || IsDocumentElementRenderer())
    {
      return true;
    }

    if (HasPotentiallyScrollableOverflow())
    {
      return true;
    }

    if (Style().GetResize() != style::Resize::None)
    {
      return true;
    }

    // FIXME: Marquees with a marquee behavior, once there are marquees (the marquee properties were dropped,
    // so the behavior is always the initial `scroll`).
    if (IsHTMLMarquee())
    {
      return true;
    }

    return false;
  }

  int32 RenderBox::VerticalScrollbarWidth() const noexcept
  {
    auto *scrollableArea = Layer() ? Layer()->GetScrollableArea() : nullptr;
    if (!scrollableArea)
    {
      return 0;
    }
    return IncludeVerticalScrollbarSize()
             ? scrollableArea->VerticalScrollbarWidth(
                 OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize, IsHorizontalWritingMode())
             : 0;
  }

  int32 RenderBox::HorizontalScrollbarHeight() const noexcept
  {
    auto *scrollableArea = Layer() ? Layer()->GetScrollableArea() : nullptr;
    if (!scrollableArea)
    {
      return 0;
    }
    return IncludeHorizontalScrollbarSize()
             ? scrollableArea->HorizontalScrollbarHeight(
                 OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize, IsHorizontalWritingMode())
             : 0;
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

#pragma region Repaint

  LayoutRect RenderBox::LocalOutlineBoundsRepaintRect() const noexcept
  {
    auto box = BorderBoundingBox();
    return ApplyVisualEffectOverflow(box);
  }

  LayoutRect RenderBox::OutlineBoundsForRepaint(const RenderLayerModelObject *repaintContainer) const noexcept
  {
    auto box = LocalOutlineBoundsRepaintRect();

    if (repaintContainer != this)
    {
      // FIXME: The geometry map, once there is one.
      auto containerRelativeQuad = LocalToContainerQuad(FloatQuad(FloatRect(box)), repaintContainer);
      box = LayoutRect(containerRelativeQuad.BoundingBox());
    }

    // FIXME: layoutDelta needs to be applied in parts before/after transforms and
    // repaint containers. https://bugs.webkit.org/show_bug.cgi?id=23308
    box.Move(View().FrameView().LayoutContext().LayoutDelta());

    return LayoutRect(SnapRectToDevicePixels(box, GetDocument().DeviceScaleFactor()));
  }

  auto RenderBox::LocalRectsForRepaint(RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    if (IsInsideEntirelyHiddenLayer())
    {
      return {};
    }

    auto overflowRect = VisualOverflowRect();
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

  auto RenderBox::ComputeVisibleRectsUsingPaintOffset(const RepaintRects &rects) const noexcept
    -> RepaintRects
  {
    auto adjustedRects = rects;
    auto *layoutState = View().FrameView().LayoutContext().LayoutState();

    if (HasLayer() && Layer()->Transform())
    {
      adjustedRects.Transform(*Layer()->Transform(), GetDocument().DeviceScaleFactor());
    }

    // We can't trust the bits on RenderObject, because this might be called while re-resolving style.
    if (Style().HasInFlowPosition() && Layer())
    {
      adjustedRects.Move(Layer()->OffsetForInFlowPosition());
    }

    adjustedRects.MoveBy(Location());
    adjustedRects.Move(layoutState->PaintOffset());
    if (layoutState->IsClipped())
    {
      adjustedRects.ClippedOverflowRect.Intersect(layoutState->ClipRect());
    }
    return adjustedRects;
  }

  auto RenderBox::ComputeVisibleRectsInContainer(const RepaintRects &rects,
                                                 const RenderLayerModelObject *container,
                                                 VisibleRectContext context) const noexcept
    -> Maybe<RepaintRects>
  {
    // The rect we compute at each step is shifted by our x/y offset in the parent container's coordinate
    // space. Only when we cross a writing mode boundary will we have to possibly flipForWritingMode (to
    // convert into a more appropriate offset corner for the enclosing container).  This allows for a fully RL
    // or BT document to repaint properly even during layout, since the rect remains flipped all the way until
    // the end.
    //
    // RenderView::computeVisibleRectInContainer then converts the rect to physical coordinates. We also
    // convert to physical when we hit a repaint container boundary. Therefore the final rect returned is
    // always in the physical coordinate space of the container.
    const RenderStyle &styleToUse = Style();
    // Paint offset cache is only valid for root-relative, non-fixed position repainting
    if (View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled() && !container
        && styleToUse.GetPosition() != PositionType::Fixed
        && !context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
    {
      return ComputeVisibleRectsUsingPaintOffset(rects);
    }

    auto adjustedRects = rects;
    // FIXME: Reflections (`-webkit-box-reflect` was dropped).

    if (container == this)
    {
      if (container->GetWritingMode().IsBlockFlipped())
      {
        FlipForWritingMode(adjustedRects);
      }
      if (context.DescendantNeedsEnclosingIntRect)
      {
        adjustedRects.EncloseToIntRects();
      }
      return adjustedRects;
    }

    bool containerIsSkipped;
    auto *localContainer = Container(container, containerIsSkipped);
    if (!localContainer)
    {
      return adjustedRects;
    }

    if (IsWritingModeRoot())
    {
      if (!IsOutOfFlowPositioned() || !context.DirtyRectIsFlipped)
      {
        FlipForWritingMode(adjustedRects);
        context.DirtyRectIsFlipped = true;
      }
    }

    auto locationOffset = LocationOffset();

    // FIXME: Widgets (which snap to integral sizes and positions) and multi-column flows (which map the rect
    // into their fragment), once there are any.

    // We are now in our parent container's coordinate space. Apply our transform to obtain a bounding box
    // in the parent's coordinate space that encloses us.
    auto position = styleToUse.GetPosition();
    if (HasLayer() && Layer()->IsTransformed())
    {
      context.HasPositionFixedDescendant = position == PositionType::Fixed;
      adjustedRects.Transform(Layer()->CurrentTransform(), GetDocument().DeviceScaleFactor());
    }
    else if (position == PositionType::Fixed)
    {
      context.HasPositionFixedDescendant = true;
    }

    adjustedRects.Move(locationOffset);

    if (position == PositionType::Absolute && localContainer->IsInFlowPositioned()
        && Is<RenderInline>(*localContainer))
    {
      auto offsetForInFlowPosition =
        Downcast<RenderInline>(*localContainer).OffsetForInFlowPositionedInline(this);
      adjustedRects.Move(offsetForInFlowPosition);
    }
    else if (styleToUse.HasInFlowPosition() && Layer())
    {
      // Apply the relative position offset when invalidating a rectangle.  The layer
      // is translated, but the render box isn't, so we need to do this to get the
      // right dirty rect.  Since this is called from RenderObject::setStyle, the relative position
      // flag on the RenderObject has been cleared, so use the one on the style().
      auto offsetForInFlowPosition = Layer()->OffsetForInFlowPosition();
      adjustedRects.Move(offsetForInFlowPosition);
    }

    if (localContainer->HasNonVisibleOverflow())
    {
      bool isEmpty = !Downcast<RenderLayerModelObject>(*localContainer)
                        .ApplyCachedClipAndScrollPosition(adjustedRects, container, context);
      if (isEmpty)
      {
        if (context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
        {
          return null;
        }
        return adjustedRects;
      }
    }

    if (containerIsSkipped)
    {
      // If the container is below localContainer, then we need to map the rect into container's coordinates.
      LayoutSize containerOffset = container->OffsetFromAncestorContainer(*localContainer);
      adjustedRects.Move(-containerOffset);
      return adjustedRects;
    }
    return localContainer->ComputeVisibleRectsInContainer(adjustedRects, container, context);
  }

  void RenderBox::RepaintDuringLayoutIfMoved(const LayoutRect &oldRect) noexcept
  {
    if (oldRect.Location() != _frameRect.Location())
    {
      LayoutRect newRect = _frameRect;
      // The child moved.  Invalidate the object's old and new positions.  We have to do this
      // since the object may not have gotten a layout.
      _frameRect = oldRect;
      Repaint();
      RepaintOverhangingFloats(true);
      _frameRect = newRect;
      Repaint();
      RepaintOverhangingFloats(true);
    }
  }

  void RenderBox::RepaintOverhangingFloats(bool) noexcept
  {
  }

  void RenderBox::FlipForWritingMode(RepaintRects &rects) const noexcept
  {
    if (!GetWritingMode().IsBlockFlipped())
    {
      return;
    }

    rects.FlipForWritingMode(Size(), IsHorizontalWritingMode());
  }

  LayoutSize RenderBox::CachedSizeForOverflowClip() const noexcept
  {
    krys_debug_assert(HasNonVisibleOverflow());
    krys_debug_assert(HasLayer());
    return LayoutSize(Layer()->Size());
  }

  bool RenderBox::UsesCompositedScrolling() const noexcept
  {
    return HasNonVisibleOverflow() && HasLayer() && Layer()->UsesCompositedScrolling();
  }

  bool RenderBox::ApplyCachedClipAndScrollPosition(RepaintRects &rects,
                                                   const RenderLayerModelObject *container,
                                                   VisibleRectContext context) const noexcept
  {
    FlipForWritingMode(rects);

    if (context.Options.Contains(VisibleRectContext::Option::ApplyCompositedContainerScrolls)
        || this != container || !UsesCompositedScrolling())
    {
      rects.MoveBy(-LayoutPoint(GetScrollPosition())); // For overflow:auto/scroll/hidden.
    }

    // Do not clip scroll layer contents to reduce the number of repaints while scrolling.
    if ((!context.Options.Contains(VisibleRectContext::Option::ApplyCompositedClips)
         && UsesCompositedScrolling())
        || (!context.Options.Contains(VisibleRectContext::Option::ApplyContainerClip) && this == container))
    {
      FlipForWritingMode(rects);
      return true;
    }

    // height() is inaccurate if we're in the middle of a layout of this RenderBox, so use the
    // layer's size instead. Even if the layer's size is wrong, the layer itself will repaint
    // anyway if its size does change.
    LayoutRect clipRect(LayoutPoint(), CachedSizeForOverflowClip());
    if (EffectiveOverflowX() == Overflow::Visible)
    {
      clipRect.ExpandToInfiniteX();
    }
    if (EffectiveOverflowY() == Overflow::Visible)
    {
      clipRect.ExpandToInfiniteY();
    }

    // FIXME: The scroll margin (for intersection observers), once there are any.

    bool intersects;
    if (context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
    {
      intersects = rects.EdgeInclusiveIntersect(clipRect);
    }
    else
    {
      intersects = rects.Intersect(clipRect);
    }

    FlipForWritingMode(rects);
    return intersects;
  }

#pragma endregion

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

  void RenderBox::AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions> options) noexcept
  {
    for (auto &child : ChildrenOfType<RenderBox>(*this))
    {
      if (!child.IsFloatingOrOutOfFlowPositioned())
      {
        AddOverflowFromContainedBox(child, options);
      }
    }
  }

  void RenderBox::AddOverflowFromFloatBox(const FloatingObject &floatBox) noexcept
  {
    if (!floatBox.Renderer())
    {
      return;
    }
    AddOverflowWithRendererOffset(*floatBox.Renderer(), floatBox.LocationOffsetOfBorderBox());
  }

  static bool HasEquivalentGridPositioningStyle(const RenderStyle &style,
                                                const RenderStyle &oldStyle) noexcept
  {
    return oldStyle.GridItemColumnStart() == style.GridItemColumnStart()
           && oldStyle.GridItemColumnEnd() == style.GridItemColumnEnd()
           && oldStyle.GridItemRowStart() == style.GridItemRowStart()
           && oldStyle.GridItemRowEnd() == style.GridItemRowEnd() && oldStyle.GetOrder() == style.GetOrder()
           && oldStyle.HasOutOfFlowPosition() == style.HasOutOfFlowPosition()
           && (oldStyle.GridTemplateColumns().Subgrid == style.GridTemplateColumns().Subgrid
               || style.GridTemplateColumns().OrderedNamedLines.Map.empty())
           && (oldStyle.GridTemplateRows().Subgrid == style.GridTemplateRows().Subgrid
               || style.GridTemplateRows().OrderedNamedLines.Map.empty());
  }

  void RenderBox::UpdateGridPositionAfterStyleChange(const RenderStyle &style,
                                                     const RenderStyle *oldStyle) noexcept
  {
    if (!oldStyle)
    {
      return;
    }
    auto *parentGrid = DynamicDowncast<RenderGrid>(Parent());
    if (!parentGrid)
    {
      return;
    }

    // Positioned items don't participate on the layout of the grid,
    // so we don't need to mark the grid as dirty if they change positions.
    if ((oldStyle->HasOutOfFlowPosition() && style.HasOutOfFlowPosition())
        || HasEquivalentGridPositioningStyle(style, *oldStyle))
    {
      return;
    }

    // It should be possible to not dirty the grid in some cases (like moving an
    // explicitly placed grid item).
    // For now, it's more simple to just always recompute the grid.
    parentGrid->SetNeedsItemPlacement();
  }

  void RenderBox::UpdateShapeOutsideInfoAfterStyleChange(const RenderStyle &style,
                                                         const RenderStyle *oldStyle) noexcept
  {
    style::ShapeOutside shapeOutside = style.GetShapeOutside();
    style::ShapeOutside oldShapeOutside =
      oldStyle ? oldStyle->GetShapeOutside() : style::ComputedStyle::InitialShapeOutside();

    style::ShapeMargin shapeMargin = style.GetShapeMargin();
    style::ShapeMargin oldShapeMargin =
      oldStyle ? oldStyle->GetShapeMargin() : style::ComputedStyle::InitialShapeMargin();

    style::ShapeImageThreshold shapeImageThreshold = style.GetShapeImageThreshold();
    style::ShapeImageThreshold oldShapeImageThreshold =
      oldStyle ? oldStyle->GetShapeImageThreshold() : style::ComputedStyle::InitialShapeImageThreshold();

    // FIXME: A future optimization would do a deep comparison for equality. (bug 100811)
    if (shapeOutside == oldShapeOutside && shapeMargin == oldShapeMargin
        && shapeImageThreshold == oldShapeImageThreshold)
    {
      return;
    }

    if (shapeOutside.IsNone())
    {
      RemoveShapeOutsideInfo();
    }
    else
    {
      EnsureShapeOutsideInfo().MarkShapeAsDirty();
    }

    if (!shapeOutside.IsNone() || shapeOutside != oldShapeOutside)
    {
      MarkShapeOutsideDependentsForLayout();
    }
  }

  namespace
  {
    // The weak map upstream is keyed by address here: a box's entry is removed when it's destroyed.
    using ShapeOutsideInfoMap = HashMap<const RenderBox *, UniquePtr<ShapeOutsideInfo>>;

    KRYS_NODISCARD ShapeOutsideInfoMap &GetShapeOutsideInfoMap() noexcept
    {
      static ShapeOutsideInfoMap staticInfoMap;
      return staticInfoMap;
    }
  }

  ShapeOutsideInfo *RenderBox::GetShapeOutsideInfo() const noexcept
  {
    if (!RenderBoxHasShapeOutsideInfo())
    {
      return nullptr;
    }

    if (!ShapeOutsideInfo::IsEnabledFor(*this))
    {
      return nullptr;
    }

    auto it = GetShapeOutsideInfoMap().find(this);
    return it != GetShapeOutsideInfoMap().end() ? it->second.get() : nullptr;
  }

  ShapeOutsideInfo &RenderBox::EnsureShapeOutsideInfo() noexcept
  {
    SetRenderBoxHasShapeOutsideInfo(true);
    auto &info = GetShapeOutsideInfoMap()[this];
    if (!info)
    {
      info = CreateUnique<ShapeOutsideInfo>(*this);
    }
    return *info;
  }

  void RenderBox::RemoveShapeOutsideInfo() noexcept
  {
    if (!RenderBoxHasShapeOutsideInfo())
    {
      return;
    }

    SetRenderBoxHasShapeOutsideInfo(false);
    GetShapeOutsideInfoMap().erase(this);
  }

  void RenderBox::AddOverflowFromContainedBox(const RenderBox &child,
                                              EnumSet<ComputeOverflowOptions> options) noexcept
  {
    krys_debug_assert(!child.IsFloating());
    AddOverflowWithRendererOffset(child, child.LocationOffset(), options);
  }

  // 'offsetFromThis' is normally the renderer's position (RenderBox::location()).
  // However in case of a float box, it may belong to a different container
  // (but is intrusive to this container hence calling this function),
  // meaning that the renderer's position may not be the same as its actual offset from this container.
  void RenderBox::AddOverflowWithRendererOffset(const RenderBox &renderer, LayoutSize offsetFromThis,
                                                EnumSet<ComputeOverflowOptions>) noexcept
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
  void RenderBox::FlipForWritingMode(FloatRect &rect) const noexcept
  {
    if (!Style().GetWritingMode().IsBlockFlipped())
    {
      return;
    }

    if (IsHorizontalWritingMode())
    {
      rect.SetY(Height().ToFloat() - rect.MaxY());
    }
    else
    {
      rect.SetX(Width().ToFloat() - rect.MaxX());
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

  LayoutRange RenderBox::ContainingBlockRangeForPositioned(const RenderBoxModelObject &container,
                                                           BoxAxis physicalAxis) const noexcept
  {
    krys_debug_assert(container.CanContainAbsolutelyPositionedObjects()
                      || container.CanContainFixedPositionObjects());

    bool isContainerInlineAxis = physicalAxis == container.Style().GetWritingMode().InlineAxis();
    auto startEdge = isContainerInlineAxis ? container.BorderLogicalLeft() : container.BorderBefore();

    if (IsFixedPositioned())
    {
      if (auto *renderView = DynamicDowncast<RenderView>(container))
      {
        return isContainerInlineAxis
                 ? LayoutRange(startEdge, renderView->ClientLogicalWidthForFixedPosition())
                 : LayoutRange(startEdge, renderView->ClientLogicalHeightForFixedPosition());
      }
    }

    // Inline containing blocks are formed by relatively-positioned inline boxes.
    // FIXME: A block's inline continuation forms the containing block (inlineContinuation), once there are
    // continuations.
    if (auto *inlineContainer = DynamicDowncast<RenderInline>(container))
    {
      return isContainerInlineAxis ? LayoutRange(startEdge, inlineContainer->InnerPaddingBoxWidth())
                                   : LayoutRange(startEdge, inlineContainer->InnerPaddingBoxHeight());
    }

    const RenderBlock *containingBlock = DynamicDowncast<RenderBlock>(container);
    if (!containingBlock)
    {
      containingBlock = container.ContainingBlock();
    }
    if (!containingBlock)
    {
      krys_debug_assert(false);
      return {};
    }

    // FIXME: The first fragment's size when the containing block is a fragmented flow, and the fragment's
    // width inside one, once there are fragmented flows.

    return BoxAxis::Horizontal == physicalAxis ? LayoutRange(startEdge, containingBlock->ClientWidth())
                                               : LayoutRange(startEdge, containingBlock->ClientHeight());
  }

  void RenderBox::ComputeOutOfFlowPositionedLogicalWidth(
    LogicalExtentComputedValues &computedValues) const noexcept
  {
    krys_debug_assert(IsOutOfFlowPositioned());

    if (auto *replaced = DynamicDowncast<RenderReplaced>(this))
    {
      return replaced->ComputeReplacedOutOfFlowPositionedLogicalWidth(computedValues);
    }

    // QUESTIONS
    // FIXME 1: Should we still deal with these the cases of 'left' or 'right' having
    // the type 'static' in determining whether to calculate the static distance?
    // NOTE: 'static' is not a legal value for 'left' or 'right' as of CSS 2.1.

    // FIXME 2: Can perhaps optimize out cases when max-width/min-width are greater
    // than or less than the computed width().  Be careful of box-sizing and
    // percentage issues.

    // The following is based off of the W3C Working Draft from April 11, 2006 of
    // CSS 2.1: Section 10.3.7 "Absolutely positioned, non-replaced elements"
    // <http://www.w3.org/TR/CSS21/visudet.html#abs-non-replaced-width>
    // (block-style-comments in this function and in computeOutOfFlowPositionedLogicalWidthUsing()
    // correspond to text from the spec)

    PositionedLayoutConstraints inlineConstraints(*this, LogicalBoxAxis::Inline);
    inlineConstraints.ComputeInsets();

    // Calculate the used width. See CSS2 § 10.3.7.
    const auto &styleToUse = Style();
    auto usedWidth =
      ComputeOutOfFlowPositionedLogicalWidthUsing(styleToUse.LogicalWidth(), inlineConstraints);

    LayoutUnit transferredMinSize = LayoutUnit::Min();
    LayoutUnit transferredMaxSize = LayoutUnit::Max();
    if (ShouldComputeLogicalHeightFromAspectRatio())
    {
      std::tie(transferredMinSize, transferredMaxSize) = ComputeMinMaxLogicalWidthFromAspectRatio();
    }

    // Clamp by max-width.
    auto usedMaxWidth = LayoutUnit::Max();
    if (const auto &logicalMaxWidth = styleToUse.LogicalMaxWidth(); !logicalMaxWidth.IsNone())
    {
      usedMaxWidth = ComputeOutOfFlowPositionedLogicalWidthUsing(logicalMaxWidth, inlineConstraints);
    }
    if (transferredMaxSize < usedMaxWidth)
    {
      usedMaxWidth = ComputeOutOfFlowPositionedLogicalWidthUsing(
        style::MaximumSize {style::MaximumSize::Fixed {transferredMaxSize.ToFloat()}}, inlineConstraints);
    }
    if (usedWidth > usedMaxWidth)
    {
      usedWidth = usedMaxWidth;
    }

    // Clamp by min-width.
    auto usedMinWidth = LayoutUnit::Min();
    if (const auto &logicalMinWidth = styleToUse.LogicalMinWidth();
        !logicalMinWidth.IsKnownZero() || logicalMinWidth.IsIntrinsic())
    {
      usedMinWidth = ComputeOutOfFlowPositionedLogicalWidthUsing(logicalMinWidth, inlineConstraints);
    }
    if (transferredMinSize > usedMinWidth)
    {
      usedMinWidth = ComputeOutOfFlowPositionedLogicalWidthUsing(
        style::MinimumSize {style::MinimumSize::Fixed {transferredMinSize.ToFloat()}}, inlineConstraints);
    }
    if (usedWidth < usedMinWidth)
    {
      usedWidth = usedMinWidth;
    }

    bool hasWidthBorderBoxAspectRatio = Style().HasAspectRatio()
                                        && Style().BoxSizingForAspectRatio() == BoxSizing::BorderBox
                                        && Style().LogicalWidth().IsAuto();

    // Set the final width value.
    computedValues.Extent =
      hasWidthBorderBoxAspectRatio ? usedWidth : usedWidth + inlineConstraints.BordersPlusPadding();

    // Calculate the position.
    inlineConstraints.ResolvePosition(computedValues);
    inlineConstraints.FixupLogicalLeftPosition(computedValues);

    // FIXME: Adjust logicalLeft for the flipped version of our writing mode in fragments, once there are
    // fragmented flows.
  }

  template <typename SizeType>
  LayoutUnit RenderBox::ComputeOutOfFlowPositionedLogicalWidthUsing(
    const SizeType &logicalWidth, const PositionedLayoutConstraints &inlineConstraints) const noexcept
  {
    auto fallback = [&]() -> LayoutUnit
    {
      bool shrinkToFit = inlineConstraints.InsetFitsContent()
                         || !inlineConstraints.AlignmentAppliesStretch(ItemPosition::Stretch);
      if (shrinkToFit)
      {
        auto preferredWidth = MaxPreferredLogicalWidth() - inlineConstraints.BordersPlusPadding();
        auto preferredMinWidth = MinPreferredLogicalWidth() - inlineConstraints.BordersPlusPadding();
        return std::min(std::max(preferredMinWidth, inlineConstraints.AvailableContentSpace()),
                        preferredWidth);
      }
      return std::max(0_lu, inlineConstraints.AvailableContentSpace());
    };

    auto intrinsic = [&]() -> LayoutUnit
    {
      auto availableSpace = inlineConstraints.ContainingSize();
      availableSpace -= inlineConstraints.InsetBeforeValue();
      availableSpace -= inlineConstraints.InsetAfterValue();
      return std::max(0_lu, ComputeIntrinsicLogicalWidthUsing(logicalWidth, availableSpace,
                                                              inlineConstraints.BordersPlusPadding())
                              - inlineConstraints.BordersPlusPadding());
    };

    if (logicalWidth.IsSpecified())
    {
      return AdjustContentBoxLogicalWidthForBoxSizing(style::Evaluate<LayoutUnit>(
        logicalWidth, inlineConstraints.ContainingSize(), Style().UsedZoomForLength()));
    }
    if (logicalWidth.IsFitContent() || logicalWidth.IsFillAvailable() || logicalWidth.IsMinContent()
        || logicalWidth.IsMaxContent())
    {
      return intrinsic();
    }
    if (logicalWidth.IsIntrinsicKeyword() || logicalWidth.IsMinIntrinsic())
    {
      if (ShouldComputeLogicalWidthFromAspectRatio())
      {
        return inlineConstraints.ContainingSize();
      }
      return fallback();
    }
    if constexpr (std::same_as<SizeType, style::MaximumSize>)
    {
      krys_debug_assert(logicalWidth.IsNone());
      return inlineConstraints.ContainingSize();
    }
    else
    {
      krys_debug_assert(logicalWidth.IsAuto());
      if constexpr (std::same_as<SizeType, style::MinimumSize>)
      {
        if (ShouldComputeLogicalWidthFromAspectRatio())
        {
          LayoutUnit minLogicalWidth;
          LayoutUnit maxLogicalWidth;
          ComputeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth);
          return minLogicalWidth;
        }
        return 0_lu;
      }
      else
      {
        if (ShouldComputeLogicalWidthFromAspectRatio())
        {
          return ComputeLogicalWidthFromAspectRatio();
        }
        return fallback();
      }
    }
  }

  void RenderBox::ComputeOutOfFlowPositionedLogicalHeight(
    LogicalExtentComputedValues &computedValues) const noexcept
  {
    krys_debug_assert(IsOutOfFlowPositioned());

    if (auto *replaced = DynamicDowncast<RenderReplaced>(this))
    {
      return replaced->ComputeReplacedOutOfFlowPositionedLogicalHeight(computedValues);
    }

    PositionedLayoutConstraints blockConstraints(*this, LogicalBoxAxis::Block);
    blockConstraints.ComputeInsets();

    // Calculate the used height. See CSS2 § 10.6.4.
    const auto &styleToUse = Style();
    LayoutUnit computedHeight = computedValues.Extent;
    LayoutUnit usedHeight = ComputeOutOfFlowPositionedLogicalHeightUsing(styleToUse.LogicalHeight(),
                                                                         computedHeight, blockConstraints);

    // Clamp by max height.
    if (const auto &logicalMaxHeight = styleToUse.LogicalMaxHeight(); !logicalMaxHeight.IsNone())
    {
      auto usedMaxHeight =
        ComputeOutOfFlowPositionedLogicalHeightUsing(logicalMaxHeight, computedHeight, blockConstraints);
      if (usedHeight > usedMaxHeight)
      {
        usedHeight = usedMaxHeight;
      }
    }

    // Clamp by min height.
    if (const auto &logicalMinHeight = styleToUse.LogicalMinHeight();
        logicalMinHeight.IsAuto() || !logicalMinHeight.IsKnownZero() || logicalMinHeight.IsIntrinsic())
    {
      auto usedMinHeight =
        ComputeOutOfFlowPositionedLogicalHeightUsing(logicalMinHeight, computedHeight, blockConstraints);
      if (usedHeight < usedMinHeight)
      {
        usedHeight = usedMinHeight;
      }
    }

    // Set the final height value.
    computedValues.Extent = usedHeight + blockConstraints.BordersPlusPadding();

    // Calculate the position.
    blockConstraints.ResolvePosition(computedValues);
    blockConstraints.AdjustLogicalTopWithLogicalHeightIfNeeded(computedValues);

    // FIXME: Adjust logicalTop for perpendicular writing modes in fragments, once there are fragmented flows.
  }

  LayoutUnit RenderBox::ComputeOutOfFlowPositionedLogicalHeightUsing(
    const style::PreferredSize &logicalHeight, LayoutUnit computedHeight,
    const PositionedLayoutConstraints &blockConstraints) const noexcept
  {
    auto contentLogicalHeight = computedHeight - blockConstraints.BordersPlusPadding();

    // Height is never unsolved for tables.
    if (IsRenderTable())
    {
      return contentLogicalHeight;
    }

    bool fromAspectRatio = ShouldComputeLogicalHeightFromAspectRatio();
    bool logicalHeightIsAuto = logicalHeight.IsAuto() && !fromAspectRatio;

    if (!logicalHeightIsAuto)
    {
      if (logicalHeight.IsIntrinsic())
      {
        return AdjustContentBoxLogicalHeightForBoxSizing(
          ComputeIntrinsicLogicalContentHeightUsing(logicalHeight, contentLogicalHeight,
                                                    blockConstraints.BordersPlusPadding())
            .value_or(0_lu));
      }
      if (fromAspectRatio)
      {
        auto resolvedLogicalHeight = BlockSizeFromAspectRatio(
          HorizontalBorderAndPaddingExtent(), VerticalBorderAndPaddingExtent(),
          static_cast<double>(LayoutUnit {Style().LogicalAspectRatio()}), Style().BoxSizingForAspectRatio(),
          LogicalWidth(), Style().GetAspectRatio(), IsRenderReplaced());
        return std::max(LayoutUnit(), resolvedLogicalHeight - blockConstraints.BordersPlusPadding());
      }
      return AdjustContentBoxLogicalHeightForBoxSizing(style::Evaluate<LayoutUnit>(
        logicalHeight, blockConstraints.ContainingSize(), Style().UsedZoomForLength()));
    }

    bool shrinkToFit =
      blockConstraints.InsetFitsContent() || !blockConstraints.AlignmentAppliesStretch(ItemPosition::Stretch);
    if (!shrinkToFit)
    {
      return std::max<LayoutUnit>(0_lu, blockConstraints.AvailableContentSpace());
    }

    return contentLogicalHeight;
  }

  LayoutUnit RenderBox::ComputeOutOfFlowPositionedLogicalHeightUsing(
    const style::MinimumSize &originalLogicalHeight, LayoutUnit computedHeight,
    const PositionedLayoutConstraints &blockConstraints) const noexcept
  {
    auto contentLogicalHeight = computedHeight - blockConstraints.BordersPlusPadding();

    // Height is never unsolved for tables.
    if (IsRenderTable())
    {
      return contentLogicalHeight;
    }

    auto logicalHeight = originalLogicalHeight;

    if (logicalHeight.IsAuto())
    {
      if (ShouldComputeLogicalHeightFromAspectRatio())
      {
        logicalHeight = style::MinimumSize::Fixed {computedHeight.ToFloat()};
      }
      else
      {
        logicalHeight = style::MinimumSize::Fixed {0};
      }
    }

    if (logicalHeight.IsIntrinsic())
    {
      return AdjustContentBoxLogicalHeightForBoxSizing(
        ComputeIntrinsicLogicalContentHeightUsing(logicalHeight, contentLogicalHeight,
                                                  blockConstraints.BordersPlusPadding())
          .value_or(0_lu));
    }
    return AdjustContentBoxLogicalHeightForBoxSizing(style::Evaluate<LayoutUnit>(
      logicalHeight, blockConstraints.ContainingSize(), Style().UsedZoomForLength()));
  }

  LayoutUnit RenderBox::ComputeOutOfFlowPositionedLogicalHeightUsing(
    const style::MaximumSize &logicalHeight, LayoutUnit computedHeight,
    const PositionedLayoutConstraints &blockConstraints) const noexcept
  {
    auto contentLogicalHeight = computedHeight - blockConstraints.BordersPlusPadding();

    // Height is never unsolved for tables.
    if (IsRenderTable())
    {
      return contentLogicalHeight;
    }

    if (logicalHeight.IsIntrinsic())
    {
      return AdjustContentBoxLogicalHeightForBoxSizing(
        ComputeIntrinsicLogicalContentHeightUsing(logicalHeight, contentLogicalHeight,
                                                  blockConstraints.BordersPlusPadding())
          .value_or(0_lu));
    }
    return AdjustContentBoxLogicalHeightForBoxSizing(style::Evaluate<LayoutUnit>(
      logicalHeight, blockConstraints.ContainingSize(), Style().UsedZoomForLength()));
  }

  bool RenderBox::RequiresLayer() const noexcept
  {
    return RenderBoxModelObject::RequiresLayer() || HasNonVisibleOverflow() || Style().SpecifiesColumns()
           || Style().UsedContain().Contains(style::ContainValue::Layout) || !Style().UsedZIndex().IsAuto()
           || HasRunningAcceleratedAnimations();
  }

  void RenderBox::ApplyTransform(TransformationMatrix &t, const RenderStyle &style,
                                 const FloatRect &boundingBox,
                                 EnumSet<style::TransformResolverOption> options) const noexcept
  {
    // FIXME: The motion path data of the box (TransformOperationData's renderer constructor), once there are
    // motion paths.
    style::TransformResolver::ApplyTransform(t, style, TransformOperationData(boundingBox), options);
  }

  FloatRect RenderBox::ReferenceBoxRect(CSSBoxType boxType) const noexcept
  {
    switch (boxType)
    {
      case CSSBoxType::ContentBox:
      case CSSBoxType::FillBox:    return FloatRect(ContentBoxRect());
      case CSSBoxType::PaddingBox: return FloatRect(PaddingBoxRect());
      case CSSBoxType::MarginBox:  return FloatRect(MarginBoxRect());
      // stroke-box, view-box compute to border-box for HTML elements.
      case CSSBoxType::StrokeBox:
      case CSSBoxType::ViewBox:
      case CSSBoxType::BorderBox:
      case CSSBoxType::BoxMissing: return FloatRect(BorderBoxRect());
    }

    krys_debug_assert(false);
    return {};
  }

  LayoutRect RenderBox::OverflowClipRect(const LayoutPoint &location,
                                         OverlayScrollbarSizeRelevancy relevancy) const noexcept
  {
    LayoutRect clipRect = BorderBoxRect();
    clipRect.SetLocation(location + clipRect.Location() + LayoutSize(BorderLeft(), BorderTop()));
    clipRect.SetSize(clipRect.Size()
                     - LayoutSize(BorderLeft() + BorderRight(), BorderTop() + BorderBottom()));
    if (Style().OverflowX() == Overflow::Clip && Style().OverflowY() == Overflow::Visible)
    {
      clipRect.ExpandToInfiniteY();
    }
    else if (Style().OverflowY() == Overflow::Clip && Style().OverflowX() == Overflow::Visible)
    {
      clipRect.ExpandToInfiniteX();
    }

    // Subtract out scrollbars if we have them.
    if (auto *scrollableArea = Layer() ? Layer()->GetScrollableArea() : nullptr)
    {
      if (Style().ShouldPlaceVerticalScrollbarOnLeft())
      {
        clipRect.Move(
          LayoutUnit(scrollableArea->VerticalScrollbarWidth(relevancy, IsHorizontalWritingMode())), 0_lu);
      }
      clipRect.Contract(
        LayoutUnit(scrollableArea->VerticalScrollbarWidth(relevancy, IsHorizontalWritingMode())),
        LayoutUnit(scrollableArea->HorizontalScrollbarHeight(relevancy, IsHorizontalWritingMode())));
    }

    return clipRect;
  }

  LayoutRect RenderBox::ClipRect(const LayoutPoint &location) const noexcept
  {
    auto borderBoxRect = this->BorderBoxRect();
    auto clipRect = LayoutRect(borderBoxRect.Location() + location, borderBoxRect.Size());

    return Style().GetClip().SwitchOn(
      [&](const css::keywords::Auto &) { return clipRect; },
      [&](const style::ClipRect &rect)
      {
        if (auto clipLeft = rect.Value->Left().TryLength())
        {
          auto c = LayoutUnit {clipLeft->ResolveZoom(style::ZoomNeeded {})};
          clipRect.Move(c, 0_lu);
          clipRect.Contract(c, 0_lu);
        }

        // We don't use the fragment-specific border box's width and height since clip offsets are (stupidly)
        // specified from the left and top edges. Therefore it's better to avoid constraining to smaller
        // widths and heights.

        if (auto clipRight = rect.Value->Right().TryLength())
        {
          clipRect.Contract(Width() - LayoutUnit {clipRight->ResolveZoom(style::ZoomNeeded {})}, 0_lu);
        }

        if (auto clipTop = rect.Value->Top().TryLength())
        {
          auto c = LayoutUnit {clipTop->ResolveZoom(style::ZoomNeeded {})};
          clipRect.Move(0_lu, c);
          clipRect.Contract(0_lu, c);
        }

        if (auto clipBottom = rect.Value->Bottom().TryLength())
        {
          clipRect.Contract(0_lu, Height() - LayoutUnit {clipBottom->ResolveZoom(style::ZoomNeeded {})});
        }

        return clipRect;
      });
  }
#pragma region Painting

  BleedAvoidance RenderBox::DetermineBleedAvoidance(GraphicsContext &context) const noexcept
  {
    if (context.PaintingDisabled())
    {
      return BleedAvoidance::None;
    }

    const RenderStyle &style = Style();

    if (!style.HasBackground() || !style.HasBorder() || !style.HasBorderRadius()
        || BorderImageIsLoadedAndCanBeRendered())
    {
      return BleedAvoidance::None;
    }

    // FIXME: The theme (mayNeedBleedAvoidance), once there are themed controls.

    AffineTransform ctm = context.GetCTM();
    FloatSize contextScaling(static_cast<float>(ctm.XScale()), static_cast<float>(ctm.YScale()));

    // Because LayoutRoundedRect uses IntRect internally the inset applied by the
    // BleedAvoidance::ShrinkBackground strategy cannot be less than one integer
    // layout coordinate, even with subpixel layout enabled. To take that into
    // account, we clamp the contextScaling to 1.0 for the following test so
    // that borderObscuresBackgroundEdge can only return true if the border
    // widths are greater than 2 in both layout coordinates and screen
    // coordinates.
    // This precaution will become obsolete if LayoutRoundedRect is ever promoted to
    // a sub-pixel representation.
    if (contextScaling.Width() > 1)
    {
      contextScaling.SetWidth(1);
    }
    if (contextScaling.Height() > 1)
    {
      contextScaling.SetHeight(1);
    }

    if (BorderObscuresBackgroundEdge(contextScaling))
    {
      return BleedAvoidance::ShrinkBackground;
    }

    if (!style.HasUsedAppearance() && BorderObscuresBackground() && BackgroundHasOpaqueTopLayer())
    {
      return BleedAvoidance::BackgroundOverBorder;
    }

    return BleedAvoidance::UseTransparencyLayer;
  }

  void RenderBox::PaintBoxDecorations(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return;
    }

    LayoutRect paintRect = BorderBoxRect();
    paintRect.MoveBy(paintOffset);
    AdjustBorderBoxRectForPainting(paintRect);

    // FIXME: The theme's adjusted paint rect (adjustedPaintRect), once there are themed controls.
    auto bleedAvoidance = DetermineBleedAvoidance(paintInfo.Context());

    BackgroundPainter backgroundPainter {*this, paintInfo};

    // FIXME: Should eventually give the theme control over whether the box shadow should paint, since
    // controls could have custom shadows of their own.
    if (!BackgroundPainter::BoxShadowShouldBeAppliedToBackground(*this, paintRect.Location(), bleedAvoidance,
                                                                 {}))
    {
      backgroundPainter.PaintBoxShadow(paintRect, Style(), style::ShadowStyle::Normal);
    }

    GraphicsContextStateSaver stateSaver(paintInfo.Context(), false);
    if (bleedAvoidance == BleedAvoidance::UseTransparencyLayer)
    {
      // To avoid the background color bleeding out behind the border, we'll render background and border
      // into a transparency layer, and then clip that in one go (which requires setting up the clip before
      // beginning the layer).
      stateSaver.Save();
      auto borderShape = BorderShape::ShapeForBorderRect(Style(), paintRect);
      borderShape.ClipToOuterShape(paintInfo.Context(), GetDocument().DeviceScaleFactor());
      paintInfo.Context().BeginTransparencyLayer(1);
    }

    // FIXME: Native theme appearances (theme().paint, paintDecorations, paintBorderOnly), once there are
    // themed controls; the CSS background and border are always painted.

    BorderPainter borderPainter {*this, paintInfo};

    if (bleedAvoidance == BleedAvoidance::BackgroundOverBorder)
    {
      borderPainter.PaintBorder(paintRect, Style(), bleedAvoidance);
    }

    backgroundPainter.PaintBackground(paintRect, bleedAvoidance);

    backgroundPainter.PaintBoxShadow(paintRect, Style(), style::ShadowStyle::Inset);

    if (bleedAvoidance != BleedAvoidance::BackgroundOverBorder)
    {
      if (Style().HasVisibleBorderDecoration())
      {
        borderPainter.PaintBorder(paintRect, Style(), bleedAvoidance);
      }
    }

    if (bleedAvoidance == BleedAvoidance::UseTransparencyLayer)
    {
      paintInfo.Context().EndTransparencyLayer();
    }
  }

  bool RenderBox::BackgroundHasOpaqueTopLayer() const noexcept
  {
    const auto &topLayer = Style().GetBackgroundLayers().UsedFirst();
    if (topLayer.GetClip() != FillBox::BorderBox)
    {
      return false;
    }

    // Clipped with local scrolling
    if (HasNonVisibleOverflow() && topLayer.GetAttachment() == FillAttachment::LocalBackground)
    {
      return false;
    }

    if (topLayer.HasOpaqueImage(*this) && topLayer.HasRepeatXY()
        && topLayer.GetImage().TryStyleImage()->CanRender(this, Style().UsedZoom()))
    {
      return true;
    }

    // If there is only one layer and no image, check whether the background color is opaque.
    if (Style().GetBackgroundLayers().UsedLength() == 1 && !topLayer.HasImage())
    {
      Color bgColor = Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
      if (bgColor.IsOpaque())
      {
        return true;
      }
    }

    return false;
  }

  void RenderBox::PaintMask(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (!paintInfo.ShouldPaintWithinRoot(*this) || Style().UsedVisibility() != Visibility::Visible
        || paintInfo.Phase != PaintPhase::Mask || paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    LayoutRect paintRect = LayoutRect(paintOffset, Size());
    AdjustBorderBoxRectForPainting(paintRect);
    PaintMaskImages(paintInfo, paintRect);
  }

  LayoutRect RenderBox::MaskClipRect(const LayoutPoint &paintOffset) noexcept
  {
    auto &maskBorder = Style().GetMaskBorder();
    if (!maskBorder.Source.IsNone())
    {
      LayoutRect borderImageRect = BorderBoxRect();

      // Apply outsets to the border box.
      borderImageRect.Expand(Style().MaskBorderOutsets());
      return borderImageRect;
    }

    LayoutRect result;
    LayoutRect borderBox = BorderBoxRect();
    for (auto &maskLayer : Style().GetMaskLayers().UsedValues())
    {
      if (maskLayer.HasImage())
      {
        // Masks should never have fixed attachment, so it's OK for paintContainer to be null.
        result.Unite(BackgroundPainter::CalculateFillLayerImageGeometry(*this, nullptr, maskLayer,
                                                                        paintOffset, borderBox)
                       .DestinationRect);
      }
    }
    return result;
  }

  void RenderBox::PaintMaskImages(const PaintInfo &paintInfo, const LayoutRect &paintRect) noexcept
  {
    // Figure out if we need to push a transparency layer to render our mask.
    bool pushTransparencyLayer = false;
    bool compositedMask = HasLayer() && Layer()->HasCompositedMask();
    bool flattenCompositingLayers = paintInfo.PaintBehavior.Contains(PaintBehavior::FlattenCompositingLayers);
    CompositeOperator compositeOp = CompositeOperator::SourceOver;

    bool allMaskImagesLoaded = true;

    if (!compositedMask || flattenCompositingLayers)
    {
      pushTransparencyLayer = true;

      // Don't render a masked element until all the mask images have loaded, to prevent a flash of unmasked
      // content.
      if (RefPtr<StyleImage> maskBorder = Style().GetMaskBorderSource().TryStyleImage())
      {
        allMaskImagesLoaded &= maskBorder->IsLoaded(this);
      }

      allMaskImagesLoaded &= style::ImagesAreLoaded(Style().GetMaskLayers(), *this);

      paintInfo.Context().SetCompositeOperation(CompositeOperator::DestinationIn);
      paintInfo.Context().BeginTransparencyLayer(1);
      compositeOp = CompositeOperator::SourceOver;
    }

    if (allMaskImagesLoaded)
    {
      BackgroundPainter {*this, paintInfo}.PaintFillLayers(Color(), Style().GetMaskLayers(), paintRect,
                                                           BleedAvoidance::None, compositeOp);
      BorderPainter {*this, paintInfo}.PaintNinePieceImage(paintRect, Style(), Style().GetMaskBorder(),
                                                           compositeOp);
    }

    if (pushTransparencyLayer)
    {
      paintInfo.Context().EndTransparencyLayer();
    }
  }

  void RenderBox::PaintClippingMask(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (!paintInfo.ShouldPaintWithinRoot(*this) || Style().UsedVisibility() != Visibility::Visible
        || paintInfo.Phase != PaintPhase::ClippingMask || paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    LayoutRect paintRect = LayoutRect(paintOffset, Size());

    // FIXME: SVG clipping masks (paintSVGClippingMask) come with the layer-based SVG engine.

    paintInfo.Context().FillRect(FloatRect(SnappedIntRect(paintRect)), Color::black);
  }

  void RenderBox::ClipToPaddingBoxShape(GraphicsContext &context, const LayoutPoint &accumulatedOffset,
                                        float deviceScaleFactor) const noexcept
  {
    auto borderShape = BorderShape::ShapeForBorderRect(Style(), LayoutRect(accumulatedOffset, Size()));
    borderShape.ClipToInnerShape(context, deviceScaleFactor);
  }

  void RenderBox::ClipToContentBoxShape(GraphicsContext &context, const LayoutPoint &accumulatedOffset,
                                        float deviceScaleFactor) const noexcept
  {
    auto borderShape = BorderShapeForContentClipping(LayoutRect {accumulatedOffset, Size()});
    borderShape.ClipToInnerShape(context, deviceScaleFactor);
  }

  bool RenderBox::PushContentsClip(PaintInfo &paintInfo, const LayoutPoint &accumulatedOffset) noexcept
  {
    if (paintInfo.Phase == PaintPhase::BlockBackground || paintInfo.Phase == PaintPhase::SelfOutline
        || paintInfo.Phase == PaintPhase::Mask)
    {
      return false;
    }

    // FIXME: Control clips (hasControlClip, controlClipRect), once there are form controls.
    bool isOverflowClip = HasNonVisibleOverflow() && !Layer()->IsSelfPaintingLayer();

    if (!isOverflowClip)
    {
      return false;
    }

    if (paintInfo.Phase == PaintPhase::Outline)
    {
      paintInfo.Phase = PaintPhase::ChildOutlines;
    }
    else if (paintInfo.Phase == PaintPhase::ChildBlockBackground)
    {
      paintInfo.Phase = PaintPhase::BlockBackground;
      PaintObject(paintInfo, accumulatedOffset);
      paintInfo.Phase = PaintPhase::ChildBlockBackgrounds;
    }
    float deviceScaleFactor = GetDocument().DeviceScaleFactor();
    FloatRect clipRect = SnapRectToDevicePixels(
      OverflowClipRect(accumulatedOffset, OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize),
      deviceScaleFactor);
    paintInfo.Context().Save();
    if (Style().HasBorderRadius())
    {
      ClipToPaddingBoxShape(paintInfo.Context(), accumulatedOffset, deviceScaleFactor);
    }

    paintInfo.Context().Clip(clipRect);

    // FIXME: Event and accessibility region clips (regionContext) come with hit testing.

    return true;
  }

  void RenderBox::PopContentsClip(PaintInfo &paintInfo, PaintPhase originalPhase,
                                  const LayoutPoint &accumulatedOffset) noexcept
  {
    krys_debug_assert(HasNonVisibleOverflow() && !Layer()->IsSelfPaintingLayer());

    paintInfo.Context().Restore();
    if (originalPhase == PaintPhase::Outline)
    {
      paintInfo.Phase = PaintPhase::SelfOutline;
      PaintObject(paintInfo, accumulatedOffset);
      paintInfo.Phase = originalPhase;
    }
    else if (originalPhase == PaintPhase::ChildBlockBackground)
    {
      paintInfo.Phase = originalPhase;
    }
  }

#pragma endregion

  bool RenderBox::HitTestVisualOverflow(const HitTestLocation &hitTestLocation,
                                        const LayoutPoint &accumulatedOffset) const noexcept
  {
    if (IsRenderView())
    {
      return true;
    }

    LayoutPoint adjustedLocation = accumulatedOffset + Location();
    LayoutRect overflowBox = VisualOverflowRect();
    FlipForWritingMode(overflowBox);
    overflowBox.MoveBy(adjustedLocation);
    return hitTestLocation.Intersects(overflowBox);
  }

  bool RenderBox::HitTestClipPath(const HitTestLocation &hitTestLocation,
                                  const LayoutPoint &accumulatedOffset) const noexcept
  {
    auto offsetFromHitTestRoot = ToLayoutSize(accumulatedOffset + Location());
    auto hitTestLocationInLocalCoordinates = hitTestLocation.Point() - offsetFromHitTestRoot;

    const auto &clipPath = Style().GetClipPath();
    if (auto basicShape = clipPath.TryBasicShape())
    {
      auto &shape = basicShape->GetShape();
      auto path = style::ComputePath(shape, ReferenceBoxRect(basicShape->ReferenceBox()));
      return path.Contains(FloatPoint(hitTestLocationInLocalCoordinates), style::ComputeWindRule(shape));
    }
    // FIXME: Reference paths (url(), clipped by SVG <clipPath> content) come with SVG.
    return true;
  }

  bool RenderBox::HitTestBorderRadius(const HitTestLocation &hitTestLocation,
                                      const LayoutPoint &accumulatedOffset) const noexcept
  {
    if (IsRenderView() || !Style().HasBorderRadius())
    {
      return true;
    }

    LayoutPoint adjustedLocation = accumulatedOffset + Location();
    LayoutRect borderRect = BorderBoxRect();
    borderRect.MoveBy(adjustedLocation);

    auto borderShape = BorderShape::ShapeForBorderRect(Style(), borderRect);
    // To handle non-round corners, BorderShape should do the hit-testing.
    return hitTestLocation.Intersects(borderShape.DeprecatedRoundedRect());
  }

  bool RenderBox::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                              const HitTestLocation &locationInContainer,
                              const LayoutPoint &accumulatedOffset, HitTestAction action) noexcept
  {
    LayoutPoint adjustedLocation = accumulatedOffset + Location();

    // Check kids first.
    for (RenderObject *child = LastChild(); child; child = child->PreviousSibling())
    {
      if (!child->HasLayer()
          && child->NodeAtPoint(request, result, locationInContainer, adjustedLocation, action))
      {
        UpdateHitTestResult(result, locationInContainer.Point() - ToLayoutSize(adjustedLocation));
        return true;
      }
    }

    // Check our bounds next. For this purpose always assume that we can only be hit in the
    // foreground phase (which is true for replaced elements like images).
    LayoutRect boundsRect = BorderBoxRect();
    boundsRect.MoveBy(adjustedLocation);
    if (VisibleToHitTesting(&request) && action == HitTestForeground
        && locationInContainer.Intersects(boundsRect))
    {
      if (!HitTestVisualOverflow(locationInContainer, accumulatedOffset))
      {
        return false;
      }

      if (!HitTestClipPath(locationInContainer, accumulatedOffset))
      {
        return false;
      }

      if (!HitTestBorderRadius(locationInContainer, accumulatedOffset))
      {
        return false;
      }

      UpdateHitTestResult(result, locationInContainer.Point() - ToLayoutSize(adjustedLocation));
      if (result.AddNodeToListBasedTestResult(NodeForHitTest(), request, locationInContainer, boundsRect)
          == HitTestProgress::Stop)
      {
        return true;
      }
    }

    return RenderBoxModelObject::NodeAtPoint(request, result, locationInContainer, accumulatedOffset, action);
  }

  void RenderBox::ImageChanged(WrappedImagePtr image, const IntRect *) noexcept
  {
    if (auto source = Style().GetBorderImageSource().TryStyleImage(); source && source->Data() == image)
    {
      if (Parent())
      {
        Repaint();
      }
      return;
    }

    if (auto source = Style().GetMaskBorderSource().TryStyleImage(); source && source->Data() == image)
    {
      if (Parent())
      {
        Repaint();
      }
      return;
    }

    if (!View().FrameView().LayoutContext().IsInRenderTreeLayout() && IsFloating())
    {
      if (auto shapeOutsideImage = Style().GetShapeOutside().GetImage();
          shapeOutsideImage && shapeOutsideImage->Data() == image)
      {
        EnsureShapeOutsideInfo().MarkShapeAsDirty();
        MarkShapeOutsideDependentsForLayout();
      }
    }

    bool didFullRepaint = false;

    auto repaintForBackgroundAndMask = [&](const RenderStyle &style)
    {
      if (!Parent())
      {
        return;
      }

      if (!didFullRepaint)
      {
        didFullRepaint = RepaintLayerRectsForImage(image, style.GetBackgroundLayers(), true);
      }
      if (!didFullRepaint)
      {
        didFullRepaint = RepaintLayerRectsForImage(image, style.GetMaskLayers(), false);
      }
    };

    repaintForBackgroundAndMask(Style());

    if (auto *firstLineStyle = Style().GetCachedPseudoStyle({PseudoElementType::FirstLine}))
    {
      repaintForBackgroundAndMask(*firstLineStyle);
    }

    // FIXME: The visually non-empty pixel count and the document's image load milestones (didLoadImage), once
    // there are those.
    bool isNonEmpty = false;
    RefPtr<StyleImage> styleImage =
      style::FindLayerUsedImage(Style().GetBackgroundLayers(), image, isNonEmpty);

    if (!IsComposited())
    {
      return;
    }

    if (Layer()->HasCompositedMask() && style::FindLayerUsedImage(Style().GetMaskLayers(), image, isNonEmpty))
    {
      Layer()->ContentChanged(ContentChangeType::MaskImage);
    }

    if (styleImage)
    {
      Layer()->ContentChanged(ContentChangeType::BackgroundImage);
    }
  }

  template <typename TLayers>
  bool RenderBox::RepaintLayerRectsForImage(WrappedImagePtr image, const TLayers &layers,
                                            bool drawingBackground) noexcept
  {
    LayoutRect rendererRect;
    RenderBox *layerRenderer = nullptr;

    for (auto &layer : layers.UsedValues())
    {
      if (auto layerImage = layer.GetImage().TryStyleImage();
          layerImage && layerImage->Data() == image
          && (layerImage->IsLoaded(this) || layerImage->CanRender(this, Style().UsedZoom())))
      {
        // Now that we know this image is being used, compute the renderer and the rect if we haven't already.
        bool drawingRootBackground =
          drawingBackground
          && (IsDocumentElementRenderer()
              || (IsBody() && !GetDocument().DocumentElement()->Renderer()->HasBackground()));
        if (!layerRenderer)
        {
          if (drawingRootBackground)
          {
            layerRenderer = &View();

            auto &renderView = Downcast<RenderView>(*layerRenderer);
            LayoutUnit rw {renderView.FrameView().ContentsSize().Width()};
            LayoutUnit rh {renderView.FrameView().ContentsSize().Height()};

            rendererRect =
              LayoutRect(-layerRenderer->MarginLeft(), -layerRenderer->MarginTop(),
                         std::max(layerRenderer->Width() + layerRenderer->HorizontalMarginExtent()
                                    + layerRenderer->BorderLeft() + layerRenderer->BorderRight(),
                                  rw),
                         std::max(layerRenderer->Height() + layerRenderer->VerticalMarginExtent()
                                    + layerRenderer->BorderTop() + layerRenderer->BorderBottom(),
                                  rh));

            // If we're drawing the root background, then we want to use the bounds of the view
            // (since root backgrounds cover the canvas, not just the element). If the root element is
            // composited though, we need to issue the repaint to that root element.
            auto *documentElementRenderer = Downcast<RenderBox>(GetDocument().DocumentElement()->Renderer());
            auto *rendererLayer = documentElementRenderer->Layer();
            if (rendererLayer && rendererLayer->IsComposited())
            {
              layerRenderer = documentElementRenderer;
            }
          }
          else
          {
            layerRenderer = this;
            rendererRect = BorderBoxRect();
          }
        }
        // FIXME: Figure out how to pass absolute position to calculateFillLayerImageGeometry (for pixel
        // snapping)
        auto geometry = BackgroundPainter::CalculateFillLayerImageGeometry(*layerRenderer, nullptr, layer,
                                                                           LayoutPoint(), rendererRect);
        if (geometry.HasNonLocalGeometry)
        {
          // Rather than incur the costs of computing the paintContainer for renderers with fixed backgrounds
          // in order to get the right destRect, just repaint the entire renderer.
          layerRenderer->Repaint();
          return true;
        }

        LayoutRect rectToRepaint = geometry.DestinationRect;
        bool shouldClipToLayer = true;

        // If this is the root background layer, we may need to extend the repaintRect if the FrameView has an
        // extendedBackground. We should only extend the rect if it is already extending the full width or
        // height of the rendererRect.
        if (drawingRootBackground && View().FrameView().HasExtendedBackgroundRectForPainting())
        {
          shouldClipToLayer = false;
          IntRect extendedBackgroundRect = View().FrameView().ExtendedBackgroundRectForPainting();
          if (rectToRepaint.Width() == rendererRect.Width())
          {
            rectToRepaint.Move(LayoutUnit {extendedBackgroundRect.X()}, 0_lu);
            rectToRepaint.SetWidth(LayoutUnit {extendedBackgroundRect.Width()});
          }
          if (rectToRepaint.Height() == rendererRect.Height())
          {
            rectToRepaint.Move(0_lu, LayoutUnit {extendedBackgroundRect.Y()});
            rectToRepaint.SetHeight(LayoutUnit {extendedBackgroundRect.Height()});
          }
        }

        layerRenderer->RepaintRectangle(rectToRepaint, shouldClipToLayer);
        if (geometry.DestinationRect == rendererRect)
        {
          return true;
        }
      }
    }
    return false;
  }

}
