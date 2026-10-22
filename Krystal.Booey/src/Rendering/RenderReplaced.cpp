#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Editing/TextAffinity.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LineSelection.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/PositionedLayoutConstraints.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    constexpr int32 DefaultWidth = 300;
    constexpr int32 DefaultHeight = 150;

    KRYS_NODISCARD bool IsSkippedContentRoot(const RenderBox &renderer) noexcept
    {
      auto *element = renderer.GetElement();
      return element && IsSkippedContentRoot(renderer.Style(), *element);
    }

    KRYS_NODISCARD bool ShouldRepaintOnSizeChange(RenderReplaced &renderer) noexcept
    {
      // FIXME: Canvases, and excluding media, once they have renderers.
      if (auto *renderImage = DynamicDowncast<RenderImage>(renderer);
          renderImage && !renderImage->IsShowingMissingOrImageError())
      {
        return true;
      }
      return false;
    }
  }

  RenderReplaced::RenderReplaced(Type type, dom::Element &element, RenderStyle &&style,
                                 EnumSet<ReplacedFlag> flags) noexcept
      : RenderBox(type, element, krys::move(style), {}, flags), _intrinsicSize(DefaultWidth, DefaultHeight)
  {
    // FIXME: Assert the element is replaced (Element::isReplaced), once replaced elements say so.
    SetBlockLevelReplacedOrAtomicInline(true);
    krys_debug_assert(IsRenderReplaced());
  }

  RenderReplaced::RenderReplaced(Type type, dom::Element &element, RenderStyle &&style,
                                 const LayoutSize &intrinsicSize, EnumSet<ReplacedFlag> flags) noexcept
      : RenderBox(type, element, krys::move(style), {}, flags), _intrinsicSize(intrinsicSize)
  {
    // FIXME: Assert the element is replaced (Element::isReplaced), once replaced elements say so.
    SetBlockLevelReplacedOrAtomicInline(true);
    krys_debug_assert(IsRenderReplaced());
  }

  RenderReplaced::RenderReplaced(Type type, dom::Document &document, RenderStyle &&style,
                                 const LayoutSize &intrinsicSize, EnumSet<ReplacedFlag> flags) noexcept
      : RenderBox(type, document, krys::move(style), {}, flags), _intrinsicSize(intrinsicSize)
  {
    SetBlockLevelReplacedOrAtomicInline(true);
    krys_debug_assert(IsRenderReplaced());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderReplaced::~RenderReplaced() noexcept = default;

  bool RenderReplaced::ShouldRespectZeroIntrinsicWidth() const noexcept
  {
    // Per CSSWG resolution, 0px intrinsic width should be respected for SVG
    // items and coerce the intrinsic height to 0px as well. Note that this is
    // not the case for 0px intrinsic height SVGs or for other RenderReplaced items.
    return false;
  }

  void RenderReplaced::WillBeDestroyed() noexcept
  {
    // FIXME: Dirty the parent's legacy line boxes (dirtyLineFromChangedChild), once there's SVG text.
    RenderBox::WillBeDestroyed();
  }

  void RenderReplaced::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBox::StyleDidChange(diff, oldStyle);
    auto previousUsedZoom = oldStyle ? oldStyle->UsedZoom() : 1.0f;
    if (previousUsedZoom != Style().UsedZoom())
    {
      IntrinsicSizeChanged();
    }
  }

  void RenderReplaced::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    LayoutRepainter repainter(*this);

    LayoutRect oldContentRect = ReplacedContentRect();

    SetHeight(MinimumReplacedHeight());

    UpdateLogicalWidth();
    UpdateLogicalHeight();

    ClearOverflow();
    AddVisualEffectOverflow();
    UpdateLayerTransform();
    InvalidateBackgroundObscurationStatus();
    repainter.RepaintAfterLayout();
    ClearNeedsLayout();

    if (ReplacedContentRect() != oldContentRect)
    {
      SetNeedsPreferredWidthsUpdate();
      if (ShouldRepaintOnSizeChange(*this))
      {
        Repaint();
      }
    }
  }

  auto RenderReplaced::LocalRectsForRepaint(RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    if (IsInsideEntirelyHiddenLayer())
    {
      return {};
    }

    // FIXME: The selection rect (which can project outside of the overflow rect), once there's selection.
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

  void RenderReplaced::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (!ShouldPaint(paintInfo, paintOffset))
    {
      return;
    }

    LayoutPoint adjustedPaintOffset = paintOffset + Location();

    if (paintInfo.Phase == PaintPhase::EventRegion)
    {
      // FIXME: Event regions come with event regions.
      return;
    }

    if (paintInfo.Phase == PaintPhase::Accessibility)
    {
      // FIXME: Accessibility bounds come with accessibility.
      return;
    }

    // FIXME: Dragged and transparent content markers (setAlpha), once there are document markers.

    if (HasVisibleBoxDecorations() && paintInfo.Phase == PaintPhase::Foreground)
    {
      PaintBoxDecorations(paintInfo, adjustedPaintOffset);
    }

    if (paintInfo.Phase == PaintPhase::Mask)
    {
      PaintMask(paintInfo, adjustedPaintOffset);
      return;
    }

    if (paintInfo.Phase == PaintPhase::ClippingMask && Style().UsedVisibility() == Visibility::Visible)
    {
      PaintClippingMask(paintInfo, adjustedPaintOffset);
      return;
    }

    LayoutRect paintRect = LayoutRect(adjustedPaintOffset, Size());
    if (paintInfo.Phase == PaintPhase::Outline || paintInfo.Phase == PaintPhase::SelfOutline)
    {
      if (HasOutline())
      {
        PaintOutline(paintInfo, paintRect);
      }
      return;
    }

    if (paintInfo.Phase != PaintPhase::Foreground && paintInfo.Phase != PaintPhase::Selection)
    {
      return;
    }

    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return;
    }

    // FIXME: The highlight colour and the selection tint, once there's selection.
    if (paintInfo.Phase == PaintPhase::Selection)
    {
      return;
    }

    bool completelyClippedOut = false;
    if (Style().HasBorderRadius())
    {
      completelyClippedOut = Size().IsEmpty();
      if (!completelyClippedOut)
      {
        // Push a clip if we have a border radius, since we want to round the foreground content that gets
        // painted.
        paintInfo.Context().Save();
        ClipToContentBoxShape(paintInfo.Context(), adjustedPaintOffset, GetDocument().DeviceScaleFactor());
      }
    }

    if (!completelyClippedOut)
    {
      if (!IsSkippedContentRoot(*this))
      {
        PaintReplaced(paintInfo, adjustedPaintOffset);
      }

      if (Style().HasBorderRadius())
      {
        paintInfo.Context().Restore();
      }
    }
  }

  bool RenderReplaced::ShouldPaint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    // FIXME: Excluding selected replaced content (PaintBehavior::ExcludeSelection), once there's selection.

    if (paintInfo.PaintBehavior.Contains(PaintBehavior::ExcludeReplacedContentExceptForIFrames))
    {
      // FIXME: Except iframes, once they have renderers.
      return false;
    }

    if (paintInfo.Phase != PaintPhase::Foreground && paintInfo.Phase != PaintPhase::Outline
        && paintInfo.Phase != PaintPhase::SelfOutline && paintInfo.Phase != PaintPhase::Selection
        && paintInfo.Phase != PaintPhase::Mask && paintInfo.Phase != PaintPhase::ClippingMask
        && paintInfo.Phase != PaintPhase::EventRegion && paintInfo.Phase != PaintPhase::Accessibility)
    {
      return false;
    }

    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return false;
    }

    // if we're invisible or haven't received a layout yet, then just bail.
    if (Style().UsedVisibility() != Visibility::Visible)
    {
      return false;
    }

    LayoutRect paintRect(VisualOverflowRect());
    paintRect.MoveBy(paintOffset + Location());

    // Early exit if the element touches the edges.
    LayoutUnit top = paintRect.Y();
    LayoutUnit bottom = paintRect.MaxY();

    LayoutRect localRepaintRect = paintInfo.Rect;
    if (paintRect.X() >= localRepaintRect.MaxX() || paintRect.MaxX() <= localRepaintRect.X())
    {
      return false;
    }

    if (top >= localRepaintRect.MaxY() || bottom <= localRepaintRect.Y())
    {
      return false;
    }

    return true;
  }

  void RenderReplaced::IntrinsicSizeChanged() noexcept
  {
    auto scaledWidth = static_cast<int32>(DefaultWidth * Style().UsedZoom());
    auto scaledHeight = static_cast<int32>(DefaultHeight * Style().UsedZoom());
    _intrinsicSize = LayoutSize(scaledWidth, scaledHeight);
    SetNeedsLayoutAndPreferredWidthsUpdate();
  }

  bool RenderReplaced::HasReplacedLogicalHeight() const noexcept
  {
    if (Style().LogicalHeight().IsAuto())
    {
      return false;
    }

    if (Style().LogicalHeight().IsFixed())
    {
      return true;
    }

    if (Style().LogicalHeight().IsPercentOrCalculated())
    {
      return !HasAutoHeightOrContainingBlockWithAutoHeight();
    }

    if (Style().LogicalHeight().IsIntrinsic())
    {
      return !Style().HasAspectRatio();
    }

    return false;
  }

  bool RenderReplaced::SetNeedsLayoutIfNeededAfterIntrinsicSizeChange() noexcept
  {
    SetNeedsPreferredWidthsUpdate();

    // If the actual area occupied by the image has changed and it is not constrained by style then a layout
    // is required.
    bool imageSizeIsConstrained =
      Style().LogicalWidth().IsSpecified() && Style().LogicalHeight().IsSpecified()
      && !Style().LogicalMinWidth().IsIntrinsic() && !Style().LogicalMaxWidth().IsIntrinsic()
      && !HasAutoHeightOrContainingBlockWithAutoHeight(UpdatePercentageHeightDescendants::No);

    // FIXME: We only need to recompute the containing block's preferred size
    // if the containing block's size depends on the image's size (i.e., the container uses shrink-to-fit
    // sizing). There's no easy way to detect that shrink-to-fit is needed, always force a layout.
    bool containingBlockNeedsToRecomputePreferredSize = Style().LogicalWidth().IsPercentOrCalculated()
                                                        || Style().LogicalMaxWidth().IsPercentOrCalculated()
                                                        || Style().LogicalMinWidth().IsPercentOrCalculated();

    // Flex and grid layout use the intrinsic image width/height even if width/height are specified.
    if (!imageSizeIsConstrained || containingBlockNeedsToRecomputePreferredSize || IsFlexItem()
        || IsGridItem())
    {
      SetNeedsLayout();
      return true;
    }

    return false;
  }

  void RenderReplaced::ComputeAspectRatioInformationForRenderBox(
    RenderBox *contentRenderer, FloatSize &constrainedSize, FloatSize &preferredAspectRatio) const noexcept
  {
    FloatSize intrinsicSize;
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      intrinsicSize = RenderReplaced::ComputeIntrinsicSize();
      preferredAspectRatio = RenderReplaced::PreferredAspectRatio();
    }
    else if (contentRenderer)
    {
      if (auto *renderReplaced = DynamicDowncast<RenderReplaced>(contentRenderer))
      {
        intrinsicSize = renderReplaced->ComputeIntrinsicSize();
        preferredAspectRatio = renderReplaced->PreferredAspectRatio();
      }
      if (Style().GetAspectRatio().IsRatio()
          || (Style().GetAspectRatio().IsAutoAndRatio() && preferredAspectRatio.IsEmpty()))
      {
        preferredAspectRatio = FloatSize::NarrowPrecision(Style().GetAspectRatio().Width().Value,
                                                          Style().GetAspectRatio().Height().Value);
      }

      // Handle zoom & vertical writing modes here, as the embedded document doesn't know about them.
      intrinsicSize.Scale(Style().UsedZoom());

      if (auto *image = DynamicDowncast<RenderImage>(*this))
      {
        intrinsicSize.Scale(image->ImageDevicePixelRatio());
      }

      // Update our intrinsic size to match what the content renderer has computed, so that when we
      // constrain the size below, the correct intrinsic size will be obtained for comparison against
      // min and max widths.
      if (!preferredAspectRatio.IsEmpty() && !intrinsicSize.IsZero())
      {
        _intrinsicSize = LayoutSize(intrinsicSize);
      }

      if (!IsHorizontalWritingMode())
      {
        if (!preferredAspectRatio.IsEmpty())
        {
          preferredAspectRatio = preferredAspectRatio.TransposedSize();
        }
        intrinsicSize = intrinsicSize.TransposedSize();
      }
    }
    else
    {
      intrinsicSize = ComputeIntrinsicSize();
      preferredAspectRatio = this->PreferredAspectRatio();
      if (!preferredAspectRatio.IsEmpty() && !intrinsicSize.IsZero())
      {
        _intrinsicSize =
          LayoutSize(IsHorizontalWritingMode() ? intrinsicSize : intrinsicSize.TransposedSize());
      }
    }
    constrainedSize = intrinsicSize;
  }

  void RenderReplaced::ComputeIntrinsicSizesConstrainedByTransferredMinMaxSizes(
    RenderBox *contentRenderer, FloatSize &intrinsicSize, FloatSize &intrinsicRatio) const noexcept
  {
    ComputeAspectRatioInformationForRenderBox(contentRenderer, intrinsicSize, intrinsicRatio);

    // Now constrain the intrinsic size along each axis according to minimum and maximum width/heights along
    // the opposite axis. So for example a maximum width that shrinks our width will result in the height we
    // compute here having to shrink in order to preserve the aspect ratio. Because we compute these values
    // independently along each axis, the final returned size may in fact not preserve the aspect ratio.
    const auto &style = Style();
    auto computedLogicalHeight = style.LogicalHeight();
    bool logicalHeightBehavesAsAuto =
      computedLogicalHeight.IsAuto()
      || (computedLogicalHeight.IsPercentOrCalculated() && !PercentageLogicalHeightIsResolvable());
    if (!intrinsicRatio.IsZero() && style.LogicalWidth().IsAuto() && logicalHeightBehavesAsAuto)
    {
      auto removeBorderAndPaddingFromMinMaxSizes =
        [](LayoutUnit &minSize, LayoutUnit &maxSize, LayoutUnit borderAndPadding)
      {
        minSize = std::max(0_lu, minSize - borderAndPadding);
        maxSize = std::max(0_lu, maxSize - borderAndPadding);
      };

      auto [minLogicalWidth, maxLogicalWidth] = ComputeMinMaxLogicalWidthFromAspectRatio();
      removeBorderAndPaddingFromMinMaxSizes(minLogicalWidth, maxLogicalWidth, BorderAndPaddingLogicalWidth());

      auto [minLogicalHeight, maxLogicalHeight] = ComputeMinMaxLogicalHeightFromAspectRatio();
      removeBorderAndPaddingFromMinMaxSizes(minLogicalHeight, maxLogicalHeight,
                                            BorderAndPaddingLogicalHeight());

      intrinsicSize.SetWidth(
        std::clamp(LayoutUnit {intrinsicSize.Width()}, minLogicalWidth, maxLogicalWidth));
      intrinsicSize.SetHeight(
        std::clamp(LayoutUnit {intrinsicSize.Height()}, minLogicalHeight, maxLogicalHeight));
    }
  }

  dom::PositionWithAffinity RenderReplaced::PositionForPoint(const LayoutPoint &point,
                                                             HitTestSource source) noexcept
  {
    auto [top, bottom] = [&]() -> std::pair<float, float>
    {
      if (auto run = inline_iterator::BoxFor(*this))
      {
        auto lineBox = run->GetLineBox();
        auto lineContentTop =
          LayoutUnit {std::min(inline_iterator::PreviousLineBoxContentBottomOrBorderAndPadding(*lineBox),
                               lineBox->ContentLogicalTop())};
        return std::make_pair(lineContentTop.ToFloat(), LineSelection::LogicalBottom(*lineBox));
      }
      return std::make_pair(LogicalTop().ToFloat(), LogicalBottom().ToFloat());
    }();

    LayoutUnit blockDirectionPosition = IsHorizontalWritingMode() ? point.Y() + Y() : point.X() + X();
    LayoutUnit lineDirectionPosition = IsHorizontalWritingMode() ? point.X() + X() : point.Y() + Y();

    if (blockDirectionPosition < top)
    {
      return CreatePositionWithAffinity(CaretMinOffset(), Affinity::Downstream); // coordinates are above
    }

    if (blockDirectionPosition >= bottom)
    {
      return CreatePositionWithAffinity(CaretMaxOffset(), Affinity::Downstream); // coordinates are below
    }

    if (GetElement())
    {
      if (lineDirectionPosition <= LogicalLeft() + (LogicalWidth() / 2))
      {
        return CreatePositionWithAffinity(0, Affinity::Downstream);
      }
      return CreatePositionWithAffinity(1, Affinity::Downstream);
    }

    return RenderBox::PositionForPoint(point, source);
  }

  LayoutRect RenderReplaced::ReplacedContentRect(const LayoutSize &intrinsicSize) const noexcept
  {
    LayoutRect contentRect = ContentBoxRect();
    if (intrinsicSize.IsEmpty())
    {
      return contentRect;
    }

    ObjectFit objectFit = Style().GetObjectFit();

    LayoutRect finalRect = contentRect;
    switch (objectFit)
    {
      case ObjectFit::Contain:
      case ObjectFit::ScaleDown:
      case ObjectFit::Cover:
        finalRect.SetSize(finalRect.Size().FitToAspectRatio(
          intrinsicSize, objectFit == ObjectFit::Cover ? AspectRatioFit::Grow : AspectRatioFit::Shrink));
        if (objectFit != ObjectFit::ScaleDown || finalRect.Width() <= intrinsicSize.Width())
        {
          break;
        }
        [[fallthrough]];
      case ObjectFit::None: finalRect.SetSize(intrinsicSize); break;
      case ObjectFit::Fill: break;
    }

    const auto &objectPosition = Style().GetObjectPosition();

    auto xOffset = style::Evaluate<LayoutUnit>(objectPosition.X, contentRect.Width() - finalRect.Width(),
                                               style::ZoomNeeded {});
    auto yOffset = style::Evaluate<LayoutUnit>(objectPosition.Y, contentRect.Height() - finalRect.Height(),
                                               style::ZoomNeeded {});

    finalRect.Move(xOffset, yOffset);

    return finalRect;
  }

  double RenderReplaced::ComputeIntrinsicAspectRatio() const noexcept
  {
    FloatSize intrinsicRatio;
    FloatSize intrinsicSize;
    ComputeAspectRatioInformationForRenderBox(EmbeddedContentBox(), intrinsicSize, intrinsicRatio);
    return intrinsicRatio.AspectRatioDouble();
  }

  FloatSize RenderReplaced::ComputeIntrinsicSize() const noexcept
  {
    // If there's an embeddedContentBox() of a remote, referenced document available, this code-path should
    // never be used.
    krys_debug_assert(!EmbeddedContentBox() || ShouldApplySizeOrInlineSizeContainment());
    return {IntrinsicLogicalWidth().ToFloat(), IntrinsicLogicalHeight().ToFloat()};
  }

  FloatSize RenderReplaced::PreferredAspectRatio() const noexcept
  {
    // If there's an embeddedContentBox() of a remote, referenced document available, this code-path should
    // never be used.
    krys_debug_assert(!EmbeddedContentBox() || ShouldApplySizeOrInlineSizeContainment());
    auto intrinsicSize = FloatSize(IntrinsicLogicalWidth().ToFloat(), IntrinsicLogicalHeight().ToFloat());
    FloatSize preferredAspectRatio;

    if (Style().HasAspectRatio())
    {
      preferredAspectRatio = FloatSize::NarrowPrecision(Style().AspectRatioLogicalWidth().Value,
                                                        Style().AspectRatioLogicalHeight().Value);
      // FIXME: Videos with their default object size keep the style's ratio, once there's video.
      if (Style().GetAspectRatio().IsRatio())
      {
        return preferredAspectRatio;
      }
    }
    // Figure out if we need to compute an intrinsic ratio.
    // FIXME: SVG roots always compute one, once there's SVG.
    if (!RenderBox::HasIntrinsicAspectRatio())
    {
      return preferredAspectRatio;
    }

    // After supporting contain-intrinsic-size, the intrinsicSize of size containment is not always empty.
    if (intrinsicSize.IsEmpty() || ShouldApplySizeContainment())
    {
      return preferredAspectRatio;
    }

    return intrinsicSize;
  }

  LayoutUnit RenderReplaced::ComputeConstrainedLogicalWidth() const noexcept
  {
    // The aforementioned 'constraint equation' used for block-level, non-replaced
    // elements in normal flow:
    // 'margin-left' + 'border-left-width' + 'padding-left' + 'width' +
    // 'padding-right' + 'border-right-width' + 'margin-right' = width of
    // containing block
    // see https://www.w3.org/TR/CSS22/visudet.html#blockwidth
    LayoutUnit logicalWidth = IsOutOfFlowPositioned() ? ContainingBlock()->ClientLogicalWidth()
                                                      : ContainingBlock()->ContentBoxLogicalWidth();

    // This solves above equation for 'width' (== logicalWidth).
    auto marginStart =
      style::EvaluateMinimum<LayoutUnit>(Style().MarginStart(), logicalWidth, Style().UsedZoomForLength());
    auto marginEnd =
      style::EvaluateMinimum<LayoutUnit>(Style().MarginEnd(), logicalWidth, Style().UsedZoomForLength());

    return std::max(
      0_lu, (logicalWidth
             - (marginStart + marginEnd + BorderLeft() + BorderRight() + PaddingLeft() + PaddingRight())));
  }

  void RenderReplaced::ComputeAspectRatioAdjustedIntrinsicLogicalWidths(
    LayoutUnit &minLogicalWidth, LayoutUnit &maxLogicalWidth) const noexcept
  {
    ComputeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth);

    if (!HasIntrinsicAspectRatio())
    {
      return;
    }

    const auto &style = Style();
    auto computedAspectRatio = ComputeIntrinsicAspectRatio();
    auto computedIntrinsicLogicalWidth = minLogicalWidth;

    if (auto fixedLogicalHeight = style.LogicalHeight().TryFixed())
    {
      computedIntrinsicLogicalWidth =
        LayoutUnit {fixedLogicalHeight->ResolveZoom(style.UsedZoomForLength()) * computedAspectRatio};
    }

    if (auto fixedLogicalMaxHeight = style.LogicalMaxHeight().TryFixed())
    {
      computedIntrinsicLogicalWidth = std::min(
        computedIntrinsicLogicalWidth,
        LayoutUnit {fixedLogicalMaxHeight->ResolveZoom(style.UsedZoomForLength()) * computedAspectRatio});
    }

    if (auto fixedLogicalMinHeight = style.LogicalMinHeight().TryFixed())
    {
      computedIntrinsicLogicalWidth = std::max(
        computedIntrinsicLogicalWidth,
        LayoutUnit {fixedLogicalMinHeight->ResolveZoom(style.UsedZoomForLength()) * computedAspectRatio});
    }

    minLogicalWidth = computedIntrinsicLogicalWidth;
    maxLogicalWidth = minLogicalWidth;
  }

  static LayoutUnit ResolveWidthForRatio(LayoutUnit borderAndPaddingLogicalHeight,
                                         LayoutUnit borderAndPaddingLogicalWidth, LayoutUnit logicalHeight,
                                         double aspectRatio, BoxSizing boxSizing) noexcept
  {
    if (boxSizing == BoxSizing::BorderBox)
    {
      return LayoutUnit((logicalHeight + borderAndPaddingLogicalHeight) * aspectRatio)
             - borderAndPaddingLogicalWidth;
    }
    return LayoutUnit(logicalHeight * aspectRatio);
  }

  static bool HasIntrinsicSize(RenderBox *, bool hasIntrinsicWidth, bool hasIntrinsicHeight) noexcept
  {
    if (hasIntrinsicWidth && hasIntrinsicHeight)
    {
      return true;
    }
    // FIXME: SVG roots may have only one of them, once there's SVG.
    return false;
  }

  LayoutUnit
    RenderReplaced::ComputeReplacedLogicalWidth(ShouldComputePreferred shouldComputePreferred) const noexcept
  {
    const auto &style = Style();
    if (style.LogicalWidth().IsSpecified())
    {
      return ComputeReplacedLogicalWidthRespectingMinMaxWidth(
        ComputeReplacedLogicalWidthUsing(style.LogicalWidth()), shouldComputePreferred);
    }
    if (style.LogicalWidth().IsIntrinsic())
    {
      return ComputeReplacedLogicalWidthRespectingMinMaxWidth(
        ComputeReplacedLogicalWidthUsing(style.LogicalWidth()), shouldComputePreferred);
    }

    RenderBox *contentRenderer = EmbeddedContentBox();

    // 10.3.2 Inline, replaced elements: http://www.w3.org/TR/CSS21/visudet.html#inline-replaced-width
    FloatSize intrinsicRatio;
    FloatSize constrainedSize;
    ComputeIntrinsicSizesConstrainedByTransferredMinMaxSizes(contentRenderer, constrainedSize,
                                                             intrinsicRatio);

    if (style.LogicalWidth().IsAuto())
    {
      bool computedHeightIsAuto = style.LogicalHeight().IsAuto();
      bool hasIntrinsicWidth = constrainedSize.Width() > 0
                               || (!constrainedSize.Width() && ShouldRespectZeroIntrinsicWidth())
                               || ShouldApplySizeOrInlineSizeContainment();
      bool hasIntrinsicHeight = constrainedSize.Height() > 0 || ShouldApplySizeContainment();

      // For flex or grid items where the logical height has been overriden then we should use that size to
      // compute the replaced width as long as the flex or grid item has an intrinsic size. It is possible
      // (indeed, common) for an SVG graphic to have an intrinsic aspect ratio but not to have an intrinsic
      // width or height. There are also elements with intrinsic sizes but without intrinsic ratio (like an
      // iframe).
      if (auto overridingLogicalHeight =
            (!intrinsicRatio.IsEmpty() && (IsFlexItem() || IsGridItem())
             && HasIntrinsicSize(contentRenderer, hasIntrinsicWidth, hasIntrinsicHeight))
              ? this->OverridingBorderBoxLogicalHeight()
              : null)
      {
        return ComputeReplacedLogicalWidthRespectingMinMaxWidth(
          LayoutUnit(ContentBoxLogicalHeight(*overridingLogicalHeight) * intrinsicRatio.AspectRatioDouble()),
          shouldComputePreferred);
      }

      // If 'height' and 'width' both have computed values of 'auto' and the element also has an intrinsic
      // width, then that intrinsic width is the used value of 'width'.
      if (computedHeightIsAuto && hasIntrinsicWidth)
      {
        return ComputeReplacedLogicalWidthRespectingMinMaxWidth(LayoutUnit(constrainedSize.Width()),
                                                                shouldComputePreferred);
      }

      if (!intrinsicRatio.IsEmpty())
      {
        // If 'height' and 'width' both have computed values of 'auto' and the element has no intrinsic width,
        // but does have an intrinsic height and intrinsic ratio; or if 'width' has a computed value of
        // 'auto', 'height' has some other computed value, and the element does have an intrinsic ratio; then
        // the used value of 'width' is: (used height) * (intrinsic ratio)
        if (!computedHeightIsAuto || (!hasIntrinsicWidth && hasIntrinsicHeight))
        {
          auto estimatedUsedWidth = [&]
          {
            if (hasIntrinsicWidth)
            {
              return LayoutUnit(constrainedSize.Width());
            }

            if (shouldComputePreferred == ShouldComputePreferred::ComputePreferred)
            {
              return ComputeReplacedLogicalWidthRespectingMinMaxWidth(
                0_lu, ShouldComputePreferred::ComputePreferred);
            }

            auto constrainedLogicalWidth = ComputeConstrainedLogicalWidth();
            return ComputeReplacedLogicalWidthRespectingMinMaxWidth(constrainedLogicalWidth,
                                                                    ShouldComputePreferred::ComputeActual);
          }();

          LayoutUnit logicalHeight = ComputeReplacedLogicalHeight(Maybe<LayoutUnit>(estimatedUsedWidth));
          auto boxSizing = style.HasAspectRatio() ? style.BoxSizingForAspectRatio() : BoxSizing::ContentBox;
          return ComputeReplacedLogicalWidthRespectingMinMaxWidth(
            ResolveWidthForRatio(BorderAndPaddingLogicalHeight(), BorderAndPaddingLogicalWidth(),
                                 logicalHeight, intrinsicRatio.AspectRatioDouble(), boxSizing),
            shouldComputePreferred);
        }

        // If 'height' and 'width' both have computed values of 'auto' and the
        // element has an intrinsic ratio but no intrinsic height or width, then
        // the used value of 'width' is undefined in CSS 2.1. However, it is
        // suggested that, if the containing block's width does not itself depend
        // on the replaced element's width, then the used value of 'width' is
        // calculated from the constraint equation used for block-level,
        // non-replaced elements in normal flow.
        if (computedHeightIsAuto && !hasIntrinsicWidth && !hasIntrinsicHeight)
        {
          bool isFlexItemComputingBaseSize =
            IsFlexItem() && Downcast<RenderFlexibleBox>(Parent())->IsComputingFlexBaseSizes();
          if (shouldComputePreferred == ShouldComputePreferred::ComputePreferred
              && !isFlexItemComputingBaseSize)
          {
            return ComputeReplacedLogicalWidthRespectingMinMaxWidth(0_lu,
                                                                    ShouldComputePreferred::ComputePreferred);
          }

          auto constrainedLogicalWidth = ComputeConstrainedLogicalWidth();
          auto [transferredMinLogicalWidth, transferredMaxLogicalWidth] =
            ComputeMinMaxLogicalWidthFromAspectRatio();
          krys_debug_assert(transferredMinLogicalWidth <= transferredMaxLogicalWidth);
          constrainedLogicalWidth =
            std::clamp(constrainedLogicalWidth, transferredMinLogicalWidth, transferredMaxLogicalWidth);
          return ComputeReplacedLogicalWidthRespectingMinMaxWidth(constrainedLogicalWidth,
                                                                  ShouldComputePreferred::ComputeActual);
        }
      }

      // Otherwise, if 'width' has a computed value of 'auto', and the element has an intrinsic width, then
      // that intrinsic width is the used value of 'width'.
      if (hasIntrinsicWidth)
      {
        return ComputeReplacedLogicalWidthRespectingMinMaxWidth(LayoutUnit(constrainedSize.Width()),
                                                                shouldComputePreferred);
      }

      // Otherwise, if 'width' has a computed value of 'auto', but none of the conditions above are met, then
      // the used value of 'width' becomes 300px. If 300px is too wide to fit the device, UAs should use the
      // width of the largest rectangle that has a 2:1 ratio and fits the device instead. Note: We fall
      // through and instead return intrinsicLogicalWidth() here - to preserve existing behavior, which might
      // or might not be correct, or desired. Changing this to return the default width, will affect lots of
      // test results. Eg. some tests assume that a blank <img> tag (which implies width/height=auto) has no
      // intrinsic size, which is wrong per CSS 2.1, but matches our behavior since a long time.
    }

    return ComputeReplacedLogicalWidthRespectingMinMaxWidth(IntrinsicLogicalWidth(), shouldComputePreferred);
  }

  LayoutUnit RenderReplaced::ComputeReplacedLogicalHeight(Maybe<LayoutUnit> estimatedUsedWidth) const noexcept
  {
    // 10.5 Content height: the 'height' property: http://www.w3.org/TR/CSS21/visudet.html#propdef-height
    if (HasReplacedLogicalHeight())
    {
      return ComputeReplacedLogicalHeightRespectingMinMaxHeight(
        ComputeReplacedLogicalHeightUsing(Style().LogicalHeight()));
    }

    RenderBox *contentRenderer = EmbeddedContentBox();

    // 10.6.2 Inline, replaced elements: http://www.w3.org/TR/CSS21/visudet.html#inline-replaced-height
    FloatSize intrinsicRatio;
    FloatSize constrainedSize;
    ComputeIntrinsicSizesConstrainedByTransferredMinMaxSizes(contentRenderer, constrainedSize,
                                                             intrinsicRatio);

    bool widthIsAuto = Style().LogicalWidth().IsAuto();
    bool hasIntrinsicWidth = constrainedSize.Width() > 0
                             || (!constrainedSize.Width() && ShouldRespectZeroIntrinsicWidth())
                             || ShouldApplySizeOrInlineSizeContainment();
    bool hasIntrinsicHeight = constrainedSize.Height() > 0 || ShouldApplySizeContainment();

    // See computeReplacedLogicalHeight() for a similar check for heights.
    if (auto overridinglogicalWidth =
          (!intrinsicRatio.IsEmpty() && (IsFlexItem() || IsGridItem())
           && HasIntrinsicSize(contentRenderer, hasIntrinsicWidth, hasIntrinsicHeight))
            ? OverridingBorderBoxLogicalWidth()
            : null)
    {
      return ComputeReplacedLogicalHeightRespectingMinMaxHeight(
        LayoutUnit(ContentBoxLogicalWidth(*overridinglogicalWidth)
                   * intrinsicRatio.TransposedSize().AspectRatioDouble()));
    }

    // If 'height' and 'width' both have computed values of 'auto' and the element also has an intrinsic
    // height, then that intrinsic height is the used value of 'height'.
    if (widthIsAuto && hasIntrinsicHeight)
    {
      return ComputeReplacedLogicalHeightRespectingMinMaxHeight(LayoutUnit(constrainedSize.Height()));
    }

    // Otherwise, if 'height' has a computed value of 'auto', and the element has an intrinsic ratio then the
    // used value of 'height' is: (used width) / (intrinsic ratio)
    if (!intrinsicRatio.IsEmpty())
    {
      LayoutUnit usedWidth = estimatedUsedWidth ? *estimatedUsedWidth : ContentBoxLogicalWidth();
      BoxSizing boxSizing = BoxSizing::ContentBox;
      if (Style().HasAspectRatio())
      {
        boxSizing = Style().BoxSizingForAspectRatio();
      }
      return ComputeReplacedLogicalHeightRespectingMinMaxHeight(
        ResolveHeightForRatio(BorderAndPaddingLogicalWidth(), BorderAndPaddingLogicalHeight(), usedWidth,
                              intrinsicRatio.TransposedSize().AspectRatioDouble(), boxSizing));
    }

    // Otherwise, if 'height' has a computed value of 'auto', and the element has an intrinsic height, then
    // that intrinsic height is the used value of 'height'.
    if (hasIntrinsicHeight)
    {
      return ComputeReplacedLogicalHeightRespectingMinMaxHeight(LayoutUnit(constrainedSize.Height()));
    }

    // Otherwise, if 'height' has a computed value of 'auto', but none of the conditions above are met, then
    // the used value of 'height' must be set to the height of the largest rectangle that has a 2:1 ratio, has
    // a height not greater than 150px, and has a width not greater than the device width.
    return ComputeReplacedLogicalHeightRespectingMinMaxHeight(IntrinsicLogicalHeight());
  }

  void RenderReplaced::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                     LayoutUnit &maxLogicalWidth) const noexcept
  {
    minLogicalWidth = maxLogicalWidth = IntrinsicLogicalWidth();
  }

  void RenderReplaced::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    // We cannot resolve any percent logical width here as the available logical
    // width may not be set on our containing block.
    if (Style().LogicalWidth().IsPercentOrCalculated())
    {
      ComputeAspectRatioAdjustedIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
    }
    else
    {
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth =
        ComputeReplacedLogicalWidth(ShouldComputePreferred::ComputePreferred);
    }

    bool ignoreMinMaxSizes = ShouldIgnoreLogicalMinMaxWidthSizes();
    const RenderStyle &styleToUse = Style();
    if (styleToUse.LogicalWidth().IsPercentOrCalculated()
        || styleToUse.LogicalMaxWidth().IsPercentOrCalculated())
    {
      _minPreferredLogicalWidth = 0_lu;
    }

    if (auto fixedLogicalMinWidth = styleToUse.LogicalMinWidth().TryFixed();
        !ignoreMinMaxSizes && fixedLogicalMinWidth && fixedLogicalMinWidth->IsPositive())
    {
      auto minWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalMinWidth->ResolveZoom(styleToUse.UsedZoomForLength())});
      _maxPreferredLogicalWidth = std::max(_maxPreferredLogicalWidth, minWidth);
      _minPreferredLogicalWidth = std::max(_minPreferredLogicalWidth, minWidth);
    }

    if (auto fixedLogicalMaxWidth = styleToUse.LogicalMaxWidth().TryFixed();
        !ignoreMinMaxSizes && fixedLogicalMaxWidth)
    {
      auto maxWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalMaxWidth->ResolveZoom(styleToUse.UsedZoomForLength())});
      _maxPreferredLogicalWidth = std::min(_maxPreferredLogicalWidth, maxWidth);
      _minPreferredLogicalWidth = std::min(_minPreferredLogicalWidth, maxWidth);
    }

    LayoutUnit borderAndPadding = BorderAndPaddingLogicalWidth();
    _minPreferredLogicalWidth += borderAndPadding;
    _maxPreferredLogicalWidth += borderAndPadding;

    ClearNeedsPreferredWidthsUpdate();
  }

  bool RenderReplaced::ShouldInvalidatePreferredWidths() const noexcept
  {
    // If the height is a percentage and the width is auto, then the containingBlocks's height changing can
    // cause this node to change it's preferred width because it maintains aspect ratio.
    return (HasRelativeLogicalHeight() || (IsGridItem() && HasStretchedLogicalHeight()))
           && Style().LogicalWidth().IsAuto();
  }

  LayoutSize RenderReplaced::IntrinsicSize() const noexcept
  {
    if (!View().FrameView().LayoutContext().IsInRenderTreeLayout())
    {
      // 'contain' removes the natural aspect ratio / width / height only for the purposes of sizing and
      // layout of the box.
      return _intrinsicSize;
    }

    auto size = _intrinsicSize;
    auto zoomValue = Style().UsedZoom();
    if (IsHorizontalWritingMode() ? ShouldApplySizeOrInlineSizeContainment() : ShouldApplySizeContainment())
    {
      size.SetWidth(ExplicitIntrinsicInnerWidth().value_or(0_lu) * zoomValue);
    }
    if (IsHorizontalWritingMode() ? ShouldApplySizeContainment() : ShouldApplySizeOrInlineSizeContainment())
    {
      size.SetHeight(ExplicitIntrinsicInnerHeight().value_or(0_lu) * zoomValue);
    }
    return size;
  }

  FloatSize RenderReplaced::IntrinsicRatio() const noexcept
  {
    FloatSize intrinsicRatio;
    FloatSize constrainedSize;
    ComputeAspectRatioInformationForRenderBox(EmbeddedContentBox(), constrainedSize, intrinsicRatio);
    return intrinsicRatio;
  }

  LayoutUnit RenderReplaced::ComputeReplacedLogicalWidthRespectingMinMaxWidth(
    LayoutUnit logicalWidth, ShouldComputePreferred shouldComputePreferred) const noexcept
  {
    if (ShouldIgnoreLogicalMinMaxWidthSizes())
    {
      return logicalWidth;
    }

    const auto &logicalMinWidth = Style().LogicalMinWidth();
    const auto &logicalMaxWidth = Style().LogicalMaxWidth();
    bool useLogicalWidthForMinWidth = (shouldComputePreferred == ShouldComputePreferred::ComputePreferred
                                       && logicalMinWidth.IsPercentOrCalculated());
    bool useLogicalWidthForMaxWidth = (shouldComputePreferred == ShouldComputePreferred::ComputePreferred
                                       && logicalMaxWidth.IsPercentOrCalculated())
                                      || logicalMaxWidth.IsNone();
    auto minLogicalWidth =
      useLogicalWidthForMinWidth ? logicalWidth : ComputeReplacedLogicalWidthUsing(logicalMinWidth);
    auto maxLogicalWidth =
      useLogicalWidthForMaxWidth ? logicalWidth : ComputeReplacedLogicalWidthUsing(logicalMaxWidth);
    return std::max(minLogicalWidth, std::min(logicalWidth, maxLogicalWidth));
  }

  template <typename SizeType>
  LayoutUnit RenderReplaced::ComputeReplacedLogicalWidthUsing(const SizeType &logicalWidth) const noexcept
  {
    auto calculateContainerWidth = [&]
    {
      if (IsOutOfFlowPositioned())
      {
        PositionedLayoutConstraints constraints(*this, LogicalBoxAxis::Inline);
        return constraints.ContainingSize();
      }
      if (IsHorizontalWritingMode() == ContainingBlock()->IsHorizontalWritingMode())
      {
        return ContainingBlockLogicalWidthForContent();
      }
      return PerpendicularContainingBlockLogicalHeight();
    };

    if (auto fixedLogicalWidth = logicalWidth.TryFixed())
    {
      return AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength())});
    }

    if (logicalWidth.IsPercentOrCalculated())
    {
      // FIXME: Handle cases when containing block width is calculated or viewport percent.
      // https://bugs.webkit.org/show_bug.cgi?id=91071
      if (auto containerWidth = calculateContainerWidth();
          containerWidth > 0
          || (!containerWidth && (ContainingBlock()->Style().LogicalWidth().IsSpecified())))
      {
        return AdjustContentBoxLogicalWidthForBoxSizing(
          style::Evaluate<LayoutUnit>(logicalWidth, containerWidth, Style().UsedZoomForLength()));
      }
      return 0_lu;
    }

    auto content = [&](LayoutUnit availableLogicalWidth)
    {
      // FIXME: Handle cases when containing block width is calculated or viewport percent.
      // https://bugs.webkit.org/show_bug.cgi?id=91071
      return ComputeIntrinsicLogicalWidthUsing(logicalWidth, availableLogicalWidth,
                                               BorderAndPaddingLogicalWidth())
             - BorderAndPaddingLogicalWidth();
    };

    if (logicalWidth.IsFitContent() || logicalWidth.IsFillAvailable())
    {
      return content(calculateContainerWidth());
    }
    if (logicalWidth.IsMinContent() || logicalWidth.IsMaxContent())
    {
      // min-content/max-content don't need the availableLogicalWidth argument.
      return content(0_lu);
    }
    if constexpr (std::same_as<SizeType, style::MinimumSize>)
    {
      if (logicalWidth.IsAuto())
      {
        return 0_lu;
      }
    }
    // intrinsic, min-intrinsic, auto and none.
    return IntrinsicLogicalWidth();
  }

  bool
    RenderReplaced::ReplacedMinMaxLogicalHeightComputesAsNone(const auto &logicalHeight,
                                                              const auto &initialLogicalHeight) const noexcept
  {
    if (logicalHeight == initialLogicalHeight)
    {
      return true;
    }

    if (IsGridItem() && logicalHeight.IsPercentOrCalculated())
    {
      if (auto gridAreaContentLogicalHeight = this->GridAreaContentLogicalHeight())
      {
        return !*gridAreaContentLogicalHeight;
      }
    }

    // Make sure % min-height and % max-height resolve to none if the containing block has auto height.
    // Note that the "height" case for replaced elements was handled by hasReplacedLogicalHeight, which is why
    // min and max-height are the only ones handled here.
    if (auto *containingBlock = ContainingBlockForAutoHeightDetection(logicalHeight))
    {
      return containingBlock->HasAutoHeightOrContainingBlockWithAutoHeight();
    }
    return false;
  }

  bool RenderReplaced::ReplacedMinLogicalHeightComputesAsNone() const noexcept
  {
    return ReplacedMinMaxLogicalHeightComputesAsNone(Style().LogicalMinHeight(),
                                                     style::ComputedStyle::InitialMinHeight());
  }

  bool RenderReplaced::ReplacedMaxLogicalHeightComputesAsNone() const noexcept
  {
    return ReplacedMinMaxLogicalHeightComputesAsNone(Style().LogicalMaxHeight(),
                                                     style::ComputedStyle::InitialMaxHeight());
  }

  LayoutUnit RenderReplaced::ComputeReplacedLogicalHeightRespectingMinMaxHeight(
    LayoutUnit logicalHeight) const noexcept
  {
    if (ShouldIgnoreLogicalMinMaxHeightSizes())
    {
      return logicalHeight;
    }

    LayoutUnit minLogicalHeight;
    if (!ReplacedMinLogicalHeightComputesAsNone())
    {
      minLogicalHeight = ComputeReplacedLogicalHeightUsing(Style().LogicalMinHeight());
    }
    LayoutUnit maxLogicalHeight = logicalHeight;
    if (!ReplacedMaxLogicalHeightComputesAsNone())
    {
      maxLogicalHeight = ComputeReplacedLogicalHeightUsing(Style().LogicalMaxHeight());
    }
    return std::max(minLogicalHeight, std::min(logicalHeight, maxLogicalHeight));
  }

  template <typename SizeType>
  LayoutUnit
    RenderReplaced::ComputeReplacedLogicalHeightUsingGeneric(const SizeType &logicalHeight) const noexcept
  {
    // This function should get called with Style::MinimumSize/Style::MaximumSize only if
    // replaced[Min|Max]LogicalHeightComputesAsNone returns false, otherwise we should not try to compute
    // those values as they may be incorrect. The caller should make sure this condition holds before calling
    // this function
    if constexpr (std::same_as<SizeType, style::MinimumSize>)
    {
      krys_debug_assert(!ReplacedMinLogicalHeightComputesAsNone());
    }
    else if constexpr (std::same_as<SizeType, style::MaximumSize>)
    {
      krys_debug_assert(!ReplacedMaxLogicalHeightComputesAsNone());
    }

    auto percentageOrCalculated = [&]
    {
      RenderElement *container = IsOutOfFlowPositioned() ? this->Container() : ContainingBlock();
      while (container && container->IsAnonymousForPercentageResolution())
      {
        // Stop at rendering context root.
        if (Is<RenderView>(*container))
        {
          break;
        }
        container = container->ContainingBlock();
      }
      bool hasPerpendicularContainingBlock =
        container->IsHorizontalWritingMode() != IsHorizontalWritingMode();
      Maybe<LayoutUnit> stretchedHeight;
      if (auto *block = DynamicDowncast<RenderBlock>(container))
      {
        block->AddPercentHeightDescendant(*const_cast<RenderReplaced *>(this));
        if (auto usedFlexItemOverridingLogicalHeightForPercentageResolutionForFlex =
              (block->IsFlexItem() ? Downcast<RenderFlexibleBox>(block->Parent())
                                       ->UsedFlexItemOverridingLogicalHeightForPercentageResolution(*block)
                                   : null))
        {
          stretchedHeight = block->ContentBoxLogicalHeight(
            *usedFlexItemOverridingLogicalHeightForPercentageResolutionForFlex);
        }
        else if (auto usedChildOverridingLogicalHeightForGrid =
                   (block->IsGridItem() && !hasPerpendicularContainingBlock
                      ? block->OverridingBorderBoxLogicalHeight()
                      : null))
        {
          stretchedHeight = block->ContentBoxLogicalHeight(*usedChildOverridingLogicalHeightForGrid);
        }
      }

      // FIXME: This calculation is not patched for block-flow yet.
      // https://bugs.webkit.org/show_bug.cgi?id=46500
      if (container->IsOutOfFlowPositioned() && container->Style().Height().IsAuto()
          && !(container->Style().Top().IsAuto() || container->Style().Bottom().IsAuto()))
      {
        auto &block = Downcast<RenderBlock>(*container);
        auto computedValues = block.ComputeLogicalHeight(block.LogicalHeight(), 0_lu);
        LayoutUnit borderPaddingAdjustment =
          IsOutOfFlowPositioned() ? block.BorderLogicalHeight() : block.BorderAndPaddingLogicalHeight();
        LayoutUnit newContentHeight =
          computedValues.Extent - LayoutUnit(block.ScrollbarLogicalHeight()) - borderPaddingAdjustment;

        return AdjustContentBoxLogicalHeightForBoxSizing(
          style::Evaluate<LayoutUnit>(logicalHeight, newContentHeight, Style().UsedZoomForLength()));
      }

      LayoutUnit availableHeight;
      if (IsOutOfFlowPositioned())
      {
        PositionedLayoutConstraints constraints(*this, LogicalBoxAxis::Block);
        availableHeight = constraints.ContainingSize();
      }
      else if (stretchedHeight)
      {
        availableHeight = *stretchedHeight;
      }
      else if (auto gridAreaLogicalHeight = IsGridItem() ? this->GridAreaContentLogicalHeight() : null;
               gridAreaLogicalHeight && *gridAreaLogicalHeight)
      {
        availableHeight = **gridAreaLogicalHeight;
      }
      else
      {
        availableHeight =
          hasPerpendicularContainingBlock
            ? ContainingBlockLogicalWidthForContent()
            : ContainingBlockLogicalHeightForContent(AvailableLogicalHeightType::IncludeMarginBorderPadding);
        // It is necessary to use the border-box to match WinIE's broken
        // box model. This is essential for sizing inside
        // table cells using percentage heights.
        // FIXME: This needs to be made block-flow-aware. If the cell and image are perpendicular block-flows,
        // this isn't right. https://bugs.webkit.org/show_bug.cgi?id=46997
        while (container && !Is<RenderView>(*container)
               && (container->Style().LogicalHeight().IsAuto()
                   || container->Style().LogicalHeight().IsPercentOrCalculated()))
        {
          if (container->IsRenderTableCell())
          {
            // Don't let table cells squeeze percent-height replaced elements
            // <http://bugs.webkit.org/show_bug.cgi?id=15359>
            availableHeight = std::max(availableHeight, IntrinsicLogicalHeight());
            return style::Evaluate<LayoutUnit>(
              logicalHeight, availableHeight - BorderAndPaddingLogicalHeight(), Style().UsedZoomForLength());
          }
          Downcast<RenderBlock>(*container).AddPercentHeightDescendant(const_cast<RenderReplaced &>(*this));
          container = container->ContainingBlock();
        }
      }

      return AdjustContentBoxLogicalHeightForBoxSizing(
        style::Evaluate<LayoutUnit>(logicalHeight, availableHeight, Style().UsedZoomForLength()));
    };

    auto content = [&]
    {
      return AdjustContentBoxLogicalHeightForBoxSizing(ComputeIntrinsicLogicalContentHeightUsing(
        logicalHeight, IntrinsicLogicalHeight(), BorderAndPaddingLogicalHeight()));
    };

    if (auto fixedLogicalHeight = logicalHeight.TryFixed())
    {
      return AdjustContentBoxLogicalHeightForBoxSizing(
        LayoutUnit {fixedLogicalHeight->ResolveZoom(Style().UsedZoomForLength())});
    }
    if (logicalHeight.IsPercentOrCalculated())
    {
      return percentageOrCalculated();
    }
    if (logicalHeight.IsFitContent() || logicalHeight.IsFillAvailable() || logicalHeight.IsMinContent()
        || logicalHeight.IsMaxContent())
    {
      return content();
    }
    if constexpr (std::same_as<SizeType, style::MinimumSize>)
    {
      if (logicalHeight.IsAuto())
      {
        return AdjustContentBoxLogicalHeightForBoxSizing(LayoutUnit {0});
      }
    }
    // intrinsic, min-intrinsic, auto and none.
    return IntrinsicLogicalHeight();
  }

  LayoutUnit RenderReplaced::ComputeReplacedLogicalHeightUsing(
    const style::PreferredSize &logicalHeight) const noexcept
  {
    return ComputeReplacedLogicalHeightUsingGeneric(logicalHeight);
  }

  LayoutUnit
    RenderReplaced::ComputeReplacedLogicalHeightUsing(const style::MinimumSize &logicalHeight) const noexcept
  {
    return ComputeReplacedLogicalHeightUsingGeneric(logicalHeight);
  }

  LayoutUnit
    RenderReplaced::ComputeReplacedLogicalHeightUsing(const style::MaximumSize &logicalHeight) const noexcept
  {
    return ComputeReplacedLogicalHeightUsingGeneric(logicalHeight);
  }

  void RenderReplaced::ComputeReplacedOutOfFlowPositionedLogicalWidth(
    LogicalExtentComputedValues &computedValues) const noexcept
  {
    PositionedLayoutConstraints inlineConstraints(*this, LogicalBoxAxis::Inline);
    inlineConstraints.ComputeInsets();

    // NOTE: This value of width is final in that the min/max width calculations
    // are dealt with in computeReplacedWidth(). This means that the steps to produce
    // correct max/min in the non-replaced version, are not necessary.
    computedValues.Extent = ComputeReplacedLogicalWidth() + BorderAndPaddingLogicalWidth();

    inlineConstraints.ResolvePosition(computedValues);
    inlineConstraints.FixupLogicalLeftPosition(computedValues);
  }

  void RenderReplaced::ComputeReplacedOutOfFlowPositionedLogicalHeight(
    LogicalExtentComputedValues &computedValues) const noexcept
  {
    PositionedLayoutConstraints blockConstraints(*this, LogicalBoxAxis::Block);
    blockConstraints.ComputeInsets();

    // NOTE: This value of height is final in that the min/max height calculations
    // are dealt with in computeReplacedHeight(). This means that the steps to produce
    // correct max/min in the non-replaced version, are not necessary.
    computedValues.Extent = ComputeReplacedLogicalHeight() + BorderAndPaddingLogicalHeight();

    blockConstraints.ResolvePosition(computedValues);
    blockConstraints.AdjustLogicalTopWithLogicalHeightIfNeeded(computedValues);
  }
}
