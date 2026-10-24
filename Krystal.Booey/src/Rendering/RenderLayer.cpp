#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/HitTestingTransformState.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderLayerFilters.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Shapes/BoxLayoutShape.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Style/Values/Masking/ClipPath.hpp"
#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo
{
  /// @brief The clip rects a layer passes to its child layers: the overflow clip, and the clips of fixed and
  /// absolutely positioned descendants.
  class ClipRects : public RefCounted<ClipRects>
  {
  private:
    bool _fixed {false};
    ClipRect _overflowClipRect;
    ClipRect _fixedClipRect;
    ClipRect _posClipRect;

  public:
    ClipRects() noexcept = default;

    ClipRects(const ClipRects &other) noexcept
        : RefCounted(), _fixed(other.Fixed()), _overflowClipRect(other.OverflowClipRect()),
          _fixedClipRect(other.FixedClipRect()), _posClipRect(other.PosClipRect())
    {
    }

    KRYS_NODISCARD static Ref<ClipRects> Create() noexcept
    {
      return AdoptRef(*new ClipRects);
    }

    void Reset() noexcept
    {
      _overflowClipRect.Reset();
      _fixedClipRect.Reset();
      _posClipRect.Reset();
      _fixed = false;
    }

    KRYS_NODISCARD const ClipRect &OverflowClipRect() const noexcept
    {
      return _overflowClipRect;
    }
    void SetOverflowClipRect(const ClipRect &clipRect) noexcept
    {
      _overflowClipRect = clipRect;
    }

    KRYS_NODISCARD const ClipRect &FixedClipRect() const noexcept
    {
      return _fixedClipRect;
    }
    void SetFixedClipRect(const ClipRect &clipRect) noexcept
    {
      _fixedClipRect = clipRect;
    }

    KRYS_NODISCARD const ClipRect &PosClipRect() const noexcept
    {
      return _posClipRect;
    }
    void SetPosClipRect(const ClipRect &clipRect) noexcept
    {
      _posClipRect = clipRect;
    }

    KRYS_NODISCARD bool Fixed() const noexcept
    {
      return _fixed;
    }
    void SetFixed(bool fixed) noexcept
    {
      _fixed = fixed;
    }

    void SetOverflowClipRectAffectedByRadius() noexcept
    {
      _overflowClipRect.SetAffectedByRadius(true);
    }

    KRYS_NODISCARD bool operator==(const ClipRects &other) const noexcept
    {
      return _overflowClipRect == other.OverflowClipRect() && _fixedClipRect == other.FixedClipRect()
             && _posClipRect == other.PosClipRect() && _fixed == other.Fixed();
    }

    ClipRects &operator=(const ClipRects &other) noexcept
    {
      _overflowClipRect = other.OverflowClipRect();
      _fixedClipRect = other.FixedClipRect();
      _posClipRect = other.PosClipRect();
      _fixed = other.Fixed();
      return *this;
    }
  };

  /// @brief A layer's cached clip rects, per clip rects type and whether the overflow clip is respected.
  class ClipRectsCache
  {
  private:
    Array<RefPtr<ClipRects>, NumCachedClipRectsTypes * 2> _clipRects;

  public:
#if KRYS_DEBUG
    Array<const RenderLayer *, NumCachedClipRectsTypes> ClipRectsRoot {};
#endif

    KRYS_NODISCARD ClipRects *GetClipRects(const RenderLayer::ClipRectsContext &context) const noexcept
    {
      return _clipRects[GetIndex(context.Type, context.RespectOverflowClip())].get();
    }

    void SetClipRects(ClipRectsType clipRectsType, bool respectOverflowClip,
                      RefPtr<ClipRects> &&clipRects) noexcept
    {
      _clipRects[GetIndex(clipRectsType, respectOverflowClip)] = krys::move(clipRects);
    }

  private:
    KRYS_NODISCARD static uint32 GetIndex(ClipRectsType clipRectsType, bool respectOverflowClip) noexcept
    {
      auto index = static_cast<uint32>(clipRectsType);
      if (respectOverflowClip)
      {
        index += static_cast<uint32>(NumCachedClipRectsTypes);
      }
      krys_assert(index < NumCachedClipRectsTypes * 2);
      return index;
    }
  };

#pragma region Compositing

  RenderLayerCompositor &RenderLayer::Compositor() const noexcept
  {
    return Renderer().View().Compositor();
  }

  utf8_string RenderLayer::Name() const noexcept
  {
    return utf8_string {Renderer().RenderName()};
  }

  void RenderLayer::ContentChanged(ContentChangeType changeType, const Maybe<FloatRect> &dirtyRect) noexcept
  {
    if (changeType == ContentChangeType::Canvas || changeType == ContentChangeType::Video
        || changeType == ContentChangeType::FullScreen || changeType == ContentChangeType::Model
        || changeType == ContentChangeType::HDRImage)
    {
      SetNeedsPostLayoutCompositingUpdate();
      SetNeedsCompositingConfigurationUpdate();
    }

    if (auto *backing = Backing())
    {
      backing->ContentChanged(changeType, dirtyRect);
    }
  }

  void RenderLayer::SetAncestorsHaveCompositingDirtyFlag(Compositing flag) noexcept
  {
    for (auto *layer = PaintOrderParent(); layer; layer = layer->PaintOrderParent())
    {
      if (layer->_compositingDirtyBits.Contains(flag))
      {
        break;
      }
      layer->_compositingDirtyBits.Add(flag);
    }
  }

  void RenderLayer::SetIntrinsicallyComposited(bool composited) noexcept
  {
    _intrinsicallyComposited = composited;
    UpdateAlwaysIncludedInZOrderLists();
  }

  void RenderLayer::UpdateAlwaysIncludedInZOrderLists() noexcept
  {
    // FIXME: View transition names, once there are view transitions.
    bool alwaysIncludedInZOrderLists = _intrinsicallyComposited;
    if (_alwaysIncludedInZOrderLists != alwaysIncludedInZOrderLists)
    {
      _alwaysIncludedInZOrderLists = alwaysIncludedInZOrderLists;
      if (alwaysIncludedInZOrderLists)
      {
        UpdateAncestorChainHasAlwaysIncludedInZOrderListsDescendants();
      }
      else
      {
        DirtyAncestorChainHasAlwaysIncludedInZOrderListsDescendants();
      }
      if (!HasVisibleContent() && !IsNormalFlowOnly())
      {
        DirtyHiddenStackingContextAncestorZOrderLists();
      }
    }
  }

  void RenderLayer::UpdateAncestorChainHasAlwaysIncludedInZOrderListsDescendants() noexcept
  {
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (!layer->_hasAlwaysIncludedInZOrderListsDescendantsStatusDirty
          && layer->_hasAlwaysIncludedInZOrderListsDescendants)
      {
        break;
      }
      layer->_hasAlwaysIncludedInZOrderListsDescendants = true;
      layer->_hasAlwaysIncludedInZOrderListsDescendantsStatusDirty = false;
    }
  }

  void RenderLayer::DirtyAncestorChainHasAlwaysIncludedInZOrderListsDescendants() noexcept
  {
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (layer->_hasAlwaysIncludedInZOrderListsDescendantsStatusDirty)
      {
        break;
      }

      layer->_hasAlwaysIncludedInZOrderListsDescendantsStatusDirty = true;
    }
  }

  // FIXME: This is terrible. Bring back a cached bit for this someday. This crawl is going to slow down all
  // painting of content inside paginated layers.
  bool RenderLayer::HasCompositedLayerInEnclosingPaginationChain() const noexcept
  {
    // No enclosing layer means no compositing in the chain.
    if (!_enclosingPaginationLayer)
    {
      return false;
    }

    // If the enclosing layer is composited, we don't have to check anything in between us and that layer.
    if (_enclosingPaginationLayer->IsComposited())
    {
      return true;
    }

    // If we are the enclosing pagination layer, then we can't be composited or we'd have passed the previous
    // check.
    if (_enclosingPaginationLayer.get() == this)
    {
      return false;
    }

    // The enclosing paginated layer is our ancestor and is not composited, so we have to check intermediate
    // layers between us and the enclosing pagination layer. Start with our own layer.
    if (IsComposited())
    {
      return true;
    }

    // For normal flow layers, we can recur up the layer tree.
    if (IsNormalFlowOnly())
    {
      return Parent()->HasCompositedLayerInEnclosingPaginationChain();
    }

    // Otherwise we have to go up the containing block chain. Find the first enclosing containing block layer
    // ancestor, and check that.
    for (const auto *containingBlock = Renderer().ContainingBlock();
         containingBlock && !Is<RenderView>(*containingBlock);
         containingBlock = containingBlock->ContainingBlock())
    {
      if (containingBlock->HasLayer())
      {
        return containingBlock->Layer()->HasCompositedLayerInEnclosingPaginationChain();
      }
    }
    return false;
  }

  void RenderLayer::SetBackingProviderLayer(RenderLayer *backingProvider,
                                            EnumSet<UpdateBackingSharingFlags> flags) noexcept
  {
    if (backingProvider == _backingProviderLayer.get())
    {
      krys_debug_assert(flags.Contains(UpdateBackingSharingFlags::DuringCompositingUpdate)
                        || backingProvider == _backingProviderLayerAtEndOfCompositingUpdate.get());
      return;
    }

    if (!Renderer().RenderTreeBeingDestroyed())
    {
      ClearClipRectsIncludingDescendants();
    }

    _backingProviderLayer = CreateWeakPtr(backingProvider);
    if (!flags.Contains(UpdateBackingSharingFlags::DuringCompositingUpdate))
    {
      _backingProviderLayerAtEndOfCompositingUpdate = CreateWeakPtr(backingProvider);
    }
  }

  void RenderLayer::DisconnectFromBackingProviderLayer(EnumSet<UpdateBackingSharingFlags> flags) noexcept
  {
    if (!_backingProviderLayer)
    {
      return;
    }

    krys_debug_assert(_backingProviderLayer->IsComposited());
    if (_backingProviderLayer->IsComposited())
    {
      _backingProviderLayer->Backing()->RemoveBackingSharingLayer(*this, flags);
    }
  }

  RenderLayer *RenderLayer::EnclosingCompositingLayer(IncludeSelfOrNot includeSelf) const noexcept
  {
    if (includeSelf == IncludeSelf && IsComposited())
    {
      return const_cast<RenderLayer *>(this);
    }

    for (const RenderLayer *curr = PaintOrderParent(); curr; curr = curr->PaintOrderParent())
    {
      if (curr->IsComposited())
      {
        return const_cast<RenderLayer *>(curr);
      }
    }

    return nullptr;
  }

  auto RenderLayer::EnclosingCompositingLayerForRepaint(IncludeSelfOrNot includeSelf) const noexcept
    -> EnclosingCompositingLayerStatus
  {
    auto repaintTargetForLayer = [](const RenderLayer &layer) -> RenderLayer *
    {
      if (CompositedWithOwnBackingStore(layer))
      {
        return const_cast<RenderLayer *>(&layer);
      }

      if (layer.PaintsIntoProvidedBacking())
      {
        return layer.BackingProviderLayer();
      }

      return nullptr;
    };
    auto isEligibleForFullRepaintCheck = [&](const RenderLayer &layer)
    {
      return layer.IsSelfPaintingLayer() && !layer.Renderer().HasPotentiallyScrollableOverflow()
             && !Is<RenderView>(layer.Renderer());
    };

    auto fullRepaintAlreadyScheduled = isEligibleForFullRepaintCheck(*this) && NeedsFullRepaint();
    RenderLayer *repaintTarget = nullptr;
    if (includeSelf == IncludeSelf && (repaintTarget = repaintTargetForLayer(*this)))
    {
      return {fullRepaintAlreadyScheduled, repaintTarget};
    }

    for (const RenderLayer *curr = PaintOrderParent(); curr; curr = curr->PaintOrderParent())
    {
      fullRepaintAlreadyScheduled =
        fullRepaintAlreadyScheduled || (isEligibleForFullRepaintCheck(*curr) && curr->NeedsFullRepaint());
      if ((repaintTarget = repaintTargetForLayer(*curr)))
      {
        return {fullRepaintAlreadyScheduled, repaintTarget};
      }
    }

    return {};
  }

  bool RenderLayer::GetOverlapBoundsIncludingChildrenAccountingForTransformAnimations(
    LayoutRect &bounds, EnumSet<CalculateLayerBoundsFlag> additionalFlags) const noexcept
  {
    // The animation will override the display transform, so don't include it.
    auto boundsFlags = DefaultCalculateLayerBoundsFlags();
    boundsFlags.Remove(IncludeSelfTransform);
    boundsFlags.Add(additionalFlags);

    bounds = CalculateLayerBounds(this, LayoutSize(), boundsFlags);

    LayoutRect animatedBounds = bounds;
    if (auto styleable = style::Styleable::FromRenderer(Renderer()))
    {
      if (styleable->ComputeAnimationExtent(animatedBounds))
      {
        bounds = animatedBounds;
        return true;
      }
    }

    return false;
  }

  LayoutRect RenderLayer::OverlapBounds() const noexcept
  {
    if (OverlapBoundsIncludeChildren())
    {
      return CalculateLayerBounds(this, {},
                                  {UseLocalClipRectExcludingCompositingIfPossible, IncludeFilterOutsets,
                                   UseFragmentBoxesExcludingCompositing});
    }

    return LocalBoundingBox();
  }

  bool RenderLayer::OverlapBoundsIncludeChildren() const noexcept
  {
    return HasFilter() && Renderer().Style().GetFilter().HasFilterThatMovesPixels();
  }

  RenderLayerBacking *RenderLayer::EnsureBacking() noexcept
  {
    if (!_backing)
    {
      _backing = CreateUnique<RenderLayerBacking>(*this);
      Compositor().LayerBecameComposited(*this);

      UpdateFilterPaintingStrategy();
    }
    return _backing.get();
  }

  void RenderLayer::ClearBacking(EnumSet<UpdateBackingSharingFlags> flags, bool layerBeingDestroyed) noexcept
  {
    if (!_backing)
    {
      return;
    }

    if (!Renderer().RenderTreeBeingDestroyed())
    {
      Compositor().LayerBecameNonComposited(*this);
    }

    _backing->WillBeDestroyed(flags);
    _backing = nullptr;

    if (!layerBeingDestroyed)
    {
      UpdateFilterPaintingStrategy();
    }
  }

  bool RenderLayer::HasCompositedMask() const noexcept
  {
    return _backing && _backing->HasMaskLayer();
  }

  namespace
  {
    void DetermineNonLayerDescendantsPaintedContent(const RenderElement &renderer, uint32 &renderersTraversed,
                                                    RenderLayer::PaintedContentRequest &request) noexcept
    {
      // Constrain the depth and breadth of the search for performance.
      static constexpr uint32 maxRendererTraversalCount = 200;

      for (const auto &child : ChildrenOfType<RenderObject>(renderer))
      {
        if (++renderersTraversed > maxRendererTraversalCount)
        {
          if (!request.IsPaintedContentSatisfied())
          {
            request.MakePaintedContentUndetermined();
          }
          if (request.IsSatisfied())
          {
            return;
          }
        }

        if (auto *renderText = DynamicDowncast<RenderText>(child))
        {
          if (!renderText->HasRenderedText())
          {
            continue;
          }

          if (renderer.Style().UsedUserSelect() != UserSelect::None)
          {
            request.SetHasPaintedContent();
          }

          if (!std::ranges::all_of(renderText->Text(), [](char8_t c) { return text::IsASCIIWhitespace(c); }))
          {
            request.SetHasPaintedContent();
          }

          if (request.IsSatisfied())
          {
            return;
          }
        }

        auto *childRenderElement = DynamicDowncast<RenderElement>(child);
        if (!childRenderElement)
        {
          continue;
        }

        if (auto *modelObject = DynamicDowncast<RenderLayerModelObject>(*childRenderElement);
            modelObject && modelObject->HasSelfPaintingLayer())
        {
          continue;
        }

        if (childRenderElement->HasVisibleBoxDecorations() || childRenderElement->Style().HasOutline())
        {
          request.SetHasPaintedContent();
          if (request.IsSatisfied())
          {
            return;
          }
        }

        if (Is<RenderReplaced>(*childRenderElement))
        {
          request.SetHasPaintedContent();

          if (request.IsSatisfied())
          {
            return;
          }
        }

        // FIXME: HDR content, once there's HDR.

        DetermineNonLayerDescendantsPaintedContent(*childRenderElement, renderersTraversed, request);
        if (request.IsSatisfied())
        {
          return;
        }
      }
    }
  }

  void RenderLayer::DetermineNonLayerDescendantsPaintedContent(PaintedContentRequest &request) const noexcept
  {
    uint32 renderersTraversed = 0;
    boo::DetermineNonLayerDescendantsPaintedContent(Renderer(), renderersTraversed, request);
  }

  bool RenderLayer::IsVisuallyNonEmpty(PaintedContentRequest *request) const noexcept
  {
    krys_debug_assert(!_visibleContentStatusDirty);

    if (!HasVisibleContent() || Renderer().Style().GetOpacity().IsTransparent())
    {
      return false;
    }

    if (Renderer().IsRenderReplaced() || (_scrollableArea && _scrollableArea->HasOverflowControls()))
    {
      if (!request)
      {
        return true;
      }

      request->SetHasPaintedContent();
      if (request->IsSatisfied())
      {
        return true;
      }
    }

    // FIXME: HDR content, once there's HDR.

    if (HasVisibleBoxDecorationsOrBackground())
    {
      if (!request)
      {
        return true;
      }

      request->SetHasPaintedContent();
      if (request->IsSatisfied())
      {
        return true;
      }
    }

    PaintedContentRequest localRequest;
    if (!request)
    {
      request = &localRequest;
    }

    DetermineNonLayerDescendantsPaintedContent(*request);
    return request->ProbablyHasPaintedContent();
  }

#pragma endregion

  bool CompositedWithOwnBackingStore(const RenderLayer &layer) noexcept
  {
    return layer.IsComposited() && !layer.Backing()->PaintsIntoCompositedAncestor();
  }

  TextStream &operator<<(TextStream &ts, IndirectCompositingReason reason) noexcept
  {
    switch (reason)
    {
      case IndirectCompositingReason::None:                      ts << u8"none"; break;
      case IndirectCompositingReason::Clipping:                  ts << u8"clipping"; break;
      case IndirectCompositingReason::Stacking:                  ts << u8"stacking"; break;
      case IndirectCompositingReason::OverflowScrollPositioning: ts << u8"overflow positioning"; break;
      case IndirectCompositingReason::Overlap:                   ts << u8"overlap"; break;
      case IndirectCompositingReason::BackgroundLayer:           ts << u8"background layer"; break;
      case IndirectCompositingReason::GraphicalEffect:           ts << u8"graphical effect"; break;
      case IndirectCompositingReason::Perspective:               ts << u8"perspective"; break;
      case IndirectCompositingReason::Preserve3D:                ts << u8"preserve-3d"; break;
    }

    return ts;
  }

  void MakeMatrixRenderable(TransformationMatrix &matrix, bool has3DRendering) noexcept
  {
    if (!has3DRendering)
    {
      matrix.MakeAffine();
    }
  }

  static ScrollingScope NextScrollingScope() noexcept
  {
    static ScrollingScope currentScope = 0;
    return ++currentScope;
  }

  RenderLayer::RenderLayer(RenderLayerModelObject &renderer) noexcept
      : _isRenderViewLayer(renderer.IsRenderView()),
        // FIXME: Media renderers force a stacking context, once there are any.
        _forcedStackingContext(false), _renderer(renderer)
  {
    SetIsNormalFlowOnly(ShouldBeNormalFlowOnly());
    SetIsCSSStackingContext(ShouldBeCSSStackingContext());
    SetCanBeBackdropRoot(ComputeCanBeBackdropRoot());
    SetNeedsPositionUpdate();

    _isSelfPaintingLayer = ShouldBeSelfPaintingLayer();

    if (IsRenderViewLayer())
    {
      _boxScrollingScope = _contentsScrollingScope = NextScrollingScope();
    }

    auto needsVisibleContentStatusUpdate = [&]()
    {
      if (renderer.FirstChild())
      {
        return false;
      }

      // Leave m_visibleContentStatusDirty = true in any case. The associated renderer needs to be inserted
      // into the render tree, before we can determine the visible content status.

      // We need the parent to know if we have skipped content or content-visibility root.
      if (renderer.Style().IsSkippedRootOrSkippedContent() && !renderer.Parent())
      {
        return false;
      }
      return true;
    }();

    if (needsVisibleContentStatusUpdate)
    {
      _visibleContentStatusDirty = false;
      _hasVisibleContent = renderer.Style().UsedVisibility() == Visibility::Visible;
    }
  }

  RenderLayer::~RenderLayer() noexcept
  {
    if (InResizeMode())
    {
      Renderer().GetDocument().Frame()->GetEventHandler().ResizeLayerDestroyed();
    }

    ClearLayerScrollableArea();
    // FIXME: The clip path, once there's an SVG clip path.
    ClearLayerFilters();

    // Child layers will be deleted by their corresponding render objects, so
    // we don't need to delete them ourselves.

    ClearBacking({}, true);

    // Layer and all its children should be removed from the tree before destruction.
    krys_assert(Renderer().RenderTreeBeingDestroyed() || !Parent());
    krys_assert(Renderer().RenderTreeBeingDestroyed() || !FirstChild());
  }

  UniquePtr<RenderLayer> RenderLayer::Create(RenderLayerModelObject &renderer) noexcept
  {
    return CreateUnique<RenderLayer>(renderer);
  }

  RenderBox *RenderLayer::GetRenderBox() const noexcept
  {
    return DynamicDowncast<RenderBox>(Renderer());
  }

  void RenderLayer::SetNextSibling(RenderLayer *next) noexcept
  {
    _next = next ? CreateWeakPtr(next) : nullptr;
  }

  void RenderLayer::SetPreviousSibling(RenderLayer *prev) noexcept
  {
    _previous = prev ? CreateWeakPtr(prev) : nullptr;
  }

  void RenderLayer::SetFirstChild(RenderLayer *first) noexcept
  {
    _first = first ? CreateWeakPtr(first) : nullptr;
  }

  void RenderLayer::SetLastChild(RenderLayer *last) noexcept
  {
    _last = last ? CreateWeakPtr(last) : nullptr;
  }

#pragma region Tree

  void RenderLayer::AddChild(RenderLayer &child, RenderLayer *beforeChild) noexcept
  {
    RenderLayer *prevSibling = beforeChild ? beforeChild->PreviousSibling() : LastChild();
    if (prevSibling)
    {
      child.SetPreviousSibling(prevSibling);
      prevSibling->SetNextSibling(&child);
      krys_debug_assert(prevSibling != &child);
    }
    else
    {
      SetFirstChild(&child);
    }

    if (beforeChild)
    {
      beforeChild->SetPreviousSibling(&child);
      child.SetNextSibling(beforeChild);
      krys_debug_assert(beforeChild != &child);
    }
    else
    {
      SetLastChild(&child);
    }

    child._parent = CreateWeakPtr(this);
    child.SetSelfAndDescendantsNeedPositionUpdate();

    DirtyPaintOrderListsOnChildChange(child);

    child.UpdateAncestorDependentState();
    DirtyAncestorChainVisibleDescendantStatus();
    child.UpdateDescendantDependentFlags();

    if (child.IsSelfPaintingLayer() || child.HasSelfPaintingLayerDescendant())
    {
      SetAncestorChainHasSelfPaintingLayerDescendant();
    }

    if (child.IsViewportConstrained() || child._hasViewportConstrainedDescendant)
    {
      SetAncestorChainHasViewportConstrainedDescendant();
    }

    if (Compositor().HasContentCompositingLayers())
    {
      SetDescendantsNeedCompositingRequirementsTraversal();
    }

    if (child.HasDescendantNeedingCompositingRequirementsTraversal()
        || child.NeedsCompositingRequirementsTraversal())
    {
      child.SetAncestorsHaveCompositingDirtyFlag(Compositing::HasDescendantNeedingRequirementsTraversal);
    }

    if (child.HasDescendantNeedingUpdateBackingOrHierarchyTraversal()
        || child.NeedsUpdateBackingOrHierarchyTraversal())
    {
      child.SetAncestorsHaveCompositingDirtyFlag(
        Compositing::HasDescendantNeedingBackingOrHierarchyTraversal);
    }

    if (child.HasBlendMode() || (child.HasNotIsolatedBlendingDescendants() && !child.IsolatesBlending()))
    {
      UpdateAncestorChainHasBlendingDescendants(); // Why not just dirty?
    }
  }

  void RenderLayer::RemoveChild(RenderLayer &oldChild) noexcept
  {
    if (!Renderer().RenderTreeBeingDestroyed())
    {
      Compositor().LayerWillBeRemoved(*this, oldChild);
    }

    // remove the child
    if (oldChild.PreviousSibling())
    {
      oldChild.PreviousSibling()->SetNextSibling(oldChild.NextSibling());
    }
    if (oldChild.NextSibling())
    {
      oldChild.NextSibling()->SetPreviousSibling(oldChild.PreviousSibling());
    }

    if (_first.get() == &oldChild)
    {
      SetFirstChild(oldChild.NextSibling());
    }
    if (_last.get() == &oldChild)
    {
      SetLastChild(oldChild.PreviousSibling());
    }

    DirtyPaintOrderListsOnChildChange(oldChild);

    oldChild.SetPreviousSibling(nullptr);
    oldChild.SetNextSibling(nullptr);
    oldChild._parent = nullptr;

    oldChild.UpdateDescendantDependentFlags();
    if (oldChild._hasVisibleContent || oldChild._hasVisibleDescendant)
    {
      DirtyAncestorChainVisibleDescendantStatus();
    }

    if (oldChild.IsSelfPaintingLayer() || oldChild.HasSelfPaintingLayerDescendant())
    {
      DirtyAncestorChainHasSelfPaintingLayerDescendantStatus();
    }

    if (oldChild.IsViewportConstrained() || oldChild._hasViewportConstrainedDescendant)
    {
      DirtyAncestorChainHasViewportConstrainedDescendantStatus();
    }

    if (Compositor().HasContentCompositingLayers())
    {
      SetDescendantsNeedCompositingRequirementsTraversal();
    }

    if (oldChild.HasBlendMode()
        || (oldChild.HasNotIsolatedBlendingDescendants() && !oldChild.IsolatesBlending()))
    {
      DirtyAncestorChainHasBlendingDescendants();
    }
    if (Renderer().Style().UsedVisibility() != Visibility::Visible)
    {
      DirtyVisibleContentStatus();
    }
  }

  void RenderLayer::DirtyPaintOrderListsOnChildChange(RenderLayer &child) noexcept
  {
    if (child.IsNormalFlowOnly())
    {
      DirtyNormalFlowList();
    }

    if (!child.IsNormalFlowOnly() || child.FirstChild())
    {
      // Dirty the z-order list in which we are contained. The stackingContext() can be null in the
      // case where we're building up generated content layers. This is ok, since the lists will start
      // off dirty in that case anyway.
      child.DirtyStackingContextZOrderLists();
    }
  }

  void RenderLayer::InsertOnlyThisLayer() noexcept
  {
    if (!_parent && Renderer().Parent())
    {
      // We need to connect ourselves when our renderer() has a parent.
      // Find our enclosingLayer and add ourselves.
      auto *parentLayer = Renderer().LayerParent();
      if (!parentLayer)
      {
        return;
      }

      auto *beforeChild = Renderer().LayerNextSibling(*parentLayer);
      parentLayer->AddChild(*this, beforeChild);
    }

    // Remove all descendant layers from the hierarchy and add them to the new position.
    for (auto &child : ChildrenOfType<RenderElement>(Renderer()))
    {
      child.MoveLayers(*this);
    }

    // Clear out all the clip rects.
    ClearClipRectsIncludingDescendants();
  }

  void RenderLayer::RemoveOnlyThisLayer() noexcept
  {
    if (!_parent)
    {
      return;
    }

    Compositor().LayerWillBeRemoved(*_parent, *this);

    // Dirty the clip rects.
    ClearClipRectsIncludingDescendants();

    RenderLayer *nextSib = NextSibling();

    // Now walk our kids and reattach them to our parent.
    RenderLayer *current = _first.get();
    while (current)
    {
      RenderLayer *next = current->NextSibling();
      RemoveChild(*current);
      _parent->AddChild(*current, nextSib);
      current->SetRepaintStatus(RepaintStatus::NeedsFullRepaint);
      current = next;
    }

    // Remove us from the parent.
    _parent->RemoveChild(*this);
    Renderer().DestroyLayer();
  }

  bool RenderLayer::IsDescendantOf(const RenderLayer &layer) const noexcept
  {
    for (const auto *ancestor = this; ancestor; ancestor = ancestor->Parent())
    {
      if (&layer == ancestor)
      {
        return true;
      }
    }
    return false;
  }

  RenderLayer *RenderLayer::CommonAncestorWithLayer(const RenderLayer &layer) const noexcept
  {
    if (this == &layer)
    {
      return const_cast<RenderLayer *>(this);
    }

    HashSet<const RenderLayer *> ancestorChain;
    for (const auto *currLayer = this; currLayer; currLayer = currLayer->Parent())
    {
      ancestorChain.insert(currLayer);
    }

    for (const auto *currLayer = &layer; currLayer; currLayer = currLayer->Parent())
    {
      if (ancestorChain.contains(currLayer))
      {
        return const_cast<RenderLayer *>(currLayer);
      }
    }
    return nullptr;
  }

#pragma endregion

