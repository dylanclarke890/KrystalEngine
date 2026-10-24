#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollingConstraints.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/LayerOverlapMap.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <algorithm>

namespace krys::boo
{
  struct RenderLayerCompositor::OverlapExtent
  {
    LayoutRect Bounds;
    LayerOverlapMap::LayerAndBoundsVector ClippingScopes;

    bool ExtentComputed {false};
    bool HasTransformAnimation {false};
    bool AnimationCausesExtentUncertainty {false};
    bool ClippingScopesComputed {false};

    KRYS_NODISCARD bool KnownToBeHaveExtentUncertainty() const noexcept
    {
      return ExtentComputed && AnimationCausesExtentUncertainty;
    }
  };

  struct RenderLayerCompositor::CompositingState
  {
    RenderLayer *CompositingAncestor;
    RenderLayer *BackingSharingAncestor {nullptr};
    RenderLayer *StackingContextAncestor {nullptr};
    bool SubtreeIsCompositing {false};
    bool TestingOverlap {true};
    bool FullPaintOrderTraversalRequired {false};
    bool DescendantsRequireCompositingUpdate {false};
    bool AncestorHasTransformAnimation {false};
    bool AncestorAllowsBackingStoreDetachingForFixed {false};
    bool HasCompositedNonContainedDescendants {false};
    bool HasNotIsolatedCompositedBlendingDescendants {false};
    bool HasBackdropFilterDescendantsWithoutRoot {false};

    explicit CompositingState(RenderLayer *compAncestor, bool testOverlap = true) noexcept
        : CompositingAncestor(compAncestor), TestingOverlap(testOverlap)
    {
    }

    KRYS_NODISCARD CompositingState StateForPaintOrderChildren(RenderLayer &layer) const noexcept
    {
      CompositingState childState(CompositingAncestor);
      if (layer.IsStackingContext())
      {
        childState.StackingContextAncestor = &layer;
      }
      else
      {
        childState.StackingContextAncestor = StackingContextAncestor;
      }

      childState.BackingSharingAncestor = BackingSharingAncestor;
      childState.SubtreeIsCompositing = false;
      childState.TestingOverlap = TestingOverlap;
      childState.FullPaintOrderTraversalRequired = FullPaintOrderTraversalRequired;
      childState.DescendantsRequireCompositingUpdate = DescendantsRequireCompositingUpdate;
      childState.AncestorHasTransformAnimation = AncestorHasTransformAnimation;
      childState.AncestorAllowsBackingStoreDetachingForFixed = AncestorAllowsBackingStoreDetachingForFixed;
      childState.HasCompositedNonContainedDescendants = false;
      // FIXME: should this only be reset for stacking contexts?
      childState.HasNotIsolatedCompositedBlendingDescendants = false;
      childState.HasBackdropFilterDescendantsWithoutRoot = false;
      return childState;
    }

    void UpdateWithDescendantStateAndLayer(const CompositingState &childState, const RenderLayer &layer,
                                           const RenderLayer *ancestorLayer, const OverlapExtent &layerExtent,
                                           bool isUnchangedSubtree = false) noexcept
    {
      // Subsequent layers in the parent stacking context also need to composite.
      SubtreeIsCompositing |= childState.SubtreeIsCompositing | layer.IsComposited();
      if (!isUnchangedSubtree)
      {
        FullPaintOrderTraversalRequired |= childState.FullPaintOrderTraversalRequired;
      }

      // Turn overlap testing off for later layers if it's already off, or if we have an animating transform.
      // Note that if the layer clips its descendants, there's no reason to propagate the child animation to
      // the parent layers. That's because we know for sure the animation is contained inside the clipping
      // rectangle, which is already added to the overlap map.
      auto canReenableOverlapTesting = [&layer]
      {
        return layer.IsComposited() && RenderLayerCompositor::ClipsCompositingDescendants(layer);
      };
      if ((!childState.TestingOverlap && !canReenableOverlapTesting())
          || layerExtent.KnownToBeHaveExtentUncertainty())
      {
        TestingOverlap = false;
      }

      auto computeHasCompositedNonContainedDescendants = [&]
      {
        if (HasCompositedNonContainedDescendants)
        {
          return true;
        }
        if (!ancestorLayer)
        {
          return false;
        }
        if (!layer.IsComposited())
        {
          return false;
        }
        if (!layer.Renderer().IsOutOfFlowPositioned())
        {
          return false;
        }
        if (layer.AncestorLayerIsInContainingBlockChain(*ancestorLayer))
        {
          return false;
        }
        return true;
      };

      HasCompositedNonContainedDescendants = computeHasCompositedNonContainedDescendants();

      if ((layer.IsComposited() && layer.HasBlendMode())
          || (layer.HasNotIsolatedCompositedBlendingDescendants() && !layer.IsolatesCompositedBlending()))
      {
        HasNotIsolatedCompositedBlendingDescendants = true;
      }

      if ((layer.IsComposited() && layer.HasBackdropFilter())
          || (layer.HasBackdropFilterDescendantsWithoutRoot() && !layer.IsBackdropRoot()))
      {
        HasBackdropFilterDescendantsWithoutRoot = true;
      }
    }

    KRYS_NODISCARD bool HasNonRootCompositedAncestor() const noexcept
    {
      return CompositingAncestor && !CompositingAncestor->IsRenderViewLayer();
    }
  };

  struct RenderLayerCompositor::UpdateBackingTraversalState
  {
    RenderLayer *CompositingAncestor;

    // List of layers in the current stacking context that are clipped by ancestor scrollers.
    List<RenderLayer *> *LayersClippedByScrollers;
    // List of layers with composited overflow:scroll.
    List<RenderLayer *> *OverflowScrollLayers;

    explicit UpdateBackingTraversalState(RenderLayer *compAncestor = nullptr,
                                         List<RenderLayer *> *clippedLayers = nullptr,
                                         List<RenderLayer *> *overflowScrollers = nullptr) noexcept
        : CompositingAncestor(compAncestor), LayersClippedByScrollers(clippedLayers),
          OverflowScrollLayers(overflowScrollers)
    {
    }

    KRYS_NODISCARD UpdateBackingTraversalState StateForDescendants() const noexcept
    {
      return UpdateBackingTraversalState(CompositingAncestor, LayersClippedByScrollers, OverflowScrollLayers);
    }
  };

  /*
      Backing sharing is used to reduce memory use by allowing multiple RenderLayers (normally siblings) which
     share the same stacking context ancestor to render into the same compositing layer. This has to be done
     in a way that preserves back-to-front paint order. The common case where this kicks in is a non-stacking
     context overflow:scroll with position:relative descendants.

      When we've determined that a layer can be composited, it becomes a candidate for backing sharing (i.e.
     layers later in paint order, with the same stacking context ancestor, might be able to paint into it).

      We maintain multiple backing provider candidates in order to have backing sharing work with sibling or
     nested overflow scrollers. When traversing layers that might be able to share with these providers, this
     is essentially a bucketing process. There are three cases to consider here:

      1. Sibling scrollers that don't overlap:
         In this case we can simply add later layers to the appropriate scroller (using scrolling scope to
     find the right one), since we know that we're traversing those layers in paint order and the scrollers
     don't overlap. As we assign layers to one or other candidate, paint order will be preserved. This is
     supported.

      2. Sibling scrollers that overlap:
         Here we can have layers share with the on-top scroller, but have to ensure that layers scrolled by
     the below scroller correctly overlap the border/background of the on-top scroller (i.e. they can't use
     sharing). So we can only do sharing with the last scroller. This is not currently supported.

      3. Nested scrollers:
         Similar to overlapping scrollers, we have to ensure that we add to the right provider (looking a
     scrolling scope), and don't break overlap with the nested scroller. This is not currently supported.

      We also track additional backing sharing providers that aren't clipped scrollers. These cannot be added
     to, since that could expand the bounds of the resulting layer. They are tracked so we can check them for
     overlap, and continue to add to the scroller backing sharing providers if the new content doesn't
     overlap.
   */

  struct RenderLayerCompositor::BackingSharingSnapshot
  {
    uint64 SequenceIdentifier;
    size_t ProviderCount {0};
  };

  class RenderLayerCompositor::BackingSharingState
  {
    KRYS_NON_COPY_MOVABLE(BackingSharingState);

  public:
    struct Provider
    {
      RenderLayer *ProviderLayer;
      ListHashSet<RenderLayer *> SharingLayers;
      LayoutRect AbsoluteBounds;
    };

  private:
    List<Provider> _backingProviderCandidates;
    RenderLayer *_backingSharingStackingContext {nullptr};
    uint64 _sequenceIdentifier {GenerateSequenceIdentifier()};
    ListHashSet<RenderLayer *> _layersPendingRepaint;
    bool _allowOverlappingProviders {false};

  public:
    explicit BackingSharingState(bool allowOverlappingProviders) noexcept
        : _allowOverlappingProviders(allowOverlappingProviders)
    {
    }

    KRYS_NODISCARD List<Provider> &BackingProviderCandidates() noexcept
    {
      return _backingProviderCandidates;
    }

    KRYS_NODISCARD const RenderLayer *FirstProviderCandidateLayer() const noexcept
    {
      return !_backingProviderCandidates.empty() ? _backingProviderCandidates.front().ProviderLayer : nullptr;
    }

    KRYS_NODISCARD RenderLayer *BackingSharingStackingContext() const noexcept
    {
      return _backingSharingStackingContext;
    }

    Provider *BackingProviderCandidateForLayer(const RenderLayer &layer,
                                               const RenderLayerCompositor &compositor,
                                               LayerOverlapMap &overlapMap, OverlapExtent &overlap) noexcept;
    Provider *ExistingBackingProviderCandidateForLayer(const RenderLayer &layer) noexcept;
    Provider *BackingProviderForLayer(const RenderLayer &layer) noexcept;

    // Add a layer that would repaint into a layer in m_backingSharingLayers. That repaint has to wait until
    // we've set the provider's backing-sharing layers.
    void AddLayerNeedingRepaint(RenderLayer &layer) noexcept
    {
      _layersPendingRepaint.add(&layer);
    }

    void AddBackingSharingCandidate(RenderLayer &candidateLayer, LayoutRect candidateAbsoluteBounds,
                                    RenderLayer &candidateStackingContext,
                                    const Maybe<BackingSharingSnapshot> &backingSharingSnapshot) noexcept;
    KRYS_NODISCARD bool IsAdditionalProviderCandidate(RenderLayer &candidateLayer,
                                                      LayoutRect candidateAbsoluteBounds,
                                                      RenderLayer *stackingContextAncestor) const noexcept;
    void StartBackingSharingSequence(RenderLayer &candidateLayer, LayoutRect candidateAbsoluteBounds,
                                     RenderLayer &candidateStackingContext) noexcept;
    void EndBackingSharingSequence(RenderLayer &endLayer) noexcept;

    KRYS_NODISCARD Maybe<BackingSharingSnapshot> Snapshot() const noexcept
    {
      if (!_backingSharingStackingContext)
      {
        return null;
      }
      return BackingSharingSnapshot {_sequenceIdentifier, _backingProviderCandidates.size()};
    }
    KRYS_NODISCARD uint64 SequenceIdentifier() const noexcept
    {
      return _sequenceIdentifier;
    }

  private:
    KRYS_NODISCARD static uint64 GenerateSequenceIdentifier() noexcept
    {
      static uint64 next = 0;
      return ++next;
    }

    void IssuePendingRepaints() noexcept;
  };

  void RenderLayerCompositor::BackingSharingState::StartBackingSharingSequence(
    RenderLayer &candidateLayer, LayoutRect candidateAbsoluteBounds,
    RenderLayer &candidateStackingContext) noexcept
  {
    krys_debug_assert(!_backingSharingStackingContext);
    krys_debug_assert(_backingProviderCandidates.empty());
    _backingProviderCandidates.push_back({&candidateLayer, {}, candidateAbsoluteBounds});
    _backingSharingStackingContext = &candidateStackingContext;
  }

  void RenderLayerCompositor::BackingSharingState::AddBackingSharingCandidate(
    RenderLayer &candidateLayer, LayoutRect candidateAbsoluteBounds, RenderLayer &candidateStackingContext,
    const Maybe<BackingSharingSnapshot> &backingSharingSnapshot) noexcept
  {
    krys_debug_assert(_backingSharingStackingContext == &candidateStackingContext);
    (void)candidateStackingContext;

    // Inserts candidateLayer into the provider list in z-order, using the state snapshot that was taken
    // before any descendant layers were traversed.

    if (!backingSharingSnapshot || _sequenceIdentifier != backingSharingSnapshot->SequenceIdentifier)
    {
      // If a new sharing sequence has been started since the snapshot was taken, then this candidate will be
      // before any of the current ones in z-order (which must have been added by descendants of this layer).
      _backingProviderCandidates.insert(_backingProviderCandidates.begin(),
                                        Provider {&candidateLayer, {}, candidateAbsoluteBounds});
    }
    else
    {
      // Otherwise insert it at the position captured in the snapshot
      _backingProviderCandidates.insert(_backingProviderCandidates.begin()
                                          + static_cast<ptrdiff_t>(backingSharingSnapshot->ProviderCount),
                                        Provider {&candidateLayer, {}, candidateAbsoluteBounds});
    }
  }

  void RenderLayerCompositor::BackingSharingState::EndBackingSharingSequence(RenderLayer &endLayer) noexcept
  {
    krys_debug_assert(_backingSharingStackingContext);

    auto candidates = krys::move(_backingProviderCandidates);
    _backingProviderCandidates.clear();

    for (auto &candidate : candidates)
    {
      candidate.SharingLayers.remove(&endLayer);
      candidate.ProviderLayer->Backing()->SetBackingSharingLayers(krys::move(candidate.SharingLayers));
    }
    _backingSharingStackingContext = nullptr;
    _sequenceIdentifier = GenerateSequenceIdentifier();

    IssuePendingRepaints();
  }

  auto RenderLayerCompositor::BackingSharingState::BackingProviderCandidateForLayer(
    const RenderLayer &layer, const RenderLayerCompositor &compositor, LayerOverlapMap &overlapMap,
    OverlapExtent &overlap) noexcept -> Provider *
  {
    // FIXME: Reflections (hasReflection), once there are reflections.

    if (!_allowOverlappingProviders)
    {
      for (auto &candidate : _backingProviderCandidates)
      {
        auto &providerLayer = *candidate.ProviderLayer;
        if (layer.AncestorLayerIsInContainingBlockChain(providerLayer))
        {
          return &candidate;
        }
      }

      return nullptr;
    }

    if (_backingProviderCandidates.empty())
    {
      return nullptr;
    }

    // First, find the frontmost provider that is an ancestor in the containing block chain.
    Maybe<size_t> candidateIndex;
    for (size_t i = _backingProviderCandidates.size(); i-- > 0;)
    {
      auto &providerLayer = *_backingProviderCandidates[i].ProviderLayer;
      if (&layer == &providerLayer)
      {
        continue;
      }

      if (!layer.AncestorLayerIsInContainingBlockChain(providerLayer))
      {
        continue;
      }

      candidateIndex = i;
      break;
    }

    if (!candidateIndex)
    {
      return nullptr;
    }

    auto &candidate = _backingProviderCandidates[*candidateIndex];

    // Only allow adding to providers that clip their descendants, unless there's only a single provider.
    // Unclipped providers in-front are tracked for overlap testing only.
    // FIXME: We could accumulate the union of the overlap bounds for a provider and its sharing layers to
    // avoid this restriction.
    if (_backingProviderCandidates.size() > 1 && !candidate.ProviderLayer->CanUseCompositedScrolling())
    {
      return nullptr;
    }

    if (*candidateIndex == _backingProviderCandidates.size() - 1)
    {
      // No other provider is in front of the candidate, so no need to check for overlap.
      return &candidate;
    }

    auto &providerLayer = *candidate.ProviderLayer;
    LayoutRect overlapBounds = candidate.AbsoluteBounds;
    if (auto *scrollableArea = providerLayer.GetScrollableArea();
        scrollableArea && providerLayer.CanUseCompositedScrolling()
        && scrollableArea->HasScrollableHorizontalOverflow()
             != scrollableArea->HasScrollableVerticalOverflow())
    {
      // If the provider uses composited scrolling but only supports scrolling in one axis, we can use the
      // clipped overlap bounds in the other axis, when checking for overlap.
      auto clippedOverlapBounds = compositor.ComputeClippedOverlapBounds(overlapMap, layer, overlap);
      if (scrollableArea->HasScrollableHorizontalOverflow())
      {
        overlapBounds.SetY(clippedOverlapBounds.Y());
        overlapBounds.SetHeight(clippedOverlapBounds.Height());
      }
      else
      {
        overlapBounds.SetX(clippedOverlapBounds.X());
        overlapBounds.SetWidth(clippedOverlapBounds.Width());
      }
    }

    // Check if any of the other candidates that are in front of the selected provider will overlap the bounds
    // of the layer to be added.
    for (size_t i = *candidateIndex + 1; i < _backingProviderCandidates.size(); ++i)
    {
      if (overlapBounds.Intersects(_backingProviderCandidates[i].AbsoluteBounds))
      {
        return nullptr;
      }
    }

    return &candidate;
  }

  auto RenderLayerCompositor::BackingSharingState::ExistingBackingProviderCandidateForLayer(
    const RenderLayer &layer) noexcept -> Provider *
  {
    krys_debug_assert(layer.PaintsIntoProvidedBacking());
    for (auto &candidate : _backingProviderCandidates)
    {
      if (layer.BackingProviderLayer() == candidate.ProviderLayer)
      {
        return &candidate;
      }
    }
    return nullptr;
  }

  auto RenderLayerCompositor::BackingSharingState::BackingProviderForLayer(const RenderLayer &layer) noexcept
    -> Provider *
  {
    for (auto &candidate : _backingProviderCandidates)
    {
      if (candidate.SharingLayers.contains(const_cast<RenderLayer *>(&layer)))
      {
        return &candidate;
      }
    }

    return nullptr;
  }

  bool RenderLayerCompositor::BackingSharingState::IsAdditionalProviderCandidate(
    RenderLayer &candidateLayer, LayoutRect candidateAbsoluteBounds,
    RenderLayer *stackingContextAncestor) const noexcept
  {
    krys_debug_assert(!_backingProviderCandidates.empty());
    if (!stackingContextAncestor || stackingContextAncestor != _backingSharingStackingContext)
    {
      return false;
    }

    if (!_allowOverlappingProviders)
    {
      // Only allow multiple providers for overflow scroll, which we know clips its descendants.
      if (!(_backingProviderCandidates[0].ProviderLayer->CanUseCompositedScrolling()
            && candidateLayer.CanUseCompositedScrolling()))
      {
        return false;
      }

      // Disallow overlap between backing providers.
      for (const auto &candidate : _backingProviderCandidates)
      {
        if (candidateAbsoluteBounds.Intersects(candidate.AbsoluteBounds))
        {
          return false;
        }
      }
      return true;
    }

    if (!_backingProviderCandidates[0].ProviderLayer->CanUseCompositedScrolling())
    {
      return false;
    }

    if (_backingProviderCandidates.size() >= 10)
    {
      return false;
    }
    return true;
  }

  void RenderLayerCompositor::BackingSharingState::IssuePendingRepaints() noexcept
  {
    for (auto *layer : _layersPendingRepaint)
    {
      layer->CompositingStatusChanged(LayoutUpToDate::Yes);
      layer->Compositor().RepaintOnCompositingChange(*layer, layer->RepaintContainer());
    }

    _layersPendingRepaint.clear();
  }

  RenderLayerCompositor::RenderLayerCompositor(RenderView &renderView) noexcept
      : _renderView(renderView),
        _updateCompositingLayersTimer(*this, &RenderLayerCompositor::UpdateCompositingLayersTimerFired),
        _updateRenderingTimer(*this, &RenderLayerCompositor::ScheduleRenderingUpdate)
  {
  }

  RenderLayerCompositor::~RenderLayerCompositor() noexcept
  {
    // Take care that the owned GraphicsLayers are deleted first as their destructors may call back here.
    GraphicsLayer::UnparentAndClear(_rootContentsLayer);

    GraphicsLayer::UnparentAndClear(_clipLayer);
    GraphicsLayer::UnparentAndClear(_scrollContainerLayer);
    GraphicsLayer::UnparentAndClear(_scrolledContentsLayer);

    GraphicsLayer::UnparentAndClear(_overflowControlsHostLayer);

    krys_debug_assert(_rootLayerAttachment == RootLayerUnattached);
  }