#pragma region Stacking contexts and z-order lists

  bool RenderLayer::ComputeCanBeBackdropRoot() const noexcept
  {
    // In order to match other impls and not the spec, the document element should only be a backdrop root
    // (and be isolated from the base background color) if another group rendering effect is present.
    // https://github.com/w3c/fxtf-drafts/issues/557
    return IsRenderViewLayer() || Renderer().IsTransparent() || Renderer().HasBackdropFilter()
           || Renderer().HasClipPath() || Renderer().HasFilter() || Renderer().HasBlendMode()
           || Renderer().HasMask()
           || (Renderer().RequiresRenderingConsolidationForViewTransition()
               && !Renderer().IsDocumentElementRenderer())
           || Renderer().Style().GetWillChange().CanBeBackdropRoot();
  }

  bool RenderLayer::SetCanBeBackdropRoot(bool canBeBackdropRoot) noexcept
  {
    if (_canBeBackdropRoot == canBeBackdropRoot)
    {
      return false;
    }
    _canBeBackdropRoot = canBeBackdropRoot;
    return true;
  }

  static bool CanCreateStackingContext(const RenderLayer &layer) noexcept
  {
    const auto &renderer = layer.Renderer();
    // FIXME: View transition captures, once there are any.
    return renderer.HasTransformRelatedProperty() || renderer.HasClipPath() || renderer.HasFilter()
           || renderer.HasMask() || renderer.HasBackdropFilter() || renderer.HasBlendMode()
           || renderer.IsTransparent() || renderer.RequiresRenderingConsolidationForViewTransition()
           || renderer.IsPositioned() // Note that this only creates stacking context in conjunction with
                                      // explicit z-index.
           || renderer.Style().HasIsolation() || renderer.ShouldApplyPaintContainment()
           || !renderer.Style().UsedZIndex().IsAuto()
           || renderer.Style().GetWillChange().CanCreateStackingContext() || layer.EstablishesTopLayer();
  }

  bool RenderLayer::ShouldBeNormalFlowOnly() const noexcept
  {
    if (CanCreateStackingContext(*this))
    {
      return false;
    }

    // FIXME: Canvases, video, embedded objects and iframes too, once there are any.
    return Renderer().HasNonVisibleOverflow()
           || (Renderer().Style().SpecifiesColumns() && !IsRenderViewLayer())
           || Renderer().IsRenderFragmentedFlow();
  }

  bool RenderLayer::ShouldBeCSSStackingContext() const noexcept
  {
    // FIXME: View transition captures and containing blocks, once there are view transitions.
    return !Renderer().Style().UsedZIndex().IsAuto() || Renderer().ShouldApplyLayoutContainment()
           || Renderer().ShouldApplyPaintContainment()
           || Renderer().RequiresRenderingConsolidationForViewTransition()
           || Renderer().IsViewTransitionRoot() || Renderer().IsViewTransitionContainingBlock()
           || IsRenderViewLayer();
  }

  bool RenderLayer::SetIsNormalFlowOnly(bool isNormalFlowOnly) noexcept
  {
    if (isNormalFlowOnly == _isNormalFlowOnly)
    {
      return false;
    }

    _isNormalFlowOnly = isNormalFlowOnly;

    if (auto *p = Parent())
    {
      p->DirtyNormalFlowList();
    }
    DirtyStackingContextZOrderLists();
    return true;
  }

  void RenderLayer::IsStackingContextChanged() noexcept
  {
    DirtyStackingContextZOrderLists();
    SetSelfAndDescendantsNeedPositionUpdate();
    if (IsStackingContext())
    {
      DirtyZOrderLists();
    }
    else
    {
      ClearZOrderLists();
    }
  }

  bool RenderLayer::SetIsOpportunisticStackingContext(bool isStacking) noexcept
  {
    bool wasStacking = IsStackingContext();
    _isOpportunisticStackingContext = isStacking;
    if (wasStacking == IsStackingContext())
    {
      return false;
    }

    IsStackingContextChanged();
    return true;
  }

  bool RenderLayer::SetIsCSSStackingContext(bool isCSSStackingContext) noexcept
  {
    bool wasStacking = IsStackingContext();
    _isCSSStackingContext = isCSSStackingContext;
    if (wasStacking == IsStackingContext())
    {
      return false;
    }

    IsStackingContextChanged();
    return true;
  }

  RenderLayer *RenderLayer::StackingContext() const noexcept
  {
    auto *layer = Parent();
    while (layer && !layer->IsStackingContext())
    {
      layer = layer->Parent();
    }

    krys_debug_assert(!layer || layer->IsStackingContext());
    return layer;
  }

  void RenderLayer::DirtyZOrderLists() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif
    krys_debug_assert(IsStackingContext());

    if (_posZOrderList)
    {
      _posZOrderList->clear();
    }
    if (_negZOrderList)
    {
      _negZOrderList->clear();
    }
    _zOrderListsDirty = true;

    // FIXME: Ideally, we'd only dirty if the lists changed.
    if (HasCompositingDescendant())
    {
      SetNeedsCompositingPaintOrderChildrenUpdate();
    }
  }

  void RenderLayer::DirtyStackingContextZOrderLists() noexcept
  {
    if (auto *sc = StackingContext())
    {
      sc->DirtyZOrderLists();
    }
  }

  void RenderLayer::DirtyHiddenStackingContextAncestorZOrderLists() noexcept
  {
    for (auto *sc = StackingContext(); sc; sc = sc->StackingContext())
    {
      sc->DirtyZOrderLists();
      if (sc->HasVisibleContent())
      {
        break;
      }
    }
  }

  void RenderLayer::DirtyNormalFlowList() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif

    if (_normalFlowList)
    {
      _normalFlowList->clear();
    }
    _normalFlowListDirty = true;

    if (HasCompositingDescendant())
    {
      SetNeedsCompositingPaintOrderChildrenUpdate();
    }
  }

  void RenderLayer::UpdateNormalFlowList() noexcept
  {
    if (!_normalFlowListDirty)
    {
      return;
    }

#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif

    for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
    {
      // Ignore non-overflow layers.
      if (child->IsNormalFlowOnly())
      {
        if (!_normalFlowList)
        {
          _normalFlowList = CreateUnique<List<RenderLayer *>>();
        }
        _normalFlowList->push_back(child);
        child->SetWasIncludedInZOrderTree();
      }
    }

    if (_normalFlowList)
    {
      _normalFlowList->shrink_to_fit();
    }

    _normalFlowListDirty = false;
  }

  void RenderLayer::ClearZOrderLists() noexcept
  {
    krys_debug_assert(!IsStackingContext());
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif

    _posZOrderList = nullptr;
    _negZOrderList = nullptr;
  }

  void RenderLayer::UpdateZOrderLists() noexcept
  {
    if (!_zOrderListsDirty)
    {
      return;
    }

    if (!IsStackingContext())
    {
      ClearZOrderLists();
      _zOrderListsDirty = false;
      return;
    }

    RebuildZOrderLists();
  }

  void RenderLayer::RebuildZOrderLists() noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(LayerListMutationAllowed());
#endif
    krys_debug_assert(IsDirtyStackingContext());

    EnumSet<Compositing> childDirtyFlags;
    RebuildZOrderLists(_posZOrderList, _negZOrderList, childDirtyFlags);
    _zOrderListsDirty = false;

    bool hasNegativeZOrderList = _negZOrderList && _negZOrderList->size();
    // Having negative z-order lists affect whether a compositing layer needs a foreground layer.
    // Ideally we'd only trigger this when having z-order children changes, but we blow away the old z-order
    // lists on dirtying so we don't know the old state.
    if (hasNegativeZOrderList != _hadNegativeZOrderList)
    {
      _hadNegativeZOrderList = hasNegativeZOrderList;
      if (IsComposited())
      {
        SetNeedsCompositingConfigurationUpdate();
      }
    }

    // Building lists may have added layers with dirty flags, so make sure we propagate dirty bits up the
    // tree.
    if (_compositingDirtyBits.ContainsAll({Compositing::DescendantsNeedRequirementsTraversal,
                                           Compositing::DescendantsNeedBackingAndHierarchyTraversal}))
    {
      return;
    }

    if (childDirtyFlags.ContainsAny(ComputeCompositingRequirementsFlags()))
    {
      SetDescendantsNeedCompositingRequirementsTraversal();
    }

    if (childDirtyFlags.ContainsAny(UpdateBackingOrHierarchyFlags()))
    {
      SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
    }
  }

  void RenderLayer::RebuildZOrderLists(UniquePtr<List<RenderLayer *>> &posZOrderList,
                                       UniquePtr<List<RenderLayer *>> &negZOrderList,
                                       EnumSet<Compositing> &accumulatedDirtyFlags) noexcept
  {
    for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
    {
      child->CollectLayers(posZOrderList, negZOrderList, accumulatedDirtyFlags);
    }

    auto compareZIndex = [](const RenderLayer *first, const RenderLayer *second) -> bool
    {
      return first->ZIndex() < second->ZIndex();
    };

    // Sort the two lists.
    if (posZOrderList)
    {
      std::stable_sort(posZOrderList->begin(), posZOrderList->end(), compareZIndex);
      posZOrderList->shrink_to_fit();
    }

    if (negZOrderList)
    {
      std::stable_sort(negZOrderList->begin(), negZOrderList->end(), compareZIndex);
      negZOrderList->shrink_to_fit();
    }

    if (IsRenderViewLayer() && Renderer().GetDocument().HasTopLayerElement())
    {
      auto topLayerLayers = TopLayerRenderLayers(Renderer().View());
      if (!topLayerLayers.empty())
      {
        if (!posZOrderList)
        {
          posZOrderList = CreateUnique<List<RenderLayer *>>();
        }
        posZOrderList->insert(posZOrderList->end(), topLayerLayers.begin(), topLayerLayers.end());
      }
    }

    // FIXME: The view transition containing block goes last in the view's list, once there are view
    // transitions.
  }

  void RenderLayer::RemoveSelfFromCompositor() noexcept
  {
    if (Parent())
    {
      Compositor().LayerWillBeRemoved(*Parent(), *this);
    }
    ClearBacking({});
  }

  void RenderLayer::RemoveDescendantsFromCompositor() noexcept
  {
    for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
    {
      child->RemoveSelfFromCompositor();
      child->RemoveDescendantsFromCompositor();
    }
  }

  void RenderLayer::SetWasOmittedFromZOrderTree() noexcept
  {
    if (_wasOmittedFromZOrderTree)
    {
      return;
    }

    krys_debug_assert(!IsNormalFlowOnly());
    RemoveSelfFromCompositor();

    // Omitting a stacking context removes the whole subtree, otherwise collectLayers will
    // visit and omit/include descendants separately.
    if (IsStackingContext())
    {
      RemoveDescendantsFromCompositor();
    }

    if (Compositor().HasContentCompositingLayers() && Parent())
    {
      Parent()->SetDescendantsNeedCompositingRequirementsTraversal();
    }

    _wasOmittedFromZOrderTree = true;
  }

  void RenderLayer::CollectLayers(UniquePtr<List<RenderLayer *>> &positiveZOrderList,
                                  UniquePtr<List<RenderLayer *>> &negativeZOrderList,
                                  EnumSet<Compositing> &accumulatedDirtyFlags) noexcept
  {
    krys_debug_assert(!DescendantDependentFlagsAreDirty());
    // FIXME: The view transition containing block is collected by the view too, once there are view
    // transitions.
    if (EstablishesTopLayer())
    {
      return;
    }

    bool isStacking = IsStackingContext();
    bool layerOrDescendantsAreVisible = _hasVisibleContent || _alwaysIncludedInZOrderLists
                                        || _hasVisibleDescendant
                                        || _hasAlwaysIncludedInZOrderListsDescendants;
    // Normal flow layers are just painted by their enclosing layers, so they don't get put in zorder lists.
    if (!IsNormalFlowOnly())
    {
      if (layerOrDescendantsAreVisible)
      {
        auto &layerList = (ZIndex() >= 0) ? positiveZOrderList : negativeZOrderList;
        if (!layerList)
        {
          layerList = CreateUnique<List<RenderLayer *>>();
        }
        layerList->push_back(this);
        accumulatedDirtyFlags.Add(_compositingDirtyBits);
        SetWasIncludedInZOrderTree();
      }
      else
      {
        SetWasOmittedFromZOrderTree();
      }
    }

    // Recur into our children to collect more layers, but only if we don't establish
    // a stacking context/container.
    if (!isStacking)
    {
      for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
      {
        child->CollectLayers(positiveZOrderList, negativeZOrderList, accumulatedDirtyFlags);
      }
    }
  }

  void RenderLayer::UpdateLayerListsIfNeeded() noexcept
  {
    UpdateDescendantDependentFlags();
    UpdateZOrderLists();
    UpdateNormalFlowList();
  }

  void RenderLayer::ForceStackingContextIfNeeded() noexcept
  {
    if (SetIsCSSStackingContext(true))
    {
      SetIsNormalFlowOnly(false);
      if (Parent())
      {
        if (!HasNotIsolatedBlendingDescendantsStatusDirty() && HasNotIsolatedBlendingDescendants())
        {
          Parent()->DirtyAncestorChainHasBlendingDescendants();
        }
      }
    }
  }

  int32 RenderLayer::ZIndex() const noexcept
  {
    auto value = Renderer().Style().UsedZIndex().TryValue();
    return value ? static_cast<int32>(value->Value) : 0;
  }

#pragma endregion

#pragma region Position updates

  void RenderLayer::SetNeedsPositionUpdate() noexcept
  {
    _layerPositionDirtyBits.Add(LayerPositionUpdates::NeedsPositionUpdate);
    for (auto *layer = Parent(); layer; layer = layer->Parent())
    {
      if (layer->_layerPositionDirtyBits.Contains(LayerPositionUpdates::DescendantNeedsPositionUpdate))
      {
        break;
      }
      layer->_layerPositionDirtyBits.Add(LayerPositionUpdates::DescendantNeedsPositionUpdate);
    }
  }

  bool RenderLayer::NeedsPositionUpdate() const noexcept
  {
    if (_layerPositionDirtyBits.ContainsAny(
          {LayerPositionUpdates::NeedsPositionUpdate, LayerPositionUpdates::DescendantNeedsPositionUpdate}))
    {
      return true;
    }
    if (Parent()
        && Parent()->_layerPositionDirtyBits.Contains(LayerPositionUpdates::AllChildrenNeedPositionUpdate))
    {
      return true;
    }
    return false;
  }

  void RenderLayer::SetSelfAndChildrenNeedPositionUpdate() noexcept
  {
    SetNeedsPositionUpdate();
    _layerPositionDirtyBits.Add({LayerPositionUpdates::DescendantNeedsPositionUpdate,
                                 LayerPositionUpdates::AllChildrenNeedPositionUpdate});
  }

  void RenderLayer::SetSelfAndDescendantsNeedPositionUpdate() noexcept
  {
    SetNeedsPositionUpdate();
    _layerPositionDirtyBits.Add({LayerPositionUpdates::DescendantNeedsPositionUpdate,
                                 LayerPositionUpdates::AllDescendantsNeedPositionUpdate});
  }

  EnumSet<RenderLayer::UpdateLayerPositionsFlag>
    RenderLayer::FlagsForUpdateLayerPositions(RenderLayer &startingLayer) noexcept
  {
    EnumSet<UpdateLayerPositionsFlag> flags = {UpdateLayerPositionsFlag::CheckForRepaint};

    if (auto *parent = startingLayer.Parent())
    {
      if (parent->HasFixedContainingBlockAncestor()
          || (!parent->IsRenderViewLayer() && parent->Renderer().CanContainFixedPositionObjects()))
      {
        flags.Add(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer);
      }

      if (parent->HasTransformedAncestor() || parent->Transform())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenTransformedLayer);
      }

      if (parent->_has3DTransformedAncestor || (parent->Transform() && !parent->Transform()->IsAffine()))
      {
        flags.Add(UpdateLayerPositionsFlag::Seen3DTransformedLayer);
      }

      if (parent->BehavesAsFixed()
          || (parent->Renderer().IsFixedPositioned() && !parent->HasFixedContainingBlockAncestor()))
      {
        flags.Add(UpdateLayerPositionsFlag::SeenFixedLayer);
      }

      if (parent->Renderer().IsStickilyPositioned())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenStickyLayer);
      }

      if (parent->_hasCompositedScrollingAncestor || parent->HasCompositedScrollableOverflow())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenCompositedScrollingLayer);
      }
    }

    return flags;
  }

  void RenderLayer::UpdateLayerPositionsAfterStyleChange(bool environmentChanged) noexcept
  {
    auto updateLayerPositionFlags = [&](bool environmentChanged)
    {
      auto flags = FlagsForUpdateLayerPositions(*this);
      if (environmentChanged)
      {
        flags.Add(UpdateLayerPositionsFlag::EnvironmentChanged);
      }
      return flags;
    };

    if (environmentChanged)
    {
      SetSelfAndDescendantsNeedPositionUpdate();
    }

    // FIXME: Invalidate the document markers' rects (willUpdateLayerPositions), once there are document
    // markers.
    RecursiveUpdateLayerPositions(updateLayerPositionFlags(environmentChanged));
  }

  void RenderLayer::UpdateLayerPositionsAfterLayout(bool didFullRepaint, bool environmentChanged) noexcept
  {
    krys_debug_assert(IsRenderViewLayer());

    auto updateLayerPositionFlags = [&](bool didFullRepaint, bool environmentChanged)
    {
      auto flags = FlagsForUpdateLayerPositions(*this);
      if (didFullRepaint)
      {
        flags.Remove(UpdateLayerPositionsFlag::CheckForRepaint);
        flags.Add(UpdateLayerPositionsFlag::NeedsFullRepaintInBacking);
      }
      if (environmentChanged)
      {
        flags.Add(UpdateLayerPositionsFlag::EnvironmentChanged);
      }
      return flags;
    };

    if (environmentChanged)
    {
      SetSelfAndDescendantsNeedPositionUpdate();
    }

    // FIXME: Invalidate the document markers' rects (willUpdateLayerPositions), once there are document
    // markers.
    RecursiveUpdateLayerPositions(updateLayerPositionFlags(didFullRepaint, environmentChanged));
  }

  bool RenderLayer::AncestorLayerPositionStateChanged(EnumSet<UpdateLayerPositionsFlag> flags) const noexcept
  {
    return _hasFixedContainingBlockAncestor
             != flags.Contains(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer)
           || _hasTransformedAncestor != flags.Contains(UpdateLayerPositionsFlag::SeenTransformedLayer)
           || _has3DTransformedAncestor != flags.Contains(UpdateLayerPositionsFlag::Seen3DTransformedLayer)
           || _hasFixedAncestor != flags.Contains(UpdateLayerPositionsFlag::SeenFixedLayer)
           || _hasPaginatedAncestor != flags.Contains(UpdateLayerPositionsFlag::UpdatePagination)
           || _hasCompositedScrollingAncestor
                != flags.Contains(UpdateLayerPositionsFlag::SeenCompositedScrollingLayer);
  }

  void RenderLayer::RecursiveUpdateLayerPositions(EnumSet<UpdateLayerPositionsFlag> flags) noexcept
  {
    if (AncestorLayerPositionStateChanged(flags))
    {
      flags.Add(UpdateLayerPositionsFlag::SubtreeNeedsUpdate);
    }

    if (!NeedsPositionUpdate() && !flags.ContainsAny(InvalidationLayerPositionsFlags()))
    {
      return;
    }

    if (_layerPositionDirtyBits.Contains(LayerPositionUpdates::AllDescendantsNeedPositionUpdate))
    {
      flags.Add(UpdateLayerPositionsFlag::SubtreeNeedsUpdate);
    }

    if (UpdateLayerPosition(&flags))
    {
      flags.Add(UpdateLayerPositionsFlag::SubtreeNeedsUpdate);
    }

    if (_scrollableArea)
    {
      _scrollableArea->ApplyPostLayoutScrollPositionIfNeeded();
    }

    // Clear our cached clip rect information.
    ClearClipRects();

    if (_scrollableArea && _scrollableArea->HasOverflowControls())
    {
      // FIXME: It looks suspicious to call convertToLayerCoords here
      // as canUseOffsetFromAncestor may be true for an ancestor layer.
      auto offsetFromRoot = OffsetFromAncestor(Root());
      (void)_scrollableArea->PositionOverflowControls(RoundedIntSize(offsetFromRoot));
    }

    UpdateDescendantDependentFlags();

    if (flags.Contains(UpdateLayerPositionsFlag::UpdatePagination))
    {
      UpdatePagination();
    }
    else if (Renderer().IsRenderFragmentedFlow())
    {
      _enclosingPaginationLayer = CreateWeakPtr(this);
      flags.Add(UpdateLayerPositionsFlag::UpdatePagination);
    }
    else
    {
      _enclosingPaginationLayer = nullptr;
    }

    auto repaintIfNecessary = [&](bool checkForRepaint)
    {
      // FIXME: Paint offset cache does not work with RenderLayers as there is not a 1-to-1
      // mapping between them and the RenderObjects. It would be neat to enable
      // LayoutState outside the layout() phase and use it here.
      krys_debug_assert(!Renderer().View().FrameView().LayoutContext().IsPaintOffsetCacheEnabled());

      const auto *repaintContainer = Renderer().ContainerForRepaint().Renderer;

      auto oldRects = RepaintRects();
      ComputeRepaintRects(repaintContainer);
      auto newRects = RepaintRects();

      if (checkForRepaint && ShouldRepaintAfterLayout() && newRects)
      {
        auto needsFullRepaint = GetRepaintStatus() == RepaintStatus::NeedsFullRepaint
                                  ? RequiresFullRepaint::Yes
                                  : RequiresFullRepaint::No;
        auto resolvedOldRects = oldRects.value_or(RenderObject::RepaintRects {});
        Renderer().RepaintAfterLayoutIfNeeded(repaintContainer, needsFullRepaint, resolvedOldRects,
                                              *newRects);
      }
    };

    repaintIfNecessary(flags.Contains(UpdateLayerPositionsFlag::CheckForRepaint));

    _repaintStatus = RepaintStatus::NeedsNormalRepaint;
    _hasFixedContainingBlockAncestor =
      flags.Contains(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer);
    _hasTransformedAncestor = flags.Contains(UpdateLayerPositionsFlag::SeenTransformedLayer);
    _has3DTransformedAncestor = flags.Contains(UpdateLayerPositionsFlag::Seen3DTransformedLayer);
    _hasFixedAncestor = flags.Contains(UpdateLayerPositionsFlag::SeenFixedLayer);
    _hasStickyAncestor = flags.Contains(UpdateLayerPositionsFlag::SeenStickyLayer);
    _hasPaginatedAncestor = flags.Contains(UpdateLayerPositionsFlag::UpdatePagination);
    _hasCompositedScrollingAncestor = flags.Contains(UpdateLayerPositionsFlag::SeenCompositedScrollingLayer);

    if (!IsRenderViewLayer())
    {
      if (Renderer().CanContainFixedPositionObjects())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenFixedContainingBlockLayer);
      }

      if (Transform())
      {
        flags.Add(UpdateLayerPositionsFlag::SeenTransformedLayer);
        if (!Transform()->IsAffine())
        {
          flags.Add(UpdateLayerPositionsFlag::Seen3DTransformedLayer);
        }
      }
    }

    // Fixed inside transform behaves like absolute (per spec).
    if (_hasFixedAncestor || (Renderer().IsFixedPositioned() && !_hasFixedContainingBlockAncestor))
    {
      SetBehavesAsFixed(true);
      flags.Add(UpdateLayerPositionsFlag::SeenFixedLayer);
    }

    if (_hasStickyAncestor || Renderer().IsStickilyPositioned())
    {
      flags.Add(UpdateLayerPositionsFlag::SeenStickyLayer);
    }

    if (HasCompositedScrollableOverflow())
    {
      flags.Add(UpdateLayerPositionsFlag::SeenCompositedScrollingLayer);
    }

    if (flags.ContainsAny(InvalidationLayerPositionsFlags())
        || _layerPositionDirtyBits.Contains(LayerPositionUpdates::DescendantNeedsPositionUpdate))
    {
      for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
      {
        child->RecursiveUpdateLayerPositions(flags);
      }
    }

    // FIXME: The marquee position, once there are marquees.

    if (Renderer().IsFixedPositioned() && Compositor().AcceleratedCompositingForFixedPositionEnabled())
    {
      bool intersectsViewport = Compositor().FixedLayerIntersectsViewport(*this);
      if (intersectsViewport != _isFixedIntersectingViewport)
      {
        _isFixedIntersectingViewport = intersectsViewport;
        SetNeedsPostLayoutCompositingUpdate();
      }
    }

    if (IsComposited())
    {
      Backing()->UpdateAfterLayout(
        flags.Contains(UpdateLayerPositionsFlag::ContainingClippingLayerChangedSize),
        flags.Contains(UpdateLayerPositionsFlag::NeedsFullRepaintInBacking));
    }

    ClearLayerPositionDirtyBits();
  }

  static LayoutRect ComputeLayerPositionAndIntegralSize(const RenderLayerModelObject &renderer) noexcept
  {
    if (auto *inlineRenderer = DynamicDowncast<RenderInline>(renderer);
        inlineRenderer && inlineRenderer->IsInline())
    {
      return {LayoutPoint(), LayoutSize(inlineRenderer->LinesBoundingBox().Size())};
    }

    if (auto *boxRenderer = DynamicDowncast<RenderBox>(renderer))
    {
      const auto &frameRect = boxRenderer->FrameRect();
      return {boxRenderer->TopLeftLocation(),
              LayoutSize(SnappedIntSize(frameRect.Size(), frameRect.Location()))};
    }

    krys_debug_assert(false);
    return {};
  }

  bool RenderLayer::UpdateLayerPosition(EnumSet<UpdateLayerPositionsFlag> *flags) noexcept
  {
    auto layerRect = ComputeLayerPositionAndIntegralSize(Renderer());
    auto localPoint = layerRect.Location();

    bool geometryChanged = false;
    if (IntSize newSize(layerRect.Width().ToInt(), layerRect.Height().ToInt()); newSize != Size())
    {
      geometryChanged = true;
      SetSize(newSize);

      if (flags && Renderer().HasNonVisibleOverflow())
      {
        flags->Add(UpdateLayerPositionsFlag::ContainingClippingLayerChangedSize);
      }
    }

    if (!Renderer().IsOutOfFlowPositioned())
    {
      auto *ancestor = Renderer().Parent();
      // We must adjust our position by walking up the render tree looking for the
      // nearest enclosing object with a layer.
      while (ancestor && !ancestor->HasLayer())
      {
        if (auto *boxRenderer = DynamicDowncast<RenderBox>(ancestor))
        {
          // Rows and cells share the same coordinate space (that of the section).
          // Omit them when computing our xpos/ypos.
          if (!Is<RenderTableRow>(*boxRenderer))
          {
            localPoint += boxRenderer->TopLeftLocationOffset();
          }
        }
        ancestor = ancestor->Parent();
      }
      if (auto *tableRow = DynamicDowncast<RenderTableRow>(ancestor))
      {
        // Put ourselves into the row coordinate space.
        localPoint -= tableRow->TopLeftLocationOffset();
      }
    }

    // Subtract our parent's scroll offset.
    RenderLayer *positionedParent;
    if (Renderer().IsOutOfFlowPositioned()
        && (positionedParent = EnclosingAncestorForPosition(Renderer().Style().GetPosition())))
    {
      // For positioned layers, we subtract out the enclosing positioned layer's scroll offset.
      if (positionedParent->Renderer().HasNonVisibleOverflow())
      {
        if (auto *positionedParentScrollableArea = positionedParent->GetScrollableArea())
        {
          localPoint -= LayoutSize(ToIntSize(positionedParentScrollableArea->GetScrollPosition()));
        }
      }
      // FIXME: Move out-of-flow boxes inside an in-flow positioned inline with it
      // (offsetForInFlowPositionedInline), once there's inline layout.
    }
    else if (auto *parentLayer = Parent())
    {
      if (parentLayer->Renderer().HasNonVisibleOverflow())
      {
        if (auto *parentLayerScrollableArea = parentLayer->GetScrollableArea())
        {
          localPoint -= LayoutSize(ToIntSize(parentLayerScrollableArea->GetScrollPosition()));
        }
      }
    }

    if (HasCompositedScrollableOverflow())
    {
      if (!_contentsScrollingScope || _contentsScrollingScope == _boxScrollingScope)
      {
        _contentsScrollingScope = NextScrollingScope();
      }
    }
    else if (!_contentsScrollingScope || _contentsScrollingScope != _boxScrollingScope)
    {
      _contentsScrollingScope = _boxScrollingScope;
    }

    if (Renderer().IsInFlowPositioned())
    {
      if (auto *boxModelObject = DynamicDowncast<RenderBoxModelObject>(Renderer()))
      {
        auto newOffset = boxModelObject->OffsetForInFlowPosition();
        geometryChanged |= newOffset != _offsetForPosition;
        _offsetForPosition = newOffset;
        localPoint.Move(_offsetForPosition);
      }
    }

    geometryChanged |= Location() != localPoint;
    SetLocation(localPoint);

    if (geometryChanged && Compositor().HasContentCompositingLayers())
    {
      if (IsComposited())
      {
        SetNeedsCompositingGeometryUpdate();
      }
      // This layer's footprint can affect the location of a composited descendant (which may be a sibling in
      // z-order), so trigger a descendant walk from the enclosing stacking context.
      if (auto *sc = StackingContext())
      {
        sc->SetDescendantsNeedCompositingRequirementsTraversal();
        sc->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
      }
    }

    return geometryChanged;
  }

  bool RenderLayer::ShouldRepaintAfterLayout() const noexcept
  {
    // FIXME: SVG containers never repaint after layout (only their contents do), once there's SVG.
    if (_repaintStatus == RepaintStatus::NeedsNormalRepaint
        || _repaintStatus == RepaintStatus::NeedsFullRepaint)
    {
      return true;
    }

    // Composited layers that were moved during a positioned movement only
    // layout, don't need to be repainted. They just need to be recomposited.
    krys_debug_assert(_repaintStatus == RepaintStatus::NeedsFullRepaintForOutOfFlowMovementLayout);
    return !IsComposited() || Backing()->PaintsIntoCompositedAncestor();
  }

  Maybe<LayoutRect> RenderLayer::CachedClippedOverflowRect() const noexcept
  {
    if (!_repaintRectsValid)
    {
      return null;
    }

    return _repaintRects.ClippedOverflowRect;
  }

  void RenderLayer::ComputeRepaintRects(const RenderLayerModelObject *repaintContainer) noexcept
  {
    krys_debug_assert(!_visibleContentStatusDirty);

    if (IsSubtreeVisibilityHiddenOrOpacityZero() || !IsSelfPaintingLayer())
    {
      ClearRepaintRects();
    }
    else
    {
      _repaintRects = Renderer().RectsForRepaintingAfterLayout(repaintContainer, RepaintOutlineBounds::Yes);
      _repaintRectsValid = true;
      // What a filter that moves pixels paints reaches past the overflow, so the layer repaints all of it
      // when it moves or changes.
      if (!repaintContainer && FilterMovesPixels())
      {
        _repaintRects.ClippedOverflowRect.Unite(AbsoluteFilterRepaintRect());
      }
    }

    if (IsSubtreeVisibilityHiddenOrOpacityZero())
    {
      _repaintContainer = nullptr;
    }
    else
    {
      _repaintContainer = CreateWeakPtr(repaintContainer);
    }
  }

  void RenderLayer::ComputeRepaintRectsIncludingDescendants() noexcept
  {
    // FIXME: computeRepaintRects() has to walk up the parent chain for every layer to compute the rects.
    // We should make this more efficient.
    ComputeRepaintRects(Renderer().ContainerForRepaint().Renderer);
    ClearClipRects(PaintingClipRects);

    for (auto *layer = FirstChild(); layer; layer = layer->NextSibling())
    {
      layer->ComputeRepaintRectsIncludingDescendants();
    }
  }

  LayoutRect RenderLayer::RepaintRectIncludingNonCompositingDescendants() const noexcept
  {
    LayoutRect repaintRect;
    if (_repaintRectsValid)
    {
      repaintRect = _repaintRects.ClippedOverflowRect;
    }
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      // Don't include repaint rects for composited child layers; they will paint themselves and have a
      // different origin.
      if (child->IsComposited())
      {
        continue;
      }

      repaintRect.UniteIfNonZero(child->RepaintRectIncludingNonCompositingDescendants());
    }
    return repaintRect;
  }

  const RenderLayerModelObject *RenderLayer::RepaintContainer() const noexcept
  {
    return _repaintContainer.get();
  }

  bool RenderLayer::BackgroundIsKnownToBeOpaqueInRect(const LayoutRect &localRect) const noexcept
  {
    if (!IsSelfPaintingLayer() && !HasSelfPaintingLayerDescendant())
    {
      return false;
    }

    if (PaintsWithTransparency({}))
    {
      return false;
    }

    if (Renderer().IsDocumentElementRenderer())
    {
      // Normally the document element doens't have a layer. If it does have a layer, its background
      // propagates to the RenderView so this layer doesn't draw it.
      return false;
    }

    // We can't use hasVisibleContent(), because that will be true if our renderer is hidden, but some child
    // is visible and that child doesn't cover the entire rect.
    if (Renderer().Style().UsedVisibility() != Visibility::Visible)
    {
      return false;
    }

    if (ShouldPaintWithFilters() && Renderer().Style().GetFilter().HasFilterThatAffectsOpacity())
    {
      return false;
    }

    // FIXME: Handle simple transforms.
    if (PaintsWithTransform({}))
    {
      return false;
    }

    // FIXME: Remove this check. This function should not be called when layer-lists are dirty. It is somehow
    // getting triggered during style update.
    if (ZOrderListsDirty() || NormalFlowListDirty())
    {
      return false;
    }

    // Table painting is special; a table paints its sections.
    if (Renderer().IsTablePart())
    {
      return false;
    }

    // A fieldset with a legend will have an irregular shape, so can't be treated as opaque.
    if (Renderer().IsFieldset())
    {
      return false;
    }

    // FIXME: The renderer's own background (RenderBox::backgroundIsKnownToBeOpaqueInRect), once boxes can
    // tell.

    // We can't consult child layers if we clip, since they might cover parts of the rect that are clipped
    // out.
    if (Renderer().HasNonVisibleOverflow())
    {
      return false;
    }

    return ListBackgroundIsKnownToBeOpaqueInRect(PositiveZOrderLayers(), localRect)
           || ListBackgroundIsKnownToBeOpaqueInRect(NegativeZOrderLayers(), localRect)
           || ListBackgroundIsKnownToBeOpaqueInRect(NormalFlowLayers(), localRect);
  }

  bool RenderLayer::ListBackgroundIsKnownToBeOpaqueInRect(const LayerList &list,
                                                          const LayoutRect &localRect) const noexcept
  {
    if (list.begin() == list.end())
    {
      return false;
    }

    for (auto iter = list.end(); iter != list.begin();)
    {
      const auto *childLayer = *--iter;
      if (childLayer->IsComposited())
      {
        continue;
      }

      if (!childLayer->CanUseOffsetFromAncestor())
      {
        continue;
      }

      LayoutRect childLocalRect(localRect);
      childLocalRect.Move(-childLayer->OffsetFromAncestor(this));

      if (childLayer->BackgroundIsKnownToBeOpaqueInRect(childLocalRect))
      {
        return true;
      }
    }
    return false;
  }

  void RenderLayer::CompositingStatusChanged(LayoutUpToDate layoutUpToDate) noexcept
  {
    UpdateDescendantDependentFlags();
    if (Parent() || IsRenderViewLayer())
    {
      ComputeRepaintRectsIncludingDescendants();
    }
    if (layoutUpToDate == LayoutUpToDate::No)
    {
      SetSelfAndDescendantsNeedPositionUpdate();
    }
  }

  void RenderLayer::SetBackingNeedsRepaint(GraphicsLayerShouldClipToLayer shouldClip) noexcept
  {
    krys_debug_assert(IsComposited());
    if (Backing()->PaintsIntoWindow())
    {
      // If we're trying to repaint the placeholder document layer, propagate the repaint to the native view
      // system.
      Renderer().View().RepaintViewRectangle(LayoutRect(AbsoluteBoundingBox()));
    }
    else
    {
      Backing()->SetContentsNeedDisplay(shouldClip);
    }
  }

  void RenderLayer::SetBackingNeedsRepaintInRect(const LayoutRect &r,
                                                 GraphicsLayerShouldClipToLayer shouldClip) noexcept
  {
    krys_debug_assert(IsComposited());
    if (!IsComposited() || Backing()->PaintsIntoWindow())
    {
      // If we're trying to repaint the placeholder document layer, propagate the repaint to the native view
      // system.
      LayoutRect absRect(r);
      absRect.Move(OffsetFromAncestor(Root()));

      Renderer().View().RepaintViewRectangle(absRect);
    }
    else
    {
      Backing()->SetContentsNeedDisplayInRect(r, shouldClip);
    }
  }

  // Since we're only painting non-composited layers, we know that they all share the same repaintContainer.
  void RenderLayer::RepaintIncludingNonCompositingDescendants(
    const RenderLayerModelObject *repaintContainer) noexcept
  {
    auto clippedOverflowRect = _repaintRectsValid
                                 ? _repaintRects.ClippedOverflowRect
                                 : Renderer().ClippedOverflowRectForRepaint(repaintContainer);
    Renderer().RepaintUsingContainer(repaintContainer, clippedOverflowRect);

    for (RenderLayer *curr = FirstChild(); curr; curr = curr->NextSibling())
    {
      if (!curr->IsComposited())
      {
        curr->RepaintIncludingNonCompositingDescendants(repaintContainer);
      }
    }
  }

  void RenderLayer::RepaintIncludingDescendants() noexcept
  {
    Renderer().Repaint();
    for (auto *current = FirstChild(); current; current = current->NextSibling())
    {
      current->RepaintIncludingDescendants();
    }
  }

  void RenderLayer::AddBlockSelectionGapsBounds(const LayoutRect &bounds) noexcept
  {
    _blockSelectionGapsBounds.Unite(EnclosingIntRect(bounds));
  }

  void RenderLayer::ClearBlockSelectionGapsBounds() noexcept
  {
    _blockSelectionGapsBounds = IntRect();
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      child->ClearBlockSelectionGapsBounds();
    }
  }

  void RenderLayer::RepaintBlockSelectionGaps() noexcept
  {
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      child->RepaintBlockSelectionGaps();
    }

    if (_blockSelectionGapsBounds.IsEmpty())
    {
      return;
    }

    LayoutRect rect = _blockSelectionGapsBounds;
    if (_scrollableArea)
    {
      rect.MoveBy(-_scrollableArea->GetScrollPosition());
    }
    if (Renderer().HasNonVisibleOverflow() && !UsesCompositedScrolling())
    {
      rect.Intersect(Downcast<RenderBox>(Renderer()).OverflowClipRect({}));
    }
    if (Renderer().HasClip())
    {
      rect.Intersect(Downcast<RenderBox>(Renderer()).ClipRect({}));
    }
    if (!rect.IsEmpty())
    {
      Renderer().RepaintRectangle(rect);
    }
  }

  void RenderLayer::SetRepaintStatus(RepaintStatus status) noexcept
  {
    if (status != _repaintStatus)
    {
      _repaintStatus = status;
      SetNeedsPositionUpdate();
    }
  }

  void RenderLayer::UpdatePagination() noexcept
  {
    _enclosingPaginationLayer = nullptr;

    if (!Parent())
    {
      return;
    }

    // Each layer that is inside a multicolumn flow thread has to be checked individually and
    // genuinely know if it is going to have to split itself up when painting only its contents (and not any
    // other descendant layers). We track an enclosingPaginationLayer instead of using a simple bit, since we
    // want to be able to get back to that layer easily.
    if (Renderer().IsRenderFragmentedFlow())
    {
      _enclosingPaginationLayer = CreateWeakPtr(this);
      return;
    }

    auto paginationLayerOf = [](const RenderLayer &layer) -> WeakPtr<RenderLayer>
    {
      // Content inside a transform is not considered to be paginated, since we simply
      // paint the transform multiple times in each column, so we don't have to use
      // fragments for the transformed content.
      if (layer.IsTransformed())
      {
        return nullptr;
      }
      auto *paginationLayer = layer.EnclosingPaginationLayer(IncludeCompositedPaginatedLayers);
      return paginationLayer ? CreateWeakPtr(paginationLayer) : nullptr;
    };

    if (IsNormalFlowOnly())
    {
      _enclosingPaginationLayer = paginationLayerOf(*Parent());
      return;
    }

    // For the new columns code, we want to walk up our containing block chain looking for an enclosing layer.
    // Once we find one, then we just check its pagination status.
    for (const auto *containingBlock = Renderer().ContainingBlock();
         containingBlock && !Is<RenderView>(*containingBlock);
         containingBlock = containingBlock->ContainingBlock())
    {
      if (containingBlock->HasLayer())
      {
        _enclosingPaginationLayer = paginationLayerOf(*containingBlock->Layer());
        return;
      }
    }
  }

#pragma endregion

#pragma region Descendant dependent flags

  void RenderLayer::SetAncestorChainHasSelfPaintingLayerDescendant() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (Renderer().ShouldApplyPaintContainment())
      {
        _hasSelfPaintingLayerDescendant = true;
        _hasSelfPaintingLayerDescendantDirty = false;
        break;
      }
      if (!layer->_hasSelfPaintingLayerDescendantDirty && layer->HasSelfPaintingLayerDescendant())
      {
        break;
      }

      layer->_hasSelfPaintingLayerDescendantDirty = false;
      layer->_hasSelfPaintingLayerDescendant = true;
    }
  }

  void RenderLayer::DirtyAncestorChainHasSelfPaintingLayerDescendantStatus() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (layer->_hasSelfPaintingLayerDescendantDirty)
      {
        break;
      }

      layer->_hasSelfPaintingLayerDescendantDirty = true;
      layer->SetNeedsPositionUpdate();
    }
  }

  void RenderLayer::SetAncestorChainHasViewportConstrainedDescendant() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (!layer->_hasViewportConstrainedDescendantStatusDirty && layer->_hasViewportConstrainedDescendant)
      {
        break;
      }

      layer->_hasViewportConstrainedDescendant = true;
      layer->_hasViewportConstrainedDescendantStatusDirty = false;
    }
  }

  void RenderLayer::DirtyAncestorChainHasViewportConstrainedDescendantStatus() noexcept
  {
    for (RenderLayer *layer = this; layer; layer = layer->Parent())
    {
      if (layer->_hasViewportConstrainedDescendantStatusDirty)
      {
        break;
      }

      layer->_hasViewportConstrainedDescendantStatusDirty = true;
    }
  }

  void RenderLayer::UpdateBlendMode() noexcept
  {
    bool hadBlendMode = _blendMode != BlendMode::Normal;
    if (Parent() && hadBlendMode != HasBlendMode())
    {
      if (HasBlendMode())
      {
        Parent()->UpdateAncestorChainHasBlendingDescendants();
      }
      else
      {
        Parent()->DirtyAncestorChainHasBlendingDescendants();
      }
    }

    BlendMode newBlendMode = Renderer().Style().GetBlendMode();
    if (newBlendMode != _blendMode)
    {
      _blendMode = newBlendMode;
    }
  }

  void RenderLayer::WillRemoveChildWithBlendMode() noexcept
  {
    Parent()->DirtyAncestorChainHasBlendingDescendants();
  }

  void RenderLayer::UpdateAncestorChainHasBlendingDescendants() noexcept
  {
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (!layer->HasNotIsolatedBlendingDescendantsStatusDirty()
          && layer->HasNotIsolatedBlendingDescendants())
      {
        break;
      }
      layer->_hasNotIsolatedBlendingDescendants = true;
      layer->_hasNotIsolatedBlendingDescendantsStatusDirty = false;

      layer->UpdateSelfPaintingLayer();

      if (layer->IsCSSStackingContext())
      {
        break;
      }
    }
  }

  void RenderLayer::DirtyAncestorChainHasBlendingDescendants() noexcept
  {
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (layer->HasNotIsolatedBlendingDescendantsStatusDirty())
      {
        break;
      }

      layer->_hasNotIsolatedBlendingDescendantsStatusDirty = true;
      layer->SetNeedsPositionUpdate();
    }
  }

  void RenderLayer::SetHasVisibleContent() noexcept
  {
    if (_hasVisibleContent && !_visibleContentStatusDirty)
    {
      krys_debug_assert(!Parent() || Parent()->_visibleDescendantStatusDirty
                        || Parent()->HasVisibleDescendant());
      return;
    }

    _visibleContentStatusDirty = false;
    _hasVisibleContent = true;
    ComputeRepaintRects(Renderer().ContainerForRepaint().Renderer);
    SetNeedsPositionUpdate();
    if (!IsNormalFlowOnly())
    {
      // We don't collect invisible layers in z-order lists if they are not composited.
      // As we became visible, we need to dirty our stacking containers ancestors to be properly
      // collected.
      DirtyHiddenStackingContextAncestorZOrderLists();
    }

    if (Parent())
    {
      Parent()->DirtyAncestorChainVisibleDescendantStatus();
    }
  }

  void RenderLayer::DirtyVisibleContentStatus() noexcept
  {
    _visibleContentStatusDirty = true;
    SetNeedsPositionUpdate();
    if (Parent())
    {
      Parent()->DirtyAncestorChainVisibleDescendantStatus();
    }
  }

  void RenderLayer::DirtyAncestorChainVisibleDescendantStatus() noexcept
  {
    SetNeedsPositionUpdate();
    for (auto *layer = this; layer; layer = layer->Parent())
    {
      if (layer->_visibleDescendantStatusDirty)
      {
        break;
      }

      layer->_visibleDescendantStatusDirty = true;
    }
  }

  void RenderLayer::UpdateAncestorDependentState() noexcept
  {
    // FIXME: The enclosing SVG hidden or resource container, and whether the layer is inside an SVG foreign
    // object (which makes it self-painting), once there's SVG.
  }

  void RenderLayer::UpdateDescendantDependentFlags() noexcept
  {
    if (_visibleDescendantStatusDirty || _hasSelfPaintingLayerDescendantDirty
        || HasNotIsolatedBlendingDescendantsStatusDirty()
        || _hasAlwaysIncludedInZOrderListsDescendantsStatusDirty
        || _hasViewportConstrainedDescendantStatusDirty)
    {
      bool hasVisibleDescendant = false;
      bool hasSelfPaintingLayerDescendant = false;
      bool hasNotIsolatedBlendingDescendants = false;
      bool hasAlwaysIncludedInZOrderListsDescendants = false;
      bool hasViewportConstrainedDescendant = false;

      if (_hasNotIsolatedBlendingDescendantsStatusDirty)
      {
        _hasNotIsolatedBlendingDescendantsStatusDirty = false;
        UpdateSelfPaintingLayer();
      }

      for (RenderLayer *child = FirstChild(); child; child = child->NextSibling())
      {
        child->UpdateDescendantDependentFlags();

        hasVisibleDescendant |= child->_hasVisibleContent || child->_hasVisibleDescendant;
        hasSelfPaintingLayerDescendant |=
          child->IsSelfPaintingLayer() || child->HasSelfPaintingLayerDescendant();
        hasNotIsolatedBlendingDescendants |=
          child->HasBlendMode() || (child->HasNotIsolatedBlendingDescendants() && !child->IsolatesBlending());
        hasAlwaysIncludedInZOrderListsDescendants |=
          child->AlwaysIncludedInZOrderLists() || child->_hasAlwaysIncludedInZOrderListsDescendants;
        hasViewportConstrainedDescendant |=
          child->_hasViewportConstrainedDescendant || child->IsViewportConstrained();
      }

      _hasVisibleDescendant = hasVisibleDescendant;
      _visibleDescendantStatusDirty = false;
      _hasSelfPaintingLayerDescendant = hasSelfPaintingLayerDescendant;
      _hasSelfPaintingLayerDescendantDirty = false;
      _hasViewportConstrainedDescendant = hasViewportConstrainedDescendant;
      _hasViewportConstrainedDescendantStatusDirty = false;

      _hasNotIsolatedBlendingDescendants = hasNotIsolatedBlendingDescendants;

      _hasAlwaysIncludedInZOrderListsDescendants = hasAlwaysIncludedInZOrderListsDescendants;
      _hasAlwaysIncludedInZOrderListsDescendantsStatusDirty = false;
    }

    if (_visibleContentStatusDirty)
    {
      // We need the parent to know if we have skipped content or content-visibility root.
      if (Renderer().Style().IsSkippedRootOrSkippedContent() && !Renderer().Parent())
      {
        return;
      }
      bool hasVisibleContent = ComputeHasVisibleContent();
      if (hasVisibleContent != _hasVisibleContent)
      {
        _hasVisibleContent = hasVisibleContent;
        if (!IsNormalFlowOnly())
        {
          // We don't collect invisible layers in z-order lists if they are not composited.
          // As we change visibility, we need to dirty our stacking containers ancestors to be properly
          // collected.
          DirtyHiddenStackingContextAncestorZOrderLists();
        }
      }
      _visibleContentStatusDirty = false;
    }

    krys_debug_assert(!DescendantDependentFlagsAreDirty());
  }

  bool RenderLayer::ComputeHasVisibleContent() const noexcept
  {
    if (_isHiddenByOverflowTruncation)
    {
      return false;
    }

    if (Renderer().IsSkippedContent())
    {
      return false;
    }

    if (Renderer().Style().UsedVisibility() == Visibility::Visible)
    {
      return true;
    }

    // Layer's renderer has visibility:hidden, but some non-layer child may have visibility:visible.
    auto nextRenderer = [&](const RenderObject &renderer) -> const RenderObject *
    {
      for (const RenderObject *ancestor = &renderer; ancestor && ancestor != &this->Renderer();
           ancestor = ancestor->Parent())
      {
        if (auto *sibling = ancestor->NextSibling())
        {
          return sibling;
        }
      }
      return {};
    };
    const RenderObject *renderer = this->Renderer().FirstChild();
    while (renderer)
    {
      if (auto *renderElement = DynamicDowncast<RenderElement>(renderer);
          renderElement && !renderElement->HasLayer())
      {
        if (renderElement->Style().UsedVisibility() == Visibility::Visible)
        {
          return true;
        }
        if (auto *firstChild = renderElement->FirstChild())
        {
          renderer = firstChild;
          continue;
        }
      }
      renderer = nextRenderer(*renderer);
    }
    return false;
  }

  void RenderLayer::SetIsHiddenByOverflowTruncation(bool isHidden) noexcept
  {
    if (_isHiddenByOverflowTruncation == isHidden)
    {
      return;
    }
    _isHiddenByOverflowTruncation = isHidden;
    DirtyVisibleContentStatus();
  }

  void RenderLayer::Dirty3DTransformedDescendantStatus() noexcept
  {
    RenderLayer *curr = StackingContext();
    if (curr)
    {
      curr->_3DTransformedDescendantStatusDirty = true;
    }

    // This propagates up through preserve-3d hierarchies to the enclosing flattening layer.
    // Note that preserves3D() creates stacking context, so we can just run up the stacking containers.
    while (curr && curr->Preserves3D())
    {
      curr->_3DTransformedDescendantStatusDirty = true;
      curr = curr->StackingContext();
    }
  }

  // Return true if this layer or any preserve-3d descendants have 3d.
  bool RenderLayer::Update3DTransformedDescendantStatus() noexcept
  {
    if (_3DTransformedDescendantStatusDirty)
    {
      _has3DTransformedDescendant = false;

      UpdateZOrderLists();

      // Transformed or preserve-3d descendants can only be in the z-order lists, not
      // in the normal flow list, so we only need to check those.
      for (auto *layer : PositiveZOrderLayers())
      {
        _has3DTransformedDescendant |= layer->Update3DTransformedDescendantStatus();
      }

      // Now check our negative z-index children.
      for (auto *layer : NegativeZOrderLayers())
      {
        _has3DTransformedDescendant |= layer->Update3DTransformedDescendantStatus();
      }

      _3DTransformedDescendantStatusDirty = false;
    }

    // If we live in a 3d hierarchy, then the layer at the root of that hierarchy needs
    // the m_has3DTransformedDescendant set.
    if (Preserves3D())
    {
      return Has3DTransform() || _has3DTransformedDescendant;
    }

    return Has3DTransform();
  }

  bool RenderLayer::ShouldBeSelfPaintingLayer() const noexcept
  {
    if (!IsNormalFlowOnly())
    {
      return true;
    }

    // FIXME: Canvases, video, embedded objects and iframes too, once there are any.
    return HasOverlayScrollbars() || HasCompositedScrollableOverflow() || Renderer().IsRenderTableRow()
           || Renderer().IsRenderFragmentedFlow();
  }

  void RenderLayer::UpdateSelfPaintingLayer() noexcept
  {
    bool isSelfPaintingLayer = ShouldBeSelfPaintingLayer();
    if (_isSelfPaintingLayer == isSelfPaintingLayer)
    {
      return;
    }

    _isSelfPaintingLayer = isSelfPaintingLayer;
    SetNeedsPositionUpdate();

    if (!Parent())
    {
      return;
    }

    if (isSelfPaintingLayer)
    {
      Parent()->SetAncestorChainHasSelfPaintingLayerDescendant();
    }
    else
    {
      Parent()->DirtyAncestorChainHasSelfPaintingLayerDescendantStatus();
      ClearRepaintRects();
      // FIXME: Update the float painter of a floating box (updateFloatPainterAfterSelfPaintingLayerChange),
      // once floats track it.
    }
  }

  bool RenderLayer::HasVisibleBoxDecorationsOrBackground() const noexcept
  {
    return Renderer().HasVisibleBoxDecorations() || Renderer().Style().HasOutline();
  }

  bool RenderLayer::HasVisibleBoxDecorations() const noexcept
  {
    if (!HasVisibleContent())
    {
      return false;
    }

    return HasVisibleBoxDecorationsOrBackground()
           || (_scrollableArea && _scrollableArea->HasOverflowControls());
  }

  bool RenderLayer::IsVisibilityHiddenOrOpacityZero() const noexcept
  {
    return !HasVisibleContent() || Renderer().Style().GetOpacity().IsTransparent();
  }

  bool RenderLayer::IsSubtreeVisibilityHiddenOrOpacityZero() const noexcept
  {
    return (!HasVisibleContent() && !HasVisibleDescendant())
           || Renderer().Style().GetOpacity().IsTransparent();
  }

#pragma endregion

#pragma region Style

  void RenderLayer::StyleChanged(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    SetIsNormalFlowOnly(ShouldBeNormalFlowOnly());
    SetCanBeBackdropRoot(ComputeCanBeBackdropRoot());

    if (SetIsCSSStackingContext(ShouldBeCSSStackingContext()))
    {
      if (Parent())
      {
        if (IsCSSStackingContext())
        {
          if (!HasNotIsolatedBlendingDescendantsStatusDirty() && HasNotIsolatedBlendingDescendants())
          {
            Parent()->DirtyAncestorChainHasBlendingDescendants();
          }
        }
        else
        {
          if (HasNotIsolatedBlendingDescendantsStatusDirty())
          {
            Parent()->DirtyAncestorChainHasBlendingDescendants();
          }
          else if (HasNotIsolatedBlendingDescendants())
          {
            Parent()->UpdateAncestorChainHasBlendingDescendants();
          }
        }
      }
    }

    UpdateLayerScrollableArea();

    // FIXME: RenderLayer already handles visibility changes through our visibility dirty bits. This logic
    // could likely be folded along with the rest.
    if (oldStyle)
    {
      bool visibilityChanged = oldStyle->UsedVisibility() != Renderer().Style().UsedVisibility();
      if (oldStyle->UsedZIndex() != Renderer().Style().UsedZIndex()
          || oldStyle->UsedContentVisibility() != Renderer().Style().UsedContentVisibility()
          || visibilityChanged)
      {
        DirtyStackingContextZOrderLists();
        if (IsStackingContext())
        {
          DirtyZOrderLists();
        }
      }

      // FIXME: The view transition name, once there are view transitions.

      // Visibility and scrollability are input to canUseCompositedScrolling().
      if (_scrollableArea)
      {
        // FIXME: Invalidate the scroll corner when the writing mode changes, once there's painting.
        if (visibilityChanged || oldStyle->IsOverflowVisible() != Renderer().Style().IsOverflowVisible())
        {
          _scrollableArea->ComputeHasCompositedScrollableOverflow(
            diff <= style::DifferenceResult::RepaintLayer ? LayoutUpToDate::Yes : LayoutUpToDate::No);
        }
      }

      if (oldStyle->IsOverflowVisible() != Renderer().Style().IsOverflowVisible())
      {
        SetSelfAndDescendantsNeedPositionUpdate();
      }

      if (oldStyle->GetOpacity().IsTransparent() != Renderer().Style().GetOpacity().IsTransparent())
      {
        SetNeedsPositionUpdate();
      }

      if (oldStyle->Preserves3D() != Preserves3D())
      {
        Dirty3DTransformedDescendantStatus();
      }
    }

    if (_scrollableArea)
    {
      // FIXME: The marquee, once there are marquees.
      _scrollableArea->UpdateScrollbarsAfterStyleChange(oldStyle);
    }

    // Overlay scrollbars can make this layer self-painting so we need
    // to recompute the bit once scrollbars have been updated.
    UpdateSelfPaintingLayer();

    // FIXME: Need to detect a swap from custom to native scrollbars (and vice versa).
    if (_scrollableArea)
    {
      _scrollableArea->UpdateAllScrollbarRelatedStyle();
    }

    UpdateDescendantDependentFlags();
    UpdateBlendMode();
    UpdateFiltersAfterStyleChange(diff, oldStyle);
    ClearClipRects();

    Compositor().LayerStyleChanged(diff, *this, oldStyle);

    UpdateTransform();
    UpdateFilterPaintingStrategy();

    if (oldStyle && oldStyle->HasViewportConstrainedPosition() != IsViewportConstrained())
    {
      DirtyAncestorChainHasViewportConstrainedDescendantStatus();
    }
  }

#pragma endregion