  void RenderLayerCompositor::EnableCompositingMode(bool enable) noexcept
  {
    if (enable != _compositing)
    {
      _compositing = enable;

      if (_compositing)
      {
        EnsureRootLayer();
      }
      else
      {
        DestroyRootLayer();
      }

      _renderView.Layer()->SetNeedsPostLayoutCompositingUpdate();
    }
  }

  void RenderLayerCompositor::CacheAcceleratedCompositingFlags() noexcept
  {
    // FIXME: The settings (acceleratedCompositingEnabled, showDebugBorders, showRepaintCounter,
    // acceleratedDrawingEnabled) and the chrome's compositing triggers, once there are settings: compositing
    // is on whenever the host makes graphics layers, as forceCompositingMode has it.
    bool hasAcceleratedCompositing = GetGraphicsLayerFactory() != nullptr;
    bool showDebugBorders = false;
    bool showRepaintCounter = false;
    bool acceleratedDrawingEnabled = false;

    // forceCompositingMode for subframes can only be computed after layout.
    bool forceCompositingMode = _forceCompositingMode;
    if (IsRootFrameCompositor())
    {
      forceCompositingMode = hasAcceleratedCompositing;
    }

    if (hasAcceleratedCompositing != _hasAcceleratedCompositing || showDebugBorders != _showDebugBorders
        || showRepaintCounter != _showRepaintCounter || forceCompositingMode != _forceCompositingMode)
    {
      if (auto *rootLayer = _renderView.Layer())
      {
        rootLayer->SetNeedsCompositingConfigurationUpdate();
        rootLayer->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
      }
    }

    _hasAcceleratedCompositing = hasAcceleratedCompositing;
    _forceCompositingMode = forceCompositingMode;
    _showDebugBorders = showDebugBorders;
    _showRepaintCounter = showRepaintCounter;
    _acceleratedDrawingEnabled = acceleratedDrawingEnabled;
    // Fixed position layers composite whenever the page does, as the compositing drawing areas set it.
    _acceleratedCompositingForFixedPositionEnabled = hasAcceleratedCompositing;

    // FIXME: The compositing policy under memory pressure (updateCompositingPolicy), once there's memory
    // pressure handling: it's always normal.
  }

  void RenderLayerCompositor::CacheAcceleratedCompositingFlagsAfterLayout() noexcept
  {
    CacheAcceleratedCompositingFlags();

    // FIXME: Subframes' forceCompositingMode (requiresCompositingForScrollableFrame), once there are
    // subframes.
  }

  bool RenderLayerCompositor::CanRender3DTransforms() const noexcept
  {
    // FIXME: 3D transforms, once Render2D draws in 3D: they're flattened, so they don't need compositing.
    return false;
  }

  void RenderLayerCompositor::WillRecalcStyle() noexcept
  {
    CacheAcceleratedCompositingFlags();
  }

  bool RenderLayerCompositor::DidRecalcStyleWithNoPendingLayout() noexcept
  {
    return UpdateCompositingLayers(CompositingUpdateType::AfterStyleChange);
  }

  bool RenderLayerCompositor::ShouldDumpPropertyForLayer(const GraphicsLayer *layer,
                                                         utf8_stringview propertyName,
                                                         EnumSet<LayerTreeAsTextOptions>) const noexcept
  {
    if (propertyName == u8"anchorPoint")
    {
      return layer->AnchorPoint() != FloatPoint3D(0.5f, 0.5f, 0);
    }

    return true;
  }

  bool RenderLayerCompositor::BackdropRootIsOpaque(const GraphicsLayer *layer) const noexcept
  {
    if (layer != RootGraphicsLayer())
    {
      return false;
    }

    return !ViewHasTransparentBackground();
  }

  void RenderLayerCompositor::NotifyFlushRequired(const GraphicsLayer *) noexcept
  {
    ScheduleRenderingUpdate();
  }

  void RenderLayerCompositor::ScheduleRenderingUpdate() noexcept
  {
    krys_debug_assert(!_flushingLayers);

    if (auto *page = _renderView.GetDocument().GetPage())
    {
      page->ScheduleRenderingUpdate({RenderingUpdateStep::LayerFlush});
    }
  }

  FloatRect RenderLayerCompositor::VisibleRectForLayerFlushing() const noexcept
  {
    const auto &frameView = _renderView.FrameView();

    // Having a _scrolledContentsLayer indicates that we're doing scrolling via GraphicsLayers.
    FloatRect visibleRect = _scrolledContentsLayer
                              ? FloatRect({}, FloatSize(frameView.SizeForVisibleContent()))
                              : FloatRect(frameView.VisibleContentRect());

    // FIXME: The view's exposed rect (viewExposedRect), once hosts can expose part of the view.
    return visibleRect;
  }

  void RenderLayerCompositor::FlushPendingLayerChanges(bool isFlushRoot) noexcept
  {
    // LocalFrameView::flushCompositingStateIncludingSubframes() flushes each subframe, but
    // GraphicsLayer::flushCompositingState() will cross frame boundaries if the GraphicsLayers are connected
    // (the RootLayerAttachedViaEnclosingFrame case). As long as we're not the root of the flush, we can bail.
    if (!isFlushRoot && GetRootLayerAttachment() == RootLayerAttachedViaEnclosingFrame)
    {
      return;
    }

    if (GetRootLayerAttachment() == RootLayerUnattached)
    {
      _shouldFlushOnReattach = true;
      return;
    }

    krys_debug_assert(!_flushingLayers);
    {
      SetForScope flushingLayersScope(_flushingLayers, true);

      if (RefPtr rootLayer = ShareRefPtr(RootGraphicsLayer()))
      {
        FloatRect visibleRect = VisibleRectForLayerFlushing();
        rootLayer->FlushCompositingState(visibleRect);
      }

      krys_debug_assert(_flushingLayers);
    }

    ++_layerFlushCount;
  }

  void RenderLayerCompositor::SetRenderingIsSuppressed(bool suppressed) noexcept
  {
    if (auto *rootLayer = RootGraphicsLayer())
    {
      rootLayer->SetRenderingIsSuppressedIncludingDescendants(suppressed);
    }
  }

  void RenderLayerCompositor::DidChangePlatformLayerForLayer(RenderLayer &, const GraphicsLayer *) noexcept
  {
    // FIXME: The scrolling coordinator's node layers, once there's async scrolling.
  }

  void RenderLayerCompositor::DidPaintBacking(RenderLayerBacking *) noexcept
  {
    // FIXME: Paint milestones (setLastPaintTime, firePaintRelatedMilestonesIfNeeded), once there are
    // milestones.
  }

  void RenderLayerCompositor::DidChangeVisibleRect() noexcept
  {
    auto *rootLayer = RootGraphicsLayer();
    if (!rootLayer)
    {
      return;
    }

    FloatRect visibleRect = VisibleRectForLayerFlushing();
    bool requiresFlush = rootLayer->VisibleRectChangeRequiresFlush(visibleRect);
    if (requiresFlush)
    {
      ScheduleRenderingUpdate();
    }
  }

  void RenderLayerCompositor::NotifySubsequentFlushRequired(const GraphicsLayer *) noexcept
  {
    if (!_updateRenderingTimer.IsActive())
    {
      _updateRenderingTimer.StartOneShot(MonotonicTime::duration::zero());
    }
  }

  void RenderLayerCompositor::ScheduleCompositingLayerUpdate() noexcept
  {
    if (!_updateCompositingLayersTimer.IsActive())
    {
      _updateCompositingLayersTimer.StartOneShot(MonotonicTime::duration::zero());
    }
  }

  void RenderLayerCompositor::UpdateCompositingLayersTimerFired() noexcept
  {
    UpdateCompositingLayers(CompositingUpdateType::AfterLayout);
  }

  void RenderLayerCompositor::CancelCompositingLayerUpdate() noexcept
  {
    _updateCompositingLayersTimer.Stop();
  }

  template <typename TApplyFunction>
  void
    RenderLayerCompositor::ApplyToCompositedLayerIncludingDescendants(RenderLayer &layer,
                                                                      const TApplyFunction &function) noexcept
  {
    if (layer.IsComposited())
    {
      function(layer);
    }
    for (auto *childLayer = layer.FirstChild(); childLayer; childLayer = childLayer->NextSibling())
    {
      ApplyToCompositedLayerIncludingDescendants(*childLayer, function);
    }
  }

  // Returns true on a successful update.
  bool RenderLayerCompositor::UpdateCompositingLayers(CompositingUpdateType updateType,
                                                      RenderLayer *updateRoot) noexcept
  {
    if (updateType == CompositingUpdateType::AfterStyleChange
        || updateType == CompositingUpdateType::AfterLayout)
    {
      CacheAcceleratedCompositingFlagsAfterLayout(); // Some flags (e.g. forceCompositingMode) depend on
                                                     // layout.
    }

    _updateCompositingLayersTimer.Stop();

    // FIXME: Documents whose visual updates aren't allowed (visualUpdatesAllowed), once visual updates can be
    // suppressed.

    // Avoid updating the layers with old values. Compositing layers will be updated after the layout is
    // finished. This happens when m_updateCompositingLayersTimer fires before layout is updated.
    if (_renderView.NeedsLayout())
    {
      return false;
    }

    // FIXME: Page overlays (pageOverlayController().overlayCount()), once there are page overlays.
    if (!_compositing && _forceCompositingMode)
    {
      EnableCompositingMode(true);
    }

    bool isPageScroll = !updateRoot || updateRoot == &RootRenderLayer();
    updateRoot = &RootRenderLayer();

    if (updateType == CompositingUpdateType::OnScroll
        || updateType == CompositingUpdateType::OnCompositedScroll)
    {
      // We only get here if we didn't scroll on the scrolling thread, so this update needs to re-position
      // viewport-constrained layers.
      if (isPageScroll)
      {
        for (const auto *renderer : _renderView.FrameView().ViewportConstrainedObjects())
        {
          if (auto *layer = renderer->Layer())
          {
            layer->SetNeedsCompositingGeometryUpdate();
          }
        }
      }

      // Scrolling can affect overlap. FIXME: avoid for page scrolling.
      updateRoot->SetDescendantsNeedCompositingRequirementsTraversal();
    }

    if (updateType == CompositingUpdateType::AfterLayout)
    {
      // Ensure that post-layout updates push new scroll position and viewport rects onto the root node.
      RootRenderLayer().SetNeedsScrollingTreeUpdate();
    }

    if (!updateRoot->HasDescendantNeedingCompositingRequirementsTraversal() && !_compositing)
    {
      return true;
    }

    if (!updateRoot->NeedsAnyCompositingTraversal())
    {
      return true;
    }

    ++_compositingUpdateCount;

    // FIXME: optimize root-only update.
    if (updateRoot->HasDescendantNeedingCompositingRequirementsTraversal()
        || updateRoot->NeedsCompositingRequirementsTraversal())
    {
      auto &rootLayer = RootRenderLayer();
      CompositingState compositingState(updateRoot);
      // FIXME: The overlappingBackingStoreProvidersEnabled setting, once there are settings: on, as
      // upstream's default.
      BackingSharingState backingSharingState(true);
      LayerOverlapMap overlapMap(rootLayer);

      ComputeCompositingRequirements(nullptr, rootLayer, overlapMap, compositingState, backingSharingState);
    }

    if (updateRoot->HasDescendantNeedingUpdateBackingOrHierarchyTraversal()
        || updateRoot->NeedsUpdateBackingOrHierarchyTraversal())
    {
      UpdateBackingTraversalState traversalState;
      List<Ref<GraphicsLayer>> childList;
      UpdateBackingAndHierarchy(*updateRoot, childList, traversalState);

      // FIXME: Document overlay layers (appendDocumentOverlayLayers), once there are page overlays.

      // Even when childList is empty, don't drop out of compositing mode if there are composited layers that
      // we didn't hit in our traversal (e.g. because of visibility:hidden).
      if (childList.empty() && !NeedsCompositingForContentOrOverlays())
      {
        DestroyRootLayer();
      }
      else if (_rootContentsLayer)
      {
        _rootContentsLayer->SetChildren(krys::move(childList));
      }
    }

    // FIXME: Only do if dirty.
    UpdateRootLayerPosition();

    if (_scrolledContentsLayer)
    {
      UpdateOverflowControlsLayers();
    }

    return true;
  }

  namespace
  {
    // Unchanged leaf compositing layers that clip their descendants can skip descendant traversal, since
    // their descendants can't contribute any new overlap to the map.
    KRYS_NODISCARD bool CanSkipComputeCompositingRequirementsForSubtree(const RenderLayer &layer,
                                                                        bool willBeComposited) noexcept
    {
      if (layer.NeedsCompositingRequirementsTraversal()
          || layer.HasDescendantNeedingCompositingRequirementsTraversal())
      {
        return false;
      }

      if (!layer.IsComposited() || !willBeComposited || layer.HasCompositingDescendant()
          || !layer.IsStackingContext())
      {
        return false;
      }

      return layer.Renderer().HasNonVisibleOverflow();
    }
  }

  bool RenderLayerCompositor::AllowBackingStoreDetachingForFixedPosition(
    RenderLayer &layer, const LayoutRect &absoluteBounds) noexcept
  {
    krys_debug_assert(layer.BehavesAsFixed());
    (void)layer;

    // We'll allow detaching if the layer is outside the layout viewport. Fixed layers inside the layout
    // viewport can be revealed by async scrolling, so we want to pin their backing store.
    const auto &frameView = _renderView.FrameView();
    LayoutRect fixedLayoutRect;
    if (frameView.UseFixedLayout())
    {
      fixedLayoutRect = _renderView.UnscaledDocumentRect();
    }
    else
    {
      fixedLayoutRect = frameView.RectForFixedPositionLayout();
    }

    return !fixedLayoutRect.Intersects(absoluteBounds);
  }