#pragma region Transforms

  void RenderLayer::UpdateTransformFromStyle(TransformationMatrix &transform, const RenderStyle &style,
                                             EnumSet<style::TransformResolverOption> options) const noexcept
  {
    // FIXME: The default scroll shift of anchor-positioned boxes goes first
    // (https://drafts.csswg.org/css-anchor-position-1/#default-scroll-shift), once there's anchor
    // positioning.

    // FIXME: Snap the reference box to device pixels when the renderer needs it
    // (snapRectToDevicePixelsIfNeeded), once there's painting.
    auto referenceBoxRect = Renderer().TransformReferenceBoxRect(style);
    Renderer().ApplyTransform(transform, style, referenceBoxRect, options);

    // FIXME: 3D rendering (canRender3DTransforms), once there's compositing.
    MakeMatrixRenderable(transform, false);
  }

  void RenderLayer::UpdateTransform() noexcept
  {
    bool hasTransform = IsTransformed();
    bool had3DTransform = Has3DTransform();

    UniquePtr<TransformationMatrix> oldTransform;
    if (_transform && hasTransform)
    {
      oldTransform = CreateUnique<TransformationMatrix>(*_transform);
    }
    if (hasTransform != static_cast<bool>(_transform))
    {
      if (hasTransform)
      {
        _transform = CreateUnique<TransformationMatrix>();
      }
      else
      {
        _transform = nullptr;
      }

      // Layers with transforms act as clip rects roots, so clear the cached clip rects here.
      ClearClipRectsIncludingDescendants();
      SetSelfAndDescendantsNeedPositionUpdate();
    }

    if (hasTransform)
    {
      _transform->MakeIdentity();
      UpdateTransformFromStyle(*_transform, Renderer().Style(),
                               style::TransformResolver::AllTransformOperations);
    }

    if (had3DTransform != Has3DTransform())
    {
      Dirty3DTransformedDescendantStatus();
    }

    if (oldTransform && _transform && *oldTransform != *_transform)
    {
      SetSelfAndDescendantsNeedPositionUpdate();
    }
  }

  TransformationMatrix
    RenderLayer::CurrentTransform(EnumSet<style::TransformResolverOption> options) const noexcept
  {
    if (!_transform)
    {
      return {};
    }

    // m_transform includes transform-origin and is affected by the choice of the transform-box.
    // Therefore we can only use the cached m_transform, if the animation doesn't alter transform-box or
    // excludes transform-origin.

    // Query the animatedStyle() to obtain the current transformation, when accelerated transform animations
    // are running.
    auto styleable = style::Styleable::FromRenderer(Renderer());
    if ((styleable && styleable->IsRunningAcceleratedTransformRelatedAnimation())
        || !options.Contains(style::TransformResolverOption::TransformOrigin))
    {
      auto animatedStyle = Renderer().AnimatedStyle();

      TransformationMatrix transform;
      UpdateTransformFromStyle(transform, *animatedStyle, options);
      return transform;
    }

    return *_transform;
  }

  TransformationMatrix RenderLayer::CurrentTransform() const noexcept
  {
    return CurrentTransform(style::TransformResolver::AllTransformOperations);
  }

  TransformationMatrix RenderLayer::PerspectiveTransform() const noexcept
  {
    if (!Renderer().HasTransformRelatedProperty())
    {
      return {};
    }

    const auto &style = Renderer().Style();
    if (!style.HasPerspective())
    {
      return {};
    }

    auto transformReferenceBoxRect = Renderer().TransformReferenceBoxRect(style);

    TransformationMatrix transform;
    style::TransformResolver transformResolver {transform, style};

    auto perspectiveOrigin = transformResolver.ComputePerspectiveOrigin(transformReferenceBoxRect);

    // In the regular case of a non-clipped, non-scrolled GraphicsLayer, all transformations
    // (via CSS 'transform' / 'perspective') are applied with respect to a predefined anchor point,
    // which depends on the chosen CSS 'transform-box' / 'transform-origin' properties.
    //
    // A transformation given by the CSS 'transform' property is applied, by translating
    // to the 'transform origin', applying the transformation, and translating back.
    // When an element specifies a CSS 'perspective' property, the perspective transformation matrix
    // that's computed here is propagated to the GraphicsLayer by calling setChildrenTransform().
    //
    // However the GraphicsLayer platform implementations (e.g. CA on macOS) apply the children transform,
    // defined on the parent, with respect to the anchor point of the parent, when rendering child elements.
    // This is wrong, as the perspective transformation (applied to a child of the element defining the
    // 3d effect), must be independent of the chosen transform-origin (the parents transform origin
    // must not affect its children).
    //
    // To circumvent this, explicitly remove the transform-origin dependency in the perspective matrix.
    auto transformOrigin = TransformOriginPixelSnappedIfNeeded();

    transformResolver.UnapplyTransformOrigin(transformOrigin);
    transformResolver.ApplyPerspective(perspectiveOrigin);
    transformResolver.ApplyTransformOrigin(transformOrigin);

    return transform;
  }

  FloatPoint3D RenderLayer::TransformOriginPixelSnappedIfNeeded() const noexcept
  {
    if (!Renderer().HasTransformRelatedProperty())
    {
      return {};
    }

    const auto &style = Renderer().Style();
    auto referenceBoxRect = Renderer().TransformReferenceBoxRect(style);

    // FIXME: Snap the origin to device pixels when the renderer needs it (rendererNeedsPixelSnapping), once
    // there's painting.
    return style::TransformResolver::ComputeTransformOrigin(style, referenceBoxRect);
  }

  FloatPoint RenderLayer::PerspectiveOrigin() const noexcept
  {
    if (!Renderer().HasTransformRelatedProperty())
    {
      return {};
    }
    return style::TransformResolver::ComputePerspectiveOrigin(
      Renderer().Style(), Renderer().TransformReferenceBoxRect(Renderer().Style()));
  }

  bool RenderLayer::IsTransformed() const noexcept
  {
    // FIXME: The anchor scroll adjustment applies a transform too, once there's anchor positioning.
    return Renderer().IsTransformed();
  }

  namespace
  {
    KRYS_NODISCARD dom::Element *FlattenedParent(dom::Element *element) noexcept
    {
      if (!element)
      {
        return nullptr;
      }
      auto *parent = element->ParentElementInComposedTree();
      while (parent)
      {
        auto *parentStyle = parent->IsConnected() ? parent->ResolveComputedStyle() : nullptr;
        if (!parentStyle || parentStyle->Display() != DisplayType::Contents)
        {
          break;
        }
        parent = parent->ParentElementInComposedTree();
      }
      return parent;
    }
  }

  bool RenderLayer::AncestorLayerIsDOMParent(const RenderLayer *ancestor) const noexcept
  {
    if (!ancestor)
    {
      return false;
    }
    auto *parent = FlattenedParent(Renderer().GetElement());
    if (parent && ancestor->Renderer().GetElement() == parent)
    {
      return true;
    }
    if (auto pseudoElementType = Renderer().Style().GetPseudoElementType())
    {
      auto parentPseudoId = ParentPseudoElement(*pseudoElementType);
      return parentPseudoId && *parentPseudoId == ancestor->Renderer().Style().GetPseudoElementType();
    }
    return false;
  }

  bool RenderLayer::ParticipatesInPreserve3D() const noexcept
  {
    return AncestorLayerIsDOMParent(Parent()) && Parent()->Preserves3D()
           && (Transform() || Renderer().Style().GetBackfaceVisibility() == BackfaceVisibility::Hidden
               || Preserves3D());
  }

  bool RenderLayer::Preserves3D() const noexcept
  {
    return Renderer().Style().Preserves3D();
  }

  bool RenderLayer::HasPerspective() const noexcept
  {
    return Renderer().Style().HasPerspective();
  }

  bool RenderLayer::PaintsWithTransform(EnumSet<PaintBehavior> paintBehavior) const noexcept
  {
    bool paintsToWindow = !IsComposited() || Backing()->PaintsIntoWindow();
    return Transform() && (paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers) || paintsToWindow);
  }

  RenderLayer *RenderLayer::EnclosingTransformedAncestor() const noexcept
  {
    RenderLayer *curr = Parent();
    while (curr && !curr->IsRenderViewLayer() && !curr->Transform())
    {
      curr = curr->Parent();
    }

    return curr;
  }

#pragma endregion

#pragma region Queries

  bool RenderLayer::IsTransparent() const noexcept
  {
    return Renderer().IsTransparent() || Renderer().HasMask();
  }

  bool RenderLayer::HasFilter() const noexcept
  {
    return Renderer().HasFilter();
  }

  bool RenderLayer::HasBackdropFilter() const noexcept
  {
    return Renderer().HasBackdropFilter();
  }

  bool RenderLayer::WillCompositeClipPath() const noexcept
  {
    if (!IsComposited())
    {
      return false;
    }

    if (!Renderer().Style().HasClipPath())
    {
      return false;
    }

    if (Renderer().HasMask())
    {
      return false;
    }

    return GraphicsLayer::SupportsLayerType(GraphicsLayer::Type::Shape);
  }

  bool RenderLayer::ShouldPaintWithFilters(EnumSet<PaintBehavior> paintBehavior) const noexcept
  {
    const auto &filter = Renderer().Style().GetFilter();
    if (filter.IsNone())
    {
      return false;
    }

    // FIXME: SVG roots with reference filters paint them themselves, once there's SVG.

    if (RenderLayerFilters::IsIdentity(Renderer()))
    {
      return false;
    }

    if (paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers))
    {
      return true;
    }

    if (IsComposited() && _backing->CanCompositeFilters())
    {
      return false;
    }

    return true;
  }

  bool RenderLayer::RequiresFullLayerImageForFilters() const noexcept
  {
    if (!ShouldPaintWithFilters())
    {
      return false;
    }

    return _filters && _filters->HasFilterThatMovesPixels();
  }

  RenderLayerFilters *RenderLayer::Filters() const noexcept
  {
    return _filters.get();
  }

  IntOutsets RenderLayer::FilterOutsets() const noexcept
  {
    if (_filters)
    {
      return _filters->CalculateOutsets(Renderer(), FloatRect(LocalBoundingBox()));
    }
    return Renderer().Style().GetFilter().Outsets();
  }

  bool RenderLayer::HasAncestorWithFilterOutsets() const noexcept
  {
    for (const RenderLayer *curr = this; curr; curr = curr->Parent())
    {
      if (curr->HasFilterOutsets())
      {
        return true;
      }
    }
    return false;
  }

  bool RenderLayer::FilterMovesPixels() const noexcept
  {
    return HasFilter() && Renderer().Style().GetFilter().HasFilterThatMovesPixels()
           && ShouldPaintWithFilters();
  }

  LayoutRect RenderLayer::AbsoluteFilterRepaintRect() const noexcept
  {
    auto bounds = CalculateLayerBounds(
      this, {}, {UseLocalClipRectIfPossible, IncludeFilterOutsets, UseFragmentBoxesExcludingCompositing});
    return LayoutRect(Renderer().LocalToAbsoluteQuad(FloatQuad(FloatRect(bounds))).EnclosingBoundingBox());
  }

  RenderLayer *RenderLayer::EnclosingFilterLayer(IncludeSelfOrNot includeSelf) const noexcept
  {
    const RenderLayer *curr = (includeSelf == IncludeSelf) ? this : Parent();
    for (; curr; curr = curr->Parent())
    {
      if (curr->RequiresFullLayerImageForFilters())
      {
        return const_cast<RenderLayer *>(curr);
      }
    }

    return nullptr;
  }

  RenderLayer *RenderLayer::EnclosingFilterRepaintLayer() const noexcept
  {
    for (const RenderLayer *curr = this; curr; curr = curr->Parent())
    {
      if ((curr != this && curr->RequiresFullLayerImageForFilters()) || CompositedWithOwnBackingStore(*curr)
          || curr->IsRenderViewLayer())
      {
        return const_cast<RenderLayer *>(curr);
      }
    }
    return nullptr;
  }

  // FIXME: This needs a better name.
  void RenderLayer::SetFilterBackendNeedsRepaintingInRect(const LayoutRect &rect) noexcept
  {
    krys_debug_assert(RequiresFullLayerImageForFilters());
    krys_debug_assert(_filters);

    if (rect.IsEmpty())
    {
      return;
    }

    LayoutRect rectForRepaint = rect;
    rectForRepaint.Expand(ToLayoutBoxExtent(FilterOutsets()));

    _filters->ExpandDirtySourceRect(rectForRepaint);

    RenderLayer *parentLayer = EnclosingFilterRepaintLayer();
    krys_debug_assert(parentLayer);
    FloatQuad repaintQuad(FloatRect {rectForRepaint});
    LayoutRect parentLayerRect = LayoutRect(
      Renderer().LocalToContainerQuad(repaintQuad, &parentLayer->Renderer()).EnclosingBoundingBox());

    if (parentLayer->IsComposited())
    {
      if (!parentLayer->Backing()->PaintsIntoWindow())
      {
        parentLayer->SetBackingNeedsRepaintInRect(parentLayerRect);
        return;
      }
      // If the painting goes to window, redirect the painting to the parent RenderView.
      parentLayer = Renderer().View().Layer();
      parentLayerRect = LayoutRect(
        Renderer().LocalToContainerQuad(repaintQuad, &parentLayer->Renderer()).EnclosingBoundingBox());
    }

    if (parentLayer->ShouldPaintWithFilters())
    {
      parentLayer->SetFilterBackendNeedsRepaintingInRect(parentLayerRect);
      return;
    }

    if (parentLayer->IsRenderViewLayer())
    {
      Downcast<RenderView>(parentLayer->Renderer()).RepaintViewRectangle(parentLayerRect);
      return;
    }

    krys_debug_assert(false);
  }

  RenderLayerFilters &RenderLayer::EnsureLayerFilters() noexcept
  {
    if (_filters)
    {
      return *_filters;
    }

    auto scale = Renderer().GetDocument().DeviceScaleFactor();
    _filters = RenderLayerFilters::Create(*this, {scale, scale});
    return *_filters;
  }

  void RenderLayer::ClearLayerFilters() noexcept
  {
    if (RefPtr filters = std::exchange(_filters, nullptr))
    {
      filters->DetachFromLayer();
    }
  }

  void RenderLayer::UpdateFiltersAfterStyleChange(style::Difference diff,
                                                  const RenderStyle *oldStyle) noexcept
  {
    if (Renderer().Style().GetFilter().HasReferenceFilter())
    {
      EnsureLayerFilters().UpdateReferenceFilterClients(Renderer().Style().GetFilter());
    }
    else if (!ShouldPaintWithFilters())
    {
      ClearLayerFilters();
    }
    else if (_filters)
    {
      _filters->RemoveReferenceFilterClients();
    }

    auto filterChanged = [&]
    {
      if (!_filters)
      {
        return false;
      }
      if (diff < style::DifferenceResult::RepaintLayer)
      {
        return false;
      }
      if (!oldStyle)
      {
        return false;
      }
      if (oldStyle->GetFilter() != Renderer().Style().GetFilter())
      {
        return true;
      }
      auto currentColorChanged = oldStyle->GetColor() != Renderer().Style().GetColor();
      if (currentColorChanged && oldStyle->GetFilter().HasFilterThatRequiresRepaintForCurrentColorChange())
      {
        return true;
      }
      return false;
    };
    if (filterChanged())
    {
      ClearLayerFilters();
    }
  }

  void RenderLayer::UpdateFilterPaintingStrategy() noexcept
  {
    // RenderLayerFilters is only used to render the filters in software mode, so we always need to run
    // updateFilterPaintingStrategy() after the composited mode might have changed for this layer.
    if (!ShouldPaintWithFilters())
    {
      // Don't delete the whole filter info here, because we might use it for loading SVG reference filter
      // files.
      if (_filters)
      {
        _filters->ClearFilter();
      }

      // Early-return only if we *don't* have reference filters. For reference filters, we still want the
      // FilterEffect graph built for us, even if we're composited.
      if (!Renderer().Style().GetFilter().HasReferenceFilter())
      {
        return;
      }
    }

    EnsureLayerFilters();
  }

  // FIXME: Why ask the renderer this given we have m_blendMode?
  bool RenderLayer::HasBlendMode() const noexcept
  {
    return Renderer().HasBlendMode();
  }

  bool RenderLayer::IsViewportConstrained() const noexcept
  {
    return Renderer().IsFixedPositioned() || Renderer().IsStickilyPositioned();
  }

  bool RenderLayer::IsRenderFragmentedFlow() const noexcept
  {
    return Renderer().IsRenderFragmentedFlow();
  }

  bool RenderLayer::CanUseOffsetFromAncestor() const noexcept
  {
    // FIXME: This really needs to know if there are transforms on this layer and any of the layers between it
    // and the ancestor in question.
    // FIXME: SVG roots too, once there's SVG.
    return !IsTransformed();
  }

  bool RenderLayer::CanUseOffsetFromAncestor(const RenderLayer &ancestor) const noexcept
  {
    for (const auto *layer = this; layer && layer != &ancestor; layer = layer->Parent())
    {
      if (!layer->CanUseOffsetFromAncestor())
      {
        return false;
      }
    }
    return true;
  }

  static bool IsContainerForPositioned(RenderLayer &layer, PositionType position,
                                       bool establishesTopLayer) noexcept
  {
    if (establishesTopLayer)
    {
      return layer.IsRenderViewLayer();
    }

    switch (position)
    {
      case PositionType::Fixed:    return layer.Renderer().CanContainFixedPositionObjects();

      case PositionType::Absolute: return layer.Renderer().CanContainAbsolutelyPositionedObjects();

      default:                     krys_debug_assert(false); return false;
    }
  }

  bool RenderLayer::AncestorLayerIsInContainingBlockChain(const RenderLayer &ancestor,
                                                          const RenderLayer *checkLimit) const noexcept
  {
    if (&ancestor == this)
    {
      return true;
    }

    for (const auto *currentBlock = Renderer().ContainingBlock();
         currentBlock && !Is<RenderView>(*currentBlock); currentBlock = currentBlock->ContainingBlock())
    {
      auto *currLayer = currentBlock->Layer();
      if (currLayer == &ancestor)
      {
        return true;
      }

      if (currLayer && currLayer == checkLimit)
      {
        return false;
      }
    }

    return false;
  }

  RenderLayer *RenderLayer::EnclosingAncestorForPosition(PositionType position) const noexcept
  {
    auto *curr = Parent();
    while (curr && !IsContainerForPositioned(*curr, position, EstablishesTopLayer()))
    {
      curr = curr->Parent();
    }

    krys_debug_assert(!EstablishesTopLayer() || !curr || curr == Renderer().View().Layer());
    return curr;
  }

  RenderLayer *RenderLayer::EnclosingLayerInContainingBlockOrder() const noexcept
  {
    for (const auto *currentBlock = Renderer().ContainingBlock(); currentBlock;
         currentBlock = currentBlock->ContainingBlock())
    {
      if (auto *layer = currentBlock->Layer())
      {
        return layer;
      }
    }

    return nullptr;
  }

  RenderLayer *RenderLayer::EnclosingContainingBlockLayer(CrossFrameBoundaries) const noexcept
  {
    // FIXME: Cross into the owner frame's layer (enclosingFrameRenderLayer), once there are frames.
    return EnclosingLayerInContainingBlockOrder();
  }

  RenderLayer *RenderLayer::EnclosingOverflowClipLayer(IncludeSelfOrNot includeSelf) const noexcept
  {
    const RenderLayer *layer = (includeSelf == IncludeSelf) ? this : Parent();
    while (layer)
    {
      if (layer->Renderer().HasPotentiallyScrollableOverflow())
      {
        return const_cast<RenderLayer *>(layer);
      }

      layer = layer->Parent();
    }
    return nullptr;
  }

  RenderLayer *RenderLayer::ClippingRootForPainting() const noexcept
  {
    if (IsComposited())
    {
      return const_cast<RenderLayer *>(this);
    }

    if (PaintsIntoProvidedBacking())
    {
      return BackingProviderLayer();
    }

    // FIXME: Hidden backfaces in preserve-3d contexts
    // (css3DTransformBackfaceVisibilityInteroperabilityEnabled), once there are settings.
    const RenderLayer *current = this;
    while (current)
    {
      if (current->IsRenderViewLayer())
      {
        return const_cast<RenderLayer *>(current);
      }

      current = current->PaintOrderParent();
      krys_debug_assert(current);
      if (current->Transform() || CompositedWithOwnBackingStore(*current))
      {
        return const_cast<RenderLayer *>(current);
      }

      if (current->PaintsIntoProvidedBacking())
      {
        return current->BackingProviderLayer();
      }
    }

    krys_debug_assert(false);
    return nullptr;
  }

#pragma endregion

#pragma region Coordinates

  void RenderLayer::ConvertToPixelSnappedLayerCoords(const RenderLayer *ancestorLayer,
                                                     IntPoint &roundedLocation,
                                                     ColumnOffsetAdjustment adjustForColumns) const noexcept
  {
    LayoutPoint location =
      ConvertToLayerCoords(ancestorLayer, LayoutPoint(roundedLocation), adjustForColumns);
    roundedLocation = RoundedIntPoint(location);
  }

  // Returns the layer reached on the walk up towards the ancestor.
  static const RenderLayer *AccumulateOffsetTowardsAncestor(const RenderLayer *layer,
                                                            const RenderLayer *ancestorLayer,
                                                            LayoutPoint &location,
                                                            RenderLayer::ColumnOffsetAdjustment) noexcept
  {
    krys_debug_assert(ancestorLayer != layer);

    const auto &renderer = layer->Renderer();
    auto position = renderer.Style().GetPosition();

    // FIXME: Positioning of out-of-flow(fixed, absolute) elements collected in a RenderFragmentedFlow
    // may need to be revisited in a future patch.
    if (position == PositionType::Fixed && (!ancestorLayer || ancestorLayer == renderer.View().Layer()))
    {
      // If the fixed layer's container is the root, just add in the offset of the view. We can obtain this by
      // calling localToAbsolute() on the RenderView.
      location.MoveBy(LayoutPoint(renderer.LocalToAbsolute({}, {MapCoordinatesMode::IsFixed})));
      return ancestorLayer;
    }

    if (position == PositionType::Fixed)
    {
      // For a fixed layers, we need to walk up to the root to see if there's a fixed position container
      // (e.g. a transformed layer). It's an error to call offsetFromAncestor() across a layer with a
      // transform, so we should always find the ancestor at or before we find the fixed position container,
      // if the container is transformed.
      RenderLayer *fixedPositionContainerLayer = nullptr;
      bool foundAncestor = false;
      for (auto *currLayer = layer->Parent(); currLayer; currLayer = currLayer->Parent())
      {
        if (currLayer == ancestorLayer)
        {
          foundAncestor = true;
        }

        if (IsContainerForPositioned(*currLayer, PositionType::Fixed, layer->EstablishesTopLayer()))
        {
          fixedPositionContainerLayer = currLayer;
          // A layer that has a transform-related property but not a
          // transform still acts as a fixed-position container.
          // Accumulating offsets across such layers is allowed.
          if (currLayer->Transform())
          {
            krys_debug_assert(foundAncestor);
          }
          break;
        }
      }

      krys_debug_assert(fixedPositionContainerLayer); // We should have hit the RenderView's layer at least.

      if (fixedPositionContainerLayer != ancestorLayer)
      {
        auto fixedContainerCoords = layer->OffsetFromAncestor(fixedPositionContainerLayer);
        auto ancestorCoords =
          foundAncestor ? ancestorLayer->OffsetFromAncestor(fixedPositionContainerLayer) : LayoutSize {};
        location.Move(fixedContainerCoords - ancestorCoords);
        return foundAncestor ? ancestorLayer : fixedPositionContainerLayer;
      }
    }

    RenderLayer *parentLayer;
    if (position == PositionType::Absolute || position == PositionType::Fixed)
    {
      // Do what enclosingAncestorForPosition() does, but check for ancestorLayer along the way.
      parentLayer = layer->Parent();
      bool foundAncestorFirst = false;
      while (parentLayer)
      {
        // RenderFragmentedFlow is a positioned container, child of RenderView, positioned at (0,0).
        // This implies that, for out-of-flow positioned elements inside a RenderFragmentedFlow,
        // we are bailing out before reaching root layer.
        if (IsContainerForPositioned(*parentLayer, position, layer->EstablishesTopLayer()))
        {
          break;
        }

        if (parentLayer == ancestorLayer)
        {
          foundAncestorFirst = true;
          break;
        }

        parentLayer = parentLayer->Parent();
      }

      if (foundAncestorFirst)
      {
        // Found ancestorLayer before the abs. positioned container, so compute offset of both relative
        // to enclosingAncestorForPosition and subtract.
        auto *positionedAncestor = parentLayer->EnclosingAncestorForPosition(position);
        auto thisCoords = layer->OffsetFromAncestor(positionedAncestor);
        auto ancestorCoords = ancestorLayer->OffsetFromAncestor(positionedAncestor);
        location.Move(thisCoords - ancestorCoords);
        return ancestorLayer;
      }
    }
    else
    {
      parentLayer = layer->Parent();
    }

    if (!parentLayer)
    {
      return nullptr;
    }

    location.MoveBy(layer->Location());

    // FIXME: Translate from the multi-column flow into its column (AdjustForColumns), once there are columns.

    return parentLayer;
  }

  LayoutPoint RenderLayer::ConvertToLayerCoords(const RenderLayer *ancestorLayer, const LayoutPoint &location,
                                                ColumnOffsetAdjustment adjustForColumns) const noexcept
  {
    if (ancestorLayer == this)
    {
      return location;
    }

    const auto *currLayer = this;
    auto locationInLayerCoords = location;
    while (currLayer && currLayer != ancestorLayer)
    {
      currLayer =
        AccumulateOffsetTowardsAncestor(currLayer, ancestorLayer, locationInLayerCoords, adjustForColumns);
    }

    // FIXME: Snap the whole SVG subtree as one block, once there's SVG.
    return locationInLayerCoords;
  }

  LayoutSize RenderLayer::OffsetFromAncestor(const RenderLayer *ancestorLayer,
                                             ColumnOffsetAdjustment adjustForColumns) const noexcept
  {
    return ToLayoutSize(ConvertToLayerCoords(ancestorLayer, LayoutPoint(), adjustForColumns));
  }

#pragma endregion

#pragma region Renderer geometry

  LayoutPoint RenderLayer::RendererLocation() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->Location();
    }
    return {};
  }

  LayoutRect RenderLayer::RendererBorderBoxRect() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->BorderBoxRect();
    }
    return {};
  }

  LayoutRect RenderLayer::RendererVisualOverflowRect() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->VisualOverflowRect();
    }
    return {};
  }

  LayoutRect RenderLayer::RendererOverflowClipRect(const LayoutPoint &location,
                                                   OverlayScrollbarSizeRelevancy relevancy) const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->OverflowClipRect(location, relevancy);
    }
    return {};
  }

  LayoutRect RenderLayer::RendererOverflowClipRectForChildLayers(
    const LayoutPoint &location, OverlayScrollbarSizeRelevancy relevancy) const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->OverflowClipRectForChildLayers(location, relevancy);
    }
    return {};
  }

  bool RenderLayer::RendererHasVisualOverflow() const noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
    {
      return box->HasVisualOverflow();
    }
    return false;
  }

#pragma endregion

#pragma region Clip rects

  Ref<ClipRects> RenderLayer::UpdateClipRects(const ClipRectsContext &clipRectsContext) noexcept
  {
    ClipRectsType clipRectsType = clipRectsContext.Type;
    krys_debug_assert(clipRectsType < NumCachedClipRectsTypes);
    krys_debug_assert(!clipRectsContext.Options.Contains(ClipRectsOption::Temporary));
    krys_debug_assert(!clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter));
    if (_clipRectsCache)
    {
      if (auto *clipRects = _clipRectsCache->GetClipRects(clipRectsContext))
      {
#if KRYS_DEBUG
        krys_debug_assert(clipRectsContext.RootLayer == _clipRectsCache->ClipRectsRoot[clipRectsType]);
#endif
        return ShareRef(*clipRects); // We have the correct cached value.
      }
    }

    if (!_clipRectsCache)
    {
      _clipRectsCache = CreateUnique<ClipRectsCache>();
    }

#if KRYS_DEBUG
    _clipRectsCache->ClipRectsRoot[clipRectsType] = clipRectsContext.RootLayer;
#endif
    krys_debug_assert((clipRectsContext.GetOverlayScrollbarSizeRelevancy()
                       == OverlayScrollbarSizeRelevancy::IncludeOverlayScrollbarSize)
                      == (clipRectsContext.Type == RootRelativeClipRects));

    RefPtr<ClipRects> parentClipRects = this->ParentClipRects(clipRectsContext);

    auto clipRects = ClipRects::Create();
    CalculateClipRects(clipRectsContext, *clipRects);

    if (parentClipRects && *parentClipRects == *clipRects)
    {
      _clipRectsCache->SetClipRects(clipRectsType, clipRectsContext.RespectOverflowClip(),
                                    RefPtr<ClipRects> {parentClipRects});
      Ref<ClipRects> sharedClipRects = parentClipRects;
      return sharedClipRects;
    }

    _clipRectsCache->SetClipRects(clipRectsType, clipRectsContext.RespectOverflowClip(),
                                  RefPtr<ClipRects> {clipRects});
    return clipRects;
  }

  bool RenderLayer::ClipCrossesPaintingBoundary() const noexcept
  {
    return Parent()->EnclosingPaginationLayer(IncludeCompositedPaginatedLayers)
             != EnclosingPaginationLayer(IncludeCompositedPaginatedLayers)
           || Parent()->EnclosingCompositingLayerForRepaint().Layer
                != EnclosingCompositingLayerForRepaint().Layer;
  }

  void RenderLayer::CalculateClipRects(const ClipRectsContext &clipRectsContext,
                                       ClipRects &clipRects) const noexcept
  {
    if (!Parent())
    {
      // The root layer's clip rect is always infinite.
      clipRects.Reset();
      return;
    }

    if (auto parentClipRects = this->ParentClipRects(clipRectsContext))
    {
      clipRects = *parentClipRects;
    }
    else
    {
      clipRects.Reset();
    }

    // A fixed object is essentially the root of its containing block hierarchy, so when
    // we encounter such an object, we reset our clip rects to the fixedClipRect.
    if (Renderer().IsFixedPositioned())
    {
      clipRects.SetPosClipRect(clipRects.FixedClipRect());
      clipRects.SetOverflowClipRect(clipRects.FixedClipRect());
      clipRects.SetFixed(true);
    }
    else if (Renderer().IsInFlowPositioned())
    {
      clipRects.SetPosClipRect(clipRects.OverflowClipRect());
    }
    else if (Renderer().ShouldUsePositionedClipping())
    {
      clipRects.SetOverflowClipRect(clipRects.PosClipRect());
    }

    // Update the clip rects that will be passed to child layers.
    if ((Renderer().HasNonVisibleOverflow()
         && (clipRectsContext.RespectOverflowClip() || this != clipRectsContext.RootLayer))
        || Renderer().HasClip())
    {
      // This layer establishes a clip of some kind.

      // FIXME: Transforming a clip doesn't make a whole lot of sense, since it we have to round out to the
      // bounding box of the transformed quad.
      // It would be better for callers to transform rects into the coordinate space of the nearest clipped
      // layer, apply the clip in local space, and then repeat until the required coordinate space is reached.
      // FIXME: Transform the clips into the root's coordinates (localToContainerQuad) when the offset can't
      // be used, once there's geometry mapping.

      auto offset = ToLayoutPoint(OffsetFromAncestor(clipRectsContext.RootLayer, AdjustForColumns));

      if (clipRects.Fixed() && &clipRectsContext.RootLayer->Renderer() == &Renderer().View())
      {
        offset -= ToLayoutSize(Renderer().View().FrameView().ScrollPositionForFixedPosition());
      }

      if (Renderer().HasNonVisibleOverflow())
      {
        ClipRect newOverflowClip =
          RendererOverflowClipRectForChildLayers({}, clipRectsContext.GetOverlayScrollbarSizeRelevancy());
        newOverflowClip.MoveBy(offset);
        newOverflowClip.SetAffectedByRadius(Renderer().Style().HasBorderRadius());
        clipRects.SetOverflowClipRect(Intersection(newOverflowClip, clipRects.OverflowClipRect()));
        if (Renderer().CanContainAbsolutelyPositionedObjects())
        {
          clipRects.SetPosClipRect(Intersection(newOverflowClip, clipRects.PosClipRect()));
        }
        if (Renderer().CanContainFixedPositionObjects())
        {
          clipRects.SetFixedClipRect(Intersection(newOverflowClip, clipRects.FixedClipRect()));
        }
      }
      if (Renderer().HasClip())
      {
        if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
        {
          LayoutRect newPosClip = box->ClipRect({});
          newPosClip.MoveBy(offset);
          clipRects.SetPosClipRect(Intersection(newPosClip, clipRects.PosClipRect()));
          clipRects.SetOverflowClipRect(Intersection(newPosClip, clipRects.OverflowClipRect()));
          clipRects.SetFixedClipRect(Intersection(newPosClip, clipRects.FixedClipRect()));
        }
      }
    }
    else if (Renderer().HasNonVisibleOverflow() && Transform() && Renderer().Style().HasBorderRadius())
    {
      clipRects.SetOverflowClipRectAffectedByRadius();
    }
  }

  RefPtr<ClipRects> RenderLayer::ParentClipRects(const ClipRectsContext &clipRectsContext) const noexcept
  {
    auto *containerLayer = Parent();
    if (clipRectsContext.RootLayer == this || !Parent())
    {
      return nullptr;
    }

    if (clipRectsContext.Type == PaintingClipRects && _suppressAncestorClippingInsideFilter
        && !clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter))
    {
      return nullptr;
    }

    auto temporaryParentClipRects = [&](const ClipRectsContext &clipContext)
    {
      auto parentClipRects = ClipRects::Create();
      containerLayer->CalculateClipRects(clipContext, *parentClipRects);
      RefPtr<ClipRects> result = parentClipRects;
      return result;
    };

    if (clipRectsContext.Options.Contains(ClipRectsOption::Temporary)
        || clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter))
    {
      return temporaryParentClipRects(clipRectsContext);
    }

    if (clipRectsContext.Type != AbsoluteClipRects && ClipCrossesPaintingBoundary())
    {
      ClipRectsContext tempClipRectsContext(clipRectsContext);
      tempClipRectsContext.Options.Add(ClipRectsOption::Temporary);
      return temporaryParentClipRects(tempClipRectsContext);
    }

    return containerLayer->UpdateClipRects(clipRectsContext);
  }

  static ClipRect BackgroundClipRectForPosition(const ClipRects &parentRects, PositionType position) noexcept
  {
    if (position == PositionType::Fixed)
    {
      return parentRects.FixedClipRect();
    }

    if (position == PositionType::Absolute)
    {
      return parentRects.PosClipRect();
    }

    return parentRects.OverflowClipRect();
  }

  ClipRect RenderLayer::BackgroundClipRect(const ClipRectsContext &clipRectsContext) const noexcept
  {
    krys_debug_assert(Parent());
    ClipRect backgroundClipRect;
    auto parentRects = ParentClipRects(clipRectsContext);
    if (!parentRects)
    {
      backgroundClipRect.Reset();
      return backgroundClipRect;
    }
    backgroundClipRect = BackgroundClipRectForPosition(*parentRects, Renderer().Style().GetPosition());
    RenderView &view = Renderer().View();
    // Note: infinite clipRects should not be scrolled here, otherwise they will accidentally no longer be
    // considered infinite.
    if (parentRects->Fixed() && &clipRectsContext.RootLayer->Renderer() == &view
        && !backgroundClipRect.IsInfinite())
    {
      backgroundClipRect.MoveBy(view.FrameView().ScrollPositionForFixedPosition());
    }
    return backgroundClipRect;
  }

  ClipRect RenderLayer::CalculateBackgroundClipRect(const ClipRectsContext &clipRectsContext,
                                                    const LayoutSize &offsetFromRoot) const noexcept
  {
    ClipRect backgroundClipRect = LayoutRect::InfiniteRect();

    if (clipRectsContext.RootLayer != this && Parent())
    {
      backgroundClipRect = this->BackgroundClipRect(clipRectsContext);
    }

    bool shouldApplyClip = clipRectsContext.Type != PaintingClipRects
                           || !_suppressAncestorClippingInsideFilter
                           || clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter);

    if (shouldApplyClip && Renderer().HasClip())
    {
      if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
      {
        // Clip applies to *us* as well, so update the damageRect.
        auto newPosClip = box->ClipRect(ToLayoutPoint(offsetFromRoot));
        backgroundClipRect.Intersect(newPosClip);
      }
    }

    return backgroundClipRect;
  }

  ClipRect RenderLayer::CalculateBackgroundRect(const ClipRectsContext &clipRectsContext,
                                                const LayoutSize &offsetFromRoot) const noexcept
  {
    auto backgroundRect = CalculateBackgroundClipRect(clipRectsContext, offsetFromRoot);

    if (clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter)
        || !Renderer().HasClipOrNonVisibleOverflow())
    {
      return backgroundRect;
    }

    if (this == clipRectsContext.RootLayer && !clipRectsContext.RespectOverflowClip())
    {
      return backgroundRect;
    }

    // If we establish a clip at all, then make sure our background rect is intersected with our layer's
    // bounds including our visual overflow, since any visual overflow like box-shadow or border-outset is not
    // clipped by overflow:auto/hidden.
    if (RendererHasVisualOverflow())
    {
      // FIXME: Does not do the right thing with CSS regions yet, since we don't yet factor in the
      // individual region boxes as overflow.
      auto layerBoundsWithVisualOverflow = RendererVisualOverflowRect();
      if (auto *box = GetRenderBox())
      {
        // Layers are in physical coordinates, so the overflow has to be flipped.
        box->FlipForWritingMode(layerBoundsWithVisualOverflow);
      }

      layerBoundsWithVisualOverflow.Move(offsetFromRoot);
      backgroundRect.Intersect(layerBoundsWithVisualOverflow);
      return backgroundRect;
    }

    auto bounds = RendererBorderBoxRect();

    // Shift the bounds to be for our region only.
    bounds.Move(offsetFromRoot);
    backgroundRect.Intersect(bounds);
    return backgroundRect;
  }

  ClipRect RenderLayer::CalculateForegroundRect(const ClipRectsContext &clipRectsContext,
                                                const LayoutSize &offsetFromRoot) const noexcept
  {
    auto foregroundRect = CalculateBackgroundClipRect(clipRectsContext, offsetFromRoot);

    if (clipRectsContext.Options.Contains(ClipRectsOption::OutsideFilter)
        || !Renderer().HasNonVisibleOverflow())
    {
      return foregroundRect;
    }

    // This layer establishes a clip of some kind.
    if (this != clipRectsContext.RootLayer || clipRectsContext.RespectOverflowClip())
    {
      auto overflowClipRect = RendererOverflowClipRect(ToLayoutPoint(offsetFromRoot),
                                                       clipRectsContext.GetOverlayScrollbarSizeRelevancy());
      foregroundRect.Intersect(overflowClipRect);
      foregroundRect.SetAffectedByRadius(true);
      return foregroundRect;
    }

    if (Transform() && Renderer().Style().HasBorderRadius())
    {
      foregroundRect.SetAffectedByRadius(true);
    }

    return foregroundRect;
  }

  LayerFragment::Rects RenderLayer::CalculateRects(const ClipRectsContext &clipRectsContext,
                                                   const LayoutSize &offsetFromRoot,
                                                   const LayoutRect &paintDirtyRect) const noexcept
  {
    auto layerBounds = LayoutRect(ToLayoutPoint(offsetFromRoot), LayoutSize(Size()));
    auto backgroundRect = CalculateBackgroundRect(clipRectsContext, offsetFromRoot);
    auto foregroundRect = CalculateForegroundRect(clipRectsContext, offsetFromRoot);
    return {layerBounds, paintDirtyRect, backgroundRect, foregroundRect};
  }

  LayoutRect RenderLayer::ClipRectRelativeToAncestor(const RenderLayer *ancestor,
                                                     LayoutSize offsetFromAncestor,
                                                     const LayoutRect &constrainingRect,
                                                     bool temporaryClipRects) const noexcept
  {
    auto options = ClipRectDefaultOptions;
    if ((_enclosingPaginationLayer && _enclosingPaginationLayer.get() != ancestor) || temporaryClipRects)
    {
      options.Add(ClipRectsOption::Temporary);
    }
    ClipRectsContext clipRectsContext(ancestor, PaintingClipRects, options);
    auto backgroundRect = CalculateBackgroundRect(clipRectsContext, offsetFromAncestor);
    return Intersection(backgroundRect.Rect(), constrainingRect);
  }

  LayoutRect RenderLayer::LocalClipRect(bool &clipExceedsBounds, LocalClipRectMode mode) const noexcept
  {
    clipExceedsBounds = false;
    // FIXME: border-radius not accounted for.
    // FIXME: Regions not accounted for.
    const RenderLayer *clippingRootLayer =
      mode == LocalClipRectMode::ExcludeCompositingState ? this : ClippingRootForPainting();
    LayoutSize offsetFromRoot = OffsetFromAncestor(clippingRootLayer);
    LayoutRect clipRect =
      ClipRectRelativeToAncestor(clippingRootLayer, offsetFromRoot, LayoutRect::InfiniteRect());
    if (clipRect.IsInfinite())
    {
      return clipRect;
    }

    if (Renderer().HasClip())
    {
      if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
      {
        // CSS clip may be larger than our border box.
        LayoutRect cssClipRect = box->ClipRect({});
        clipExceedsBounds =
          !cssClipRect.IsEmpty()
          && (clipRect.Width() < cssClipRect.Width() || clipRect.Height() < cssClipRect.Height());
      }
    }

    clipRect.Move(-offsetFromRoot);
    return clipRect;
  }

  void RenderLayer::ClearClipRectsIncludingDescendants(ClipRectsType typeToClear) noexcept
  {
    // FIXME: it's not clear how this layer not having clip rects guarantees that no descendants have any.
    if (!_clipRectsCache)
    {
      return;
    }

    ClearClipRects(typeToClear);

    for (RenderLayer *l = FirstChild(); l; l = l->NextSibling())
    {
      l->ClearClipRectsIncludingDescendants(typeToClear);
    }
  }

  void RenderLayer::ClearClipRects(ClipRectsType typeToClear) noexcept
  {
    if (typeToClear == AllClipRectTypes)
    {
      _clipRectsCache = nullptr;
    }
    else if (_clipRectsCache)
    {
      krys_debug_assert(typeToClear < NumCachedClipRectsTypes);
      _clipRectsCache->SetClipRects(typeToClear, RespectOverflowClip, nullptr);
      _clipRectsCache->SetClipRects(typeToClear, IgnoreOverflowClip, nullptr);
    }
  }

#pragma endregion

#pragma region Bounds

  bool RenderLayer::IntersectsDamageRect(const LayoutRect &layerBounds, const LayoutRect &damageRect,
                                         const RenderLayer *rootLayer, const LayoutSize &offsetFromRoot,
                                         const Maybe<LayoutRect> &cachedBoundingBox) const noexcept
  {
    // Always examine the canvas and the root.
    // FIXME: Could eliminate the isDocumentElementRenderer() check if we fix background painting so that the
    // RenderView paints the root's background.
    if (IsRenderViewLayer() || Renderer().IsDocumentElementRenderer())
    {
      return true;
    }

    if (damageRect.IsInfinite())
    {
      return true;
    }

    if (damageRect.IsEmpty())
    {
      return false;
    }

    // If we aren't an inline flow, and our layer bounds do intersect the damage rect, then we can return
    // true.
    if (!Renderer().IsRenderInline() && layerBounds.Intersects(damageRect))
    {
      return true;
    }

    // Otherwise we need to compute the bounding box of this single layer and see if it intersects
    // the damage rect. It's possible the fragment computed the bounding box already, in which case we
    // can use the cached value.
    if (cachedBoundingBox)
    {
      return cachedBoundingBox->Intersects(damageRect);
    }

    return BoundingBox(rootLayer, offsetFromRoot).Intersects(damageRect);
  }

  LayoutRect RenderLayer::LocalBoundingBox(EnumSet<CalculateLayerBoundsFlag> flags) const noexcept
  {
    // There are three special cases we need to consider.
    // (1) Inline Flows.  For inline flows we will create a bounding box that fully encompasses all of the
    // lines occupied by the inline.  In other words, if some <span> wraps to three lines, we'll create a
    // bounding box that fully encloses the line boxes of all three lines (including overflow on those lines).
    // (2) Left/Top Overflow.  The width/height of layers already includes right/bottom overflow.  However, in
    // the case of left/top overflow, we have to create a bounding box that will extend to include this
    // overflow. (3) Floats.  When a layer has overhanging floats that it paints, we need to make sure to
    // include these overhanging floats as part of our bounding box.  We do this because we are the
    // responsible layer for both hit testing and painting those floats.
    LayoutRect result;
    if (auto *renderInline = DynamicDowncast<RenderInline>(Renderer()); renderInline && Renderer().IsInline())
    {
      // FIXME: The visual overflow of the inline's lines (linesVisualOverflowBoundingBox), once there's
      // inline layout.
      result = LayoutRect(LayoutPoint(), LayoutSize(renderInline->LinesBoundingBox().Size()));
    }
    else if (auto *tableRow = DynamicDowncast<RenderTableRow>(Renderer()))
    {
      // FIXME: Our bounding box is just the union of all of our cells' border/overflow rects, once there's
      // table layout.
      result = tableRow->VisualOverflowRect();
    }
    else
    {
      RenderBox *box = GetRenderBox();
      krys_debug_assert(box);
      if (!flags.Contains(DontConstrainForMask) && box->HasMask())
      {
        result = box->MaskClipRect(LayoutPoint());
        // The mask clip rect is in physical coordinates, so we have to flip, since localBoundingBox is not.
        box->FlipForWritingMode(result);
      }
      else
      {
        result = box->VisualOverflowRect();
      }

      if (flags.Contains(IncludeRootBackgroundPaintingArea) && Renderer().IsDocumentElementRenderer())
      {
        // If the root layer becomes composited (e.g. because some descendant with negative z-index is
        // composited), then it has to be big enough to cover the viewport in order to display the background.
        // This is akin to the code in RenderBox::paintRootBoxFillLayers().
        // FIXME: The frame view's contents size (contentsWidth, contentsHeight), once it has one; it's set
        // from the view's document rect.
        const auto contentsSize = Renderer().View().UnscaledDocumentRect().Size();
        result.SetWidth(std::max(result.Width(), LayoutUnit(contentsSize.Width()) - result.X()));
        result.SetHeight(std::max(result.Height(), LayoutUnit(contentsSize.Height()) - result.Y()));
      }
    }
    return result;
  }

  LayoutRect RenderLayer::BoundingBox(const RenderLayer *ancestorLayer, const LayoutSize &offsetFromRoot,
                                      EnumSet<CalculateLayerBoundsFlag> flags) const noexcept
  {
    LayoutRect result = LocalBoundingBox(flags);
    if (Renderer().View().FrameView().HasFlippedBlockRenderers())
    {
      if (auto *box = GetRenderBox())
      {
        box->FlipForWritingMode(result);
      }
      else
      {
        Renderer().ContainingBlock()->FlipForWritingMode(result);
      }
    }

    // FIXME: Split the box into the fragment boxes of its pagination layers (fragmentsBoundingBox), once
    // there's fragmentation.
    (void)ancestorLayer;

    result.Move(offsetFromRoot);
    return result;
  }

  IntRect RenderLayer::AbsoluteBoundingBox() const noexcept
  {
    const RenderLayer *rootLayer = Root();
    return SnappedIntRect(BoundingBox(rootLayer, OffsetFromAncestor(rootLayer)));
  }

  LayoutRect RenderLayer::CalculateLayerBounds(const RenderLayer *ancestorLayer,
                                               const LayoutSize &offsetFromRoot,
                                               EnumSet<CalculateLayerBoundsFlag> flags) const noexcept
  {
    if (!IsSelfPaintingLayer())
    {
      return LayoutRect();
    }

    // FIXME: This could be improved to do a check like hasVisibleNonCompositingDescendantLayers() (bug
    // 92580).
    if (flags.Contains(ExcludeHiddenDescendants) && this != ancestorLayer && !HasVisibleContent()
        && !HasVisibleDescendant())
    {
      return LayoutRect();
    }

    // FIXME: Descendants captured in a view transition (ExcludeViewTransitionCapturedDescendants), once there
    // are view transitions.

    if (IsRenderViewLayer())
    {
      // The root layer is always just the size of the document.
      // FIXME: The unscaled document rect, once the view knows it.
      return Renderer().View().LayoutOverflowRect();
    }

    LayoutRect boundingBoxRect = LocalBoundingBox(flags | IncludeRootBackgroundPaintingArea);
    if (Renderer().View().FrameView().HasFlippedBlockRenderers())
    {
      if (auto *box = DynamicDowncast<RenderBox>(Renderer()))
      {
        box->FlipForWritingMode(boundingBoxRect);
      }
      else
      {
        Renderer().ContainingBlock()->FlipForWritingMode(boundingBoxRect);
      }
    }

    LayoutRect unionBounds = boundingBoxRect;

    if (flags.ContainsAny({UseLocalClipRectIfPossible, UseLocalClipRectExcludingCompositingIfPossible}))
    {
      bool clipExceedsBounds = false;
      LayoutRect localClipRect =
        this->LocalClipRect(clipExceedsBounds, flags.Contains(UseLocalClipRectExcludingCompositingIfPossible)
                                                 ? LocalClipRectMode::ExcludeCompositingState
                                                 : LocalClipRectMode::IncludeCompositingState);
      if (!localClipRect.IsInfinite() && !clipExceedsBounds)
      {
        if (flags.Contains(IncludeSelfTransform) && PaintsWithTransform({}))
        {
          localClipRect = Transform()->MapRect(localClipRect);
        }

        localClipRect.Move(OffsetFromAncestor(ancestorLayer));
        return localClipRect;
      }
    }

    // FIXME: should probably just pass 'flags' down to descendants.
    EnumSet<CalculateLayerBoundsFlag> descendantFlags = DefaultCalculateLayerBoundsFlags();
    if (flags.Contains(PreserveAncestorFlags))
    {
      descendantFlags = flags;
    }
    else
    {
      if (flags.Contains(ExcludeHiddenDescendants))
      {
        descendantFlags.Add(ExcludeHiddenDescendants);
      }
      if (flags.Contains(IncludeCompositedDescendants))
      {
        descendantFlags.Add(IncludeCompositedDescendants);
      }
    }

    const_cast<RenderLayer *>(this)->UpdateLayerListsIfNeeded();

    krys_debug_assert(IsStackingContext() || !PositiveZOrderLayers().size());

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(const_cast<RenderLayer &>(*this));
#endif

    auto computeLayersUnion = [this, &unionBounds, flags, descendantFlags](const RenderLayer &childLayer)
    {
      if (!flags.Contains(IncludeCompositedDescendants)
          && (childLayer.IsComposited() || childLayer.PaintsIntoProvidedBacking()))
      {
        return;
      }
      LayoutRect childBounds =
        childLayer.CalculateLayerBounds(this, childLayer.OffsetFromAncestor(this), descendantFlags);
      // Ignore child layer (and behave as if we had overflow: hidden) when it is positioned off the parent
      // layer so much that we hit the max LayoutUnit value.
      (void)unionBounds.CheckedUnite(childBounds);
    };

    for (auto *childLayer : NegativeZOrderLayers())
    {
      computeLayersUnion(*childLayer);
    }

    for (auto *childLayer : PositiveZOrderLayers())
    {
      computeLayersUnion(*childLayer);
    }

    for (auto *childLayer : NormalFlowLayers())
    {
      computeLayersUnion(*childLayer);
    }

    if (flags.Contains(IncludeFilterOutsets)
        || (flags.Contains(IncludePaintedFilterOutsets)
            && ShouldPaintWithFilters(flags.Contains(IncludeCompositedDescendants)
                                        ? EnumSet<PaintBehavior> {PaintBehavior::FlattenCompositingLayers}
                                        : EnumSet<PaintBehavior> {})))
    {
      unionBounds.Expand(ToLayoutBoxExtent(FilterOutsets()));
    }

    if (flags.Contains(IncludeSelfTransform) && PaintsWithTransform({}))
    {
      TransformationMatrix *affineTrans = Transform();
      boundingBoxRect = affineTrans->MapRect(boundingBoxRect);
      unionBounds = affineTrans->MapRect(unionBounds);
    }
    unionBounds.Move(offsetFromRoot);
    return unionBounds;
  }

#pragma endregion