  void RenderLayerCompositor::ComputeCompositingRequirements(
    RenderLayer *ancestorLayer, RenderLayer &layer, LayerOverlapMap &overlapMap,
    CompositingState &compositingState, BackingSharingState &backingSharingState) noexcept
  {
    uint32 treeDepth = 0;

    layer.UpdateDescendantDependentFlags();
    layer.UpdateLayerListsIfNeeded();

    if (!layer.HasDescendantNeedingCompositingRequirementsTraversal()
        && !layer.NeedsCompositingRequirementsTraversal() && !compositingState.FullPaintOrderTraversalRequired
        && !compositingState.DescendantsRequireCompositingUpdate)
    {
      TraverseUnchangedSubtree(ancestorLayer, layer, overlapMap, compositingState, backingSharingState);
      return;
    }

    // FIXME: maybe we can avoid updating all remaining layers in paint order.
    compositingState.FullPaintOrderTraversalRequired |= layer.NeedsCompositingRequirementsTraversal();
    compositingState.DescendantsRequireCompositingUpdate |=
      layer.DescendantsNeedCompositingRequirementsTraversal();

    // We updated compositing for direct reasons in layerStyleChanged(). Here, check for compositing that can
    // only be evaluated after layout.
    RequiresCompositingData queryData;
    bool wasComposited = layer.IsComposited();
    bool willBeComposited = wasComposited;
    bool becameCompositedAfterDescendantTraversal = false;
    IndirectCompositingReason compositingReason = compositingState.SubtreeIsCompositing
                                                    ? IndirectCompositingReason::Stacking
                                                    : IndirectCompositingReason::None;

    if (layer.NeedsPostLayoutCompositingUpdate() || compositingState.FullPaintOrderTraversalRequired
        || compositingState.DescendantsRequireCompositingUpdate)
    {
      layer.SetIndirectCompositingReason(IndirectCompositingReason::None);
      willBeComposited = NeedsToBeComposited(layer, queryData);
    }

    compositingState.FullPaintOrderTraversalRequired |=
      layer.SubsequentLayersNeedCompositingRequirementsTraversal();

    OverlapExtent layerExtent;

    // Use the fact that we're composited as a hint to check for an animating transform.
    // FIXME: Maybe needsToBeComposited() should return a bitmask of reasons, to avoid the need to recompute
    // things.
    if (willBeComposited && !layer.IsRenderViewLayer())
    {
      layerExtent.HasTransformAnimation = IsRunningTransformAnimation(layer.Renderer());
    }

    bool respectTransforms = !layerExtent.HasTransformAnimation;
    overlapMap.GeometryMap().PushMappingsToAncestor(&layer, ancestorLayer, respectTransforms);

    RenderLayer *providedBackingLayer = nullptr;
    if (!willBeComposited && compositingState.SubtreeIsCompositing && CanBeComposited(layer))
    {
      if (auto *provider =
            backingSharingState.BackingProviderCandidateForLayer(layer, *this, overlapMap, layerExtent))
      {
        provider->SharingLayers.add(&layer);
        compositingReason = IndirectCompositingReason::None;
        providedBackingLayer = provider->ProviderLayer;
      }
    }

    // If we know for sure the layer is going to be composited, don't bother looking it up in the overlap map
    if (!willBeComposited && !providedBackingLayer && !overlapMap.IsEmpty()
        && compositingState.TestingOverlap)
    {
      // If we're testing for overlap, we only need to composite if we overlap something that is already
      // composited.
      if (LayerOverlaps(overlapMap, layer, layerExtent))
      {
        compositingReason = IndirectCompositingReason::Overlap;
      }
      else
      {
        compositingReason = IndirectCompositingReason::None;
      }
    }

    // FIXME: Video controls, which composite over accelerated video, once there's video.

    if (compositingReason != IndirectCompositingReason::None)
    {
      layer.SetIndirectCompositingReason(compositingReason);
    }

    // Check if the computed indirect reason will force the layer to become composited.
    if (!willBeComposited && layer.MustCompositeForIndirectReasons() && CanBeComposited(layer))
    {
      willBeComposited = true;
      providedBackingLayer = nullptr;
    }

    // The children of this layer don't need to composite, unless there is a compositing layer among them, so
    // start by inheriting the compositing ancestor with subtreeIsCompositing set to false.
    CompositingState currentState = compositingState.StateForPaintOrderChildren(layer);
    bool didPushOverlapContainer = false;

    auto layerWillComposite = [&]
    {
      // This layer is going to be composited, so children can safely ignore the fact that there's an
      // animation running behind this layer, meaning they can rely on the overlap map testing again.
      currentState.TestingOverlap = true;
      // This layer now acts as the ancestor for kids.
      currentState.CompositingAncestor = &layer;
      // Compositing turns off backing sharing.
      currentState.BackingSharingAncestor = nullptr;

      if (providedBackingLayer)
      {
        providedBackingLayer = nullptr;
        // providedBackingLayer was only valid for layers that would otherwise composite because of overlap.
        // If we can no longer share, put this this indirect reason back on the layer so that
        // requiresOwnBackingStore() sees it.
        layer.SetIndirectCompositingReason(IndirectCompositingReason::Overlap);
      }
      else
      {
        if (!didPushOverlapContainer)
        {
          overlapMap.PushCompositingContainer(layer);
          didPushOverlapContainer = true;
        }
      }

      willBeComposited = true;
    };

    // Unless we leave the containing block chain, or have an animated transform, then we can continue to use
    // the inherited backing store attachment.
    bool allowsBackingStoreDetachingForFixed = false;
    if (currentState.AncestorAllowsBackingStoreDetachingForFixed && ancestorLayer
        && layer.AncestorLayerIsInContainingBlockChain(*ancestorLayer) && !layerExtent.HasTransformAnimation)
    {
      allowsBackingStoreDetachingForFixed = true;
    }

    auto layerWillCompositePostDescendants = [&]
    {
      layerWillComposite();
      currentState.SubtreeIsCompositing = true;
      becameCompositedAfterDescendantTraversal = true;
      if (layer.BehavesAsFixed())
      {
        allowsBackingStoreDetachingForFixed =
          AllowBackingStoreDetachingForFixedPosition(layer, layerExtent.Bounds);
      }
    };

    if (willBeComposited)
    {
      layerWillComposite();

      ComputeExtent(overlapMap, layer, layerExtent);
      currentState.AncestorHasTransformAnimation |= layerExtent.HasTransformAnimation;

      if (!allowsBackingStoreDetachingForFixed && layer.BehavesAsFixed())
      {
        currentState.AncestorAllowsBackingStoreDetachingForFixed = allowsBackingStoreDetachingForFixed =
          AllowBackingStoreDetachingForFixedPosition(layer, layerExtent.Bounds);
      }

      // Too hard to compute animated bounds if both us and some ancestor is animating transform.
      layerExtent.AnimationCausesExtentUncertainty |=
        layerExtent.HasTransformAnimation && compositingState.AncestorHasTransformAnimation;
    }
    else if (providedBackingLayer)
    {
      currentState.BackingSharingAncestor = &layer;
      overlapMap.PushCompositingContainer(layer);
      didPushOverlapContainer = true;
    }

    auto backingSharingSnapshot = UpdateBackingSharingBeforeDescendantTraversal(
      backingSharingState, treeDepth, overlapMap, layer, layerExtent, willBeComposited,
      compositingState.StackingContextAncestor);

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(layer);
#endif

    bool descendantsAddedToOverlap = currentState.HasNonRootCompositedAncestor();

    if (!CanSkipComputeCompositingRequirementsForSubtree(layer, willBeComposited))
    {
      if (layer.HasNegativeZOrderLayers())
      {
        // Speculatively push this layer onto the overlap map.
        bool didSpeculativelyPushOverlapContainer = false;
        if (!didPushOverlapContainer)
        {
          overlapMap.PushSpeculativeCompositingContainer(layer);
          didPushOverlapContainer = true;
          didSpeculativelyPushOverlapContainer = true;
        }

        for (auto *childLayer : layer.NegativeZOrderLayers())
        {
          ComputeCompositingRequirements(&layer, *childLayer, overlapMap, currentState, backingSharingState);

          // If we have to make a layer for this child, make one now so we can have a contents layer (since we
          // need to ensure that the -ve z-order child renders underneath our contents).
          if (!willBeComposited && currentState.SubtreeIsCompositing)
          {
            layer.SetIndirectCompositingReason(IndirectCompositingReason::BackgroundLayer);
            layerWillComposite();
            overlapMap.ConfirmSpeculativeCompositingContainer();
          }
        }

        if (didSpeculativelyPushOverlapContainer)
        {
          if (overlapMap.MaybePopSpeculativeCompositingContainer())
          {
            didPushOverlapContainer = false;
          }
          else if (!willBeComposited)
          {
            layer.SetIndirectCompositingReason(IndirectCompositingReason::BackgroundLayer);
            layerWillComposite();
          }
        }
      }

      for (auto *childLayer : layer.NormalFlowLayers())
      {
        ComputeCompositingRequirements(&layer, *childLayer, overlapMap, currentState, backingSharingState);
      }

      for (auto *childLayer : layer.PositiveZOrderLayers())
      {
        ComputeCompositingRequirements(&layer, *childLayer, overlapMap, currentState, backingSharingState);
      }

      // Set the flag to say that this layer has compositing children.
      layer.SetHasCompositingDescendant(currentState.SubtreeIsCompositing);
      layer.SetHasCompositedNonContainedDescendants(currentState.HasCompositedNonContainedDescendants);
    }

    // If we just entered compositing mode, the root will have become composited (as long as accelerated
    // compositing is enabled).
    if (layer.IsRenderViewLayer())
    {
      if (UsesCompositing() && _hasAcceleratedCompositing)
      {
        willBeComposited = true;
      }
    }

    bool isolatedCompositedBlending = layer.IsolatesCompositedBlending();
    layer.SetHasNotIsolatedCompositedBlendingDescendants(
      currentState.HasNotIsolatedCompositedBlendingDescendants);
    if (layer.IsolatesCompositedBlending() != isolatedCompositedBlending)
    {
      // isolatedCompositedBlending affects the result of clippedByAncestor().
      layer.SetChildrenNeedCompositingGeometryUpdate();
    }

    krys_debug_assert(!layer.HasNotIsolatedCompositedBlendingDescendants()
                      || layer.HasNotIsolatedBlendingDescendants());

    bool isBackdropRoot = layer.IsBackdropRoot();
    layer.SetHasBackdropFilterDescendantsWithoutRoot(currentState.HasBackdropFilterDescendantsWithoutRoot);
    if (layer.IsBackdropRoot() != isBackdropRoot)
    {
      layer.SetNeedsCompositingConfigurationUpdate();
    }

    // Now check for reasons to become composited that depend on the state of descendant layers.
    if (!willBeComposited && CanBeComposited(layer))
    {
      layer.Update3DTransformedDescendantStatus();
      auto indirectReason =
        ComputeIndirectCompositingReason(layer, currentState.SubtreeIsCompositing,
                                         layer.Has3DTransformedDescendant(), providedBackingLayer != nullptr);
      if (indirectReason != IndirectCompositingReason::None)
      {
        layer.SetIndirectCompositingReason(indirectReason);
        layerWillCompositePostDescendants();
      }
    }

    // FIXME: Reflections (reflectionLayer), once there are reflections.

    // If we're back at the root, and no other layers need to be composited, and the root layer itself doesn't
    // need to be composited, then we can drop out of compositing mode altogether. However, don't drop out of
    // compositing mode if there are composited layers that we didn't hit in our traversal (e.g. because of
    // visibility:hidden).
    RequiresCompositingData rootLayerQueryData;
    if (layer.IsRenderViewLayer() && !currentState.SubtreeIsCompositing
        && !RequiresCompositingLayer(layer, rootLayerQueryData) && !_forceCompositingMode
        && !NeedsCompositingForContentOrOverlays())
    {
      EnableCompositingMode(false);
      willBeComposited = false;
    }

    krys_debug_assert(willBeComposited == NeedsToBeComposited(layer, queryData));

    // Create or destroy backing here. However, we can't update geometry because layers above us may become
    // composited during post-order traversal (e.g. for clipping).
    bool needsCompositingStatusUpdate = false;
    if (UpdateBacking(layer, queryData, &backingSharingState,
                      willBeComposited ? BackingRequired::Yes : BackingRequired::No))
    {
      // This layer and all of its descendants have cached repaints rects that are relative to the repaint
      // container, so change when compositing changes; we need to update them here, as long as shared backing
      // isn't going to change our repaint container.
      needsCompositingStatusUpdate = true;
    }

    // FIXME: clarify needsCompositingPaintOrderChildrenUpdate. If a composited layer gets a new ancestor, it
    // needs geometry computations.
    if (layer.NeedsCompositingPaintOrderChildrenUpdate())
    {
      layer.SetChildrenNeedCompositingGeometryUpdate();
      layer.SetNeedsCompositingLayerConnection();
    }

    layer.ClearCompositingRequirementsTraversalState();

    // Compute state passed to the caller.
    compositingState.UpdateWithDescendantStateAndLayer(currentState, layer, ancestorLayer, layerExtent);
    UpdateBackingSharingAfterDescendantTraversal(backingSharingState, treeDepth, overlapMap, layer,
                                                 layerExtent, compositingState.StackingContextAncestor,
                                                 backingSharingSnapshot);

    needsCompositingStatusUpdate |=
      layer.BackingProviderLayerAtEndOfCompositingUpdate() != providedBackingLayer;

    // Update the cached repaint rects now that we've finished updating backing sharing state on descendants
    if (needsCompositingStatusUpdate)
    {
      // Repaint in the old container before we recompute the repaint container.
      if (!wasComposited && layer.RepaintContainer() && layer.RepaintContainer()->IsComposited())
      {
        RepaintOnCompositingChange(layer, layer.RepaintContainer());
      }

      // Compute the new repaint container, and repaint our bounds in it (unless this layer is newly
      // compositing, in which case the layer will fully repaint already).
      if (!layer.IsComposited())
      {
        // If this layer is going to participate in backing sharing, defer until that's complete, since
        // repaint container computation depends on all the state being in-place.
        if (LayerRepaintTargetsBackingSharingLayer(layer, backingSharingState))
        {
          backingSharingState.AddLayerNeedingRepaint(layer);
        }
        else
        {
          layer.CompositingStatusChanged(LayoutUpToDate::Yes);
          RepaintOnCompositingChange(layer, layer.RepaintContainer());
        }
      }
      else
      {
        layer.CompositingStatusChanged(LayoutUpToDate::Yes);
      }
    }

    layer.SetBackingProviderLayerAtEndOfCompositingUpdate(providedBackingLayer);

    bool layerContributesToOverlap =
      (currentState.CompositingAncestor && !currentState.CompositingAncestor->IsRenderViewLayer())
      || currentState.BackingSharingAncestor;
    UpdateOverlapMap(overlapMap, layer, layerExtent, didPushOverlapContainer, layerContributesToOverlap,
                     becameCompositedAfterDescendantTraversal && !descendantsAddedToOverlap);

    if (layer.IsComposited())
    {
      layer.Backing()->UpdateAllowsBackingStoreDetaching(allowsBackingStoreDetachingForFixed);
    }

    overlapMap.GeometryMap().PopMappingsToAncestor(ancestorLayer);
  }

  // We have to traverse unchanged layers to fill in the overlap map.
  void RenderLayerCompositor::TraverseUnchangedSubtree(RenderLayer *ancestorLayer, RenderLayer &layer,
                                                       LayerOverlapMap &overlapMap,
                                                       CompositingState &compositingState,
                                                       BackingSharingState &backingSharingState) noexcept
  {
    uint32 treeDepth = 0;

    layer.UpdateDescendantDependentFlags();
    layer.UpdateLayerListsIfNeeded();

    krys_debug_assert(!compositingState.FullPaintOrderTraversalRequired);
    krys_debug_assert(!layer.HasDescendantNeedingCompositingRequirementsTraversal());
    krys_debug_assert(!layer.NeedsCompositingRequirementsTraversal());

    bool layerIsComposited = layer.IsComposited();
    bool layerPaintsIntoProvidedBacking = false;
    bool didPushOverlapContainer = false;

    OverlapExtent layerExtent;
    if (layerIsComposited && !layer.IsRenderViewLayer())
    {
      layerExtent.HasTransformAnimation = IsRunningTransformAnimation(layer.Renderer());
    }

    bool respectTransforms = !layerExtent.HasTransformAnimation;
    overlapMap.GeometryMap().PushMappingsToAncestor(&layer, ancestorLayer, respectTransforms);

    // If we know for sure the layer is going to be composited, don't bother looking it up in the overlap map
    if (!layerIsComposited && !overlapMap.IsEmpty() && compositingState.TestingOverlap)
    {
      ComputeExtent(overlapMap, layer, layerExtent);
    }

    if (layer.PaintsIntoProvidedBacking())
    {
      auto *provider = backingSharingState.ExistingBackingProviderCandidateForLayer(layer);
      krys_assert(provider);
      provider->SharingLayers.add(&layer);
      layerPaintsIntoProvidedBacking = true;
    }

    CompositingState currentState = compositingState.StateForPaintOrderChildren(layer);

    if (layerIsComposited)
    {
      // This layer is going to be composited, so children can safely ignore the fact that there's an
      // animation running behind this layer, meaning they can rely on the overlap map testing again.
      currentState.TestingOverlap = true;
      // This layer now acts as the ancestor for kids.
      currentState.CompositingAncestor = &layer;
      currentState.BackingSharingAncestor = nullptr;
      overlapMap.PushCompositingContainer(layer);
      didPushOverlapContainer = true;

      ComputeExtent(overlapMap, layer, layerExtent);
      currentState.AncestorHasTransformAnimation |= layerExtent.HasTransformAnimation;
      // Too hard to compute animated bounds if both us and some ancestor is animating transform.
      layerExtent.AnimationCausesExtentUncertainty |=
        layerExtent.HasTransformAnimation && compositingState.AncestorHasTransformAnimation;
    }
    else if (layerPaintsIntoProvidedBacking)
    {
      overlapMap.PushCompositingContainer(layer);
      currentState.BackingSharingAncestor = &layer;
      didPushOverlapContainer = true;
    }

    auto backingSharingSnapshot = UpdateBackingSharingBeforeDescendantTraversal(
      backingSharingState, treeDepth, overlapMap, layer, layerExtent, layerIsComposited,
      compositingState.StackingContextAncestor);

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(layer);
#endif

    if (!CanSkipComputeCompositingRequirementsForSubtree(layer, layerIsComposited))
    {
      for (auto *childLayer : layer.NegativeZOrderLayers())
      {
        TraverseUnchangedSubtree(&layer, *childLayer, overlapMap, currentState, backingSharingState);
        if (currentState.SubtreeIsCompositing)
        {
          krys_debug_assert(layerIsComposited);
        }
      }

      for (auto *childLayer : layer.NormalFlowLayers())
      {
        TraverseUnchangedSubtree(&layer, *childLayer, overlapMap, currentState, backingSharingState);
      }

      for (auto *childLayer : layer.PositiveZOrderLayers())
      {
        TraverseUnchangedSubtree(&layer, *childLayer, overlapMap, currentState, backingSharingState);
      }

      // Set the flag to say that this layer has compositing children.
      krys_debug_assert(layer.HasCompositingDescendant() == currentState.SubtreeIsCompositing);
    }

    krys_debug_assert(!currentState.FullPaintOrderTraversalRequired);
    compositingState.UpdateWithDescendantStateAndLayer(currentState, layer, ancestorLayer, layerExtent, true);
    UpdateBackingSharingAfterDescendantTraversal(backingSharingState, treeDepth, overlapMap, layer,
                                                 layerExtent, compositingState.StackingContextAncestor,
                                                 backingSharingSnapshot);

    bool layerContributesToOverlap =
      (currentState.CompositingAncestor && !currentState.CompositingAncestor->IsRenderViewLayer())
      || currentState.BackingSharingAncestor;
    UpdateOverlapMap(overlapMap, layer, layerExtent, didPushOverlapContainer, layerContributesToOverlap);

    overlapMap.GeometryMap().PopMappingsToAncestor(ancestorLayer);

    krys_debug_assert(!layer.NeedsCompositingRequirementsTraversal());
  }

  void RenderLayerCompositor::UpdateBackingAndHierarchy(RenderLayer &layer,
                                                        List<Ref<GraphicsLayer>> &childLayersOfEnclosingLayer,
                                                        UpdateBackingTraversalState &traversalState,
                                                        EnumSet<UpdateLevel> updateLevel) noexcept
  {
    layer.UpdateDescendantDependentFlags();
    layer.UpdateLayerListsIfNeeded();

    bool layerNeedsUpdate = !updateLevel.IsEmpty();
    if (layer.DescendantsNeedUpdateBackingAndHierarchyTraversal())
    {
      updateLevel.Add(UpdateLevel::AllDescendants);
    }

    UpdateBackingTraversalState traversalStateForDescendants = traversalState.StateForDescendants();
    List<RenderLayer *> layersClippedByScrollers;
    List<RenderLayer *> compositedOverflowScrollLayers;

    auto *layerBacking = layer.Backing();
    if (layerBacking)
    {
      updateLevel.Remove(UpdateLevel::CompositedChildren);

      // We updated the composited bounds in RenderLayerBacking::updateAfterLayout(), but it may have changed
      // based on which descendants are now composited.
      if (layerBacking->UpdateCompositedBounds())
      {
        layer.SetNeedsCompositingGeometryUpdate();
        // Our geometry can affect descendants.
        updateLevel.Add(UpdateLevel::CompositedChildren);
      }

      if (layerNeedsUpdate || layer.NeedsCompositingConfigurationUpdate())
      {
        if (layerBacking->UpdateConfiguration(traversalState.CompositingAncestor))
        {
          layerNeedsUpdate = true; // We also need to update geometry.
          layer.SetNeedsCompositingLayerConnection();
        }

        layerBacking->UpdateDebugIndicators(_showDebugBorders, _showRepaintCounter);
      }

      if (layerNeedsUpdate || layer.NeedsCompositingGeometryUpdate())
      {
        layerBacking->UpdateGeometry(traversalState.CompositingAncestor);
      }

      // FIXME: Reflections (reflectionLayer), once there are reflections.

      if (!layer.Parent())
      {
        UpdateRootLayerPosition();
      }

      // FIXME: The scrolling tree (updateScrollCoordinationForLayer), once there's async scrolling.

      traversalStateForDescendants.CompositingAncestor = &layer;
      traversalStateForDescendants.LayersClippedByScrollers = &layersClippedByScrollers;
      traversalStateForDescendants.OverflowScrollLayers = &compositedOverflowScrollLayers;
    }

    if (layer.ChildrenNeedCompositingGeometryUpdate())
    {
      updateLevel.Add(UpdateLevel::CompositedChildren);
    }

    // If this layer has backing, then we are collecting its children, otherwise appending to the compositing
    // child list of an enclosing layer.
    List<Ref<GraphicsLayer>> layerChildren;
    auto &childList = layerBacking ? layerChildren : childLayersOfEnclosingLayer;

    bool requireDescendantTraversal =
      layer.HasDescendantNeedingUpdateBackingOrHierarchyTraversal()
      || (layer.HasCompositingDescendant()
          && (!layerBacking || layer.NeedsCompositingLayerConnection() || !updateLevel.IsEmpty()));

    bool requiresChildRebuild =
      layerBacking && layer.NeedsCompositingLayerConnection() && !layer.HasCompositingDescendant();

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(layer);
#endif

    auto appendForegroundLayerIfNecessary = [&]
    {
      // If a negative z-order child is compositing, we get a foreground layer which needs to get parented.
      if (layer.NegativeZOrderLayers().size())
      {
        if (layerBacking && layerBacking->ForegroundLayer())
        {
          childList.push_back(ShareRef(*layerBacking->ForegroundLayer()));
        }
      }
    };

    if (requireDescendantTraversal)
    {
      for (auto *renderLayer : layer.NegativeZOrderLayers())
      {
        UpdateBackingAndHierarchy(*renderLayer, childList, traversalStateForDescendants, updateLevel);
      }

      appendForegroundLayerIfNecessary();

      for (auto *renderLayer : layer.NormalFlowLayers())
      {
        UpdateBackingAndHierarchy(*renderLayer, childList, traversalStateForDescendants, updateLevel);
      }

      for (auto *renderLayer : layer.PositiveZOrderLayers())
      {
        UpdateBackingAndHierarchy(*renderLayer, childList, traversalStateForDescendants, updateLevel);
      }
    }
    else if (requiresChildRebuild)
    {
      appendForegroundLayerIfNecessary();
    }

    if (layerBacking)
    {
      if (requireDescendantTraversal || requiresChildRebuild)
      {
        // FIXME: Widgets' content layers (attachWidgetContentLayersIfNecessary), once there are iframes and
        // plug-ins; view transitions' new content layers (collectViewTransitionNewContentLayers), once there
        // are view transitions.

        // If the layer has a clipping layer the overflow controls layers will be siblings of the clipping
        // layer. Otherwise, the overflow control layers are normal children.
        if (!layerBacking->HasClippingLayer() && !layerBacking->HasScrollingLayer())
        {
          if (auto *overflowControlLayer = layerBacking->OverflowControlsContainer())
          {
            layerChildren.push_back(ShareRef(*overflowControlLayer));
          }
        }

        AdjustOverflowScrollbarContainerLayers(layer, compositedOverflowScrollLayers,
                                               layersClippedByScrollers, layerChildren);
        layerBacking->ParentForSublayers()->SetChildren(krys::move(layerChildren));
      }

      childLayersOfEnclosingLayer.push_back(ShareRef(*layerBacking->ChildForSuperlayers()));

      if (layerBacking->HasAncestorClippingLayers()
          && layerBacking->AncestorClippingStack()->HasAnyScrollingLayers())
      {
        traversalState.LayersClippedByScrollers->push_back(&layer);
      }

      if (layer.HasCompositedScrollableOverflow())
      {
        traversalState.OverflowScrollLayers->push_back(&layer);
      }

      layerBacking->UpdateAfterDescendants();
    }

    layer.ClearUpdateBackingOrHierarchyTraversalState();
  }

  auto RenderLayerCompositor::UpdateBackingSharingBeforeDescendantTraversal(
    BackingSharingState &sharingState, uint32, const LayerOverlapMap &overlapMap, RenderLayer &layer,
    OverlapExtent &layerExtent, bool willBeComposited, RenderLayer *stackingContextAncestor) noexcept
    -> Maybe<BackingSharingSnapshot>
  {
    layer.SetBackingProviderLayer(nullptr, {UpdateBackingSharingFlags::DuringCompositingUpdate});

    auto shouldEndSharingSequence = [&]
    {
      if (!sharingState.BackingSharingStackingContext())
      {
        return false;
      }

      if (!willBeComposited)
      {
        return false;
      }

      // If this layer is composited, we can only continue the sequence if it's a new provider candidate.
      ComputeExtent(overlapMap, layer, layerExtent);
      return !sharingState.IsAdditionalProviderCandidate(layer, layerExtent.Bounds, stackingContextAncestor);
    }();

    // A layer that composites resets backing-sharing, since subsequent layers need to composite to overlap
    // it.
    if (shouldEndSharingSequence)
    {
      sharingState.EndBackingSharingSequence(layer);
    }

    return sharingState.Snapshot();
  }