#pragma region Scrolling

  RenderLayerScrollableArea *RenderLayer::GetScrollableArea() const noexcept
  {
    return _scrollableArea.get();
  }

  RenderLayerScrollableArea *RenderLayer::EnsureLayerScrollableArea() noexcept
  {
    bool hadScrollableArea = GetScrollableArea();

    if (!_scrollableArea)
    {
      _scrollableArea = CreateUnique<RenderLayerScrollableArea>(*this);
    }

    if (!hadScrollableArea)
    {
      // FIXME: The compositing configuration update for asynchronous overflow scrolling, once there's
      // compositing.
      _scrollableArea->RestoreScrollPosition();
    }

    return _scrollableArea.get();
  }

  void RenderLayer::ClearLayerScrollableArea() noexcept
  {
    if (_scrollableArea)
    {
      _scrollableArea->Clear();
      _scrollableArea = nullptr;
    }
  }

  void RenderLayer::UpdateLayerScrollableArea() noexcept
  {
    bool hasScrollableArea = GetScrollableArea();
    bool needsScrollableArea = [&]
    {
      auto *box = DynamicDowncast<RenderBox>(Renderer());
      return box && box->RequiresLayerWithScrollableArea();
    }();

    if (needsScrollableArea == hasScrollableArea)
    {
      return;
    }

    if (needsScrollableArea)
    {
      (void)EnsureLayerScrollableArea();
    }
    else
    {
      ClearLayerScrollableArea();
      // FIXME: The compositing configuration update for asynchronous overflow scrolling, once there's
      // compositing.
    }

    // FIXME: Tell the inspector the scrollbars changed, once there's an inspector.
  }

  IntSize RenderLayer::VisibleSize() const noexcept
  {
    RenderBox *box = GetRenderBox();
    if (!box)
    {
      return IntSize();
    }

    return IntSize(RoundToInt(box->ClientWidth()), RoundToInt(box->ClientHeight()));
  }

  int32 RenderLayer::ScrollWidth() const noexcept
  {
    if (_scrollableArea)
    {
      return _scrollableArea->ScrollWidth();
    }

    RenderBox *box = GetRenderBox();
    krys_debug_assert(box);
    LayoutRect overflowRect(box->LayoutOverflowRect());
    box->FlipForWritingMode(overflowRect);
    return RoundToInt(overflowRect.MaxX() - overflowRect.X());
  }

  int32 RenderLayer::ScrollHeight() const noexcept
  {
    if (_scrollableArea)
    {
      return _scrollableArea->ScrollHeight();
    }

    RenderBox *box = GetRenderBox();
    krys_debug_assert(box);
    LayoutRect overflowRect(box->LayoutOverflowRect());
    box->FlipForWritingMode(overflowRect);
    return RoundToInt(overflowRect.MaxY() - overflowRect.Y());
  }

  RenderLayer *RenderLayer::EnclosingScrollableLayer(IncludeSelfOrNot includeSelf,
                                                     CrossFrameBoundaries crossFrameBoundaries) const noexcept
  {
    auto isConsideredScrollable = [](const RenderLayer &layer)
    {
      auto *box = DynamicDowncast<RenderBox>(layer.Renderer());
      return box && box->CanBeScrolledAndHasScrollableArea();
    };

    if (includeSelf == IncludeSelf && isConsideredScrollable(*this))
    {
      return const_cast<RenderLayer *>(this);
    }

    for (auto *nextLayer = EnclosingContainingBlockLayer(crossFrameBoundaries); nextLayer;
         nextLayer = nextLayer->EnclosingContainingBlockLayer(crossFrameBoundaries))
    {
      if (isConsideredScrollable(*nextLayer))
      {
        return nextLayer;
      }
    }

    return nullptr;
  }

  void RenderLayer::UpdateScrollInfoAfterLayout() noexcept
  {
    UpdateLayerScrollableArea();
    if (_scrollableArea)
    {
      _scrollableArea->UpdateScrollInfoAfterLayout();
    }
  }

  void RenderLayer::UpdateScrollbarSteps() noexcept
  {
    if (_scrollableArea)
    {
      _scrollableArea->UpdateScrollbarSteps();
    }
  }

  bool RenderLayer::CanUseCompositedScrolling() const noexcept
  {
    if (_scrollableArea)
    {
      return _scrollableArea->CanUseCompositedScrolling();
    }
    return false;
  }

  bool RenderLayer::HasCompositedScrollableOverflow() const noexcept
  {
    if (_scrollableArea)
    {
      return _scrollableArea->HasCompositedScrollableOverflow();
    }
    return false;
  }

  void RenderLayer::ComputeHasCompositedScrollableOverflow(LayoutUpToDate layoutUpToDate) noexcept
  {
    if (_scrollableArea)
    {
      _scrollableArea->ComputeHasCompositedScrollableOverflow(layoutUpToDate);
    }
  }

  bool RenderLayer::HasOverlayScrollbars() const noexcept
  {
    if (_scrollableArea)
    {
      return _scrollableArea->HasOverlayScrollbars();
    }
    return false;
  }

  bool RenderLayer::UsesCompositedScrolling() const noexcept
  {
    if (_scrollableArea)
    {
      return _scrollableArea->UsesCompositedScrolling();
    }
    return false;
  }

  bool RenderLayer::CanResize() const noexcept
  {
    // We need a special case for <iframe> because they never have
    // hasNonVisibleOverflow(). However, they do "implicitly" clip their contents, so
    // we want to allow resizing them also.
    // FIXME: iframes too, once there are any.
    return Renderer().HasNonVisibleOverflow() && Renderer().Style().GetResize() != style::Resize::None;
  }

  LayoutSize RenderLayer::MinimumSizeForResizing(float zoomFactor) const noexcept
  {
    // Use the resizer size as the strict minimum size
    auto resizerRect = OverflowControlsRects().Resizer;
    auto minWidth =
      style::EvaluateMinimum<LayoutUnit>(Renderer().Style().MinWidth(), Renderer().ContainingBlock()->Width(),
                                         Renderer().Style().UsedZoomForLength());
    auto minHeight = style::EvaluateMinimum<LayoutUnit>(Renderer().Style().MinHeight(),
                                                        Renderer().ContainingBlock()->Height(),
                                                        Renderer().Style().UsedZoomForLength());
    minWidth = std::max(LayoutUnit(minWidth / zoomFactor), LayoutUnit(resizerRect.Width()));
    minHeight = std::max(LayoutUnit(minHeight / zoomFactor), LayoutUnit(resizerRect.Height()));
    return LayoutSize(minWidth, minHeight);
  }

  void RenderLayer::Resize(const PlatformMouseEvent &event, const LayoutSize &oldOffset) noexcept
  {
    // FIXME: This should be possible on generated content but is not right now.
    if (!InResizeMode() || !CanResize())
    {
      return;
    }

    // FIXME: This should be possible on all elements but is not right now.
    RefPtr<dom::StyledElement> styledElement =
      ShareRefPtr(DynamicDowncast<dom::StyledElement>(Renderer().GetElement()));
    if (!styledElement)
    {
      return;
    }

    // FIXME: The only case where renderer->element()->renderer() != renderer is with continuations. Do they
    // matter here? If they do it would still be better to deal with them explicitly.
    auto *renderer = Downcast<RenderBox>(styledElement->Renderer());

    Ref<dom::Document> document = ShareRef(styledElement->NodeDocument());
    if (!document->Frame()->GetEventHandler().MousePressed())
    {
      return;
    }

    float zoomFactor = renderer->Style().UsedZoom();

    auto absolutePoint = document->View()->WindowToContents(FlooredIntPoint(event.Position()));
    auto localPoint = RoundedIntPoint(AbsoluteToContents(LayoutPoint(absolutePoint)));

    LayoutSize newOffset = OffsetFromResizeCorner(localPoint);
    newOffset.SetWidth(newOffset.Width() / zoomFactor);
    newOffset.SetHeight(newOffset.Height() / zoomFactor);

    LayoutSize currentSize = LayoutSize(renderer->Width() / zoomFactor, renderer->Height() / zoomFactor);

    LayoutSize adjustedOldOffset =
      LayoutSize(oldOffset.Width() / zoomFactor, oldOffset.Height() / zoomFactor);
    if (renderer->Style().ShouldPlaceVerticalScrollbarOnLeft())
    {
      newOffset.SetWidth(-newOffset.Width());
      adjustedOldOffset.SetWidth(-adjustedOldOffset.Width());
    }

    LayoutSize difference =
      (currentSize + newOffset - adjustedOldOffset).ExpandedTo(MinimumSizeForResizing(zoomFactor))
      - currentSize;

    // FIXME: A single mutation record for the style attribute (StyleAttributeMutationScope), with mutation
    // observers of style changes.
    bool isBoxSizingBorder = renderer->Style().GetBoxSizing() == BoxSizing::BorderBox;

    auto resize = renderer->Style().GetResize();
    bool canResizeWidth = resize == style::Resize::Horizontal || resize == style::Resize::Both
                          || (renderer->IsHorizontalWritingMode() ? resize == style::Resize::Inline
                                                                  : resize == style::Resize::Block);
    if (canResizeWidth && difference.Width())
    {
      if (Is<html::HTMLFormControlElement>(*styledElement))
      {
        // Make implicit margins from the theme explicit (see <http://bugs.webkit.org/show_bug.cgi?id=9547>).
        (void)styledElement->SetInlineStyleProperty(
          css::PropertyId::MarginLeft, static_cast<double>(renderer->MarginLeft().ToFloat() / zoomFactor),
          css::UnitType::px);
        (void)styledElement->SetInlineStyleProperty(
          css::PropertyId::MarginRight, static_cast<double>(renderer->MarginRight().ToFloat() / zoomFactor),
          css::UnitType::px);
      }
      LayoutUnit baseWidth =
        renderer->Width() - (isBoxSizingBorder ? 0_lu : renderer->HorizontalBorderAndPaddingExtent());
      baseWidth = baseWidth / zoomFactor;
      (void)styledElement->SetInlineStyleProperty(
        css::PropertyId::Width, RoundToInt(baseWidth + difference.Width()), css::UnitType::px);
    }

    bool canResizeHeight = resize == style::Resize::Vertical || resize == style::Resize::Both
                           || (renderer->IsHorizontalWritingMode() ? resize == style::Resize::Block
                                                                   : resize == style::Resize::Inline);
    if (canResizeHeight && difference.Height())
    {
      if (Is<html::HTMLFormControlElement>(*styledElement))
      {
        // Make implicit margins from the theme explicit (see <http://bugs.webkit.org/show_bug.cgi?id=9547>).
        (void)styledElement->SetInlineStyleProperty(
          css::PropertyId::MarginTop, static_cast<double>(renderer->MarginTop().ToFloat() / zoomFactor),
          css::UnitType::px);
        (void)styledElement->SetInlineStyleProperty(
          css::PropertyId::MarginBottom, static_cast<double>(renderer->MarginBottom().ToFloat() / zoomFactor),
          css::UnitType::px);
      }
      LayoutUnit baseHeight =
        renderer->Height() - (isBoxSizingBorder ? 0_lu : renderer->VerticalBorderAndPaddingExtent());
      baseHeight = baseHeight / zoomFactor;
      (void)styledElement->SetInlineStyleProperty(
        css::PropertyId::Height, RoundToInt(baseHeight + difference.Height()), css::UnitType::px);
    }

    document->UpdateLayout();

    // FIXME (Radar 4118564): We should also autoscroll the window as necessary to keep the point under the
    // cursor in view.
  }

  IntSize RenderLayer::OffsetFromResizeCorner(const IntPoint &localPoint) const noexcept
  {
    auto resizerRect = OverflowControlsRects().Resizer;
    auto resizeCorner = Renderer().Style().ShouldPlaceVerticalScrollbarOnLeft()
                          ? resizerRect.MinXMaxYCorner()
                          : resizerRect.MaxXMaxYCorner();
    return IntSize(localPoint.X() - resizeCorner.X(), localPoint.Y() - resizeCorner.Y());
  }

  bool RenderLayer::IsPointInResizeControl(IntPoint localPoint) const noexcept
  {
    if (!CanResize())
    {
      return false;
    }

    return OverflowControlsRects().Resizer.Contains(localPoint);
  }

  LayoutPoint RenderLayer::AbsoluteToContents(const LayoutPoint &absolutePoint) const noexcept
  {
    // We don't use convertToLayerCoords because it doesn't know about transforms
    return LayoutPoint(
      Renderer().AbsoluteToLocal(FloatPoint(absolutePoint), MapCoordinatesMode::UseTransforms));
  }

  RenderLayer::OverflowControlRects RenderLayer::OverflowControlsRects() const noexcept
  {
    if (_scrollableArea)
    {
      return _scrollableArea->OverflowControlsRects();
    }

    const auto &renderBox = Downcast<RenderBox>(Renderer());
    // Scrollbars sit inside the border box.
    auto overflowControlsPositioningRect = SnappedIntRect(renderBox.PaddingBoxRectIncludingScrollbar());

    bool placeVerticalScrollbarOnTheLeft = renderBox.Style().ShouldPlaceVerticalScrollbarOnLeft();
    bool haveResizer =
      Renderer().Style().GetResize() != style::Resize::None && !Renderer().Style().GetPseudoElementType();

    OverflowControlRects result;
    auto cornerRect = [&](IntSize cornerSize)
    {
      if (placeVerticalScrollbarOnTheLeft)
      {
        auto bottomLeftCorner = overflowControlsPositioningRect.MinXMaxYCorner();
        return IntRect {{bottomLeftCorner.X(), bottomLeftCorner.Y() - cornerSize.Height()}, cornerSize};
      }
      return IntRect {overflowControlsPositioningRect.MaxXMaxYCorner() - cornerSize, cornerSize};
    };

    if (haveResizer)
    {
      auto scrollbarThickness = ScrollbarTheme::Theme().ScrollbarThickness();
      result.Resizer = cornerRect({scrollbarThickness, scrollbarThickness});
    }

    return result;
  }

  void RenderLayer::UpdateLayerPositionsAfterOverflowScroll() noexcept
  {
    // FIXME: Invalidate the document markers' rects (willUpdateLayerPositions), once there are document
    // markers.

    // FIXME: why is it OK to not check the ancestors of this layer in order to
    // initialize the HasSeenViewportConstrainedAncestor and HasSeenAncestorWithOverflowClip flags?
    RecursiveUpdateLayerPositionsAfterScroll(UpdateLayerPositionsAfterScrollFlag::IsOverflowScroll);
  }

  void RenderLayer::UpdateLayerPositionsAfterDocumentScroll() noexcept
  {
    krys_debug_assert(IsRenderViewLayer());

    // FIXME: Invalidate the document markers' rects (willUpdateLayerPositions), once there are document
    // markers.
    RecursiveUpdateLayerPositionsAfterScroll();
  }

  void RenderLayer::RecursiveUpdateLayerPositionsAfterScroll(
    EnumSet<UpdateLayerPositionsAfterScrollFlag> flags) noexcept
  {
    // FIXME: This shouldn't be needed, but there are some corner cases where
    // these flags are still dirty. Update so that the check below is valid.
    UpdateDescendantDependentFlags();

    // If we have no visible content and no visible descendants, there is no point recomputing
    // our rectangles as they will be empty. If our visibility changes, we are expected to
    // recompute all our positions anyway.
    if (!_hasVisibleDescendant && !_hasVisibleContent)
    {
      return;
    }

    bool positionChanged = UpdateLayerPosition();
    if (positionChanged)
    {
      flags.Add(UpdateLayerPositionsAfterScrollFlag::HasChangedAncestor);
    }

    if (flags.ContainsAny({UpdateLayerPositionsAfterScrollFlag::HasChangedAncestor,
                           UpdateLayerPositionsAfterScrollFlag::HasSeenViewportConstrainedAncestor,
                           UpdateLayerPositionsAfterScrollFlag::IsOverflowScroll}))
    {
      ClearClipRects();
    }

    if (Renderer().Style().HasViewportConstrainedPosition())
    {
      flags.Add(UpdateLayerPositionsAfterScrollFlag::HasSeenViewportConstrainedAncestor);
    }

    if (Renderer().HasNonVisibleOverflow())
    {
      flags.Add(UpdateLayerPositionsAfterScrollFlag::HasSeenAncestorWithOverflowClip);
    }

    bool shouldComputeRepaintRects =
      (flags.Contains(UpdateLayerPositionsAfterScrollFlag::HasSeenViewportConstrainedAncestor)
       || flags.ContainsAll({UpdateLayerPositionsAfterScrollFlag::IsOverflowScroll,
                             UpdateLayerPositionsAfterScrollFlag::HasSeenAncestorWithOverflowClip}))
      && IsSelfPaintingLayer();
    // FIXME: We could track the repaint container as we walk down the tree.
    if (shouldComputeRepaintRects)
    {
      ComputeRepaintRects(Renderer().ContainerForRepaint().Renderer);
    }

    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      child->RecursiveUpdateLayerPositionsAfterScroll(flags);
    }

    // FIXME: The marquee position, once there are marquees.
  }

#pragma endregion

  bool
    RenderLayer::ShouldTryToScrollForScrollIntoView(const ScrollRectToVisibleOptions &options) const noexcept
  {
    if (!Renderer().IsRenderBox() || !Renderer().HasNonVisibleOverflow())
    {
      return false;
    }

    if (options.AllowOverflowHidden == AllowScrollingOverflowHidden::No)
    {
      if (Renderer().Style().OverflowX() == Overflow::Hidden
          && Renderer().Style().OverflowY() == Overflow::Hidden)
      {
        return false;
      }
    }

    // FIXME: Boxes being autoscrolled (autoscrollInProgress), once there's autoscrolling.

    auto &box = *GetRenderBox();

    // Programmatic scrolls can scroll overflow: hidden but not overflow: clip.
    return box.HasPotentiallyScrollableOverflow()
           && (box.HasHorizontalOverflow() || box.HasVerticalOverflow());
  }