  void RenderLayerCompositor::UpdateBackingSharingAfterDescendantTraversal(
    BackingSharingState &sharingState, uint32, const LayerOverlapMap &overlapMap, RenderLayer &layer,
    OverlapExtent &layerExtent, RenderLayer *stackingContextAncestor,
    const Maybe<BackingSharingSnapshot> &backingSharingSnapshot) noexcept
  {
    if (layer.IsComposited())
    {
      // If this layer is being composited, clean up sharing-related state.
      layer.DisconnectFromBackingProviderLayer({UpdateBackingSharingFlags::DuringCompositingUpdate});
      for (auto &candidate : sharingState.BackingProviderCandidates())
      {
        candidate.SharingLayers.remove(&layer);
      }
    }

    // Backing sharing is constrained to layers in the same stacking context.
    if (&layer == sharingState.BackingSharingStackingContext())
    {
      sharingState.EndBackingSharingSequence(layer);

      if (layer.IsComposited())
      {
        layer.Backing()->ClearBackingSharingLayers({UpdateBackingSharingFlags::DuringCompositingUpdate});
      }

      return;
    }

    if (!layer.IsComposited())
    {
      return;
    }

    if (!stackingContextAncestor)
    {
      return;
    }

    bool canBeBackingProvider = !layer.HasCompositingDescendant();
    if (canBeBackingProvider)
    {
      if (!sharingState.BackingSharingStackingContext())
      {
        ComputeExtent(overlapMap, layer, layerExtent);
        sharingState.StartBackingSharingSequence(layer, layerExtent.Bounds, *stackingContextAncestor);
        return;
      }

      ComputeExtent(overlapMap, layer, layerExtent);
      if (sharingState.IsAdditionalProviderCandidate(layer, layerExtent.Bounds, stackingContextAncestor))
      {
        sharingState.AddBackingSharingCandidate(layer, layerExtent.Bounds, *stackingContextAncestor,
                                                backingSharingSnapshot);
        return;
      }
    }

    layer.Backing()->ClearBackingSharingLayers({UpdateBackingSharingFlags::DuringCompositingUpdate});

    // A layer that composites resets backing-sharing, since subsequent layers need to composite to overlap
    // it. If a descendant didn't already end the sharing sequence that was current when processing of this
    // layer started, end it now.
    if (backingSharingSnapshot
        && backingSharingSnapshot->SequenceIdentifier == sharingState.SequenceIdentifier())
    {
      sharingState.EndBackingSharingSequence(layer);
    }
  }

  // Finds the set of overflow:scroll layers whose overflow controls hosting layer needs to be reparented, to
  // ensure that the scrollbars show on top of positioned content inside the scroller.
  void RenderLayerCompositor::AdjustOverflowScrollbarContainerLayers(
    RenderLayer &stackingContextLayer, const List<RenderLayer *> &overflowScrollLayers,
    const List<RenderLayer *> &layersClippedByScrollers, List<Ref<GraphicsLayer>> &layerChildren) noexcept
  {
    if (layersClippedByScrollers.empty())
    {
      return;
    }

    HashMap<RenderLayer *, RenderLayer *> overflowScrollToLastContainedLayerMap;

    for (auto *clippedLayer : layersClippedByScrollers)
    {
      auto *clippingStack = clippedLayer->Backing()->AncestorClippingStack();

      for (const auto &stackEntry : clippingStack->Stack())
      {
        if (!stackEntry.ClipData.IsOverflowScroll)
        {
          continue;
        }

        if (auto *layer = stackEntry.ClipData.ClippingLayer.get())
        {
          overflowScrollToLastContainedLayerMap.insert_or_assign(layer, clippedLayer);
        }
      }
    }

    for (auto *overflowScrollingLayer : overflowScrollLayers)
    {
      auto it = overflowScrollToLastContainedLayerMap.find(overflowScrollingLayer);
      if (it == overflowScrollToLastContainedLayerMap.end())
      {
        continue;
      }

      auto *lastContainedDescendant = it->second;
      if (!lastContainedDescendant || !lastContainedDescendant->IsComposited())
      {
        continue;
      }

      auto *lastContainedDescendantBacking = lastContainedDescendant->Backing();
      auto *overflowBacking = overflowScrollingLayer->Backing();
      if (!overflowBacking)
      {
        continue;
      }

      RefPtr<GraphicsLayer> overflowContainerLayer =
        ShareRefPtr(overflowBacking->OverflowControlsContainer());
      if (!overflowContainerLayer)
      {
        continue;
      }

      overflowContainerLayer->RemoveFromParent();

      if (overflowBacking->HasAncestorClippingLayers())
      {
        overflowBacking->EnsureOverflowControlsHostLayerAncestorClippingStack(&stackingContextLayer);
      }

      if (auto *overflowControlsAncestorClippingStack =
            overflowBacking->OverflowControlsHostLayerAncestorClippingStack())
      {
        List<Ref<GraphicsLayer>> children;
        children.push_back(ShareRef(*overflowContainerLayer));
        overflowControlsAncestorClippingStack->LastLayer()->SetChildren(krys::move(children));
        overflowContainerLayer = ShareRefPtr(overflowControlsAncestorClippingStack->FirstLayer());
      }

      auto *lastDescendantGraphicsLayer = lastContainedDescendantBacking->ChildForSuperlayers();
      auto *overflowScrollerGraphicsLayer = overflowBacking->ChildForSuperlayers();

      Maybe<size_t> lastDescendantLayerIndex;
      Maybe<size_t> scrollerLayerIndex;
      for (size_t i = 0; i < layerChildren.size(); ++i)
      {
        const GraphicsLayer *graphicsLayer = layerChildren[i].get();
        if (graphicsLayer == lastDescendantGraphicsLayer)
        {
          lastDescendantLayerIndex = i;
        }
        else if (graphicsLayer == overflowScrollerGraphicsLayer)
        {
          scrollerLayerIndex = i;
        }
      }

      if (lastDescendantLayerIndex && scrollerLayerIndex)
      {
        auto insertionIndex = std::max(*lastDescendantLayerIndex + 1, *scrollerLayerIndex + 1);
        layerChildren.insert(layerChildren.begin() + static_cast<ptrdiff_t>(insertionIndex),
                             ShareRef(*overflowContainerLayer));
      }

      overflowBacking->AdjustOverflowControlsPositionRelativeToAncestor(stackingContextLayer);
    }
  }

  bool RenderLayerCompositor::NeedsCompositingForContentOrOverlays() const noexcept
  {
    // FIXME: Page overlays, once there are any.
    return _contentLayersCount;
  }

  void RenderLayerCompositor::LayerBecameComposited(const RenderLayer &layer) noexcept
  {
    if (&layer != _renderView.Layer())
    {
      ++_contentLayersCount;
    }
  }

  void RenderLayerCompositor::LayerBecameNonComposited(const RenderLayer &layer) noexcept
  {
    if (&layer != _renderView.Layer())
    {
      krys_debug_assert(_contentLayersCount > 0);
      --_contentLayersCount;
    }
  }

  namespace
  {
    KRYS_NODISCARD bool ClippingChanged(const RenderStyle &oldStyle, const RenderStyle &newStyle) noexcept
    {
      return oldStyle.OverflowX() != newStyle.OverflowX() || oldStyle.OverflowY() != newStyle.OverflowY()
             || oldStyle.GetClip() != newStyle.GetClip();
    }

    KRYS_NODISCARD bool StyleAffectsLayerGeometry(const RenderStyle &style) noexcept
    {
      return style.HasClip() || style.HasClipPath() || style.HasBorderRadius();
    }

    KRYS_NODISCARD bool RecompositeChangeRequiresGeometryUpdate(const RenderStyle &oldStyle,
                                                                const RenderStyle &newStyle) noexcept
    {
      return oldStyle.GetTransform() != newStyle.GetTransform()
             || oldStyle.GetTranslate() != newStyle.GetTranslate()
             || oldStyle.GetScale() != newStyle.GetScale() || oldStyle.GetRotate() != newStyle.GetRotate()
             || oldStyle.GetTransformBox() != newStyle.GetTransformBox()
             || oldStyle.GetTransformOriginX() != newStyle.GetTransformOriginX()
             || oldStyle.GetTransformOriginY() != newStyle.GetTransformOriginY()
             || oldStyle.GetTransformOriginZ() != newStyle.GetTransformOriginZ()
             || oldStyle.UsedTransformStyle3D() != newStyle.UsedTransformStyle3D()
             || oldStyle.GetPerspective() != newStyle.GetPerspective()
             || oldStyle.GetPerspectiveOrigin() != newStyle.GetPerspectiveOrigin()
             || oldStyle.GetBackfaceVisibility() != newStyle.GetBackfaceVisibility()
             || oldStyle.GetOffsetPath() != newStyle.GetOffsetPath()
             || oldStyle.GetOffsetAnchor() != newStyle.GetOffsetAnchor()
             || oldStyle.GetOffsetPosition() != newStyle.GetOffsetPosition()
             || oldStyle.GetOffsetDistance() != newStyle.GetOffsetDistance()
             || oldStyle.GetOffsetRotate() != newStyle.GetOffsetRotate()
             || oldStyle.GetClipPath() != newStyle.GetClipPath()
             || oldStyle.OverscrollBehaviorX() != newStyle.OverscrollBehaviorX()
             || oldStyle.OverscrollBehaviorY() != newStyle.OverscrollBehaviorY();
    }

    KRYS_NODISCARD bool RecompositeChangeRequiresChildrenGeometryUpdate(const RenderStyle &oldStyle,
                                                                        const RenderStyle &newStyle) noexcept
    {
      return oldStyle.HasPerspective() != newStyle.HasPerspective()
             || oldStyle.UsedTransformStyle3D() != newStyle.UsedTransformStyle3D();
    }
  }

  void RenderLayerCompositor::LayerGainedCompositedScrollableOverflow(RenderLayer &layer) noexcept
  {
    RequiresCompositingData queryData;
    queryData.LayoutUpToDate = LayoutUpToDate::No;

    UpdateExplicitBacking(layer, queryData, BackingRequired::Yes);

    auto *backing = layer.Backing();
    if (!backing)
    {
      return;
    }

    backing->UpdateConfigurationAfterStyleChange();
  }

  void RenderLayerCompositor::LayerStyleChanged(style::Difference diff, RenderLayer &layer,
                                                const RenderStyle *oldStyle) noexcept
  {
    if (diff == style::DifferenceResult::Equal)
    {
      return;
    }

    // Create or destroy backing here so that code that runs during layout can reliably use isComposited()
    // (though this is only true for layers composited for direct reasons). Also, it allows us to avoid a tree
    // walk in updateCompositingLayers() when no layer changed its compositing state.
    RequiresCompositingData queryData;
    queryData.LayoutUpToDate = LayoutUpToDate::No;

    UpdateExplicitBacking(layer, queryData);
    layer.SetIntrinsicallyComposited(queryData.Intrinsic);

    if (queryData.ReevaluateAfterLayout)
    {
      layer.SetNeedsPostLayoutCompositingUpdate();
    }

    const auto &newStyle = layer.Renderer().Style();

    if (HasContentCompositingLayers())
    {
      if (diff >= style::DifferenceResult::LayoutOutOfFlowMovementOnly)
      {
        layer.SetNeedsPostLayoutCompositingUpdate();
        layer.SetNeedsCompositingGeometryUpdate();
      }

      if (diff >= style::DifferenceResult::Layout)
      {
        // FIXME: only set flags here if we know we have a composited descendant, but we might not know at
        // this point.
        if (oldStyle && ClippingChanged(*oldStyle, newStyle))
        {
          if (layer.IsStackingContext())
          {
            // Layer needs to become composited if it has composited descendants.
            layer.SetNeedsPostLayoutCompositingUpdate();
            // If already composited, layer needs to create/destroy clipping layer.
            layer.SetNeedsCompositingConfigurationUpdate();
            // Clipping layers on this layer affect descendant layer geometry.
            layer.SetChildrenNeedCompositingGeometryUpdate();
          }
          else
          {
            // Descendant (in containing block order) compositing layers need to re-evaluate their clipping,
            // but they might be siblings in z-order so go up to our stacking context.
            if (auto *stackingContext = layer.StackingContext())
            {
              stackingContext->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();
            }
          }
        }

        // This ensures that the viewport anchor layer will be updated when updating compositing layers upon
        // style change
        auto styleChangeAffectsAnchorLayer = [](const RenderStyle *oldStyle, const RenderStyle &newStyle)
        {
          if (!oldStyle)
          {
            return false;
          }

          return oldStyle->HasViewportConstrainedPosition() != newStyle.HasViewportConstrainedPosition();
        };

        if (styleChangeAffectsAnchorLayer(oldStyle, newStyle))
        {
          layer.SetNeedsCompositingConfigurationUpdate();
        }

        // These properties trigger compositing if some descendant is composited.
        if (oldStyle && StyleChangeMayAffectIndirectCompositingReasons(*oldStyle, newStyle))
        {
          layer.SetNeedsPostLayoutCompositingUpdate();
        }

        layer.SetNeedsCompositingGeometryUpdate();
      }
    }

    if (diff >= style::DifferenceResult::Repaint && oldStyle)
    {
      // This ensures that we update border-radius clips on layers that are descendants in containing-block
      // order but not paint order. This is necessary even when the current layer is not composited.
      bool changeAffectsClippingOfNonPaintOrderDescendants = !layer.IsStackingContext()
                                                             && layer.Renderer().HasNonVisibleOverflow()
                                                             && oldStyle->Border() != newStyle.Border();
      if (changeAffectsClippingOfNonPaintOrderDescendants)
      {
        if (auto *parent = layer.PaintOrderParent())
        {
          parent->SetChildrenNeedCompositingGeometryUpdate();
        }
      }
    }

    auto *backing = layer.Backing();
    if (!backing)
    {
      return;
    }

    backing->UpdateConfigurationAfterStyleChange();

    if (diff >= style::DifferenceResult::Repaint)
    {
      // Visibility change may affect geometry of the enclosing composited layer.
      if (oldStyle && oldStyle->UsedVisibility() != newStyle.UsedVisibility())
      {
        layer.SetNeedsCompositingGeometryUpdate();
      }

      // We'll get a diff of Repaint when things like clip-path change; these might affect layer or
      // inner-layer geometry.
      if (layer.IsComposited() && oldStyle)
      {
        if (StyleAffectsLayerGeometry(*oldStyle) || StyleAffectsLayerGeometry(newStyle))
        {
          layer.SetNeedsCompositingGeometryUpdate();
        }
      }

      // image rendering mode can determine whether we use device pixel ratio for the backing store.
      if (oldStyle && oldStyle->GetImageRendering() != newStyle.GetImageRendering())
      {
        layer.SetNeedsCompositingConfigurationUpdate();
      }
    }

    if (diff >= style::DifferenceResult::RecompositeLayer)
    {
      if (layer.IsComposited())
      {
        bool hitTestingStateChanged =
          oldStyle && (oldStyle->UsedPointerEvents() != newStyle.UsedPointerEvents());
        if (hitTestingStateChanged)
        {
          layer.SetNeedsCompositingConfigurationUpdate();
        }
        // If we're changing to/from 0 opacity, then we need to reconfigure the layer since we try to skip
        // backing store allocation for opacity:0.
        if (oldStyle && oldStyle->GetOpacity() != newStyle.GetOpacity()
            && (oldStyle->GetOpacity().IsTransparent() || newStyle.GetOpacity().IsTransparent()))
        {
          layer.SetNeedsCompositingConfigurationUpdate();
        }
      }
      if (oldStyle && RecompositeChangeRequiresGeometryUpdate(*oldStyle, newStyle))
      {
        // FIXME: transform changes really need to trigger layout. See RenderElement::adjustStyleDifference().
        layer.SetNeedsPostLayoutCompositingUpdate();
        layer.SetNeedsCompositingGeometryUpdate();
      }
      if (oldStyle && RecompositeChangeRequiresChildrenGeometryUpdate(*oldStyle, newStyle))
      {
        layer.SetChildrenNeedCompositingGeometryUpdate();
      }
    }
  }

  void RenderLayerCompositor::EstablishesTopLayerWillChangeForLayer(RenderLayer &layer) noexcept
  {
    ClearBackingProviderSequencesInStackingContextOfLayer(layer);
  }

  namespace
  {
    // This is a recursive walk similar to RenderLayer::collectLayers().
    void ClearBackingSharingWithinStackingContext(RenderLayer &stackingContextRoot,
                                                  RenderLayer &curLayer) noexcept
    {
      if (curLayer.EstablishesTopLayer())
      {
        return;
      }

      if (&curLayer != &stackingContextRoot && curLayer.IsStackingContext())
      {
        return;
      }

      for (auto *child = curLayer.FirstChild(); child; child = child->NextSibling())
      {
        if (child->IsComposited())
        {
          child->Backing()->ClearBackingSharingLayers({});
        }

        ClearBackingSharingWithinStackingContext(stackingContextRoot, *child);
      }
    }
  }

  void
    RenderLayerCompositor::ClearBackingProviderSequencesInStackingContextOfLayer(RenderLayer &layer) noexcept
  {
    // We can't rely on z-order lists to be up-to-date here. For fullscreen, we may already have done a style
    // update which dirties them.
    if (auto *stackingContextLayer = layer.StackingContext())
    {
      ClearBackingSharingWithinStackingContext(*stackingContextLayer, *stackingContextLayer);
    }
  }

  bool RenderLayerCompositor::UpdateExplicitBacking(RenderLayer &layer, RequiresCompositingData &queryData,
                                                    BackingRequired backingRequired) noexcept
  {
    if (backingRequired == BackingRequired::Unknown)
    {
      backingRequired = NeedsToBeComposited(layer, queryData) ? BackingRequired::Yes : BackingRequired::No;
    }
    else
    {
      // Need to fetch viewportConstrainedNotCompositedReason, but without doing all the work that
      // needsToBeComposited does.
      (void)RequiresCompositingForPosition(layer.Renderer(), layer, queryData);
    }

    bool hadBacking = layer.IsComposited();
    if (backingRequired == BackingRequired::Yes)
    {
      // If we need to repaint, do so before making backing and disconnecting from the backing provider layer.
      if (!layer.Backing())
      {
        RepaintOnCompositingChange(layer, layer.RepaintContainer());
      }
    }

    (void)UpdateBacking(layer, queryData, nullptr, backingRequired);

    if (hadBacking != layer.IsComposited())
    {
      layer.CompositingStatusChanged(queryData.LayoutUpToDate);
    }

    if (backingRequired == BackingRequired::No && hadBacking)
    {
      RepaintOnCompositingChange(layer, layer.RepaintContainer());
    }

    return hadBacking != layer.IsComposited();
  }

  bool RenderLayerCompositor::UpdateBacking(RenderLayer &layer, RequiresCompositingData &queryData,
                                            BackingSharingState *backingSharingState,
                                            BackingRequired backingRequired) noexcept
  {
    bool layerChanged = false;
    bool repaintRequired = false;
    if (backingRequired == BackingRequired::Unknown)
    {
      backingRequired = NeedsToBeComposited(layer, queryData) ? BackingRequired::Yes : BackingRequired::No;
    }
    else
    {
      // Need to fetch viewportConstrainedNotCompositedReason, but without doing all the work that
      // needsToBeComposited does.
      (void)RequiresCompositingForPosition(layer.Renderer(), layer, queryData);
    }

    EnumSet<UpdateBackingSharingFlags> updateBackingSharingFlags;
    if (backingSharingState)
    {
      updateBackingSharingFlags.Add(UpdateBackingSharingFlags::DuringCompositingUpdate);
    }

    if (backingRequired == BackingRequired::Yes)
    {
      // If we need to repaint, do so before making backing and disconnecting from the backing provider layer.
      if (!layer.Backing())
      {
        repaintRequired = true;
      }

      layer.DisconnectFromBackingProviderLayer(updateBackingSharingFlags);

      EnableCompositingMode();

      if (!layer.Backing())
      {
        layer.EnsureBacking();

        if (layer.IsRenderViewLayer())
        {
          // FIXME: The scrolling coordinator (frameViewRootLayerDidChange), header and footer layers, and the
          // tiled backing's obscured insets, once there are any.
          UpdateRootContentLayerClipping();
        }

        layer.SetNeedsCompositingGeometryUpdate();
        layer.SetNeedsCompositingConfigurationUpdate();
        layer.SetNeedsCompositingPaintOrderChildrenUpdate();

        layerChanged = true;
      }
    }
    else
    {
      if (layer.Backing())
      {
        layer.ClearBacking(updateBackingSharingFlags);
        layerChanged = true;

        // If we need to repaint, do so now that we've removed the backing.
        repaintRequired = true;
      }
    }

    // FIXME: Video and widgets, once there are any, hook up to the layer when it changes.

    if (layerChanged)
    {
      layer.ClearClipRectsIncludingDescendants(ClipRectsType::PaintingClipRects);
    }

    if (layer.Renderer().IsFixedPositioned())
    {
      if (layer.GetViewportConstrainedNotCompositedReason() != queryData.NonCompositedForPositionReason
          && !queryData.ReevaluateAfterLayout)
      {
        layer.SetViewportConstrainedNotCompositedReason(queryData.NonCompositedForPositionReason);
        layerChanged = true;
      }
      // FIXME: The scrolling coordinator (frameViewFixedObjectsDidChange), once there's async scrolling.
    }
    else
    {
      layer.SetViewportConstrainedNotCompositedReason(RenderLayer::NoNotCompositedReason);
    }

    if (layer.Backing())
    {
      layer.Backing()->UpdateDebugIndicators(_showDebugBorders, _showRepaintCounter);
    }

    if (layerChanged)
    {
      layer.SetChildrenNeedCompositingGeometryUpdate();
      layer.SetNeedsCompositingLayerConnection();
      layer.SetSubsequentLayersNeedCompositingRequirementsTraversal();
      // Ancestor layers that composited for indirect reasons (things listed in
      // styleChangeMayAffectIndirectCompositingReasons()) need to get updated. This could be optimized by
      // only setting this flag on layers with the relevant styles.
      layer.SetNeedsPostLayoutCompositingUpdateOnAncestors();
    }

    return repaintRequired;
  }

  void
    RenderLayerCompositor::RepaintOnCompositingChange(RenderLayer &layer,
                                                      const RenderLayerModelObject *repaintContainer) noexcept
  {
    // If the renderer is not attached yet, no need to repaint.
    if (&layer.Renderer() != &_renderView && !layer.Renderer().Parent())
    {
      return;
    }

    layer.RepaintIncludingNonCompositingDescendants(repaintContainer);
  }

  // This method assumes that layout is up-to-date, unlike repaintOnCompositingChange().
  void RenderLayerCompositor::RepaintInCompositedAncestor(RenderLayer &layer, const LayoutRect &rect) noexcept
  {
    auto *compositedAncestor = layer.EnclosingCompositingLayerForRepaint(IncludeSelfOrNot::ExcludeSelf).Layer;
    if (!compositedAncestor)
    {
      return;
    }

    krys_debug_assert(compositedAncestor->Backing());
    LayoutRect repaintRect = rect;
    repaintRect.Move(layer.OffsetFromAncestor(compositedAncestor));
    compositedAncestor->SetBackingNeedsRepaintInRect(repaintRect);
  }

  void RenderLayerCompositor::LayerWillBeRemoved(RenderLayer &parent, RenderLayer &child) noexcept
  {
    if (parent.Renderer().RenderTreeBeingDestroyed())
    {
      return;
    }

    if (child.IsComposited())
    {
      // FIXME: do via dirty bits?
      RepaintInCompositedAncestor(child, child.Backing()->CompositedBounds());
    }
    else if (child.PaintsIntoProvidedBacking())
    {
      auto *backingProviderLayer = child.BackingProviderLayer();
      // FIXME: Optimize this repaint.
      backingProviderLayer->SetBackingNeedsRepaint();
      backingProviderLayer->Backing()->RemoveBackingSharingLayer(child, {});
    }
    else
    {
      return;
    }

    child.ClearRepaintContainer();
    child.SetNeedsCompositingLayerConnection();
  }

  RenderLayer *
    RenderLayerCompositor::EnclosingNonStackingClippingLayer(const RenderLayer &layer) const noexcept
  {
    for (auto *parent = layer.Parent(); parent; parent = parent->Parent())
    {
      if (parent->IsStackingContext())
      {
        return nullptr;
      }
      if (parent->Renderer().HasClipOrNonVisibleOverflow())
      {
        return parent;
      }
    }
    return nullptr;
  }

  void RenderLayerCompositor::ComputeExtent(const LayerOverlapMap &overlapMap, const RenderLayer &layer,
                                            OverlapExtent &extent) const noexcept
  {
    if (extent.ExtentComputed)
    {
      return;
    }

    const RenderLayerModelObject &renderer = layer.Renderer();
    if (renderer.IsStickilyPositioned())
    {
      // Use rectangle that represents union of all possible sticky element positions, because it could be
      // moved around without re-computing overlap.
      const auto &box = Downcast<RenderBoxModelObject>(renderer);
      StickyPositionViewportConstraints constraints;
      auto constrainingRectForStickyPosition = box.ConstrainingRectForStickyPosition();
      box.ComputeStickyPositionConstraints(constraints, constrainingRectForStickyPosition);
      extent.Bounds = LayoutRect(constraints.ComputeStickyExtent());
      extent.ExtentComputed = true;
      return;
    }

    LayoutRect layerBounds;
    if (extent.HasTransformAnimation)
    {
      extent.AnimationCausesExtentUncertainty =
        !layer.GetOverlapBoundsIncludingChildrenAccountingForTransformAnimations(layerBounds);
    }
    else
    {
      layerBounds = layer.OverlapBounds();
    }

    // In the animating transform case, we avoid double-accounting for the transform because we told
    // pushMappingsToAncestor() to ignore transforms earlier.
    extent.Bounds = EnclosingLayoutRect(overlapMap.GeometryMap().AbsoluteRect(FloatRect(layerBounds)));

    // Empty rects never intersect, but we need them to for the purposes of overlap testing.
    if (extent.Bounds.IsEmpty())
    {
      extent.Bounds.SetSize(LayoutSize(1, 1));
    }

    if (renderer.IsFixedPositioned() && renderer.Container() == &_renderView)
    {
      // Because fixed elements get moved around without re-computing overlap, we have to compute an overlap
      // rect that covers all the locations that the fixed element could move to.
      extent.Bounds = _renderView.FrameView().FixedScrollableAreaBoundsInflatedForScrolling(extent.Bounds);
    }

    extent.ExtentComputed = true;
  }

  namespace
  {
    enum class AncestorTraversal : bool
    {
      Continue,
      Stop
    };

    // This is a simplified version of containing block walking that only handles absolute and fixed position.
    template <typename TFunction>
    AncestorTraversal TraverseAncestorLayers(const RenderLayer &layer, TFunction &&function) noexcept
    {
      auto positioningBehavior = layer.Renderer().Style().GetPosition();
      RenderLayer *nextPaintOrderParent = layer.PaintOrderParent();

      for (const auto *ancestorLayer = layer.Parent(); ancestorLayer; ancestorLayer = ancestorLayer->Parent())
      {
        bool inContainingBlockChain = true;

        switch (positioningBehavior)
        {
          case PositionType::Static:
          case PositionType::Relative:
          case PositionType::Sticky:   break;
          case PositionType::Absolute:
            inContainingBlockChain = ancestorLayer->Renderer().CanContainAbsolutelyPositionedObjects();
            break;
          case PositionType::Fixed:
            inContainingBlockChain = ancestorLayer->Renderer().CanContainFixedPositionObjects();
            break;
        }

        if (function(*ancestorLayer, inContainingBlockChain, ancestorLayer == nextPaintOrderParent)
            == AncestorTraversal::Stop)
        {
          return AncestorTraversal::Stop;
        }

        if (inContainingBlockChain)
        {
          positioningBehavior = ancestorLayer->Renderer().Style().GetPosition();
        }

        if (ancestorLayer == nextPaintOrderParent)
        {
          nextPaintOrderParent = ancestorLayer->PaintOrderParent();
        }
      }

      return AncestorTraversal::Continue;
    }
  }

  void RenderLayerCompositor::ComputeClippingScopes(const RenderLayer &layer,
                                                    OverlapExtent &extent) const noexcept
  {
    if (extent.ClippingScopesComputed)
    {
      return;
    }

    // FIXME: constrain the scopes (by composited stacking context ancestor I think).
    auto populateEnclosingClippingScopes = [](const RenderLayer &layer, const RenderLayer &rootLayer,
                                              LayerOverlapMap::LayerAndBoundsVector &clippingScopes)
    {
      auto createsClippingScope = [](const RenderLayer &layer)
      {
        return layer.HasCompositedScrollableOverflow();
      };

      clippingScopes.push_back({&rootLayer, {}});

      if (!layer.HasCompositedScrollingAncestor())
      {
        return;
      }

      TraverseAncestorLayers(
        layer,
        [&](const RenderLayer &ancestorLayer, bool inContainingBlockChain, bool)
        {
          if (inContainingBlockChain && createsClippingScope(ancestorLayer))
          {
            LayoutRect clipRect;
            if (auto *box = DynamicDowncast<RenderBox>(ancestorLayer.Renderer()))
            {
              // FIXME: This is expensive. Broken with transforms.
              LayoutPoint offsetFromRoot = ancestorLayer.ConvertToLayerCoords(&rootLayer, {});
              clipRect = box->OverflowClipRect(offsetFromRoot);
            }

            // Order is roots to leaves.
            clippingScopes.insert(clippingScopes.begin() + 1,
                                  LayerOverlapMap::LayerAndBounds {&ancestorLayer, clipRect});
          }
          return AncestorTraversal::Continue;
        });
    };

    populateEnclosingClippingScopes(layer, RootRenderLayer(), extent.ClippingScopes);
    extent.ClippingScopesComputed = true;
  }

  LayoutRect RenderLayerCompositor::ComputeClippedOverlapBounds(LayerOverlapMap &overlapMap,
                                                                const RenderLayer &layer,
                                                                OverlapExtent &extent) const noexcept
  {
    ComputeExtent(overlapMap, layer, extent);
    ComputeClippingScopes(layer, extent);

    LayoutRect clipRect;
    if (layer.HasCompositedScrollingAncestor())
    {
      // Compute a clip up to the composited scrolling ancestor, then convert it to absolute coordinates.
      auto &scrollingScope = extent.ClippingScopes.back();
      const auto *scopeLayer = scrollingScope.Layer;
      clipRect = layer
                   .BackgroundClipRect(RenderLayer::ClipRectsContext(
                     scopeLayer, PaintingClipRects, {RenderLayer::ClipRectsOption::Temporary}))
                   .Rect();
      if (!clipRect.IsInfinite())
      {
        clipRect.SetLocation(scopeLayer->ConvertToLayerCoords(&RootRenderLayer(), clipRect.Location()));
      }
    }
    else
    {
      // FIXME: Incorrect for CSS regions.
      clipRect =
        layer.BackgroundClipRect(RenderLayer::ClipRectsContext(&RootRenderLayer(), AbsoluteClipRects)).Rect();
    }

    auto clippedBounds = extent.Bounds;
    if (!clipRect.IsInfinite())
    {
      // FIXME: Page scale, once there's page scaling.
      clippedBounds.Intersect(clipRect);
    }

    return clippedBounds;
  }

  void RenderLayerCompositor::AddToOverlapMap(LayerOverlapMap &overlapMap, const RenderLayer &layer,
                                              OverlapExtent &extent) const noexcept
  {
    if (layer.IsRenderViewLayer())
    {
      return;
    }

    auto clippedBounds = ComputeClippedOverlapBounds(overlapMap, layer, extent);

    ComputeClippingScopes(layer, extent);
    overlapMap.Add(layer, clippedBounds, extent.ClippingScopes);
  }

  void RenderLayerCompositor::AddDescendantsToOverlapMapRecursive(
    LayerOverlapMap &overlapMap, const RenderLayer &layer, const RenderLayer *ancestorLayer) const noexcept
  {
    if (!CanBeComposited(layer))
    {
      return;
    }

    // A null ancestorLayer is an indication that 'layer' has already been pushed.
    if (ancestorLayer)
    {
      overlapMap.GeometryMap().PushMappingsToAncestor(&layer, ancestorLayer);

      OverlapExtent layerExtent;
      AddToOverlapMap(overlapMap, layer, layerExtent);
    }

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(const_cast<RenderLayer &>(layer));
#endif

    for (auto *renderLayer : layer.NegativeZOrderLayers())
    {
      AddDescendantsToOverlapMapRecursive(overlapMap, *renderLayer, &layer);
    }

    for (auto *renderLayer : layer.NormalFlowLayers())
    {
      AddDescendantsToOverlapMapRecursive(overlapMap, *renderLayer, &layer);
    }

    for (auto *renderLayer : layer.PositiveZOrderLayers())
    {
      AddDescendantsToOverlapMapRecursive(overlapMap, *renderLayer, &layer);
    }

    if (ancestorLayer)
    {
      overlapMap.GeometryMap().PopMappingsToAncestor(ancestorLayer);
    }
  }

  void RenderLayerCompositor::UpdateOverlapMap(LayerOverlapMap &overlapMap, const RenderLayer &layer,
                                               OverlapExtent &layerExtent, bool didPushContainer,
                                               bool addLayerToOverlap,
                                               bool addDescendantsToOverlap) const noexcept
  {
    if (addLayerToOverlap)
    {
      AddToOverlapMap(overlapMap, layer, layerExtent);
    }

    if (addDescendantsToOverlap)
    {
      // If this is the first non-root layer to composite, we need to add all the descendants we already
      // traversed to the overlap map.
      AddDescendantsToOverlapMapRecursive(overlapMap, layer);
    }

    if (didPushContainer)
    {
      overlapMap.PopCompositingContainer(layer);
    }
  }

  bool RenderLayerCompositor::LayerOverlaps(const LayerOverlapMap &overlapMap, const RenderLayer &layer,
                                            OverlapExtent &extent) const noexcept
  {
    ComputeExtent(overlapMap, layer, extent);
    ComputeClippingScopes(layer, extent);

    return overlapMap.OverlapsLayers(layer, extent.Bounds, extent.ClippingScopes);
  }

  void RenderLayerCompositor::FrameViewDidChangeLocation(FloatPoint contentsOffset) noexcept
  {
    if (_overflowControlsHostLayer)
    {
      _overflowControlsHostLayer->SetPosition(contentsOffset);
    }
  }

  void RenderLayerCompositor::FrameViewDidChangeSize() noexcept
  {
    if (auto *layer = _renderView.Layer())
    {
      layer->SetNeedsCompositingGeometryUpdate();
    }

    if (_scrolledContentsLayer)
    {
      UpdateScrollLayerClipping();
      FrameViewDidScroll();
      UpdateOverflowControlsLayers();
    }
  }

  bool RenderLayerCompositor::HasCoordinatedScrolling() const noexcept
  {
    // FIXME: The scrolling coordinator, once there's async scrolling.
    return false;
  }

  void RenderLayerCompositor::UpdateScrollLayerPosition() noexcept
  {
    krys_debug_assert(!HasCoordinatedScrolling());
    krys_debug_assert(_scrolledContentsLayer);

    auto &frameView = _renderView.FrameView();
    IntPoint scrollPosition = frameView.GetScrollPosition();

    // We use scroll position here because the root content layer is offset to account for scrollOrigin (see
    // LocalFrameView::positionForRootContentLayer).
    _scrolledContentsLayer->SetPosition(
      FloatPoint(static_cast<float>(-scrollPosition.X()), static_cast<float>(-scrollPosition.Y())));

    if (auto *fixedBackgroundLayer = FixedRootBackgroundLayer())
    {
      fixedBackgroundLayer->SetPosition(FloatPoint(frameView.ScrollPositionForFixedPosition()));
    }
  }

  void RenderLayerCompositor::UpdateScrollLayerClipping() noexcept
  {
    auto *layerForClipping = LayerForClipping();
    if (!layerForClipping)
    {
      return;
    }

    auto layerRect = InsetClipLayerRect();
    layerForClipping->SetSize(layerRect.Size());
    layerForClipping->SetPosition(layerRect.Location());
  }

  FloatRect RenderLayerCompositor::InsetClipLayerRect() const noexcept
  {
    auto &frameView = _renderView.FrameView();

    // FIXME: Obscured content insets, once there are any.
    FloatRect insetClipLayerRect({}, FloatSize(frameView.SizeForVisibleContent()));
    insetClipLayerRect.Move(static_cast<float>(frameView.InsetForLeftScrollbarSpace()), 0);
    return insetClipLayerRect;
  }

  void RenderLayerCompositor::FrameViewDidScroll() noexcept
  {
    if (!_scrolledContentsLayer)
    {
      return;
    }

    // If there's a scrolling coordinator that manages scrolling for this frame view, it will also manage
    // updating the scroll layer position.
    if (HasCoordinatedScrolling())
    {
      // We have to schedule a flush in order for the main TiledBacking to update its tile coverage.
      ScheduleRenderingUpdate();
      return;
    }

    UpdateScrollLayerPosition();
  }

  void RenderLayerCompositor::FrameViewDidAddOrRemoveScrollbars() noexcept
  {
    UpdateOverflowControlsLayers();
  }

  void RenderLayerCompositor::FrameViewDidLayout() noexcept
  {
    // FIXME: Tiled backing coverage (adjustTiledBackingCoverage), once there's a tiled backing.
  }

  void RenderLayerCompositor::RootLayerConfigurationChanged() noexcept
  {
    // FIXME: Frame layers with a tiled backing, once there are any.
  }

  void RenderLayerCompositor::UpdateCompositingForLayerTreeAsTextDump() noexcept
  {
    (void)_renderView.GetDocument().UpdateLayout();

    UpdateCompositingLayers(CompositingUpdateType::AfterLayout);

    if (!_rootContentsLayer)
    {
      return;
    }

    FlushPendingLayerChanges(true);
  }

  utf8_string RenderLayerCompositor::LayerTreeAsText(EnumSet<LayerTreeAsTextOptions> options,
                                                     uint32 baseIndent) noexcept
  {
    UpdateCompositingForLayerTreeAsTextDump();

    // Exclude any implicitly created layers that wrap the root contents layer, unless the caller explicitly
    // requested the true root to be included.
    GraphicsLayer *dumpRootLayer = options.Contains(LayerTreeAsTextOptions::IncludeRootLayers)
                                     ? RootGraphicsLayer()
                                     : _rootContentsLayer.get();

    if (!dumpRootLayer)
    {
      return {};
    }

    // We skip dumping the scroll and clip layers to keep layerTreeAsText output similar between platforms.
    return dumpRootLayer->LayerTreeAsText(options, baseIndent);
  }

  void RenderLayerCompositor::RepaintCompositedLayers() noexcept
  {
    RecursiveRepaintLayer(RootRenderLayer());
  }

  void RenderLayerCompositor::RecursiveRepaintLayer(RenderLayer &layer) noexcept
  {
    layer.UpdateLayerListsIfNeeded();

    // FIXME: This method does not work correctly with transforms.
    if (layer.IsComposited() && !layer.Backing()->PaintsIntoCompositedAncestor())
    {
      layer.SetBackingNeedsRepaint();
    }

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(layer);
#endif

    if (layer.HasCompositingDescendant())
    {
      for (auto *renderLayer : layer.NegativeZOrderLayers())
      {
        RecursiveRepaintLayer(*renderLayer);
      }

      for (auto *renderLayer : layer.PositiveZOrderLayers())
      {
        RecursiveRepaintLayer(*renderLayer);
      }
    }

    for (auto *renderLayer : layer.NormalFlowLayers())
    {
      RecursiveRepaintLayer(*renderLayer);
    }
  }

  bool RenderLayerCompositor::LayerRepaintTargetsBackingSharingLayer(
    RenderLayer &layer, BackingSharingState &sharingState) const noexcept
  {
    if (sharingState.BackingProviderCandidates().empty())
    {
      return false;
    }

    for (const auto *currLayer = &layer; currLayer; currLayer = currLayer->PaintOrderParent())
    {
      if (CompositedWithOwnBackingStore(*currLayer))
      {
        return false;
      }

      if (currLayer->PaintsIntoProvidedBacking())
      {
        return false;
      }

      if (sharingState.BackingProviderForLayer(*currLayer))
      {
        return true;
      }
    }

    return false;
  }