#pragma region Painting

  namespace
  {
    KRYS_NODISCARD float RendererOpacity(const RenderLayerModelObject &renderer) noexcept
    {
      return renderer.Style().GetOpacity().Value.Value;
    }

    enum TransparencyClipBoxBehavior
    {
      PaintingTransparencyClipBox,
      HitTestingTransparencyClipBox
    };

    enum TransparencyClipBoxMode
    {
      DescendantsOfTransparencyClipBox,
      RootOfTransparencyClipBox
    };

    KRYS_NODISCARD LayoutRect TransparencyClipBox(const RenderLayer &, const RenderLayer *rootLayer,
                                                  TransparencyClipBoxBehavior, TransparencyClipBoxMode,
                                                  EnumSet<PaintBehavior> = {},
                                                  const LayoutRect *paintDirtyRect = nullptr) noexcept;

    void ExpandClipRectForDescendantsAndReflection(LayoutRect &clipRect, const RenderLayer &layer,
                                                   const RenderLayer *rootLayer,
                                                   TransparencyClipBoxBehavior transparencyBehavior,
                                                   EnumSet<PaintBehavior> paintBehavior,
                                                   const LayoutRect *paintDirtyRect) noexcept
    {
      // If we have a mask, then the clip is limited to the border box area (and there is
      // no need to examine child layers).
      if (!layer.Renderer().HasMask())
      {
        // Note: we don't have to walk z-order lists since transparent elements always establish
        // a stacking container. This means we can just walk the layer tree directly.
        // FIXME: Reflection layers (isReflectionLayer) are skipped upstream, once there are reflections.
        for (RenderLayer *curr = layer.FirstChild(); curr; curr = curr->NextSibling())
        {
          clipRect.Unite(TransparencyClipBox(*curr, rootLayer, transparencyBehavior,
                                             DescendantsOfTransparencyClipBox, paintBehavior,
                                             paintDirtyRect));
        }
      }

      // FIXME: Reflections (the reflected rect of the clip, united into it) come with reflections.
    }

    LayoutRect TransparencyClipBox(const RenderLayer &layer, const RenderLayer *rootLayer,
                                   TransparencyClipBoxBehavior transparencyBehavior,
                                   TransparencyClipBoxMode transparencyMode,
                                   EnumSet<PaintBehavior> paintBehavior,
                                   const LayoutRect *paintDirtyRect) noexcept
    {
      // FIXME: Although this function completely ignores CSS-imposed clipping, we did already intersect with
      // the paintDirtyRect, and that should cut down on the amount we have to paint.  Still it would be
      // better to respect clips.

      if (rootLayer != &layer
          && ((transparencyBehavior == PaintingTransparencyClipBox
               && layer.PaintsWithTransform(paintBehavior))
              || (transparencyBehavior == HitTestingTransparencyClipBox && layer.IsTransformed())))
      {
        // The best we can do here is to use enclosed bounding boxes to establish a "fuzzy" enough clip to
        // encompass the transformed layer and all of its children.
        // FIXME: Transformed layers in a paginated layer (split across its fragments with
        // fragmentsBoundingBox) come with fragmentation.
        (void)transparencyMode;
        LayoutSize delta = layer.OffsetFromAncestor(rootLayer);

        TransformationMatrix transform;
        transform.Translate(delta.Width(), delta.Height());
        transform.Multiply(*layer.Transform());

        // We don't use fragment boxes when collecting a transformed layer's bounding box, since it always
        // paints unfragmented.
        LayoutRect clipRect = layer.BoundingBox(&layer);
        ExpandClipRectForDescendantsAndReflection(clipRect, layer, &layer, transparencyBehavior,
                                                  paintBehavior, paintDirtyRect);
        clipRect.Expand(ToLayoutBoxExtent(layer.FilterOutsets()));
        LayoutRect result = transform.MapRect(clipRect);
        if (paintDirtyRect)
        {
          result = Intersection(result, *paintDirtyRect);
        }
        return result;
      }

      EnumSet<RenderLayer::CalculateLayerBoundsFlag> flags =
        transparencyBehavior == HitTestingTransparencyClipBox
          ? RenderLayer::UseFragmentBoxesIncludingCompositing
          : RenderLayer::UseFragmentBoxesExcludingCompositing;
      flags.Add(RenderLayer::IncludeRootBackgroundPaintingArea);
      auto clipRect = layer.BoundingBox(rootLayer, layer.OffsetFromAncestor(rootLayer), flags);
      ExpandClipRectForDescendantsAndReflection(clipRect, layer, rootLayer, transparencyBehavior,
                                                paintBehavior, paintDirtyRect);
      clipRect.Expand(ToLayoutBoxExtent(layer.FilterOutsets()));

      if (paintDirtyRect)
      {
        clipRect = Intersection(clipRect, *paintDirtyRect);
      }

      return clipRect;
    }
  }

  void RenderLayer::Paint(GraphicsContext &context, const LayoutRect &damageRect,
                          const LayoutSize &subpixelOffset, EnumSet<PaintBehavior> paintBehavior,
                          RenderObject *subtreePaintRoot, EnumSet<PaintLayerFlag> paintFlags) noexcept
  {
    // FIXME: Overlap test requests (for widgets), the security origin paint policy and region contexts come
    // with widgets and hit testing.
    LayerPaintingInfo paintingInfo(this, LayoutRect(EnclosingIntRect(damageRect)), paintBehavior,
                                   subpixelOffset, subtreePaintRoot);
    PaintLayer(context, paintingInfo, paintFlags);
  }

  bool RenderLayer::PaintsWithTransparency(EnumSet<PaintBehavior> paintBehavior) const noexcept
  {
    if (!Renderer().IsTransparent() && !HasNonOpacityTransparency())
    {
      return false;
    }
    return paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers) || !IsComposited();
  }

  bool RenderLayer::HasNonOpacityTransparency() const noexcept
  {
    if (Renderer().HasMask())
    {
      return true;
    }

    if (HasBlendMode() || IsolatesBlending())
    {
      return true;
    }

    // FIXME: SVG clip paths that use clipping masks, once there's the layer-based SVG engine.
    return false;
  }

  TransformationMatrix RenderLayer::RenderableTransform(EnumSet<PaintBehavior> paintBehavior) const noexcept
  {
    if (!_transform)
    {
      return TransformationMatrix();
    }

    if (paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers))
    {
      TransformationMatrix matrix = *_transform;
      MakeMatrixRenderable(matrix, false /* flatten 3d */);
      return matrix;
    }

    return *_transform;
  }

  bool RenderLayer::ShouldPaintMask(EnumSet<PaintBehavior> paintBehavior,
                                    EnumSet<PaintLayerFlag> paintFlags) const noexcept
  {
    if (!Renderer().HasMask())
    {
      return false;
    }

    bool paintsToWindow = !IsComposited() || Backing()->PaintsIntoWindow();
    if (paintsToWindow || paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers))
    {
      return true;
    }

    return paintFlags.Contains(PaintLayerFlag::PaintingCompositingMaskPhase);
  }

  bool RenderLayer::ShouldApplyClipPath(EnumSet<PaintBehavior> paintBehavior,
                                        EnumSet<PaintLayerFlag> paintFlags) const noexcept
  {
    if (!Renderer().HasClipPath())
    {
      return false;
    }

    bool paintsToWindow = !IsComposited() || Backing()->PaintsIntoWindow();
    if (paintsToWindow || paintBehavior.Contains(PaintBehavior::FlattenCompositingLayers))
    {
      return true;
    }

    return paintFlags.Contains(PaintLayerFlag::PaintingCompositingClipPathPhase)
           || paintFlags.Contains(PaintLayerFlag::CollectingEventRegion);
  }

  FloatRect RenderLayer::ReferenceBoxRectForClipPath(CSSBoxType boxType, const LayoutSize &offsetFromRoot,
                                                     const LayoutRect &rootRelativeBounds) const noexcept
  {
    // FIXME: Layer-aware SVG renderers, once there's the layer-based SVG engine.
    bool isReferenceBox = Renderer().IsRenderBox();

    // FIXME: Support different reference boxes for inline content.
    // https://bugs.webkit.org/show_bug.cgi?id=129047
    if (!isReferenceBox)
    {
      return FloatRect(rootRelativeBounds);
    }

    auto referenceBoxRect = Renderer().ReferenceBoxRect(boxType);
    referenceBoxRect.Move(FloatSize(offsetFromRoot));
    return referenceBoxRect;
  }

  RenderLayer *RenderLayer::TransparentPaintingAncestor(const LayerPaintingInfo &info) noexcept
  {
    if (this == info.RootLayer || IsComposited() || PaintsIntoProvidedBacking())
    {
      return nullptr;
    }
    for (auto *ancestor = Parent(); ancestor; ancestor = ancestor->Parent())
    {
      if (ancestor->IsStackingContext())
      {
        if (ancestor->IsTransparent())
        {
          return ancestor;
        }
      }
      if (ancestor == info.RootLayer)
      {
        return nullptr;
      }
    }
    return nullptr;
  }

  void RenderLayer::BeginTransparencyLayers(GraphicsContext &context, const LayerPaintingInfo &paintingInfo,
                                            const LayoutRect &dirtyRect) noexcept
  {
    if (context.PaintingDisabled()
        || (PaintsWithTransparency(paintingInfo.PaintBehavior) && _usedTransparency))
    {
      return;
    }

    if (auto *ancestor = TransparentPaintingAncestor(paintingInfo))
    {
      ancestor->BeginTransparencyLayers(context, paintingInfo, dirtyRect);
    }

    if (PaintsWithTransparency(paintingInfo.PaintBehavior))
    {
      krys_debug_assert(IsStackingContext());
      _usedTransparency = true;
      if (CanPaintTransparencyWithSetOpacity())
      {
        _savedAlphaForTransparency = context.Alpha();
        context.SetAlpha(context.Alpha() * RendererOpacity(Renderer()));
        return;
      }
      context.Save();
      LayoutRect adjustedClipRect =
        TransparencyClipBox(*this, paintingInfo.RootLayer, PaintingTransparencyClipBox,
                            RootOfTransparencyClipBox, paintingInfo.PaintBehavior, &dirtyRect);
      adjustedClipRect.Move(paintingInfo.SubpixelOffset);
      auto snappedClipRect = SnapRectToDevicePixelsIfNeeded(adjustedClipRect, Renderer());
      context.Clip(snappedClipRect);

      // FIXME: Legacy SVG roots directly under the view don't composite with their blend mode, once there's
      // SVG.
      bool usesCompositeOperation = HasBlendMode();
      if (usesCompositeOperation)
      {
        context.SetCompositeOperation(context.CompositeOperation(), GetBlendMode());
      }

      context.BeginTransparencyLayer(RendererOpacity(Renderer()));

      if (usesCompositeOperation)
      {
        context.SetCompositeOperation(context.CompositeOperation(), BlendMode::Normal);
      }
    }
  }

  void RenderLayer::ClipToRect(GraphicsContext &context, GraphicsContextStateSaver &stateSaver,
                               const LayerPaintingInfo &paintingInfo, EnumSet<PaintBehavior> paintBehavior,
                               const ClipRect &clipRect, BorderRadiusClippingRule rule) noexcept
  {
    float deviceScaleFactor = Renderer().GetDocument().DeviceScaleFactor();
    bool needsClipping = !clipRect.IsInfinite() && clipRect.Rect() != paintingInfo.PaintDirtyRect;
    if (needsClipping || clipRect.AffectedByRadius())
    {
      stateSaver.Save();
    }

    if (needsClipping)
    {
      LayoutRect adjustedClipRect = clipRect.Rect();
      adjustedClipRect.Move(paintingInfo.SubpixelOffset);
      auto snappedClipRect = SnapRectToDevicePixelsIfNeeded(adjustedClipRect, Renderer());
      context.Clip(snappedClipRect);
    }

    if (clipRect.AffectedByRadius())
    {
      // If the clip rect has been tainted by a border radius, then we have to walk up our layer chain
      // applying the clips from any layers with overflow. The condition for being able to apply these clips
      // is that the overflow object be in our containing block chain so we check that also.
      for (RenderLayer *layer = rule == IncludeSelfForBorderRadius ? this : Parent(); layer;
           layer = layer->Parent())
      {
        if (paintBehavior.Contains(PaintBehavior::CompositedOverflowScrollContent)
            && layer->UsesCompositedScrolling())
        {
          break;
        }

        if (layer->Renderer().HasNonVisibleOverflow() && layer->Renderer().Style().HasBorderRadius()
            && AncestorLayerIsInContainingBlockChain(*layer))
        {
          auto adjustedClipRect =
            LayoutRect {LayoutPoint {layer->OffsetFromAncestor(paintingInfo.RootLayer, AdjustForColumns)},
                        layer->RendererBorderBoxRect().Size()};
          adjustedClipRect.Move(paintingInfo.SubpixelOffset);
          auto borderShape = BorderShape::ShapeForBorderRect(layer->Renderer().Style(), adjustedClipRect);
          if (borderShape.InnerShapeContains(paintingInfo.PaintDirtyRect))
          {
            context.Clip(SnapRectToDevicePixels(Intersection(paintingInfo.PaintDirtyRect, adjustedClipRect),
                                                deviceScaleFactor));
          }
          else
          {
            borderShape.ClipToInnerShape(context, deviceScaleFactor);
          }
        }

        if (layer == paintingInfo.RootLayer)
        {
          break;
        }
      }
    }
  }

  static bool ShouldDoSoftwarePaint(const RenderLayer *layer, bool paintingReflection) noexcept
  {
    return paintingReflection && !layer->Has3DTransform();
  }

  static bool PaintForFixedRootBackground(const RenderLayer *layer,
                                          EnumSet<RenderLayer::PaintLayerFlag> paintFlags) noexcept
  {
    return layer->Renderer().IsDocumentElementRenderer()
           && paintFlags.Contains(RenderLayer::PaintLayerFlag::PaintingRootBackgroundOnly);
  }

  void RenderLayer::PaintLayer(GraphicsContext &context, const LayerPaintingInfo &paintingInfo,
                               EnumSet<PaintLayerFlag> paintFlags) noexcept
  {
    auto shouldContinuePaint = [&]()
    {
      return Backing()->PaintsIntoWindow() || Backing()->PaintsIntoCompositedAncestor()
             || ShouldDoSoftwarePaint(this, paintFlags.Contains(PaintLayerFlag::PaintingReflection))
             || PaintForFixedRootBackground(this, paintFlags);
    };

    auto paintsIntoDifferentCompositedDestination = [&]()
    {
      if (PaintsIntoProvidedBacking())
      {
        return true;
      }

      if (IsComposited() && !shouldContinuePaint())
      {
        return true;
      }

      return false;
    };

    if (paintsIntoDifferentCompositedDestination())
    {
      if (!context.PerformingPaintInvalidation()
          && !paintingInfo.PaintBehavior.Contains(PaintBehavior::FlattenCompositingLayers))
      {
        return;
      }

      paintFlags.Add(PaintLayerFlag::TemporaryClipRects);
    }

    if (GetViewportConstrainedNotCompositedReason() == NotCompositedForBoundsOutOfView
        && !paintingInfo.PaintBehavior.Contains(PaintBehavior::Snapshotting))
    {
      // Don't paint out-of-view viewport constrained layers (when doing prepainting) because they will never
      // be visible unless their position or viewport size is changed.
      krys_debug_assert(Renderer().IsFixedPositioned());
      return;
    }

    PaintLayerWithEffects(context, paintingInfo, paintFlags);
  }

  void RenderLayer::PaintLayerWithEffects(GraphicsContext &context, const LayerPaintingInfo &paintingInfo,
                                          EnumSet<PaintLayerFlag> paintFlags) noexcept
  {
    // Non self-painting leaf layers don't need to be painted as their renderer() should properly paint
    // itself.
    if (!IsSelfPaintingLayer() && !HasSelfPaintingLayerDescendant())
    {
      return;
    }

    // FIXME: Suppressed painting while visual updates aren't allowed (visualUpdatesAllowed), once documents
    // defer visual updates.

    // If this layer is totally invisible then there is nothing to paint.
    // FIXME: Accessibility and event region paints continue, once there are those.
    if (!RendererOpacity(Renderer()))
    {
      return;
    }

    if (PaintsWithTransparency(paintingInfo.PaintBehavior))
    {
      paintFlags.Add(PaintLayerFlag::HaveTransparency);
    }

    // PaintLayerFlag::AppliedTransform is used in RenderReplica, to avoid applying the transform twice.
    if (PaintsWithTransform(paintingInfo.PaintBehavior)
        && !paintFlags.Contains(PaintLayerFlag::AppliedTransform))
    {
      TransformationMatrix layerTransform = RenderableTransform(paintingInfo.PaintBehavior);
      // If the transform can't be inverted, then don't paint anything.
      if (!layerTransform.IsInvertible())
      {
        return;
      }

      // If we have a transparency layer enclosing us and we are the root of a transform, then we need to
      // establish the transparency layer from the parent now, assuming there is a parent
      if (paintFlags.Contains(PaintLayerFlag::HaveTransparency))
      {
        if (this != paintingInfo.RootLayer && Parent())
        {
          Parent()->BeginTransparencyLayers(context, paintingInfo, paintingInfo.PaintDirtyRect);
        }
        else
        {
          BeginTransparencyLayers(context, paintingInfo, paintingInfo.PaintDirtyRect);
        }
      }

      // FIXME: Transformed layers in a paginated layer (paintTransformedLayerIntoFragments) come with
      // fragmentation; they paint unfragmented.

      // Make sure the parent's clip rects have been calculated.
      ClipRect clipRect = paintingInfo.PaintDirtyRect;
      GraphicsContextStateSaver stateSaver(context, false);
      if (Parent())
      {
        auto options = paintFlags.Contains(PaintLayerFlag::PaintingOverflowContents)
                         ? ClipRectOptionsForPaintingOverflowContents
                         : ClipRectDefaultOptions;
        if (ShouldHaveFiltersForPainting(context, paintFlags, paintingInfo.PaintBehavior))
        {
          options.Add(ClipRectsOption::OutsideFilter);
        }
        if (paintFlags.Contains(PaintLayerFlag::TemporaryClipRects))
        {
          options.Add(ClipRectsOption::Temporary);
        }
        auto clipRectsContext = ClipRectsContext(paintingInfo.RootLayer, PaintingClipRects, options);
        clipRect = BackgroundClipRect(clipRectsContext);
        clipRect.Intersect(paintingInfo.PaintDirtyRect);

        EnumSet<PaintBehavior> paintBehavior;
        if (paintFlags.Contains(PaintLayerFlag::PaintingOverflowContents))
        {
          paintBehavior.Add(PaintBehavior::CompositedOverflowScrollContent);
        }

        // FIXME: SVG viewport clipping (shouldApplyViewportClip) comes with SVG.

        // Push the parent coordinate space's clip.
        Parent()->ClipToRect(context, stateSaver, paintingInfo, paintBehavior, clipRect);
      }

      PaintLayerByApplyingTransform(context, paintingInfo, paintFlags);
      return;
    }

    PaintLayerContentsAndReflection(context, paintingInfo, paintFlags);
  }

  void RenderLayer::PaintLayerContentsAndReflection(GraphicsContext &context,
                                                    const LayerPaintingInfo &paintingInfo,
                                                    EnumSet<PaintLayerFlag> paintFlags) noexcept
  {
    krys_debug_assert(IsSelfPaintingLayer() || HasSelfPaintingLayerDescendant());

    auto localPaintFlags = paintFlags;
    localPaintFlags.Remove(PaintLayerFlag::AppliedTransform);
    localPaintFlags.Remove(PaintLayerFlag::PaintingOverflowContentsRoot);

    // FIXME: Reflections (painted first, from the reflection layer) come with reflections.

    localPaintFlags.Add(PaintLayerPaintingCompositingAllPhasesFlags());
    PaintLayerContents(context, paintingInfo, localPaintFlags);
  }

  Pair<Path, WindRule>
    RenderLayer::ComputeClipPath(const LayoutSize &offsetFromRoot,
                                 const LayoutRect &rootRelativeBoundsForNonBoxes) const noexcept
  {
    const auto &clipPath = Renderer().Style().GetClipPath();

    if (auto basicShape = clipPath.TryBasicShape())
    {
      auto referenceBoxRect = ReferenceBoxRectForClipPath(basicShape->ReferenceBox(), offsetFromRoot,
                                                          rootRelativeBoundsForNonBoxes);
      auto snappedReferenceBoxRect = SnapRectToDevicePixelsIfNeeded(referenceBoxRect, Renderer());
      return {style::ComputePath(basicShape->GetShape(), snappedReferenceBoxRect),
              style::ComputeWindRule(basicShape->GetShape())};
    }

    if (auto box = clipPath.TryBox())
    {
      if (const auto *renderBox = DynamicDowncast<RenderBox>(Renderer()))
      {
        auto shapeRect = ComputeRoundedRectForBoxShape(box->ReferenceBox(), *renderBox)
                           .PixelSnappedRoundedRectForPainting(Renderer().GetDocument().DeviceScaleFactor());
        shapeRect.Move(FloatSize(offsetFromRoot));
        return {shapeRect.GetPath(), WindRule::NonZero};
      }
    }

    return {Path(), WindRule::NonZero};
  }

  void RenderLayer::SetupClipPath(GraphicsContext &context, GraphicsContextStateSaver &stateSaver,
                                  const LayerPaintingInfo &paintingInfo, EnumSet<PaintLayerFlag> &,
                                  const LayoutSize &offsetFromRoot) noexcept
  {
    // FIXME: Event region clips (CollectingEventRegion) come with hit testing.
    if (!Renderer().HasClipPath() || context.PaintingDisabled() || paintingInfo.PaintDirtyRect.IsEmpty())
    {
      return;
    }

    // FIXME: Clipping inside SVG resource containers (<clipPath>, <mask>), once there's SVG.

    auto clippedContentBounds =
      CalculateLayerBounds(paintingInfo.RootLayer, offsetFromRoot, {UseLocalClipRectIfPossible});

    const auto &style = Renderer().Style();
    LayoutSize paintingOffsetFromRoot =
      LayoutSize(SnapSizeToDevicePixel(offsetFromRoot + paintingInfo.SubpixelOffset, LayoutPoint(),
                                       Renderer().GetDocument().DeviceScaleFactor()));
    krys_debug_assert(!style.GetClipPath().IsNone());
    if (style.GetClipPath().IsBasicShape() || (style.GetClipPath().IsBox() && Is<RenderBox>(Renderer())))
    {
      // clippedContentBounds is used as the reference box for inlines, which is also poorly specified:
      // https://github.com/w3c/csswg-drafts/issues/6383.
      auto [path, windRule] = ComputeClipPath(paintingOffsetFromRoot, clippedContentBounds);

      stateSaver.Save();
      context.ClipPath(path, windRule);
      return;
    }

    // FIXME: SVG clip paths (`url()` references to <clipPath>, applied as path clips or clipping masks) come
    // with SVG.
  }

  bool RenderLayer::ShouldHaveFiltersForPainting(GraphicsContext &context, EnumSet<PaintLayerFlag> paintFlags,
                                                 EnumSet<PaintBehavior> paintBehavior) const noexcept
  {
    if (context.PaintingDisabled())
    {
      return false;
    }

    if (paintFlags.Contains(PaintLayerFlag::PaintingOverlayScrollbars))
    {
      return false;
    }

    if (!ShouldPaintWithFilters(paintBehavior))
    {
      return false;
    }

    return true;
  }

  RenderLayerFilters *RenderLayer::FiltersForPainting(GraphicsContext &context,
                                                      EnumSet<PaintLayerFlag> paintFlags,
                                                      EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    if (!ShouldHaveFiltersForPainting(context, paintFlags, paintBehavior))
    {
      return nullptr;
    }

    return &EnsureLayerFilters();
  }

  GraphicsContext *RenderLayer::SetupFilters(GraphicsContext &destinationContext,
                                             LayerPaintingInfo &paintingInfo,
                                             EnumSet<PaintLayerFlag> &paintFlags,
                                             const LayoutSize &offsetFromRoot,
                                             const ClipRect &backgroundRect) noexcept
  {
    auto *paintingFilters = FiltersForPainting(destinationContext, paintFlags, paintingInfo.PaintBehavior);
    if (!paintingFilters)
    {
      return nullptr;
    }

    LayoutRect filterRepaintRect = paintingFilters->DirtySourceRect();
    filterRepaintRect.Move(offsetFromRoot);

    auto rootRelativeBounds =
      CalculateLayerBounds(paintingInfo.RootLayer, offsetFromRoot, {RenderLayer::PreserveAncestorFlags});

    GraphicsContext *filterContext = paintingFilters->BeginFilterEffect(
      Renderer(), destinationContext, LayoutRect(EnclosingIntRect(rootRelativeBounds)),
      LayoutRect(EnclosingIntRect(paintingInfo.PaintDirtyRect)),
      LayoutRect(EnclosingIntRect(filterRepaintRect)), backgroundRect.Rect());
    if (!filterContext)
    {
      return nullptr;
    }

    paintingInfo.PaintDirtyRect = paintingFilters->RepaintRect();
    if (paintingFilters->HasFilterThatMovesPixels())
    {
      _suppressAncestorClippingInsideFilter = true;
      paintFlags.Add(PaintLayerFlag::TemporaryClipRects);
    }
    paintingInfo.RequireSecurityOriginAccessForWidgets =
      paintingFilters->HasFilterThatShouldBeRestrictedBySecurityOrigin();

    return filterContext;
  }

  void RenderLayer::ApplyFilters(GraphicsContext &originalContext, const LayerPaintingInfo &paintingInfo,
                                 EnumSet<PaintBehavior> behavior, const ClipRect &backgroundRect) noexcept
  {
    GraphicsContextStateSaver stateSaver(originalContext, false);
    bool needsClipping = _filters->HasSourceImage();

    _suppressAncestorClippingInsideFilter = false;

    if (needsClipping)
    {
      ClipToRect(originalContext, stateSaver, paintingInfo, behavior, backgroundRect);
    }

    _filters->ApplyFilterEffect(originalContext);
  }

  void RenderLayer::PaintLayerContents(GraphicsContext &context, const LayerPaintingInfo &paintingInfo,
                                       EnumSet<PaintLayerFlag> paintFlags) noexcept
  {
    krys_debug_assert(IsSelfPaintingLayer() || HasSelfPaintingLayerDescendant());

    // FIXME: Contentful paint detection (detectingContentfulPaint) comes with paint timing.

    auto localPaintFlags = paintFlags;
    localPaintFlags.Remove(PaintLayerFlag::AppliedTransform);

    bool haveTransparency = localPaintFlags.Contains(PaintLayerFlag::HaveTransparency);
    bool isPaintingOverlayScrollbars = localPaintFlags.Contains(PaintLayerFlag::PaintingOverlayScrollbars);
    bool isPaintingCompositedForeground =
      localPaintFlags.Contains(PaintLayerFlag::PaintingCompositingForegroundPhase);
    bool isPaintingCompositedBackground =
      localPaintFlags.Contains(PaintLayerFlag::PaintingCompositingBackgroundPhase);
    bool isPaintingOverflowContents = localPaintFlags.Contains(PaintLayerFlag::PaintingOverflowContents);
    bool isCollectingEventRegion = localPaintFlags.Contains(PaintLayerFlag::CollectingEventRegion);

    bool isSelfPaintingLayer = IsSelfPaintingLayer();
    bool isInsideSkippedSubtree = Renderer().IsSkippedContent();

    auto hasVisibleContent = [&]() -> bool
    {
      if (isInsideSkippedSubtree)
      {
        return false;
      }

      // FIXME: Hidden SVG containers and SVG resource layers (painted indirectly), once there's SVG.
      return _hasVisibleContent;
    };

    auto shouldSkipNonFixedTopDocumentContent = [&]
    {
      if (!paintingInfo.PaintBehavior.Contains(PaintBehavior::FixedAndStickyLayersOnly))
      {
        return false;
      }

      if (_hasFixedAncestor || _hasStickyAncestor)
      {
        return false;
      }

      if (IsViewportConstrained())
      {
        return false;
      }

      // Every view is its page's main frame.
      return true;
    };

    // FIXME: Accessibility regions come with accessibility.
    bool shouldPaintContent = hasVisibleContent() && isSelfPaintingLayer && !isPaintingOverlayScrollbars
                              && !isCollectingEventRegion && !shouldSkipNonFixedTopDocumentContent();

    bool shouldPaintOutline = [&]()
    {
      if (!isSelfPaintingLayer)
      {
        return false;
      }

      if (!shouldPaintContent)
      {
        return false;
      }

      if (isPaintingOverlayScrollbars || isCollectingEventRegion)
      {
        return false;
      }

      // For the current layer, the outline has been painted by the primary GraphicsLayer.
      if (localPaintFlags.Contains(PaintLayerFlag::PaintingOverflowContentsRoot))
      {
        return false;
      }

      // Paint outlines in the background phase for a scroll container so that they don't scroll with the
      // content.
      // FIXME: inset outlines will have the wrong z-ordering with scrolled content. See also
      // webkit.org/b/249457.
      if (localPaintFlags.Contains(PaintLayerFlag::PaintingOverflowContainer))
      {
        return isPaintingCompositedBackground;
      }

      return isPaintingCompositedForeground;
    }();

    bool shouldPaintNegativeZIndexChildren = [&]()
    {
      if (localPaintFlags.Contains(PaintLayerFlag::PaintingOverflowContainer))
      {
        return false;
      }

      if (localPaintFlags.Contains(PaintLayerFlag::PaintingOverflowContents))
      {
        // Overflow contents has the "PaintingCompositingForegroundPhase" phase,
        // but we need to paint negative z-index layers here so they scroll with the content.
        return true;
      }

      return isPaintingCompositedBackground;
    }();

    auto shouldExcludeBasedOnContainingBlock = [&]()
    {
      if (auto *rootAsBlock = DynamicDowncast<RenderBlock>(paintingInfo.SubtreePaintRoot))
      {
        return !rootAsBlock->IsContainingBlockAncestorFor(Renderer());
      }
      return false;
    };

    if (paintingInfo.PaintBehavior.Contains(PaintBehavior::DraggableSnapshot)
        && paintingInfo.SubtreePaintRoot)
    {
      if (paintingInfo.SubtreePaintRoot->HasLayer())
      {
        auto *subtreeRootLayer = paintingInfo.SubtreePaintRoot->EnclosingLayer();
        bool isLayerInSubtree = (this == subtreeRootLayer) || IsDescendantOf(*subtreeRootLayer);

        if (!isLayerInSubtree
            || (paintingInfo.SubtreePaintRoot != &Renderer() && shouldExcludeBasedOnContainingBlock()))
        {
          shouldPaintContent = false;
        }
      }
      else if (Renderer().IsAbsolutelyPositioned() && paintingInfo.SubtreePaintRoot != &Renderer()
               && shouldExcludeBasedOnContainingBlock())
      {
        shouldPaintContent = false;
      }
    }

    if (localPaintFlags.Contains(PaintLayerFlag::PaintingRootBackgroundOnly) && !Renderer().IsRenderView()
        && !Renderer().IsDocumentElementRenderer())
    {
      // If beginTransparencyLayers was called prior to this, ensure the transparency state is cleaned up
      // before returning.
      if (haveTransparency && _usedTransparency)
      {
        if (_savedAlphaForTransparency)
        {
          context.SetAlpha(*_savedAlphaForTransparency);
          _savedAlphaForTransparency = std::nullopt;
        }
        else
        {
          context.EndTransparencyLayer();
          context.Restore();
        }
        _usedTransparency = false;
      }

      return;
    }

    UpdateLayerListsIfNeeded();

    LayoutSize offsetFromRoot = OffsetFromAncestor(paintingInfo.RootLayer);

    // FIXME: Font subpixel quantization (setupFontSubpixelQuantization) comes with text painting.

    // FIXME: Clip paths and filters in a fragmented flow use the column-aware offset
    // (enclosingFragmentedFlow), once there's fragmentation.
    LayoutSize columnAwareOffsetFromRoot = offsetFromRoot;

    GraphicsContextStateSaver stateSaver(context, false);

    // Apply clip-path to context.
    if (ShouldApplyClipPath(paintingInfo.PaintBehavior, localPaintFlags))
    {
      SetupClipPath(context, stateSaver, paintingInfo, localPaintFlags, columnAwareOffsetFromRoot);
    }

    bool applySVGClippingMask = localPaintFlags.Contains(PaintLayerFlag::PaintingSVGClippingMask);
    if (applySVGClippingMask)
    {
      localPaintFlags.Remove(PaintLayerFlag::PaintingSVGClippingMask);
    }

    bool selectionAndBackgroundsOnly =
      paintingInfo.PaintBehavior.Contains(PaintBehavior::SelectionAndBackgroundsOnly);
    bool selectionOnly = paintingInfo.PaintBehavior.Contains(PaintBehavior::SelectionOnly);

    LayerFragments layerFragments;
    RenderObject *subtreePaintRootForRenderer = nullptr;

    auto paintBehavior = [&]()
    {
      static constexpr EnumSet<PaintBehavior> flagsToCopy {
        PaintBehavior::FlattenCompositingLayers,
        PaintBehavior::Snapshotting,
        PaintBehavior::ExcludeSelection,
        PaintBehavior::ExcludeReplacedContentExceptForIFrames,
        PaintBehavior::ExcludeText,
        PaintBehavior::FixedAndStickyLayersOnly,
        PaintBehavior::DrawsHDRContent,
      };
      EnumSet<PaintBehavior> paintBehavior = paintingInfo.PaintBehavior & flagsToCopy;

      if (localPaintFlags.Contains(PaintLayerFlag::PaintingSkipRootBackground))
      {
        paintBehavior.Add(PaintBehavior::SkipRootBackground);
      }
      else if (localPaintFlags.Contains(PaintLayerFlag::PaintingRootBackgroundOnly))
      {
        paintBehavior.Add(PaintBehavior::RootBackgroundOnly);
      }

      // FIXME: This seems wrong. We should retain the DefaultAsynchronousImageDecode flag for all
      // RenderLayers painted into the root tile cache.
      if (paintingInfo.PaintBehavior.Contains(PaintBehavior::DefaultAsynchronousImageDecode)
          && IsRenderViewLayer())
      {
        paintBehavior.Add(PaintBehavior::DefaultAsynchronousImageDecode);
      }

      if (isPaintingOverflowContents)
      {
        paintBehavior.Add(PaintBehavior::CompositedOverflowScrollContent);
      }

      return paintBehavior;
    }();

    { // Scope for filter-related state changes.
      ClipRect backgroundRect;

      if (ShouldHaveFiltersForPainting(context, paintFlags, paintBehavior))
      {
        // When we called collectFragments() last time, paintDirtyRect was reset to represent the filter
        // bounds. Now we need to compute the backgroundRect uncontaminated by filters, in order to clip the
        // filtered result. Note that we also use paintingInfo here, not localPaintingInfo which filters also
        // contaminated.
        LayerFragments filterLayerFragments;
        auto clipRectOptions =
          isPaintingOverflowContents ? ClipRectOptionsForPaintingOverflowContents : ClipRectDefaultOptions;
        clipRectOptions.Add(ClipRectsOption::OutsideFilter);
        if (localPaintFlags.Contains(PaintLayerFlag::TemporaryClipRects))
        {
          clipRectOptions.Add(ClipRectsOption::Temporary);
        }
        CollectFragments(filterLayerFragments, paintingInfo.RootLayer, paintingInfo.PaintDirtyRect,
                         PaintingClipRects, clipRectOptions, offsetFromRoot);
        UpdatePaintingInfoForFragments(filterLayerFragments, paintingInfo, localPaintFlags,
                                       shouldPaintContent, offsetFromRoot);

        // FIXME: Handle more than one fragment.
        backgroundRect = filterLayerFragments.empty() ? ClipRect() : filterLayerFragments[0].BackgroundRect();

        if (haveTransparency)
        {
          // If we have a filter and transparency, we have to eagerly start a transparency layer here, rather
          // than risk a child layer lazily starts one with the wrong context.
          BeginTransparencyLayers(context, paintingInfo, paintingInfo.PaintDirtyRect);
        }
      }

      LayerPaintingInfo localPaintingInfo(paintingInfo);
      GraphicsContext *filterContext =
        SetupFilters(context, localPaintingInfo, localPaintFlags, columnAwareOffsetFromRoot, backgroundRect);
      GraphicsContext &currentContext = filterContext ? *filterContext : context;

      if (filterContext)
      {
        localPaintingInfo.PaintBehavior.Add(PaintBehavior::DontShowVisitedLinks);
      }

      // If this layer's renderer is a child of the subtreePaintRoot, we render unconditionally, which
      // is done by passing a nil subtreePaintRoot down to our renderer (as if no subtreePaintRoot was ever
      // set). Otherwise, our renderer tree may or may not contain the subtreePaintRoot root, so we pass that
      // root along so it will be tested against as we descend through the renderers.
      if (localPaintingInfo.SubtreePaintRoot
          && !Renderer().IsDescendantOf(localPaintingInfo.SubtreePaintRoot))
      {
        subtreePaintRootForRenderer = localPaintingInfo.SubtreePaintRoot;
      }

      LayoutRect paintDirtyRect = localPaintingInfo.PaintDirtyRect;
      if (shouldPaintContent || shouldPaintOutline || isPaintingOverlayScrollbars || isCollectingEventRegion)
      {
        // Collect the fragments. This will compute the clip rectangles and paint offsets for each layer
        // fragment, as well as whether or not the content of each fragment should paint.
        auto clipRectOptions =
          isPaintingOverflowContents ? ClipRectOptionsForPaintingOverflowContents : ClipRectDefaultOptions;
        if (localPaintFlags.Contains(PaintLayerFlag::TemporaryClipRects))
        {
          clipRectOptions.Add(ClipRectsOption::Temporary);
        }
        CollectFragments(layerFragments, localPaintingInfo.RootLayer, paintDirtyRect, PaintingClipRects,
                         clipRectOptions, offsetFromRoot);
        UpdatePaintingInfoForFragments(layerFragments, localPaintingInfo, localPaintFlags, shouldPaintContent,
                                       offsetFromRoot);
      }

      if (isPaintingCompositedBackground)
      {
        // Paint only the backgrounds for all of the fragments of the layer.
        if (shouldPaintContent && !selectionOnly)
        {
          PaintBackgroundForFragments(layerFragments, currentContext, context, paintingInfo.PaintDirtyRect,
                                      haveTransparency, localPaintingInfo, paintBehavior,
                                      subtreePaintRootForRenderer);
        }
      }

      // Now walk the sorted list of children with negative z-indices.
      if (shouldPaintNegativeZIndexChildren)
      {
        PaintList(NegativeZOrderLayers(), currentContext, paintingInfo, localPaintFlags);
      }

      if (isPaintingCompositedForeground && shouldPaintContent)
      {
        PaintForegroundForFragments(layerFragments, currentContext, context, paintingInfo.PaintDirtyRect,
                                    haveTransparency, localPaintingInfo, paintBehavior,
                                    subtreePaintRootForRenderer);
      }

      // FIXME: Event and accessibility regions (collectEventRegionForFragments,
      // collectAccessibilityRegionsForFragments) come with hit testing and accessibility.

      if (shouldPaintOutline)
      {
        PaintOutlineForFragments(layerFragments, currentContext, localPaintingInfo, paintBehavior,
                                 subtreePaintRootForRenderer);
      }

      if (isPaintingCompositedForeground)
      {
        // Paint any child layers that have overflow.
        PaintList(NormalFlowLayers(), currentContext, paintingInfo, localPaintFlags);

        // Now walk the sorted list of children with positive z-indices.
        PaintList(PositiveZOrderLayers(), currentContext, localPaintingInfo, localPaintFlags);
      }

      // FIXME: Overlay scrollbars (paintOverflowControlsForFragments) come with scrollbars.

      if (filterContext)
      {
        ApplyFilters(context, paintingInfo, paintBehavior, backgroundRect);
        // Painting a snapshot might have temporarily overriden the filter painting strategy, make sure it
        // gets reset.
        UpdateFilterPaintingStrategy();
      }
    }

    if (shouldPaintContent && !(selectionOnly || selectionAndBackgroundsOnly))
    {
      if (ShouldPaintMask(paintingInfo.PaintBehavior, localPaintFlags))
      {
        // Paint the mask for the fragments.
        PaintMaskForFragments(layerFragments, context, paintingInfo, paintBehavior,
                              subtreePaintRootForRenderer);
      }

      if (applySVGClippingMask
          || (!paintFlags.Contains(PaintLayerFlag::PaintingCompositingMaskPhase)
              && paintFlags.Contains(PaintLayerFlag::PaintingCompositingClipPathPhase)))
      {
        // Re-use paintChildClippingMaskForFragments to paint black for the compositing clipping mask.
        PaintChildClippingMaskForFragments(layerFragments, context, paintingInfo, paintBehavior,
                                           subtreePaintRootForRenderer);
      }

      if (localPaintFlags.Contains(PaintLayerFlag::PaintingChildClippingMaskPhase))
      {
        // Paint the border radius mask for the fragments.
        PaintChildClippingMaskForFragments(layerFragments, context, paintingInfo, paintBehavior,
                                           subtreePaintRootForRenderer);
      }
    }

    // End our transparency layer
    if (haveTransparency && _usedTransparency)
    {
      if (_savedAlphaForTransparency)
      {
        context.SetAlpha(*_savedAlphaForTransparency);
        _savedAlphaForTransparency = std::nullopt;
      }
      else
      {
        context.EndTransparencyLayer();
        context.Restore();
      }
      _usedTransparency = false;
    }
  }

  void RenderLayer::PaintLayerByApplyingTransform(GraphicsContext &context,
                                                  const LayerPaintingInfo &paintingInfo,
                                                  EnumSet<PaintLayerFlag> paintFlags,
                                                  const LayoutSize &translationOffset) noexcept
  {
    // This involves subtracting out the position of the layer in our current coordinate space, but preserving
    // the accumulated error for sub-pixel layout.
    float deviceScaleFactor = Renderer().GetDocument().DeviceScaleFactor();
    LayoutSize offsetFromParent = OffsetFromAncestor(paintingInfo.RootLayer);
    offsetFromParent += translationOffset;
    TransformationMatrix transform(RenderableTransform(paintingInfo.PaintBehavior));
    // Add the subpixel accumulation to the current layer's offset so that we can always snap the
    // translateRight value to where the renderer() is supposed to be painting.
    LayoutSize offsetForThisLayer = offsetFromParent + paintingInfo.SubpixelOffset;
    FloatSize alignedOffsetForThisLayer =
      ToFloatSize(RoundPointToDevicePixels(ToLayoutPoint(offsetForThisLayer), deviceScaleFactor));
    // We handle accumulated subpixels through nested layers here. Since the context gets translated to device
    // pixels, all we need to do is add the delta to the accumulated pixels coming from ancestor layers.
    // Translate the graphics context to the snapping position to avoid off-device-pixel positing.
    transform.TranslateRight(alignedOffsetForThisLayer.Width(), alignedOffsetForThisLayer.Height());
    // Apply the transform.
    auto oldTransform = context.GetCTM();
    auto affineTransform = transform.ToAffineTransform();
    context.ConcatCTM(affineTransform);

    LayoutSize adjustedSubpixelOffset = offsetForThisLayer - LayoutSize(alignedOffsetForThisLayer);

    // Now do a paint with the root layer shifted to be us.
    LayerPaintingInfo transformedPaintingInfo(paintingInfo);
    transformedPaintingInfo.RootLayer = this;
    if (!transformedPaintingInfo.PaintDirtyRect.IsInfinite())
    {
      transformedPaintingInfo.PaintDirtyRect = LayoutRect(EncloseRectToDevicePixels(
        transform.Inverse().value_or(TransformationMatrix()).MapRect(paintingInfo.PaintDirtyRect),
        deviceScaleFactor));
    }

    paintFlags.Remove(PaintLayerFlag::PaintingOverflowContents);

    transformedPaintingInfo.SubpixelOffset = adjustedSubpixelOffset;
    PaintLayerContentsAndReflection(context, transformedPaintingInfo, paintFlags);

    context.SetCTM(oldTransform);
  }

  void RenderLayer::PaintList(LayerList layerIterator, GraphicsContext &context,
                              const LayerPaintingInfo &paintingInfo,
                              EnumSet<PaintLayerFlag> paintFlags) noexcept
  {
    if (layerIterator.begin() == layerIterator.end())
    {
      return;
    }

    if (!HasSelfPaintingLayerDescendant())
    {
      return;
    }

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(*this);
#endif

    // FIXME: Skipping view transition captures (PaintingSkipDescendantViewTransition), once there are view
    // transitions.
    for (auto *childLayer : layerIterator)
    {
      childLayer->PaintLayer(context, paintingInfo, paintFlags);
    }
  }

  void RenderLayer::CollectFragments(LayerFragments &fragments, const RenderLayer *rootLayer,
                                     const LayoutRect &dirtyRect, ClipRectsType clipRectsType,
                                     EnumSet<ClipRectsOption> clipRectOptions,
                                     const LayoutSize &offsetFromRoot) const noexcept
  {
    // FIXME: Paginated layers (one fragment per column or page, from the enclosing fragmented flow) come with
    // fragmentation; every layer has one fragment.
    ClipRectsContext clipRectsContext(rootLayer, clipRectsType, clipRectOptions);
    fragments.push_back(LayerFragment {CalculateRects(clipRectsContext, offsetFromRoot, dirtyRect)});
  }

  void RenderLayer::UpdatePaintingInfoForFragments(LayerFragments &fragments,
                                                   const LayerPaintingInfo &localPaintingInfo,
                                                   EnumSet<PaintLayerFlag> localPaintFlags,
                                                   bool shouldPaintContent,
                                                   const LayoutSize &offsetFromRoot) const noexcept
  {
    for (auto &fragment : fragments)
    {
      fragment.ShouldPaintContent = shouldPaintContent;
      if (this != localPaintingInfo.RootLayer
          || !localPaintFlags.Contains(PaintLayerFlag::PaintingOverflowContents))
      {
        LayoutSize newOffsetFromRoot = offsetFromRoot + fragment.PaginationOffset;
        fragment.ShouldPaintContent &=
          IntersectsDamageRect(fragment.LayerBounds(), fragment.DirtyBackgroundRect().Rect(),
                               localPaintingInfo.RootLayer, newOffsetFromRoot, fragment.BoundingBox());
      }
    }
  }

  void RenderLayer::PaintBackgroundForFragments(
    const LayerFragments &layerFragments, GraphicsContext &context,
    GraphicsContext &contextForTransparencyLayer, const LayoutRect &transparencyPaintDirtyRect,
    bool haveTransparency, const LayerPaintingInfo &localPaintingInfo, EnumSet<PaintBehavior> paintBehavior,
    RenderObject *subtreePaintRootForRenderer) noexcept
  {
    for (const auto &fragment : layerFragments)
    {
      if (!fragment.ShouldPaintContent)
      {
        continue;
      }

      // Begin transparency layers lazily now that we know we have to paint something.
      if (haveTransparency)
      {
        BeginTransparencyLayers(contextForTransparencyLayer, localPaintingInfo, transparencyPaintDirtyRect);
      }

      GraphicsContextStateSaver stateSaver(context, false);

      // Paint our background first, before painting any child layers.
      // Establish the clip used to paint our background.
      ClipToRect(context, stateSaver, localPaintingInfo, paintBehavior, fragment.DirtyBackgroundRect(),
                 DoNotIncludeSelfForBorderRadius); // Background painting will handle clipping to self.

      // Paint the background.
      PaintInfo paintInfo(context, fragment.DirtyBackgroundRect().Rect(), PaintPhase::BlockBackground,
                          paintBehavior, subtreePaintRootForRenderer,
                          &localPaintingInfo.RootLayer->Renderer(), this);
      Renderer().Paint(paintInfo, PaintOffsetForRenderer(fragment, localPaintingInfo));
    }
  }

  void RenderLayer::PaintForegroundForFragments(
    const LayerFragments &layerFragments, GraphicsContext &context,
    GraphicsContext &contextForTransparencyLayer, const LayoutRect &transparencyPaintDirtyRect,
    bool haveTransparency, const LayerPaintingInfo &localPaintingInfo, EnumSet<PaintBehavior> paintBehavior,
    RenderObject *subtreePaintRootForRenderer) noexcept
  {
    // Begin transparency if we have something to paint.
    if (haveTransparency)
    {
      for (const auto &fragment : layerFragments)
      {
        if (fragment.ShouldPaintContent && !fragment.DirtyForegroundRect().IsEmpty())
        {
          BeginTransparencyLayers(contextForTransparencyLayer, localPaintingInfo, transparencyPaintDirtyRect);
          break;
        }
      }
    }

    EnumSet<PaintBehavior> localPaintBehavior;
    if (localPaintingInfo.PaintBehavior.Contains(PaintBehavior::ForceBlackText))
    {
      localPaintBehavior = PaintBehavior::ForceBlackText;
    }
    else if (localPaintingInfo.PaintBehavior.Contains(PaintBehavior::ForceWhiteText))
    {
      localPaintBehavior = PaintBehavior::ForceWhiteText;
    }
    else
    {
      localPaintBehavior = paintBehavior;
    }

    // FIXME: It's unclear if this flag copying is necessary.
    static constexpr EnumSet<PaintBehavior> flagsToCopy {
      PaintBehavior::ExcludeSelection,
      PaintBehavior::Snapshotting,
      PaintBehavior::DefaultAsynchronousImageDecode,
      PaintBehavior::CompositedOverflowScrollContent,
      PaintBehavior::ForceSynchronousImageDecode,
      PaintBehavior::ExcludeReplacedContentExceptForIFrames,
      PaintBehavior::ExcludeText,
      PaintBehavior::FixedAndStickyLayersOnly,
      PaintBehavior::DontShowVisitedLinks,
      PaintBehavior::DrawsHDRContent,
    };
    localPaintBehavior.Add(localPaintingInfo.PaintBehavior & flagsToCopy);

    GraphicsContextStateSaver stateSaver(context, false);

    // Optimize clipping for the single fragment case.
    bool shouldClip = layerFragments.size() == 1 && layerFragments[0].ShouldPaintContent
                      && !layerFragments[0].DirtyForegroundRect().IsEmpty();
    if (shouldClip)
    {
      ClipToRect(context, stateSaver, localPaintingInfo, localPaintBehavior,
                 layerFragments[0].DirtyForegroundRect());
    }

    // We have to loop through every fragment multiple times, since we have to repaint in each specific phase
    // in order for interleaving of the fragments to work properly.
    bool selectionOnly = localPaintingInfo.PaintBehavior.Contains(PaintBehavior::SelectionOnly);
    bool selectionAndBackgroundsOnly =
      localPaintingInfo.PaintBehavior.Contains(PaintBehavior::SelectionAndBackgroundsOnly);

    // FIXME: SVG renderers that aren't containers (painted in the foreground phase only), once there's SVG.

    if (!selectionOnly)
    {
      PaintForegroundForFragmentsWithPhase(PaintPhase::ChildBlockBackgrounds, layerFragments, context,
                                           localPaintingInfo, localPaintBehavior,
                                           subtreePaintRootForRenderer);
    }

    if (selectionOnly || selectionAndBackgroundsOnly)
    {
      PaintForegroundForFragmentsWithPhase(PaintPhase::Selection, layerFragments, context, localPaintingInfo,
                                           localPaintBehavior, subtreePaintRootForRenderer);
    }
    else
    {
      PaintForegroundForFragmentsWithPhase(PaintPhase::Float, layerFragments, context, localPaintingInfo,
                                           localPaintBehavior, subtreePaintRootForRenderer);
      PaintForegroundForFragmentsWithPhase(PaintPhase::Foreground, layerFragments, context, localPaintingInfo,
                                           localPaintBehavior, subtreePaintRootForRenderer);
      PaintForegroundForFragmentsWithPhase(PaintPhase::ChildOutlines, layerFragments, context,
                                           localPaintingInfo, localPaintBehavior,
                                           subtreePaintRootForRenderer);
    }
  }

  void RenderLayer::PaintForegroundForFragmentsWithPhase(PaintPhase phase,
                                                         const LayerFragments &layerFragments,
                                                         GraphicsContext &context,
                                                         const LayerPaintingInfo &localPaintingInfo,
                                                         EnumSet<PaintBehavior> paintBehavior,
                                                         RenderObject *subtreePaintRootForRenderer) noexcept
  {
    bool shouldClip = layerFragments.size() > 1;

    for (const auto &fragment : layerFragments)
    {
      if (!fragment.ShouldPaintContent || fragment.DirtyForegroundRect().IsEmpty())
      {
        continue;
      }

      GraphicsContextStateSaver stateSaver(context, false);

      if (shouldClip)
      {
        ClipToRect(context, stateSaver, localPaintingInfo, paintBehavior, fragment.DirtyForegroundRect());
      }

      PaintInfo paintInfo(context, fragment.DirtyForegroundRect().Rect(), phase, paintBehavior,
                          subtreePaintRootForRenderer, &localPaintingInfo.RootLayer->Renderer(), this,
                          localPaintingInfo.RequireSecurityOriginAccessForWidgets);
      Renderer().Paint(paintInfo, PaintOffsetForRenderer(fragment, localPaintingInfo));
    }
  }

  void RenderLayer::PaintOutlineForFragments(const LayerFragments &layerFragments, GraphicsContext &context,
                                             const LayerPaintingInfo &localPaintingInfo,
                                             EnumSet<PaintBehavior> paintBehavior,
                                             RenderObject *subtreePaintRootForRenderer) noexcept
  {
    for (const auto &fragment : layerFragments)
    {
      if (fragment.DirtyBackgroundRect().IsEmpty())
      {
        continue;
      }

      // Paint our own outline
      PaintInfo paintInfo(context, fragment.DirtyBackgroundRect().Rect(), PaintPhase::SelfOutline,
                          paintBehavior, subtreePaintRootForRenderer,
                          &localPaintingInfo.RootLayer->Renderer(), this);

      GraphicsContextStateSaver stateSaver(context, false);

      ClipToRect(context, stateSaver, localPaintingInfo, paintBehavior, fragment.DirtyBackgroundRect(),
                 DoNotIncludeSelfForBorderRadius);
      Renderer().Paint(paintInfo, PaintOffsetForRenderer(fragment, localPaintingInfo));
    }
  }

  void RenderLayer::PaintMaskForFragments(const LayerFragments &layerFragments, GraphicsContext &context,
                                          const LayerPaintingInfo &localPaintingInfo,
                                          EnumSet<PaintBehavior> paintBehavior,
                                          RenderObject *subtreePaintRootForRenderer) noexcept
  {
    for (const auto &fragment : layerFragments)
    {
      if (!fragment.ShouldPaintContent)
      {
        continue;
      }

      GraphicsContextStateSaver stateSaver(context, false);

      ClipToRect(context, stateSaver, localPaintingInfo, paintBehavior, fragment.DirtyBackgroundRect(),
                 DoNotIncludeSelfForBorderRadius); // Mask painting will handle clipping to self.

      // Paint the mask.
      PaintInfo paintInfo(context, fragment.DirtyBackgroundRect().Rect(), PaintPhase::Mask, paintBehavior,
                          subtreePaintRootForRenderer, &localPaintingInfo.RootLayer->Renderer(), this);
      Renderer().Paint(paintInfo, PaintOffsetForRenderer(fragment, localPaintingInfo));
    }
  }

  void RenderLayer::PaintChildClippingMaskForFragments(const LayerFragments &layerFragments,
                                                       GraphicsContext &context,
                                                       const LayerPaintingInfo &localPaintingInfo,
                                                       EnumSet<PaintBehavior> paintBehavior,
                                                       RenderObject *subtreePaintRootForRenderer) noexcept
  {
    for (const auto &fragment : layerFragments)
    {
      if (!fragment.ShouldPaintContent)
      {
        continue;
      }

      GraphicsContextStateSaver stateSaver(context, false);

      ClipToRect(context, stateSaver, localPaintingInfo, paintBehavior, fragment.DirtyForegroundRect(),
                 IncludeSelfForBorderRadius); // Child clipping mask painting will handle clipping to self.

      // Paint the clipped mask.
      PaintInfo paintInfo(context, fragment.DirtyBackgroundRect().Rect(), PaintPhase::ClippingMask,
                          paintBehavior, subtreePaintRootForRenderer,
                          &localPaintingInfo.RootLayer->Renderer(), this);
      Renderer().Paint(paintInfo, PaintOffsetForRenderer(fragment, localPaintingInfo));
    }
  }