  RenderLayer &RenderLayerCompositor::RootRenderLayer() const noexcept
  {
    return *_renderView.Layer();
  }

  GraphicsLayer *RenderLayerCompositor::RootGraphicsLayer() const noexcept
  {
    if (_overflowControlsHostLayer)
    {
      return _overflowControlsHostLayer.get();
    }
    return _rootContentsLayer.get();
  }

  void RenderLayerCompositor::SetIsInWindow(bool isInWindow) noexcept
  {
    if (!UsesCompositing())
    {
      return;
    }

    if (auto *rootLayer = RootGraphicsLayer())
    {
      GraphicsLayer::Traverse(*rootLayer,
                              [isInWindow](GraphicsLayer &layer) { layer.SetIsInWindow(isInWindow); });
    }

    if (isInWindow)
    {
      if (_rootLayerAttachment != RootLayerUnattached)
      {
        return;
      }

      AttachRootLayer(RootLayerAttachedViaChromeClient);
    }
    else
    {
      if (_rootLayerAttachment == RootLayerUnattached)
      {
        return;
      }

      DetachRootLayer();
    }
  }

  void RenderLayerCompositor::ClearBackingForAllLayers() noexcept
  {
    ApplyToCompositedLayerIncludingDescendants(*_renderView.Layer(),
                                               [](RenderLayer &layer) { layer.ClearBacking({}); });
  }

  void RenderLayerCompositor::UpdateRootLayerPosition() noexcept
  {
    if (_rootContentsLayer)
    {
      auto &frameView = _renderView.FrameView();
      _rootContentsLayer->SetSize(FloatSize(frameView.ContentsSize()));
      _rootContentsLayer->SetPosition(frameView.PositionForRootContentLayer());
      _rootContentsLayer->SetAnchorPoint(FloatPoint3D());
    }

    UpdateScrollLayerClipping();
  }

  bool RenderLayerCompositor::Has3DContent() const noexcept
  {
    return LayerHas3DContent(RootRenderLayer());
  }

  bool RenderLayerCompositor::NeedsToBeComposited(const RenderLayer &layer,
                                                  RequiresCompositingData &queryData) const noexcept
  {
    if (!CanBeComposited(layer))
    {
      return false;
    }

    return RequiresCompositingLayer(layer, queryData) || layer.MustCompositeForIndirectReasons()
           || (UsesCompositing() && layer.IsRenderViewLayer());
  }

  // Note: this specifies whether the RL needs a compositing layer for intrinsic reasons. Use
  // needsToBeComposited() to determine if a RL actually needs a compositing layer.
  // FIXME: is clipsCompositingDescendants() an intrinsic reason?
  bool RenderLayerCompositor::RequiresCompositingLayer(const RenderLayer &layer,
                                                       RequiresCompositingData &queryData) const noexcept
  {
    auto &renderer = layer.Renderer();

    if (!renderer.Layer())
    {
      krys_debug_assert(false);
      return false;
    }

    // The root layer always has a compositing layer, but it may not have backing.
    // FIXME: Canvas, video, model, frame and plug-in reasons, once there are any.
    if (RequiresCompositingForTransform(renderer) || RequiresCompositingForAnimation(renderer)
        || RequiresCompositingForPosition(renderer, *renderer.Layer(), queryData)
        || RequiresCompositingForFilters(renderer) || RequiresCompositingForWillChange(renderer)
        || RequiresCompositingForBackfaceVisibility(renderer)
        || RequiresCompositingForOverflowScrolling(*renderer.Layer(), queryData)
        || RequiresCompositingForAnchorPositioning(*renderer.Layer()))
    {
      queryData.Intrinsic = true;
      return true;
    }
    return false;
  }

  bool RenderLayerCompositor::CanBeComposited(const RenderLayer &layer) const noexcept
  {
    if (_hasAcceleratedCompositing && layer.IsSelfPaintingLayer())
    {
      if (layer.Renderer().IsSkippedContent())
      {
        return false;
      }

      // FIXME: Fragmented flows, once there are any.
      return true;
    }
    return false;
  }

  bool
    RenderLayerCompositor::RequiresOwnBackingStore(const RenderLayer &layer,
                                                   const RenderLayer *compositingAncestorLayer,
                                                   const LayoutRect &layerCompositedBoundsInAncestor,
                                                   const LayoutRect &ancestorCompositedBounds) const noexcept
  {
    auto &renderer = layer.Renderer();

    if (compositingAncestorLayer
        && !(compositingAncestorLayer->Backing()->GetGraphicsLayer()->DrawsContent()
             || compositingAncestorLayer->Backing()->PaintsIntoWindow()
             || compositingAncestorLayer->Backing()->PaintsIntoCompositedAncestor()))
    {
      return true;
    }

    RequiresCompositingData queryData;
    if (layer.IsRenderViewLayer() || layer.Transform() // note: excludes perspective and transformStyle3D.
        || RequiresCompositingForAnimation(renderer)
        || RequiresCompositingForPosition(renderer, layer, queryData)
        || RequiresCompositingForFilters(renderer) || RequiresCompositingForWillChange(renderer)
        || RequiresCompositingForBackfaceVisibility(renderer)
        || RequiresCompositingForOverflowScrolling(layer, queryData)
        || RequiresCompositingForAnchorPositioning(layer) || NeedsContentsCompositingLayer(layer)
        || renderer.IsTransparent() || renderer.HasMask() || renderer.HasFilter()
        || renderer.HasBackdropFilter())
    {
      return true;
    }

    if (layer.IsComposited() && layer.Backing()->HasBackingSharingLayers())
    {
      return true;
    }

    // FIXME: We really need to keep track of the ancestor layer that has its own backing store.
    if (!ancestorCompositedBounds.Contains(layerCompositedBoundsInAncestor))
    {
      return true;
    }

    if (layer.MustCompositeForIndirectReasons())
    {
      IndirectCompositingReason reason = layer.GetIndirectCompositingReason();
      // preserve-3d has to create backing store to ensure that 3d-transformed elements intersect.
      return reason == IndirectCompositingReason::Overlap
             || reason == IndirectCompositingReason::OverflowScrollPositioning
             || reason == IndirectCompositingReason::Stacking
             || reason == IndirectCompositingReason::BackgroundLayer
             || reason == IndirectCompositingReason::GraphicalEffect
             || reason == IndirectCompositingReason::Preserve3D;
    }

    return false;
  }

  EnumSet<CompositingReason>
    RenderLayerCompositor::ReasonsForCompositing(const RenderLayer &layer) const noexcept
  {
    EnumSet<CompositingReason> reasons;

    if (!layer.IsComposited())
    {
      return reasons;
    }

    RequiresCompositingData queryData;

    auto &renderer = layer.Renderer();

    if (RequiresCompositingForTransform(renderer))
    {
      reasons.Add(CompositingReason::Transform3D);
    }

    if ((CanRender3DTransforms() && renderer.Style().GetBackfaceVisibility() == BackfaceVisibility::Hidden))
    {
      reasons.Add(CompositingReason::BackfaceVisibilityHidden);
    }

    if (RequiresCompositingForAnimation(renderer))
    {
      reasons.Add(CompositingReason::Animation);
    }

    if (RequiresCompositingForFilters(renderer))
    {
      reasons.Add(CompositingReason::Filters);
    }

    if (RequiresCompositingForWillChange(renderer))
    {
      reasons.Add(CompositingReason::WillChange);
    }

    if (RequiresCompositingForPosition(renderer, *renderer.Layer(), queryData))
    {
      reasons.Add(renderer.IsFixedPositioned() ? CompositingReason::PositionFixed
                                               : CompositingReason::PositionSticky);
    }

    if (RequiresCompositingForOverflowScrolling(*renderer.Layer(), queryData))
    {
      reasons.Add(CompositingReason::OverflowScrolling);
    }

    if (RequiresCompositingForAnchorPositioning(*renderer.Layer()))
    {
      reasons.Add(CompositingReason::AnchorPositioning);
    }

    switch (renderer.Layer()->GetIndirectCompositingReason())
    {
      case IndirectCompositingReason::None: break;
      case IndirectCompositingReason::Clipping:
        reasons.Add(CompositingReason::ClipsCompositingDescendants);
        break;
      case IndirectCompositingReason::Stacking: reasons.Add(CompositingReason::Stacking); break;
      case IndirectCompositingReason::OverflowScrollPositioning:
        reasons.Add(CompositingReason::OverflowScrollPositioning);
        break;
      case IndirectCompositingReason::Overlap: reasons.Add(CompositingReason::Overlap); break;
      case IndirectCompositingReason::BackgroundLayer:
        reasons.Add(CompositingReason::NegativeZIndexChildren);
        break;
      case IndirectCompositingReason::GraphicalEffect:
        if (renderer.IsTransformed())
        {
          reasons.Add(CompositingReason::TransformWithCompositedDescendants);
        }

        if (renderer.IsTransparent())
        {
          reasons.Add(CompositingReason::OpacityWithCompositedDescendants);
        }

        if (renderer.HasMask())
        {
          reasons.Add(CompositingReason::MaskWithCompositedDescendants);
        }

        if (renderer.HasFilter() || renderer.HasBackdropFilter())
        {
          reasons.Add(CompositingReason::FilterWithCompositedDescendants);
        }

        if (layer.IsBackdropRoot())
        {
          reasons.Add(CompositingReason::BackdropRoot);
        }

        if (layer.IsolatesCompositedBlending())
        {
          reasons.Add(CompositingReason::IsolatesCompositedBlendingDescendants);
        }

        if (layer.HasBlendMode())
        {
          reasons.Add(CompositingReason::BlendingWithCompositedDescendants);
        }

        if (renderer.HasClipPath())
        {
          reasons.Add(CompositingReason::ClipsCompositingDescendants);
        }
        break;
      case IndirectCompositingReason::Perspective: reasons.Add(CompositingReason::Perspective); break;
      case IndirectCompositingReason::Preserve3D:  reasons.Add(CompositingReason::Preserve3D); break;
    }

    if (UsesCompositing() && renderer.Layer()->IsRenderViewLayer())
    {
      reasons.Add(CompositingReason::Root);
    }

    return reasons;
  }

  namespace
  {
    KRYS_NODISCARD bool CanUseDescendantClippingLayer(const RenderLayer &layer) noexcept
    {
      if (layer.IsolatesCompositedBlending())
      {
        return false;
      }

      // We can only use the "descendant clipping layer" strategy when the clip rect is entirely within the
      // border box, because of interactions with border-radius clipping and compositing.
      if (auto *renderer = layer.GetRenderBox(); renderer && renderer->HasClip())
      {
        auto borderBoxRect = renderer->BorderBoxRect();
        auto clipRect = renderer->ClipRect({});

        bool clipRectInsideBorderRect = Intersection(borderBoxRect, clipRect) == clipRect;
        return clipRectInsideBorderRect;
      }

      return true;
    }
  }

  // Return true if the given layer has some ancestor in the RenderLayer hierarchy that clips, up to the
  // enclosing compositing ancestor. This is required because compositing layers are parented according to the
  // z-order hierarchy, yet clipping goes down the renderer hierarchy. Thus, a RenderLayer can be clipped by a
  // RenderLayer that is an ancestor in the renderer hierarchy, but a sibling in the z-order hierarchy.
  // FIXME: can we do this without a tree walk?
  bool RenderLayerCompositor::ClippedByAncestor(RenderLayer &layer,
                                                const RenderLayer *compositingAncestor) const noexcept
  {
    krys_debug_assert(layer.IsComposited());
    if (!compositingAncestor)
    {
      return false;
    }

    // If the compositingAncestor clips, that will be taken care of by clipsCompositingDescendants(), so we
    // only care about clipping between its first child that is our ancestor (the computeClipRoot), and layer.
    // The exception is when the compositingAncestor isolates composited blending children, in this case it is
    // not allowed to clipsCompositingDescendants() and each of its children will be clippedByAncestor()s,
    // including the compositingAncestor.
    const auto *computeClipRoot = compositingAncestor;
    if (CanUseDescendantClippingLayer(*compositingAncestor))
    {
      computeClipRoot = nullptr;
      auto *parent = &layer;
      while (parent)
      {
        auto *next = parent->Parent();
        if (next == compositingAncestor)
        {
          computeClipRoot = parent;
          break;
        }
        parent = next;
      }

      if (!computeClipRoot || computeClipRoot == &layer)
      {
        return false;
      }
    }

    auto backgroundClipRect = layer.BackgroundClipRect(RenderLayer::ClipRectsContext(
      computeClipRoot, PaintingClipRects, RenderLayer::ClipRectTemporaryOptions));
    return !backgroundClipRect.IsInfinite(); // FIXME: Incorrect for CSS regions.
  }

  bool
    RenderLayerCompositor::UpdateAncestorClippingStack(const RenderLayer &layer,
                                                       const RenderLayer *compositingAncestor) const noexcept
  {
    krys_debug_assert(layer.IsComposited());

    auto clippingStack = ComputeAncestorClippingStack(layer, compositingAncestor);
    return layer.Backing()->UpdateAncestorClippingStack(krys::move(clippingStack));
  }

  List<CompositedClipData>
    RenderLayerCompositor::ComputeAncestorClippingStack(const RenderLayer &layer,
                                                        const RenderLayer *compositingAncestor) const noexcept
  {
    // On first pass, the root may not have become composited yet.
    if (!compositingAncestor)
    {
      return {};
    }

    // We'll start by building a child-to-ancestors stack.
    List<CompositedClipData> newStack;

    // Walk up the containing block chain to composited ancestor, prepending an entry to the clip stack for:
    // * each composited scrolling layer
    // * each set of RenderLayers which contribute a clip.
    bool haveNonScrollableClippingIntermediateLayer = false;
    const RenderLayer *currentClippedLayer = &layer;

    auto pushNonScrollableClip = [&](const RenderLayer &clippedLayer, const RenderLayer &clippingRoot,
                                     ShouldRespectOverflowClip respectClip = IgnoreOverflowClip)
    {
      // Use IgnoreOverflowClip to ignore overflow contributed by clippingRoot (which may be a scroller).
      EnumSet<RenderLayer::ClipRectsOption> options {RenderLayer::ClipRectsOption::Temporary};
      if (respectClip == RespectOverflowClip)
      {
        options.Add(RenderLayer::ClipRectsOption::RespectOverflowClip);
      }

      auto backgroundClip = clippedLayer.BackgroundClipRect(
        RenderLayer::ClipRectsContext(&clippingRoot, PaintingClipRects, options));
      krys_debug_assert(!backgroundClip.AffectedByRadius());
      auto clipRect = backgroundClip.Rect();
      if (clipRect.IsInfinite())
      {
        return;
      }

      auto infiniteRect = LayoutRect::InfiniteRect();
      // Return a infinite-like rect whose values are such that, when converted to float pixel values, they
      // can reasonably represent device pixels.
      auto renderableInfiniteRect = LayoutRect(LayoutUnit::NearlyMin() / 32, LayoutUnit::NearlyMin() / 32,
                                               LayoutUnit::NearlyMax() / 16, LayoutUnit::NearlyMax() / 16);

      if (clipRect.Width() == infiniteRect.Width())
      {
        clipRect.SetX(renderableInfiniteRect.X());
        clipRect.SetWidth(renderableInfiniteRect.Width());
      }

      if (clipRect.Height() == infiniteRect.Height())
      {
        clipRect.SetY(renderableInfiniteRect.Y());
        clipRect.SetHeight(renderableInfiniteRect.Height());
      }

      auto offset = layer.ConvertToLayerCoords(&clippingRoot, {}, RenderLayer::AdjustForColumns);
      clipRect.MoveBy(-offset);

      newStack.insert(newStack.begin(), CompositedClipData {const_cast<RenderLayer *>(&clippedLayer),
                                                            LayoutRoundedRect {clipRect}, false});
    };

    // Surprisingly, the deprecated CSS "clip" property on abspos ancestors of fixedpos elements clips them
    // <https://github.com/w3c/csswg-drafts/issues/8336>.
    bool checkAbsoluteAncestorForClip = layer.Renderer().IsFixedPositioned();

    TraverseAncestorLayers(
      layer,
      [&](const RenderLayer &ancestorLayer, bool isContainingBlockChain, bool /*isPaintOrderAncestor*/)
      {
        if (&ancestorLayer == compositingAncestor)
        {
          bool canUseDescendantClip = CanUseDescendantClippingLayer(ancestorLayer);
          if (haveNonScrollableClippingIntermediateLayer)
          {
            pushNonScrollableClip(*currentClippedLayer, ancestorLayer,
                                  !canUseDescendantClip ? RespectOverflowClip : IgnoreOverflowClip);
          }
          else if (!canUseDescendantClip && newStack.empty())
          {
            pushNonScrollableClip(*currentClippedLayer, ancestorLayer, RespectOverflowClip);
          }

          return AncestorTraversal::Stop;
        }

        auto ancestorLayerMayClip = [&]()
        {
          if (checkAbsoluteAncestorForClip && ancestorLayer.Renderer().HasClip())
          {
            return true;
          }

          return isContainingBlockChain && ancestorLayer.Renderer().HasClipOrNonVisibleOverflow();
        };

        if (ancestorLayerMayClip())
        {
          auto *box = ancestorLayer.GetRenderBox();
          if (!box)
          {
            return AncestorTraversal::Continue;
          }

          if (ancestorLayer.HasCompositedScrollableOverflow())
          {
            if (haveNonScrollableClippingIntermediateLayer)
            {
              pushNonScrollableClip(*currentClippedLayer, ancestorLayer);
              haveNonScrollableClippingIntermediateLayer = false;
            }

            auto clipRoundedRect = ParentRelativeScrollableRect(ancestorLayer, &ancestorLayer);
            auto offset = layer.ConvertToLayerCoords(&ancestorLayer, {}, RenderLayer::AdjustForColumns);
            clipRoundedRect.MoveBy(-offset);

            newStack.insert(newStack.begin(), CompositedClipData {const_cast<RenderLayer *>(&ancestorLayer),
                                                                  clipRoundedRect, true});
            currentClippedLayer = &ancestorLayer;
          }
          else if (box->HasNonVisibleOverflow() && box->Style().HasBorderRadius())
          {
            if (haveNonScrollableClippingIntermediateLayer)
            {
              pushNonScrollableClip(*currentClippedLayer, ancestorLayer);
              haveNonScrollableClippingIntermediateLayer = false;
            }

            auto borderShape = BorderShape::ShapeForBorderRect(box->Style(), box->BorderBoxRect());
            auto clipRoundedRect = borderShape.DeprecatedInnerRoundedRect();

            auto offset = layer.ConvertToLayerCoords(&ancestorLayer, {}, RenderLayer::AdjustForColumns);
            auto rect = clipRoundedRect.Rect();
            rect.MoveBy(-offset);
            clipRoundedRect.SetRect(rect);

            newStack.insert(newStack.begin(), CompositedClipData {const_cast<RenderLayer *>(&ancestorLayer),
                                                                  clipRoundedRect, false});
            currentClippedLayer = &ancestorLayer;
          }
          else
          {
            haveNonScrollableClippingIntermediateLayer = true;
          }
        }

        return AncestorTraversal::Continue;
      });

    return newStack;
  }

  // Return true if the given layer is a stacking context and has compositing child layers that it needs to
  // clip. In this case we insert a clipping GraphicsLayer into the hierarchy between this layer and its
  // children in the z-order hierarchy.
  bool RenderLayerCompositor::ClipsCompositingDescendants(const RenderLayer &layer) noexcept
  {
    if (!(layer.HasCompositingDescendant() && layer.Renderer().HasClipOrNonVisibleOverflow()))
    {
      return false;
    }

    if (layer.HasCompositedNonContainedDescendants())
    {
      return false;
    }

    return CanUseDescendantClippingLayer(layer);
  }