#pragma endregion

#pragma region Hit testing

  namespace
  {
    // Compute the z-offset of the point in the transformState.
    // This is effectively projecting a ray normal to the plane of ancestor, finding where that
    // ray intersects target, and computing the z delta between those two points.
    KRYS_NODISCARD double ComputeZOffset(const HitTestingTransformState &transformState) noexcept
    {
      // We got an affine transform, so no z-offset
      if (transformState.AccumulatedTransform.IsAffine())
      {
        return 0;
      }

      // Flatten the point into the target plane
      FloatPoint targetPoint = transformState.MappedPoint();

      // Now map the point back through the transform, which computes Z.
      FloatPoint3D backmappedPoint = transformState.AccumulatedTransform.MapPoint(FloatPoint3D(targetPoint));
      return backmappedPoint.Z();
    }

    /// @brief The nearest link at or above @p node in the composed tree.
    // FIXME: Images (which are never the link: an image map's area is) come with images.
    KRYS_NODISCARD dom::Element *EnclosingLinkEventParentOrSelf(dom::Node &node) noexcept
    {
      for (dom::Node *current = &node; current; current = current->ParentInComposedTree())
      {
        if (auto *element = DynamicDowncast<dom::Element>(*current); element && css::IsLink(*element))
        {
          return element;
        }
      }
      return nullptr;
    }
  }

  void RenderLayer::Autoscroll(const IntPoint &positionInWindow) noexcept
  {
    IntPoint currentDocumentPosition = Renderer().View().FrameView().WindowToContents(positionInWindow);
    LocalFrameView::ScrollRectToVisible(
      LayoutRect(LayoutPoint(currentDocumentPosition), LayoutSize(1_lu, 1_lu)), Renderer(), false,
      {SelectionRevealMode::Reveal, ScrollAlignment::AlignToEdgeIfNeeded,
       ScrollAlignment::AlignToEdgeIfNeeded});
  }

  bool RenderLayer::HitTest(const HitTestRequest &request, HitTestResult &result) noexcept
  {
    return HitTest(request, result.GetHitTestLocation(), result);
  }

  bool RenderLayer::HitTest(const HitTestRequest &request, const HitTestLocation &hitTestLocation,
                            HitTestResult &result) noexcept
  {
    krys_debug_assert(IsSelfPaintingLayer() || HasSelfPaintingLayerDescendant());
    krys_debug_assert(!Renderer().View().NeedsLayout());

    LayoutRect hitTestArea = Renderer().View().DocumentRect();
    if (!request.IgnoreClipping())
    {
      // FIXME: The visual viewport (clientCoordinatesRelativeToLayoutViewport) comes with visual viewports.
      hitTestArea.Intersect(Renderer().View().FrameView().VisibleContentRect());
    }

    auto insideLayer = HitTestLayer(this, nullptr, request, result, hitTestArea, hitTestLocation, false);
    if (!insideLayer.Layer)
    {
      // We didn't hit any layer. If we are the root layer and the mouse is -- or just was -- down,
      // return ourselves. We do this so mouse events continue getting delivered after a drag has
      // exited the view, and so hit testing over a scrollbar hits the content document.
      // In addtion, it is possible for the mouse to stay in the document but there is no element.
      // At that time, the events of the mouse should be fired.
      LayoutPoint hitPoint = hitTestLocation.Point();
      bool moveRequestIsOverDocument = request.Move() && hitTestArea.Contains(hitPoint);
      if (!request.IsChildFrameHitTest()
          && (request.Active() || request.Release() || moveRequestIsOverDocument) && IsRenderViewLayer())
      {
        Renderer().UpdateHitTestResult(
          result, Downcast<RenderView>(Renderer()).FlipForWritingMode(hitTestLocation.Point()));
        insideLayer = {this};
      }
    }

    // Now determine if the result is inside an anchor - if the urlElement isn't already set.
    auto *node = result.InnerNode();
    if (node && !result.URLElement())
    {
      result.SetURLElement(EnclosingLinkEventParentOrSelf(*node));
    }

    // Now return whether we were inside this layer (this will always be true for the root
    // layer).
    return insideLayer.Layer;
  }

  dom::Element *RenderLayer::EnclosingElement() const noexcept
  {
    for (RenderElement *r = &Renderer(); r; r = r->Parent())
    {
      if (auto *e = r->GetElement())
      {
        return e;
      }
    }
    return nullptr;
  }

  List<RenderLayer *> RenderLayer::TopLayerRenderLayers(const RenderView &renderView) noexcept
  {
    List<RenderLayer *> layers;
    for (auto &element : renderView.GetDocument().TopLayerElements())
    {
      auto *renderer = element->Renderer();
      if (!renderer)
      {
        continue;
      }

      auto *backdropRenderer = renderer->BackdropRenderer();
      if (backdropRenderer && backdropRenderer->HasLayer() && backdropRenderer->Layer()->Parent())
      {
        layers.push_back(backdropRenderer->Layer());
      }

      if (renderer->HasLayer())
      {
        auto &modelObject = Downcast<RenderLayerModelObject>(*renderer);
        if (modelObject.Layer()->Parent())
        {
          layers.push_back(modelObject.Layer());
        }
      }
    }
    return layers;
  }

  bool RenderLayer::EstablishesTopLayer() const noexcept
  {
    return dom::IsInTopLayerOrBackdrop(Renderer().Style(), Renderer().GetElement());
  }

  void RenderLayer::EstablishesTopLayerWillChange() noexcept
  {
    Compositor().EstablishesTopLayerWillChangeForLayer(*this);

    if (auto *parentLayer = Parent())
    {
      parentLayer->RemoveChild(*this);
    }
  }

  void RenderLayer::EstablishesTopLayerDidChange() noexcept
  {
    if (auto *parentLayer = Renderer().LayerParent())
    {
      (void)SetIsNormalFlowOnly(ShouldBeNormalFlowOnly());
      auto *beforeChild = Renderer().LayerNextSibling(*parentLayer);
      parentLayer->AddChild(*this, beforeChild);
    }
  }

  Ref<HitTestingTransformState> RenderLayer::CreateLocalTransformState(
    RenderLayer *rootLayer, RenderLayer *containerLayer, const LayoutRect &hitTestRect,
    const HitTestLocation &hitTestLocation, const HitTestingTransformState *containerTransformState,
    const LayoutSize &translationOffset) const noexcept
  {
    RefPtr<HitTestingTransformState> transformState;
    LayoutSize offset;
    if (containerTransformState)
    {
      // If we're already computing transform state, then it's relative to the container (which we know is
      // non-null).
      transformState = HitTestingTransformState::Create(*containerTransformState);
      offset = OffsetFromAncestor(containerLayer);
    }
    else
    {
      // If this is the first time we need to make transform state, then base it off of hitTestLocation,
      // which is relative to rootLayer.
      transformState = HitTestingTransformState::Create(
        hitTestLocation.TransformedPoint(), hitTestLocation.TransformedRect(), FloatQuad(hitTestRect));
      offset = OffsetFromAncestor(rootLayer);
    }
    offset += translationOffset;

    if (Renderer().ShouldUseTransformFromContainer(containerLayer ? &containerLayer->Renderer() : nullptr))
    {
      TransformationMatrix containerTransform;
      Renderer().GetTransformFromContainer(offset, containerTransform);
      transformState->ApplyTransform(containerTransform);
    }
    else
    {
      transformState->Translate(offset.Width(), offset.Height());
    }

    Ref<HitTestingTransformState> result = transformState;
    return result;
  }

  // hitTestLocation and hitTestRect are relative to rootLayer.
  // A 'flattening' layer is one preserves3D() == false.
  // transformState.AccumulatedTransform holds the transform from the containing flattening layer.
  // transformState.LastPlanarPoint is the hitTestLocation in the plane of the containing flattening layer.
  // transformState.LastPlanarQuad is the hitTestRect as a quad in the plane of the containing flattening
  // layer.
  //
  // If zOffset is non-null (which indicates that the caller wants z offset information),
  //  *zOffset on return is the z offset of the hit point relative to the containing flattening layer.
  RenderLayer::HitLayer RenderLayer::HitTestLayer(
    RenderLayer *rootLayer, RenderLayer *containerLayer, const HitTestRequest &request, HitTestResult &result,
    const LayoutRect &hitTestRect, const HitTestLocation &hitTestLocation, bool appliedTransform,
    const HitTestingTransformState *transformState, double *zOffset) noexcept
  {
    UpdateLayerListsIfNeeded();

    if (!IsSelfPaintingLayer() && !HasSelfPaintingLayerDescendant())
    {
      return {};
    }

    // FIXME: Renderers captured in a view transition, SVG resources and hidden SVG containers, and fixed
    // container sampling (isForFixedContainerSampling) come with view transitions, SVG and scrolling.

    // The natural thing would be to keep HitTestingTransformState on the stack, but it's big, so we
    // heap-allocate.

    // Apply a transform if we have one.
    if (Transform() && !appliedTransform)
    {
      // FIXME: Transformed layers in a paginated layer (hitTestTransformedLayerInFragments) come with
      // fragmentation; they're hit tested unfragmented.

      // Make sure the parent's clip rects have been calculated.
      if (Parent())
      {
        ClipRectsContext clipRectsContext(rootLayer, RootRelativeClipRects,
                                          {ClipRectsOption::RespectOverflowClip});
        ClipRect clipRect = BackgroundClipRect(clipRectsContext);
        // Test the enclosing clip now.
        if (!clipRect.Intersects(hitTestLocation))
        {
          return {};
        }
      }

      return HitTestLayerByApplyingTransform(rootLayer, containerLayer, request, result, hitTestRect,
                                             hitTestLocation, transformState, zOffset);
    }

    // Ensure our lists and 3d status are up-to-date.
    Update3DTransformedDescendantStatus();

    RefPtr<HitTestingTransformState> localTransformState;
    if (appliedTransform)
    {
      // We computed the correct state in the caller (above code), so just reference it.
      krys_debug_assert(transformState);
      localTransformState = ShareRefPtr(const_cast<HitTestingTransformState *>(transformState));
    }
    else if (transformState || Has3DTransformedDescendant() || Preserves3D())
    {
      // We need transform state for the first time, or to offset the container state, so create it here.
      localTransformState =
        CreateLocalTransformState(rootLayer, containerLayer, hitTestRect, hitTestLocation, transformState);
    }

    // Check for hit test on backface if backface-visibility is 'hidden'
    if (localTransformState && Renderer().Style().GetBackfaceVisibility() == BackfaceVisibility::Hidden)
    {
      Maybe<TransformationMatrix> invertedMatrix = localTransformState->AccumulatedTransform.Inverse();
      // If the z-vector of the matrix is negative, the back is facing towards the viewer.
      if (invertedMatrix && invertedMatrix.value().M33() < 0)
      {
        return {};
      }
    }

    // The following are used for keeping track of the z-depth of the hit point of 3d-transformed
    // descendants.
    double localZOffset = -std::numeric_limits<double>::infinity();
    double *zOffsetForDescendantsPtr = nullptr;

    bool depthSortDescendants = false;
    if (Preserves3D())
    {
      depthSortDescendants = true;
      // Our layers can depth-test with our container, so share the z depth pointer with the container, if it
      // passed one down.
      zOffsetForDescendantsPtr = zOffset ? zOffset : &localZOffset;
    }
    else if (zOffset)
    {
      zOffsetForDescendantsPtr = nullptr;
    }

    double selfZOffset = localTransformState ? ComputeZOffset(*localTransformState) : 0;

    // This variable tracks which layer the mouse ends up being inside.
    auto candidateLayer = HitLayer {nullptr, -std::numeric_limits<double>::infinity()};
#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(*this);
#endif

    auto offsetFromRoot = OffsetFromAncestor(rootLayer);
    // FIXME: We need to correctly hit test the clip-path when we have a RenderInline too.
    if (auto *rendererBox = GetRenderBox();
        rendererBox
        && !rendererBox->HitTestClipPath(hitTestLocation,
                                         ToLayoutPoint(offsetFromRoot - ToLayoutSize(RendererLocation()))))
    {
      return {};
    }

    // Begin by walking our list of positive layers from highest z-index down to the lowest z-index.
    auto hitLayer =
      HitTestList(PositiveZOrderLayers(), rootLayer, request, result, hitTestRect, hitTestLocation,
                  localTransformState.get(), zOffsetForDescendantsPtr, depthSortDescendants);
    if (hitLayer.Layer)
    {
      if (!depthSortDescendants)
      {
        return hitLayer;
      }
      if (hitLayer.ZOffset > candidateLayer.ZOffset)
      {
        candidateLayer = hitLayer;
      }
    }

    // Now check our overflow objects.
    {
      HitTestResult tempResult(result.GetHitTestLocation());
      hitLayer = HitTestList(NormalFlowLayers(), rootLayer, request, tempResult, hitTestRect, hitTestLocation,
                             localTransformState.get(), zOffsetForDescendantsPtr, depthSortDescendants);

      if (request.ResultIsElementList())
      {
        result.Append(tempResult, request);
      }

      if (hitLayer.Layer)
      {
        if (!depthSortDescendants || hitLayer.ZOffset > candidateLayer.ZOffset)
        {
          if (!request.ResultIsElementList())
          {
            result = tempResult;
          }

          candidateLayer = hitLayer;
        }

        if (!depthSortDescendants)
        {
          return hitLayer;
        }
      }
    }

    // Collect the fragments. This will compute the clip rectangles for each layer fragment.
    LayerFragments layerFragments;
    CollectFragments(layerFragments, rootLayer, hitTestRect, RootRelativeClipRects,
                     {ClipRectsOption::RespectOverflowClip}, offsetFromRoot);

    LayoutPoint localPoint;
    if (CanResize() && _scrollableArea
        && _scrollableArea->HitTestResizerInFragments(layerFragments, hitTestLocation, localPoint))
    {
      Renderer().UpdateHitTestResult(result, localPoint);
      return {this, selfZOffset};
    }

    auto isHitCandidate = [&]()
    {
      return !depthSortDescendants || selfZOffset > candidateLayer.ZOffset;
    };

    // Next we want to see if the mouse pos is inside the child RenderObjects of the layer. Check
    // every fragment in reverse order.
    if (IsSelfPaintingLayer())
    {
      // Hit test with a temporary HitTestResult, because we only want to commit to 'result' if we know we're
      // frontmost.
      HitTestResult tempResult(result.GetHitTestLocation());
      bool insideFragmentForegroundRect = false;
      if (HitTestContentsForFragments(layerFragments, request, tempResult, hitTestLocation,
                                      HitTestDescendants, insideFragmentForegroundRect)
          && isHitCandidate())
      {
        if (request.ResultIsElementList())
        {
          result.Append(tempResult, request);
        }
        else
        {
          result = tempResult;
        }

        if (!depthSortDescendants)
        {
          return {this, selfZOffset};
        }

        // Foreground can depth-sort with descendant layers, so keep this as a candidate.
        candidateLayer = {this, selfZOffset};
      }
      else if (insideFragmentForegroundRect && request.ResultIsElementList())
      {
        result.Append(tempResult, request);
      }
    }

    // Now check our negative z-index children.
    {
      HitTestResult tempResult(result.GetHitTestLocation());
      hitLayer =
        HitTestList(NegativeZOrderLayers(), rootLayer, request, tempResult, hitTestRect, hitTestLocation,
                    localTransformState.get(), zOffsetForDescendantsPtr, depthSortDescendants);

      if (request.ResultIsElementList())
      {
        result.Append(tempResult, request);
      }

      if (hitLayer.Layer)
      {
        if (!depthSortDescendants || hitLayer.ZOffset > candidateLayer.ZOffset)
        {
          if (!request.ResultIsElementList())
          {
            result = tempResult;
          }

          candidateLayer = hitLayer;
        }

        if (!depthSortDescendants)
        {
          return hitLayer;
        }
      }
    }

    // If we found a layer, return. Child layers, and foreground always render in front of background.
    if (candidateLayer.Layer && !depthSortDescendants)
    {
      return candidateLayer;
    }

    if (IsSelfPaintingLayer())
    {
      HitTestResult tempResult(result.GetHitTestLocation());
      bool insideFragmentBackgroundRect = false;
      if (HitTestContentsForFragments(layerFragments, request, tempResult, hitTestLocation, HitTestSelf,
                                      insideFragmentBackgroundRect)
          && isHitCandidate())
      {
        if (request.ResultIsElementList())
        {
          result.Append(tempResult, request);
        }
        else
        {
          result = tempResult;
        }

        if (!depthSortDescendants)
        {
          return {this, selfZOffset};
        }

        candidateLayer = {this, selfZOffset};
      }

      if (insideFragmentBackgroundRect && request.ResultIsElementList())
      {
        result.Append(tempResult, request);
      }
    }

    return candidateLayer;
  }

  bool RenderLayer::HitTestContentsForFragments(const LayerFragments &layerFragments,
                                                const HitTestRequest &request, HitTestResult &result,
                                                const HitTestLocation &hitTestLocation,
                                                HitTestFilter hitTestFilter,
                                                bool &insideClipRect) const noexcept
  {
    for (size_t i = layerFragments.size(); i;)
    {
      --i;
      const auto &fragment = layerFragments[i];
      if ((hitTestFilter == HitTestSelf && !fragment.DirtyBackgroundRect().Intersects(hitTestLocation))
          || (hitTestFilter == HitTestDescendants
              && !fragment.DirtyForegroundRect().Intersects(hitTestLocation)))
      {
        continue;
      }
      insideClipRect = true;
      if (HitTestContents(request, result, fragment.LayerBounds(), hitTestLocation, hitTestFilter))
      {
        return true;
      }
    }

    return false;
  }

  RenderLayer::HitLayer RenderLayer::HitTestLayerByApplyingTransform(
    RenderLayer *rootLayer, RenderLayer *containerLayer, const HitTestRequest &request, HitTestResult &result,
    const LayoutRect &hitTestRect, const HitTestLocation &hitTestLocation,
    const HitTestingTransformState *transformState, double *zOffset,
    const LayoutSize &translationOffset) noexcept
  {
    // Create a transform state to accumulate this transform.
    Ref<HitTestingTransformState> newTransformState = CreateLocalTransformState(
      rootLayer, containerLayer, hitTestRect, hitTestLocation, transformState, translationOffset);

    // If the transform can't be inverted, then don't hit test this layer at all.
    if (!newTransformState->AccumulatedTransform.IsInvertible())
    {
      return {};
    }

    // Compute the point and the hit test rect in the coords of this layer by using the values
    // from the transformState, which store the point and quad in the coords of the last flattened
    // layer, and the accumulated transform which lets up map through preserve-3d layers.
    //
    // We can't just map hitTestLocation and hitTestRect because they may have been flattened (losing z)
    // by our container.
    auto localPoint = newTransformState->MappedPoint();
    auto localHitTestRect = newTransformState->BoundsOfMappedArea();
    auto newHitTestLocation = [&]
    {
      if (hitTestLocation.IsRectBasedTest())
      {
        return HitTestLocation(localPoint, newTransformState->MappedQuad());
      }
      return HitTestLocation(localPoint, FloatQuad(FloatRect(newTransformState->BoundsOfMappedQuad())));
    }();

    // Now do a hit test with the root layer shifted to be us.
    return HitTestLayer(this, containerLayer, request, result, localHitTestRect, newHitTestLocation, true,
                        newTransformState.get(), zOffset);
  }

  bool RenderLayer::HitTestContents(const HitTestRequest &request, HitTestResult &result,
                                    const LayoutRect &layerBounds, const HitTestLocation &hitTestLocation,
                                    HitTestFilter hitTestFilter) const noexcept
  {
    krys_debug_assert(IsSelfPaintingLayer() || HasSelfPaintingLayerDescendant());

    if (!Renderer().HitTest(request, result, hitTestLocation,
                            ToLayoutPoint(layerBounds.Location() - RendererLocation()), hitTestFilter))
    {
      // It's wrong to set innerNode, but then claim that you didn't hit anything, unless it is
      // a rect-based test.
      krys_debug_assert(!result.InnerNode()
                        || (request.ResultIsElementList() && result.ListBasedTestResult().size()));
      return false;
    }

    // For positioned generated content, we might still not have a
    // node by the time we get to the layer level, since none of
    // the content in the layer has an element. So just walk up
    // the tree.
    if (!result.InnerNode() || !result.InnerNonSharedNode())
    {
      auto *e = EnclosingElement();
      if (!result.InnerNode())
      {
        result.SetInnerNode(e);
      }
      if (!result.InnerNonSharedNode())
      {
        result.SetInnerNonSharedNode(e);
      }
    }

    return true;
  }

  RenderLayer::HitLayer RenderLayer::HitTestList(LayerList layerIterator, RenderLayer *rootLayer,
                                                 const HitTestRequest &request, HitTestResult &result,
                                                 const LayoutRect &hitTestRect,
                                                 const HitTestLocation &hitTestLocation,
                                                 const HitTestingTransformState *transformState,
                                                 double *zOffsetForDescendants,
                                                 bool depthSortDescendants) noexcept
  {
    if (layerIterator.begin() == layerIterator.end())
    {
      return {};
    }

    if (!HasSelfPaintingLayerDescendant())
    {
      return {};
    }

    if (auto *renderBox = GetRenderBox(); renderBox && IsSkippedContentRoot(*renderBox))
    {
      return {};
    }

    auto resultLayer = HitLayer {nullptr, -std::numeric_limits<double>::infinity()};

    RefPtr<HitTestingTransformState> flattenedTransformState;
    double unflattenedZOffset = 0;
    for (auto iter = layerIterator.end(); iter != layerIterator.begin();)
    {
      auto *childLayer = *--iter;

      // If we're about to cross a flattening boundary, then pass the (lazily-initialized)
      // flattened transfomState to the child layer.
      auto *transformStateForChild = transformState;
      if (transformState && !childLayer->ParticipatesInPreserve3D())
      {
        if (!flattenedTransformState)
        {
          flattenedTransformState = HitTestingTransformState::Create(*transformState);
          flattenedTransformState->Flatten();
          unflattenedZOffset = ComputeZOffset(*transformState);
        }
        transformStateForChild = flattenedTransformState.get();
      }

      HitTestResult tempResult(result.GetHitTestLocation());
      auto hitLayer =
        childLayer->HitTestLayer(rootLayer, this, request, tempResult, hitTestRect, hitTestLocation, false,
                                 transformStateForChild, zOffsetForDescendants);

      // If it is a list-based test, we can safely append the temporary result since it might had hit
      // nodes but not necessarily had hitLayer set.
      krys_debug_assert(!result.IsRectBasedTest() || request.ResultIsElementList());
      if (request.ResultIsElementList())
      {
        result.Append(tempResult, request);
      }

      if (hitLayer.Layer)
      {
        // If the child was flattened, then override the returned depth with the depth of the
        // plane we flattened into (ourselves) instead.
        if (transformStateForChild == flattenedTransformState.get())
        {
          hitLayer.ZOffset = unflattenedZOffset;
        }

        if (!depthSortDescendants || hitLayer.ZOffset > resultLayer.ZOffset)
        {
          resultLayer = hitLayer;
          if (!request.ResultIsElementList())
          {
            result = tempResult;
          }
          if (!depthSortDescendants)
          {
            break;
          }
        }
      }
    }

    return resultLayer;
  }

#pragma endregion
}