  bool RenderLayerCompositor::RequiresCompositingForAnimation(
    const RenderLayerModelObject &renderer) const noexcept
  {
    // All compositing triggers are allowed, including animations and animated opacity.
    if (auto styleable = style::Styleable::FromRenderer(renderer))
    {
      if (styleable->HasRunningAcceleratedAnimations())
      {
        return true;
      }
      if (auto *effectsStack = styleable->GetKeyframeEffectStack())
      {
        return effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Opacity)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Filter)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::BackdropFilter)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Translate)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Scale)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Rotate)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Transform)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::OffsetAnchor)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::OffsetDistance)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::OffsetPath)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::OffsetPosition)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::OffsetRotate);
      }
    }

    return false;
  }

  namespace
  {
    KRYS_NODISCARD bool StyleHas3DTransformOperation(const RenderStyle &style) noexcept
    {
      return style.GetTransform().Has3DOperation() || style.GetTranslate().Is3DOperation()
             || style.GetScale().Is3DOperation() || style.GetRotate().Is3DOperation();
    }

    KRYS_NODISCARD bool StyleTransformOperationsAreRepresentableIn2D(const RenderStyle &style) noexcept
    {
      return style.GetTransform().IsRepresentableIn2D() && style.GetTranslate().IsRepresentableIn2D()
             && style.GetScale().IsRepresentableIn2D() && style.GetRotate().IsRepresentableIn2D();
    }
  }

  bool RenderLayerCompositor::RequiresCompositingForTransform(
    const RenderLayerModelObject &renderer) const noexcept
  {
    // Note that we ask the renderer if it has a transform, because the style may have transforms, but the
    // renderer may be an inline that doesn't suppport them.
    if (!renderer.IsTransformed())
    {
      return false;
    }

    switch (_compositingPolicy)
    {
      case CompositingPolicy::Normal: return StyleHas3DTransformOperation(renderer.Style());
      case CompositingPolicy::Conservative:
        // Continue to allow pages to avoid the very slow software filter path.
        if (StyleHas3DTransformOperation(renderer.Style()) && renderer.HasFilter())
        {
          return true;
        }
        return !StyleTransformOperationsAreRepresentableIn2D(renderer.Style());
    }
    return false;
  }

  bool RenderLayerCompositor::RequiresCompositingForBackfaceVisibility(
    const RenderLayerModelObject &renderer) const noexcept
  {
    if (renderer.Style().GetBackfaceVisibility() != BackfaceVisibility::Hidden)
    {
      return false;
    }

    if (renderer.Layer()->Has3DTransformedAncestor())
    {
      return true;
    }

    // FIXME: workaround for webkit.org/b/132801
    auto *stackingContext = renderer.Layer()->StackingContext();
    if (stackingContext && stackingContext->Renderer().Style().Preserves3D())
    {
      return true;
    }

    return false;
  }

  bool RenderLayerCompositor::RequiresCompositingForFilters(
    const RenderLayerModelObject &renderer) const noexcept
  {
    if (renderer.HasBackdropFilter())
    {
      return true;
    }

    return renderer.HasFilter();
  }

  bool RenderLayerCompositor::RequiresCompositingForWillChange(
    const RenderLayerModelObject &renderer) const noexcept
  {
    if (!renderer.Style().GetWillChange().CanTriggerCompositing())
    {
      return false;
    }

    if (_compositingPolicy == CompositingPolicy::Conservative)
    {
      return false;
    }

    if (Is<RenderBox>(renderer))
    {
      return true;
    }

    return renderer.Style().GetWillChange().CanTriggerCompositingOnInline();
  }

  bool
    RenderLayerCompositor::RequiresCompositingForPosition(const RenderLayerModelObject &renderer,
                                                          const RenderLayer &layer,
                                                          RequiresCompositingData &queryData) const noexcept
  {
    // position:fixed elements that create their own stacking context (e.g. have an explicit z-index, opacity,
    // transform) can get their own composited layer. A stacking context is required otherwise z-index and
    // clipping will be broken.
    if (!renderer.IsPositioned())
    {
      return false;
    }

    auto position = renderer.Style().GetPosition();
    bool isFixed = renderer.IsFixedPositioned();
    if (isFixed && !layer.IsStackingContext())
    {
      return false;
    }

    bool isSticky = renderer.IsInFlowPositioned() && position == PositionType::Sticky;
    if (!isFixed && !isSticky)
    {
      return false;
    }

    if (!_acceleratedCompositingForFixedPositionEnabled)
    {
      return false;
    }

    if (isSticky)
    {
      return IsAsyncScrollableStickyLayer(layer);
    }

    if (queryData.LayoutUpToDate == LayoutUpToDate::No)
    {
      queryData.ReevaluateAfterLayout = true;
      return layer.IsComposited();
    }

    auto *container = renderer.Container();
    krys_debug_assert(container);

    // Don't promote fixed position elements that are descendants of a non-view container, e.g. transformed
    // elements. They will stay fixed wrt the container rather than the enclosing frame.
    if (container != &_renderView)
    {
      queryData.NonCompositedForPositionReason = RenderLayer::NotCompositedForNonViewContainer;
      return false;
    }

    bool paintsContent = layer.IsVisuallyNonEmpty() || layer.HasVisibleDescendant();
    if (!paintsContent)
    {
      queryData.NonCompositedForPositionReason = RenderLayer::NotCompositedForNoVisibleContent;
      return false;
    }

    bool intersectsViewport = FixedLayerIntersectsViewport(layer);
    if (!intersectsViewport)
    {
      queryData.NonCompositedForPositionReason = RenderLayer::NotCompositedForBoundsOutOfView;
      return false;
    }

    return true;
  }

  bool RenderLayerCompositor::RequiresCompositingForOverflowScrolling(
    const RenderLayer &layer, RequiresCompositingData &queryData) const noexcept
  {
    if (!layer.CanUseCompositedScrolling())
    {
      return false;
    }

    if (queryData.LayoutUpToDate == LayoutUpToDate::No)
    {
      queryData.ReevaluateAfterLayout = true;
      return layer.IsComposited();
    }

    const_cast<RenderLayer &>(layer).ComputeHasCompositedScrollableOverflow(LayoutUpToDate::Yes);
    return layer.HasCompositedScrollableOverflow();
  }

  bool RenderLayerCompositor::RequiresCompositingForAnchorPositioning(const RenderLayer &layer) const noexcept
  {
    return layer.AnchorScrollAdjustment().has_value();
  }

  IndirectCompositingReason RenderLayerCompositor::ComputeIndirectCompositingReason(
    const RenderLayer &layer, bool hasCompositedDescendants, bool has3DTransformedDescendants,
    bool paintsIntoProvidedBacking) const noexcept
  {
    // When a layer has composited descendants, some effects, like 2d transforms, filters, masks etc must be
    // implemented via compositing so that they also apply to those composited descendants.
    auto &renderer = layer.Renderer();
    if (hasCompositedDescendants
        && (layer.IsolatesCompositedBlending() || layer.IsBackdropRoot() || layer.Transform()
            || renderer.CreatesGroup()))
    {
      return IndirectCompositingReason::GraphicalEffect;
    }

    // A layer with preserve-3d or perspective only needs to be composited if there are descendant layers that
    // will be affected by the preserve-3d or perspective.
    if (has3DTransformedDescendants)
    {
      if (renderer.Style().Preserves3D())
      {
        return IndirectCompositingReason::Preserve3D;
      }

      if (renderer.Style().HasPerspective())
      {
        return IndirectCompositingReason::Perspective;
      }
    }

    // If this layer scrolls independently from the layer that it would paint into, it needs to get
    // composited.
    if (!paintsIntoProvidedBacking && layer.HasCompositedScrollingAncestor())
    {
      auto *paintDestination = layer.PaintOrderParent();
      if (paintDestination
          && LayerScrollBehahaviorRelativeToCompositedAncestor(layer, *paintDestination)
               != ScrollPositioningBehavior::None)
      {
        return IndirectCompositingReason::OverflowScrollPositioning;
      }
    }

    // Check for clipping last; if compositing just for clipping, the layer doesn't need its own backing
    // store.
    if (hasCompositedDescendants && ClipsCompositingDescendants(layer))
    {
      return IndirectCompositingReason::Clipping;
    }

    return IndirectCompositingReason::None;
  }

  bool RenderLayerCompositor::StyleChangeMayAffectIndirectCompositingReasons(
    const RenderStyle &oldStyle, const RenderStyle &newStyle) noexcept
  {
    if (RenderElement::CreatesGroupForStyle(newStyle) != RenderElement::CreatesGroupForStyle(oldStyle))
    {
      return true;
    }
    if (newStyle.GetIsolation() != oldStyle.GetIsolation())
    {
      return true;
    }
    if (newStyle.HasTransform() != oldStyle.HasTransform())
    {
      return true;
    }
    if (newStyle.UsedTransformStyle3D() != oldStyle.UsedTransformStyle3D())
    {
      return true;
    }
    if (newStyle.HasPerspective() != oldStyle.HasPerspective())
    {
      return true;
    }

    return false;
  }

  bool RenderLayerCompositor::IsAsyncScrollableStickyLayer(
    const RenderLayer &layer, const RenderLayer **enclosingAcceleratedOverflowLayer) const noexcept
  {
    krys_debug_assert(layer.Renderer().IsStickilyPositioned());

    auto *enclosingOverflowLayer = layer.EnclosingOverflowClipLayer(ExcludeSelf);

    if (enclosingOverflowLayer && enclosingOverflowLayer->HasCompositedScrollableOverflow())
    {
      if (enclosingAcceleratedOverflowLayer)
      {
        *enclosingAcceleratedOverflowLayer = enclosingOverflowLayer;
      }
      return true;
    }

    // If the layer is inside normal overflow, it's not async-scrollable.
    if (enclosingOverflowLayer)
    {
      return false;
    }

    // No overflow ancestor, so see if the frame supports async scrolling.
    if (HasCoordinatedScrolling())
    {
      return true;
    }

    return false;
  }

  ViewportConstrainedSublayers
    RenderLayerCompositor::GetViewportConstrainedSublayers(const RenderLayer &layer,
                                                           const RenderLayer *) const noexcept
  {
    // FIXME: View transitions and content inset background fill, once there are any.
    if (layer.Renderer().IsStickilyPositioned())
    {
      const RenderLayer *overflowLayer = nullptr;
      if (!IsAsyncScrollableStickyLayer(layer, &overflowLayer))
      {
        return ViewportConstrainedSublayers::None;
      }

      return ViewportConstrainedSublayers::Anchor;
    }

    if (!(layer.Renderer().IsFixedPositioned() && layer.BehavesAsFixed()))
    {
      return ViewportConstrainedSublayers::None;
    }

    for (auto *ancestor = layer.Parent(); ancestor; ancestor = ancestor->Parent())
    {
      if (ancestor->HasCompositedScrollableOverflow())
      {
        return ViewportConstrainedSublayers::Anchor;
      }

      if (ancestor->IsStackingContext() && ancestor->IsComposited()
          && ancestor->Renderer().IsFixedPositioned())
      {
        return ViewportConstrainedSublayers::None;
      }
    }

    return ViewportConstrainedSublayers::Anchor;
  }

  bool RenderLayerCompositor::FixedLayerIntersectsViewport(const RenderLayer &layer) const noexcept
  {
    krys_debug_assert(layer.Renderer().IsFixedPositioned());

    // Fixed position elements that are invisible in the current view don't get their own layer.
    // FIXME: We shouldn't have to check useFixedLayout() here; one of the viewport rects needs to give the
    // correct answer.
    LayoutRect viewBounds;
    auto &frameView = _renderView.FrameView();
    if (frameView.UseFixedLayout())
    {
      viewBounds = _renderView.UnscaledDocumentRect();
    }
    else
    {
      viewBounds = frameView.RectForFixedPositionLayout();
    }

    LayoutRect layerBounds = layer.CalculateLayerBounds(
      &layer, LayoutSize(),
      {RenderLayer::UseLocalClipRectIfPossible, RenderLayer::IncludeFilterOutsets,
       RenderLayer::UseFragmentBoxesExcludingCompositing, RenderLayer::ExcludeHiddenDescendants,
       RenderLayer::DontConstrainForMask, RenderLayer::IncludeCompositedDescendants});
    // Map to the render view to ignore page scale.
    FloatRect absoluteBounds =
      layer.Renderer().LocalToContainerQuad(FloatQuad(FloatRect(layerBounds)), &_renderView).BoundingBox();
    return viewBounds.Intersects(LayoutRect(EnclosingIntRect(absoluteBounds)));
  }

  bool RenderLayerCompositor::UseCoordinatedScrollingForLayer(const RenderLayer &layer) const noexcept
  {
    // FIXME: The scrolling coordinator's overflow layers, once there's async scrolling.
    return layer.IsRenderViewLayer() && HasCoordinatedScrolling();
  }

  ScrollPositioningBehavior RenderLayerCompositor::LayerScrollBehahaviorRelativeToCompositedAncestor(
    const RenderLayer &layer, const RenderLayer &compositedAncestor) noexcept
  {
    if (!layer.HasCompositedScrollingAncestor())
    {
      return ScrollPositioningBehavior::None;
    }

    auto needsMovesNode = [&]
    {
      bool result = false;
      TraverseAncestorLayers(layer,
                             [&](const RenderLayer &ancestorLayer, bool isContainingBlockChain, bool)
                             {
                               if (&ancestorLayer == &compositedAncestor)
                               {
                                 return AncestorTraversal::Stop;
                               }

                               if (isContainingBlockChain && ancestorLayer.HasCompositedScrollableOverflow())
                               {
                                 result = true;
                                 return AncestorTraversal::Stop;
                               }

                               return AncestorTraversal::Continue;
                             });

      return result;
    };

    if (needsMovesNode())
    {
      return ScrollPositioningBehavior::Moves;
    }

    if (layer.BoxScrollingScope() != compositedAncestor.ContentsScrollingScope())
    {
      return ScrollPositioningBehavior::Stationary;
    }

    return ScrollPositioningBehavior::None;
  }

  ScrollPositioningBehavior
    RenderLayerCompositor::ComputeCoordinatedPositioningForLayer(const RenderLayer &,
                                                                 const RenderLayer *) const noexcept
  {
    // FIXME: The scrolling coordinator, once there's async scrolling.
    return ScrollPositioningBehavior::None;
  }

  bool
    RenderLayerCompositor::IsRunningTransformAnimation(const RenderLayerModelObject &renderer) const noexcept
  {
    if (auto styleable = style::Styleable::FromRenderer(renderer))
    {
      if (auto *effectsStack = styleable->GetKeyframeEffectStack())
      {
        return effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Transform)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Rotate)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Scale)
               || effectsStack->IsCurrentlyAffectingProperty(css::PropertyId::Translate);
      }
    }
    return false;
  }

  // If an element has composited negative z-index children, those children render in front of the layer
  // background, so we need an extra 'contents' layer for the foreground of the layer object.
  bool RenderLayerCompositor::NeedsContentsCompositingLayer(const RenderLayer &layer) const noexcept
  {
    for (auto *childLayer : layer.NegativeZOrderLayers())
    {
      if (childLayer->IsComposited() || childLayer->HasCompositingDescendant())
      {
        return true;
      }
    }

    return false;
  }

  bool RenderLayerCompositor::RequiresScrollLayer(RootLayerAttachment) const noexcept
  {
    // We need to handle our own scrolling: there's no platform widget.
    return true;
  }

  namespace
  {
    KRYS_NODISCARD utf8_stringview CompositingReasonToString(CompositingReason reason) noexcept
    {
      switch (reason)
      {
        case CompositingReason::Transform3D:                 return u8"3D transform";
        case CompositingReason::Video:                       return u8"video";
        case CompositingReason::Canvas:                      return u8"canvas";
        case CompositingReason::Plugin:                      return u8"plugin";
        case CompositingReason::IFrame:                      return u8"iframe";
        case CompositingReason::BackfaceVisibilityHidden:    return u8"backface-visibility: hidden";
        case CompositingReason::ClipsCompositingDescendants: return u8"clips compositing descendants";
        case CompositingReason::Animation:                   return u8"animation";
        case CompositingReason::Filters:                     return u8"filters";
        case CompositingReason::PositionFixed:               return u8"position: fixed";
        case CompositingReason::PositionSticky:              return u8"position: sticky";
        case CompositingReason::OverflowScrolling:           return u8"async overflow scrolling";
        case CompositingReason::Stacking:                    return u8"stacking";
        case CompositingReason::Overlap:                     return u8"overlap";
        case CompositingReason::OverflowScrollPositioning:   return u8"overflow scroll positioning";
        case CompositingReason::NegativeZIndexChildren:      return u8"negative z-index children";
        case CompositingReason::TransformWithCompositedDescendants:
          return u8"transform with composited descendants";
        case CompositingReason::OpacityWithCompositedDescendants:
          return u8"opacity with composited descendants";
        case CompositingReason::MaskWithCompositedDescendants: return u8"mask with composited descendants";
        case CompositingReason::ReflectionWithCompositedDescendants:
          return u8"reflection with composited descendants";
        case CompositingReason::FilterWithCompositedDescendants:
          return u8"filter with composited descendants";
        case CompositingReason::BlendingWithCompositedDescendants:
          return u8"blending with composited descendants";
        case CompositingReason::IsolatesCompositedBlendingDescendants:
          return u8"isolates composited blending descendants";
        case CompositingReason::Perspective:       return u8"perspective";
        case CompositingReason::Preserve3D:        return u8"preserve-3d";
        case CompositingReason::WillChange:        return u8"will-change";
        case CompositingReason::Root:              return u8"root";
        case CompositingReason::Model:             return u8"model";
        case CompositingReason::BackdropRoot:      return u8"backdrop root";
        case CompositingReason::AnchorPositioning: return u8"anchor positioning";
      }
      return u8"";
    }
  }

  void PaintScrollbar(Scrollbar *scrollbar, GraphicsContext &context, const IntRect &clip,
                      const Color &backgroundColor) noexcept
  {
    if (!scrollbar)
    {
      return;
    }

    context.Save();
    const IntRect &scrollbarRect = scrollbar->FrameRect();
    context.Translate(
      -FloatSize(static_cast<float>(scrollbarRect.X()), static_cast<float>(scrollbarRect.Y())));
    IntRect transformedClip = clip;
    transformedClip.MoveBy(scrollbarRect.Location());
    if (!scrollbar->IsOverlayScrollbar() && backgroundColor.IsVisible())
    {
      context.FillRect(FloatRect(transformedClip), backgroundColor);
    }
    scrollbar->Paint(context, transformedClip);
    context.Restore();
  }

  void RenderLayerCompositor::PaintContents(const GraphicsLayer &graphicsLayer, GraphicsContext &context,
                                            const FloatRect &clip,
                                            EnumSet<GraphicsLayerPaintBehavior>) noexcept
  {
    IntRect pixelSnappedRectForIntegralPositionedItems = SnappedIntRect(LayoutRect(clip));
    auto &frameView = _renderView.FrameView();
    if (&graphicsLayer == LayerForHorizontalScrollbar())
    {
      PaintScrollbar(frameView.HorizontalScrollbar(), context, pixelSnappedRectForIntegralPositionedItems,
                     _viewBackgroundColor);
    }
    else if (&graphicsLayer == LayerForVerticalScrollbar())
    {
      PaintScrollbar(frameView.VerticalScrollbar(), context, pixelSnappedRectForIntegralPositionedItems,
                     _viewBackgroundColor);
    }
    else if (&graphicsLayer == LayerForScrollCorner())
    {
      const IntRect &scrollCorner = frameView.ScrollCornerRect();
      context.Save();
      context.Translate(
        -FloatSize(static_cast<float>(scrollCorner.X()), static_cast<float>(scrollCorner.Y())));
      IntRect transformedClip = pixelSnappedRectForIntegralPositionedItems;
      transformedClip.MoveBy(scrollCorner.Location());
      // The theme draws no corner, so the view's background shows there, as it does behind the scrollbars.
      if (_viewBackgroundColor.IsVisible())
      {
        context.FillRect(FloatRect(transformedClip), _viewBackgroundColor);
      }
      frameView.PaintScrollCorner(context, transformedClip);
      context.Restore();
    }
  }

  bool RenderLayerCompositor::SupportsFixedRootBackgroundCompositing() const noexcept
  {
    // FIXME: Frame layers with a tiled backing, once there are any.
    return false;
  }

  bool RenderLayerCompositor::NeedsFixedRootBackgroundLayer(const RenderLayer &layer) const noexcept
  {
    if (!layer.IsRenderViewLayer())
    {
      return false;
    }

    return SupportsFixedRootBackgroundCompositing() && _renderView.RootBackgroundIsEntirelyFixed();
  }

  GraphicsLayer *RenderLayerCompositor::FixedRootBackgroundLayer() const noexcept
  {
    // Get the fixed root background from the RenderView layer's backing.
    auto *viewLayer = _renderView.Layer();
    if (!viewLayer)
    {
      return nullptr;
    }

    if (viewLayer->IsComposited() && viewLayer->Backing()->BackgroundLayerPaintsFixedRootBackground())
    {
      return viewLayer->Backing()->BackgroundLayer();
    }

    return nullptr;
  }

  void RenderLayerCompositor::ResetTrackedRepaintRects() noexcept
  {
    if (auto *rootLayer = RootGraphicsLayer())
    {
      GraphicsLayer::Traverse(*rootLayer, [](GraphicsLayer &layer) { layer.ResetTrackedRepaints(); });
    }
  }

  float RenderLayerCompositor::DeviceScaleFactor() const noexcept
  {
    // FIXME: The device scale factor, once pages have one.
    return 1;
  }

  float RenderLayerCompositor::PageScaleFactor() const noexcept
  {
    // FIXME: Page scale, once there's page scaling.
    return 1;
  }

  float RenderLayerCompositor::ZoomedOutPageScaleFactor() const noexcept
  {
    return 0;
  }

  FloatSize RenderLayerCompositor::EnclosingFrameViewVisibleSize() const noexcept
  {
    auto &frameView = _renderView.FrameView();
    if (_scrolledContentsLayer)
    {
      return FloatSize(frameView.SizeForVisibleContent());
    }
    return FloatSize(frameView.VisibleContentRect().Size());
  }

  float RenderLayerCompositor::ContentsScaleMultiplierForNewTiles(const GraphicsLayer *) const noexcept
  {
    return 1;
  }

  bool RenderLayerCompositor::DocumentUsesTiledBacking() const noexcept
  {
    // FIXME: Frame layers with a tiled backing, once there are any.
    return false;
  }

  bool RenderLayerCompositor::IsRootFrameCompositor() const noexcept
  {
    // FIXME: Frames other than the main frame, once there are any.
    return true;
  }

  bool RenderLayerCompositor::IsMainFrameCompositor() const noexcept
  {
    return true;
  }

  bool RenderLayerCompositor::ShouldCompositeOverflowControls() const noexcept
  {
    auto &frameView = _renderView.FrameView();

    if (!frameView.ManagesScrollbars())
    {
      return false;
    }

    if (DocumentUsesTiledBacking())
    {
      return true;
    }

    if (_overflowControlsHostLayer && IsRootFrameCompositor())
    {
      return true;
    }

    return true;
  }

  bool RenderLayerCompositor::RequiresHorizontalScrollbarLayer() const noexcept
  {
    return ShouldCompositeOverflowControls() && _renderView.FrameView().HorizontalScrollbar();
  }

  bool RenderLayerCompositor::RequiresVerticalScrollbarLayer() const noexcept
  {
    return ShouldCompositeOverflowControls() && _renderView.FrameView().VerticalScrollbar();
  }

  bool RenderLayerCompositor::RequiresScrollCornerLayer() const noexcept
  {
    return ShouldCompositeOverflowControls() && _renderView.FrameView().IsScrollCornerVisible();
  }

  bool RenderLayerCompositor::ViewHasTransparentBackground(Color *backgroundColor) const noexcept
  {
    auto &frameView = _renderView.FrameView();
    if (frameView.IsTransparent())
    {
      if (backgroundColor)
      {
        *backgroundColor = Color(); // Return an invalid color.
      }
      return true;
    }

    Color documentBackgroundColor = frameView.DocumentBackgroundColor();
    if (!documentBackgroundColor.IsValid())
    {
      documentBackgroundColor = frameView.BaseBackgroundColor();
    }

    krys_debug_assert(documentBackgroundColor.IsValid());

    if (backgroundColor)
    {
      *backgroundColor = documentBackgroundColor;
    }

    return !documentBackgroundColor.IsOpaque();
  }

  // We can't rely on getting layerStyleChanged() for a style change that affects the root background, because
  // the style change may be on the body which has no RenderLayer.
  void RenderLayerCompositor::RootOrBodyStyleChanged(RenderElement &renderer,
                                                     const RenderStyle *oldStyle) noexcept
  {
    if (!UsesCompositing())
    {
      return;
    }

    Color oldBackgroundColor;
    if (oldStyle)
    {
      oldBackgroundColor = oldStyle->VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
    }

    if (oldBackgroundColor
        != renderer.Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor))
    {
      RootBackgroundColorOrTransparencyChanged();
    }

    bool hadFixedBackground = oldStyle && style::HasEntirelyFixedBackground(oldStyle->GetBackgroundLayers());
    if (hadFixedBackground != style::HasEntirelyFixedBackground(renderer.Style().GetBackgroundLayers()))
    {
      RootLayerConfigurationChanged();
    }

    if (oldStyle
        && (oldStyle->OverscrollBehaviorX() != renderer.Style().OverscrollBehaviorX()
            || oldStyle->OverscrollBehaviorY() != renderer.Style().OverscrollBehaviorY()))
    {
      if (auto *layer = _renderView.Layer())
      {
        layer->SetNeedsCompositingGeometryUpdate();
      }
    }
  }

  void RenderLayerCompositor::UpdateRootContentsLayerBackgroundColor() noexcept
  {
    if (!_rootContentsLayer)
    {
      return;
    }

    // FIXME: The root element captured in a view transition, once there are view transitions.
    _rootContentsLayer->SetBackgroundColor(Color());
  }

  void RenderLayerCompositor::RootBackgroundColorOrTransparencyChanged() noexcept
  {
    if (!UsesCompositing())
    {
      return;
    }

    Color backgroundColor;
    bool isTransparent = ViewHasTransparentBackground(&backgroundColor);

    // FIXME: Extending the background beyond the page, once there's a setting for it.
    Color extendedBackgroundColor;

    bool transparencyChanged = _viewBackgroundIsTransparent != isTransparent;
    bool backgroundColorChanged = _viewBackgroundColor != backgroundColor;
    bool extendedBackgroundColorChanged = _rootExtendedBackgroundColor != extendedBackgroundColor;

    if (!transparencyChanged && !backgroundColorChanged && !extendedBackgroundColorChanged)
    {
      return;
    }

    _viewBackgroundIsTransparent = isTransparent;
    _viewBackgroundColor = backgroundColor;
    _rootExtendedBackgroundColor = extendedBackgroundColor;

    if (extendedBackgroundColorChanged)
    {
      UpdateRootContentsLayerBackgroundColor();
    }

    RootLayerConfigurationChanged();
  }

  void RenderLayerCompositor::UpdateOverflowControlsLayers() noexcept
  {
    if (RequiresHorizontalScrollbarLayer())
    {
      if (!_layerForHorizontalScrollbar)
      {
        _layerForHorizontalScrollbar = GraphicsLayer::Create(GetGraphicsLayerFactory(), *this);
        _layerForHorizontalScrollbar->SetAllowsBackingStoreDetaching(false);
        _layerForHorizontalScrollbar->SetAllowsTiling(false);
        _layerForHorizontalScrollbar->SetShowDebugBorder(_showDebugBorders);
        _layerForHorizontalScrollbar->SetName(u8"horizontal scrollbar container");
        _overflowControlsHostLayer->AddChild(ShareRef(*_layerForHorizontalScrollbar));
      }
    }
    else if (_layerForHorizontalScrollbar)
    {
      GraphicsLayer::UnparentAndClear(_layerForHorizontalScrollbar);
    }

    if (RequiresVerticalScrollbarLayer())
    {
      if (!_layerForVerticalScrollbar)
      {
        _layerForVerticalScrollbar = GraphicsLayer::Create(GetGraphicsLayerFactory(), *this);
        _layerForVerticalScrollbar->SetAllowsBackingStoreDetaching(false);
        _layerForVerticalScrollbar->SetAllowsTiling(false);
        _layerForVerticalScrollbar->SetShowDebugBorder(_showDebugBorders);
        _layerForVerticalScrollbar->SetName(u8"vertical scrollbar container");
        _overflowControlsHostLayer->AddChild(ShareRef(*_layerForVerticalScrollbar));
      }
    }
    else if (_layerForVerticalScrollbar)
    {
      GraphicsLayer::UnparentAndClear(_layerForVerticalScrollbar);
    }

    if (RequiresScrollCornerLayer())
    {
      if (!_layerForScrollCorner)
      {
        _layerForScrollCorner = GraphicsLayer::Create(GetGraphicsLayerFactory(), *this);
        _layerForScrollCorner->SetAllowsBackingStoreDetaching(false);
        _layerForScrollCorner->SetShowDebugBorder(_showDebugBorders);
        _layerForScrollCorner->SetName(u8"scroll corner");
        _overflowControlsHostLayer->AddChild(ShareRef(*_layerForScrollCorner));
      }
    }
    else
    {
      GraphicsLayer::UnparentAndClear(_layerForScrollCorner);
    }

    _renderView.FrameView().PositionScrollbarLayers();
  }

  void RenderLayerCompositor::EnsureRootLayer() noexcept
  {
    RootLayerAttachment expectedAttachment = RootLayerAttachedViaChromeClient;
    if (expectedAttachment == _rootLayerAttachment)
    {
      return;
    }

    if (!_rootContentsLayer)
    {
      _rootContentsLayer = GraphicsLayer::Create(GetGraphicsLayerFactory(), *this);
      _rootContentsLayer->SetName(u8"content root");
      IntRect overflowRect = SnappedIntRect(_renderView.LayoutOverflowRect());
      _rootContentsLayer->SetSize(
        FloatSize(static_cast<float>(overflowRect.MaxX()), static_cast<float>(overflowRect.MaxY())));
      _rootContentsLayer->SetPosition(FloatPoint());

      // Need to clip to prevent transformed content showing outside this frame
      UpdateRootContentLayerClipping();
      UpdateRootContentsLayerBackgroundColor();
    }

    if (RequiresScrollLayer(expectedAttachment))
    {
      if (!_overflowControlsHostLayer)
      {
        krys_debug_assert(!_scrolledContentsLayer);
        krys_debug_assert(!_clipLayer);

        // Create a layer to host the clipping layer and the overflow controls layers.
        _overflowControlsHostLayer = GraphicsLayer::Create(GetGraphicsLayerFactory(), *this);
        _overflowControlsHostLayer->SetName(u8"overflow controls host");

        _scrolledContentsLayer =
          GraphicsLayer::Create(GetGraphicsLayerFactory(), *this, GraphicsLayer::Type::ScrolledContents);
        _scrolledContentsLayer->SetName(u8"frame scrolled contents");
        _scrolledContentsLayer->SetAnchorPoint({});

        // FIXME: _scrollContainerLayer and _clipLayer have similar roles here, but _clipLayer has some
        // special positioning to account for clipping and top content inset (see insetClipLayerRect()).
        if (!_scrollContainerLayer)
        {
          _clipLayer = GraphicsLayer::Create(GetGraphicsLayerFactory(), *this);
          _clipLayer->SetName(u8"frame clipping");
          _clipLayer->SetMasksToBounds(true);
          _clipLayer->SetAnchorPoint({});

          _clipLayer->AddChild(ShareRef(*_scrolledContentsLayer));
          _overflowControlsHostLayer->AddChild(ShareRef(*_clipLayer));
        }

        _scrolledContentsLayer->AddChild(ShareRef(*_rootContentsLayer));

        UpdateScrollLayerClipping();
        UpdateOverflowControlsLayers();

        if (HasCoordinatedScrolling())
        {
          ScheduleRenderingUpdate();
        }
        else
        {
          UpdateScrollLayerPosition();
        }
      }
    }
    else
    {
      if (_overflowControlsHostLayer)
      {
        GraphicsLayer::UnparentAndClear(_overflowControlsHostLayer);
        GraphicsLayer::UnparentAndClear(_clipLayer);
        GraphicsLayer::UnparentAndClear(_scrollContainerLayer);
        GraphicsLayer::UnparentAndClear(_scrolledContentsLayer);
      }
    }

    // Check to see if we have to change the attachment
    if (_rootLayerAttachment != RootLayerUnattached)
    {
      DetachRootLayer();
    }

    AttachRootLayer(expectedAttachment);
  }

  void RenderLayerCompositor::DestroyRootLayer() noexcept
  {
    if (!_rootContentsLayer)
    {
      return;
    }

    DetachRootLayer();

    auto &frameView = _renderView.FrameView();

    if (_layerForHorizontalScrollbar)
    {
      GraphicsLayer::UnparentAndClear(_layerForHorizontalScrollbar);
      if (auto *horizontalScrollbar = frameView.HorizontalScrollbar())
      {
        frameView.InvalidateScrollbar(*horizontalScrollbar,
                                      IntRect(IntPoint(0, 0), horizontalScrollbar->FrameRect().Size()));
      }
    }

    if (_layerForVerticalScrollbar)
    {
      GraphicsLayer::UnparentAndClear(_layerForVerticalScrollbar);
      if (auto *verticalScrollbar = frameView.VerticalScrollbar())
      {
        frameView.InvalidateScrollbar(*verticalScrollbar,
                                      IntRect(IntPoint(0, 0), verticalScrollbar->FrameRect().Size()));
      }
    }

    if (_layerForScrollCorner)
    {
      GraphicsLayer::UnparentAndClear(_layerForScrollCorner);
      frameView.InvalidateScrollCorner(frameView.ScrollCornerRect());
    }

    if (_overflowControlsHostLayer)
    {
      GraphicsLayer::UnparentAndClear(_overflowControlsHostLayer);
      GraphicsLayer::UnparentAndClear(_clipLayer);
      GraphicsLayer::UnparentAndClear(_scrollContainerLayer);
      GraphicsLayer::UnparentAndClear(_scrolledContentsLayer);
    }
    krys_debug_assert(!_scrolledContentsLayer);
    GraphicsLayer::UnparentAndClear(_rootContentsLayer);
  }

  void RenderLayerCompositor::AttachRootLayer(RootLayerAttachment attachment) noexcept
  {
    if (!_rootContentsLayer)
    {
      return;
    }

    switch (attachment)
    {
      case RootLayerUnattached: krys_debug_assert(false); break;
      case RootLayerAttachedViaChromeClient:
        if (auto *page = GetPage())
        {
          page->GetChromeClient().AttachRootGraphicsLayer(_renderView.FrameView().Frame(),
                                                          RootGraphicsLayer());
        }
        break;
      case RootLayerAttachedViaEnclosingFrame:
        // FIXME: Frames other than the main frame, once there are any.
        krys_debug_assert(false);
        break;
    }

    _rootLayerAttachment = attachment;
    RootLayerAttachmentChanged();

    if (_shouldFlushOnReattach)
    {
      ScheduleRenderingUpdate();
      _shouldFlushOnReattach = false;
    }
  }

  void RenderLayerCompositor::DetachRootLayer() noexcept
  {
    if (!_rootContentsLayer || _rootLayerAttachment == RootLayerUnattached)
    {
      return;
    }

    switch (_rootLayerAttachment)
    {
      case RootLayerAttachedViaEnclosingFrame:
        // FIXME: Frames other than the main frame, once there are any.
        break;
      case RootLayerAttachedViaChromeClient:
        if (auto *page = GetPage())
        {
          page->GetChromeClient().AttachRootGraphicsLayer(_renderView.FrameView().Frame(), nullptr);
        }
        break;
      case RootLayerUnattached: break;
    }

    _rootLayerAttachment = RootLayerUnattached;
    RootLayerAttachmentChanged();
  }

  void RenderLayerCompositor::UpdateRootLayerAttachment() noexcept
  {
    EnsureRootLayer();
  }

  void RenderLayerCompositor::RootLayerAttachmentChanged() noexcept
  {
    if (_rootLayerAttachment == RootLayerUnattached)
    {
      return;
    }

    // The attachment can affect whether the RenderView layer's paintsIntoWindow() behavior, so call
    // updateDrawsContent() to update that.
    auto *layer = _renderView.Layer();
    if (auto *backing = layer ? layer->Backing() : nullptr)
    {
      backing->UpdateDrawsContent();
    }

    // FIXME: Page overlays (the document overlays' layer), once there are any.
  }

  bool RenderLayerCompositor::LayerHas3DContent(const RenderLayer &layer) const noexcept
  {
    const RenderStyle &style = layer.Renderer().Style();

    if (style.Preserves3D() || style.HasPerspective() || StyleHas3DTransformOperation(style))
    {
      return true;
    }

    const_cast<RenderLayer &>(layer).UpdateLayerListsIfNeeded();

#if KRYS_DEBUG
    LayerListMutationDetector mutationChecker(const_cast<RenderLayer &>(layer));
#endif

    for (auto *renderLayer : layer.NegativeZOrderLayers())
    {
      if (LayerHas3DContent(*renderLayer))
      {
        return true;
      }
    }

    for (auto *renderLayer : layer.PositiveZOrderLayers())
    {
      if (LayerHas3DContent(*renderLayer))
      {
        return true;
      }
    }

    for (auto *renderLayer : layer.NormalFlowLayers())
    {
      if (LayerHas3DContent(*renderLayer))
      {
        return true;
      }
    }

    return false;
  }

  void RenderLayerCompositor::DeviceOrPageScaleFactorChanged() noexcept
  {
    // Page scale will only be applied at to the RenderView and sublayers, but the device scale factor needs
    // to be applied at the level of rootGraphicsLayer().
    if (auto *rootLayer = RootGraphicsLayer())
    {
      rootLayer->NoteDeviceOrPageScaleFactorChangedIncludingDescendants();
    }
  }

  LayoutRoundedRect
    RenderLayerCompositor::ParentRelativeScrollableRect(const RenderLayer &layer,
                                                        const RenderLayer *ancestorLayer) const noexcept
  {
    // FIXME: ancestorLayer needs to be always non-null, so should become a reference.
    if (!ancestorLayer)
    {
      if (!layer.GetScrollableArea())
      {
        return LayoutRoundedRect {LayoutRect {}};
      }
      return LayoutRoundedRect {LayoutRect({}, LayoutSize(layer.GetScrollableArea()->VisibleSize()))};
    }

    LayoutRoundedRect scrollableRect(LayoutRect {});
    {
      auto *box = DynamicDowncast<RenderBox>(layer.Renderer());
      if (!box)
      {
        return LayoutRoundedRect {LayoutRect {}};
      }

      scrollableRect = LayoutRoundedRect {box->PaddingBoxRect()};
      if (box->Style().HasBorderRadius())
      {
        auto borderShape = BorderShape::ShapeForBorderRect(box->Style(), box->BorderBoxRect());
        scrollableRect = borderShape.DeprecatedInnerRoundedRect();
      }
    }

    // FIXME: broken for columns.
    auto offset = layer.ConvertToLayerCoords(ancestorLayer, scrollableRect.Rect().Location());
    auto rect = scrollableRect.Rect();
    rect.SetLocation(offset);
    scrollableRect.SetRect(rect);
    return scrollableRect;
  }

  GraphicsLayerFactory *RenderLayerCompositor::GetGraphicsLayerFactory() const noexcept
  {
    auto *page = GetPage();
    return page ? page->GetChromeClient().GetGraphicsLayerFactory() : nullptr;
  }

  Page *RenderLayerCompositor::GetPage() const noexcept
  {
    return _renderView.GetDocument().GetPage();
  }

  void RenderLayerCompositor::UpdateRootContentLayerClipping() noexcept
  {
    // FIXME: Extending the background beyond the page, once there's a setting for it.
    _rootContentsLayer->SetMasksToBounds(true);
  }

  TextStream &operator<<(TextStream &ts, CompositingUpdateType updateType) noexcept
  {
    switch (updateType)
    {
      case CompositingUpdateType::AfterStyleChange:   ts << u8"after style change"; break;
      case CompositingUpdateType::AfterLayout:        ts << u8"after layout"; break;
      case CompositingUpdateType::OnScroll:           ts << u8"on scroll"; break;
      case CompositingUpdateType::OnCompositedScroll: ts << u8"on composited scroll"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, CompositingPolicy compositingPolicy) noexcept
  {
    switch (compositingPolicy)
    {
      case CompositingPolicy::Normal:       ts << u8"normal"; break;
      case CompositingPolicy::Conservative: ts << u8"conservative"; break;
    }
    return ts;
  }

  TextStream &operator<<(TextStream &ts, CompositingReason compositingReason) noexcept
  {
    return ts << CompositingReasonToString(compositingReason);
  }
}
