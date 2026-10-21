#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerClient.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/LayerAncestorClippingStack.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsLayerFactory;
  class LayerOverlapMap;
  class LocalFrameView;
  class Page;
  class RenderLayerBacking;
  class RenderView;
  class Scrollbar;

  enum class CompositingUpdateType : uint8
  {
    AfterStyleChange,
    AfterLayout,
    OnScroll,
    OnCompositedScroll
  };

  enum class CompositingReason : uint8
  {
    Transform3D,
    Video,
    Canvas,
    Plugin,
    IFrame,
    BackfaceVisibilityHidden,
    ClipsCompositingDescendants,
    Animation,
    Filters,
    PositionFixed,
    PositionSticky,
    OverflowScrolling,
    Stacking,
    Overlap,
    OverflowScrollPositioning,
    NegativeZIndexChildren,
    TransformWithCompositedDescendants,
    OpacityWithCompositedDescendants,
    MaskWithCompositedDescendants,
    ReflectionWithCompositedDescendants,
    FilterWithCompositedDescendants,
    BlendingWithCompositedDescendants,
    Perspective,
    Preserve3D,
    WillChange,
    Root,
    IsolatesCompositedBlendingDescendants,
    Model,
    BackdropRoot,
    AnchorPositioning,
  };

  enum class CompositingPolicy : bool
  {
    Normal,
    Conservative, // Used in low memory situations.
  };

  enum class ViewportConstrainedSublayers : uint8
  {
    None,
    Anchor,
    ClippingAndAnchor,
  };

  /// @brief Manages the hierarchy of composited RenderLayers. It determines which RenderLayers become
  /// compositing, and creates and maintains a hierarchy of GraphicsLayers based on the RenderLayer painting
  /// order.
  ///
  /// There is one RenderLayerCompositor per RenderView.
  // FIXME: The scrolling coordinator and asynchronous scrolling, iframes and plug-ins, video, canvas and
  // model elements, view transitions, page overlays, event regions, rubber banding (overhang areas, headers
  // and footers), tiled backings, settings and the compositing policy under memory pressure, once there are
  // any.
  class RenderLayerCompositor final : public GraphicsLayerClient,
                                      public CanMakeCheckedPtr<RenderLayerCompositor>
  {
    KRYS_NON_COPY_MOVABLE(RenderLayerCompositor);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLayerCompositor);

  public:
    struct RequiresCompositingData
    {
      boo::LayoutUpToDate LayoutUpToDate {boo::LayoutUpToDate::Yes};
      RenderLayer::ViewportConstrainedNotCompositedReason NonCompositedForPositionReason {
        RenderLayer::NoNotCompositedReason};
      bool ReevaluateAfterLayout {false};
      bool Intrinsic {false};
    };

    enum RootLayerAttachment : uint8
    {
      RootLayerUnattached,
      RootLayerAttachedViaChromeClient,
      RootLayerAttachedViaEnclosingFrame
    };

    class BackingSharingState;

  private:
    struct CompositingState;
    struct OverlapExtent;
    struct UpdateBackingTraversalState;
    struct BackingSharingSnapshot;

    // Make or destroy the backing for this layer; returns true if backing changed.
    enum class BackingRequired : uint8
    {
      No,
      Yes,
      Unknown
    };

    enum class UpdateLevel : uint8
    {
      AllDescendants,
      CompositedChildren,
    };

    RenderView &_renderView;
    Timer _updateCompositingLayersTimer;
    Timer _updateRenderingTimer;

    bool _hasAcceleratedCompositing {true};

    CompositingPolicy _compositingPolicy {CompositingPolicy::Normal};

    bool _showDebugBorders {false};
    bool _showRepaintCounter {false};
    bool _acceleratedDrawingEnabled {false};
    // FIXME: Composited fixed position layers (acceleratedCompositingForFixedPositionEnabled), once scrolling
    // moves layers.
    bool _acceleratedCompositingForFixedPositionEnabled {false};

    bool _compositing {false};
    bool _flushingLayers {false};
    bool _shouldFlushOnReattach {false};
    bool _forceCompositingMode {false};
    bool _isTrackingRepaints {false}; // Used for testing.

    uint32 _contentLayersCount {0};
    uint32 _layerFlushCount {0};
    uint32 _compositingUpdateCount {0};

    RootLayerAttachment _rootLayerAttachment {RootLayerUnattached};

    RefPtr<GraphicsLayer> _rootContentsLayer;

    // Enclosing clipping layer for iframe content
    RefPtr<GraphicsLayer> _clipLayer;
    RefPtr<GraphicsLayer> _scrollContainerLayer;
    RefPtr<GraphicsLayer> _scrolledContentsLayer;

    // Enclosing layer for overflow controls and the clipping layer
    RefPtr<GraphicsLayer> _overflowControlsHostLayer;

    // Layers for overflow controls
    RefPtr<GraphicsLayer> _layerForHorizontalScrollbar;
    RefPtr<GraphicsLayer> _layerForVerticalScrollbar;
    RefPtr<GraphicsLayer> _layerForScrollCorner;

    bool _viewBackgroundIsTransparent {false};

    Color _viewBackgroundColor;
    Color _rootExtendedBackgroundColor;

  public:
    explicit RenderLayerCompositor(RenderView &renderView) noexcept;
    ~RenderLayerCompositor() noexcept override;

    // Return true if this RenderView is in "compositing mode" (i.e. has one or more composited RenderLayers)
    KRYS_NODISCARD bool UsesCompositing() const noexcept
    {
      return _compositing;
    }
    // This will make a compositing layer at the root automatically, and hook up to the native view/window
    // system.
    void EnableCompositingMode(bool enable = true) noexcept;

    KRYS_NODISCARD bool InForcedCompositingMode() const noexcept
    {
      return _forceCompositingMode;
    }

    // True when some content element other than the root is composited.
    KRYS_NODISCARD bool HasContentCompositingLayers() const noexcept
    {
      return _contentLayersCount;
    }

    // Returns true if the accelerated compositing is enabled
    KRYS_NODISCARD bool HasAcceleratedCompositing() const noexcept
    {
      return _hasAcceleratedCompositing;
    }

    KRYS_NODISCARD bool AcceleratedCompositingForFixedPositionEnabled() const noexcept
    {
      return _acceleratedCompositingForFixedPositionEnabled;
    }

    KRYS_NODISCARD bool CanRender3DTransforms() const noexcept;

    void WillRecalcStyle() noexcept;

    // Returns true if the composited layers were actually updated.
    bool DidRecalcStyleWithNoPendingLayout() noexcept;

    // GraphicsLayers buffer state, which gets pushed to the underlying platform layers at specific times.
    void NotifyFlushRequired(const GraphicsLayer *) noexcept override;
    void NotifySubsequentFlushRequired(const GraphicsLayer *) noexcept override;

    void FlushPendingLayerChanges(bool isFlushRoot = true) noexcept;

    void SetRenderingIsSuppressed(bool suppressed) noexcept;

    // Called when the GraphicsLayer for the given RenderLayer has flushed changes inside of
    // flushPendingLayerChanges().
    void DidChangePlatformLayerForLayer(RenderLayer &, const GraphicsLayer *) noexcept;

    // Called when something outside affects the visible rect (e.g. delegated scrolling). Might schedule a
    // layer flush.
    void DidChangeVisibleRect() noexcept;

    // Rebuild the tree of compositing layers
    bool UpdateCompositingLayers(CompositingUpdateType updateType,
                                 RenderLayer *updateRoot = nullptr) noexcept;
    // This is only used when state changes and we do not exepect a style update or layout to happen soon
    // (e.g. when we discover that an iframe is overlapped during painting).
    void ScheduleCompositingLayerUpdate() noexcept;
    // This is used to cancel any pending update timers when the document goes into back/forward cache.
    void CancelCompositingLayerUpdate() noexcept;

    // Whether layer's backing needs a graphics layer to do clipping by an ancestor (non-stacking-context
    // parent with overflow).
    KRYS_NODISCARD bool ClippedByAncestor(RenderLayer &layer,
                                          const RenderLayer *compositingAncestor) const noexcept;

    bool UpdateAncestorClippingStack(const RenderLayer &layer,
                                     const RenderLayer *compositingAncestor) const noexcept;

    // Whether layer's backing needs a graphics layer to clip z-order children of the given layer.
    KRYS_NODISCARD static bool ClipsCompositingDescendants(const RenderLayer &layer) noexcept;

    // Whether the given layer needs an extra 'contents' layer.
    KRYS_NODISCARD bool NeedsContentsCompositingLayer(const RenderLayer &layer) const noexcept;

    KRYS_NODISCARD bool FixedLayerIntersectsViewport(const RenderLayer &layer) const noexcept;

    KRYS_NODISCARD bool SupportsFixedRootBackgroundCompositing() const noexcept;
    KRYS_NODISCARD bool NeedsFixedRootBackgroundLayer(const RenderLayer &layer) const noexcept;
    KRYS_NODISCARD GraphicsLayer *FixedRootBackgroundLayer() const noexcept;

    void RootOrBodyStyleChanged(RenderElement &renderer, const RenderStyle *oldStyle) noexcept;

    // Called after the view transparency, or the document or base background color change.
    void RootBackgroundColorOrTransparencyChanged() noexcept;

    // Repaint the appropriate layers when the given RenderLayer starts or stops being composited.
    void RepaintOnCompositingChange(RenderLayer &layer,
                                    const RenderLayerModelObject *repaintContainer) noexcept;

    void RepaintInCompositedAncestor(RenderLayer &layer, const LayoutRect &rect) noexcept;

    // Notify us that a layer has been removed
    void LayerWillBeRemoved(RenderLayer &parent, RenderLayer &child) noexcept;

    void LayerStyleChanged(style::Difference diff, RenderLayer &layer, const RenderStyle *oldStyle) noexcept;

    void LayerGainedCompositedScrollableOverflow(RenderLayer &layer) noexcept;

    void EstablishesTopLayerWillChangeForLayer(RenderLayer &layer) noexcept;

    // Get the nearest ancestor layer that has overflow or clip, but is not a stacking context
    KRYS_NODISCARD RenderLayer *EnclosingNonStackingClippingLayer(const RenderLayer &layer) const noexcept;

    // Repaint all composited layers.
    void RepaintCompositedLayers() noexcept;

    // Returns true if the given layer needs it own backing store.
    KRYS_NODISCARD bool RequiresOwnBackingStore(const RenderLayer &layer,
                                                const RenderLayer *compositingAncestorLayer,
                                                const LayoutRect &layerCompositedBoundsInAncestor,
                                                const LayoutRect &ancestorCompositedBounds) const noexcept;

    KRYS_NODISCARD RenderLayer &RootRenderLayer() const noexcept;
    KRYS_NODISCARD GraphicsLayer *RootGraphicsLayer() const noexcept;

    KRYS_NODISCARD GraphicsLayer *ScrollContainerLayer() const noexcept
    {
      return _scrollContainerLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *ScrolledContentsLayer() const noexcept
    {
      return _scrolledContentsLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *ClipLayer() const noexcept
    {
      return _clipLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *RootContentsLayer() const noexcept
    {
      return _rootContentsLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *LayerForClipping() const noexcept
    {
      return _clipLayer ? _clipLayer.get() : _scrollContainerLayer.get();
    }

    KRYS_NODISCARD RootLayerAttachment GetRootLayerAttachment() const noexcept
    {
      return _rootLayerAttachment;
    }
    void UpdateRootLayerAttachment() noexcept;
    void UpdateRootLayerPosition() noexcept;

    void SetIsInWindow(bool isInWindow) noexcept;

    void ClearBackingForAllLayers() noexcept;

    void LayerBecameComposited(const RenderLayer &layer) noexcept;
    void LayerBecameNonComposited(const RenderLayer &layer) noexcept;

    // Walk the tree looking for layers with 3d transforms. Useful in case you need to know if there is
    // non-affine content, e.g. for drawing into an image.
    KRYS_NODISCARD bool Has3DContent() const noexcept;

    // Update the geometry of the layers used for clipping and scrolling in frames.
    void FrameViewDidChangeLocation(FloatPoint contentsOffset) noexcept;
    void FrameViewDidChangeSize() noexcept;
    void FrameViewDidScroll() noexcept;
    void FrameViewDidAddOrRemoveScrollbars() noexcept;
    void FrameViewDidLayout() noexcept;
    void RootLayerConfigurationChanged() noexcept;

    KRYS_NODISCARD utf8_string LayerTreeAsText(EnumSet<LayerTreeAsTextOptions> options = {},
                                               uint32 baseIndent = 0) noexcept;

    KRYS_NODISCARD float DeviceScaleFactor() const noexcept override;
    KRYS_NODISCARD float ContentsScaleMultiplierForNewTiles(const GraphicsLayer *) const noexcept override;
    KRYS_NODISCARD float PageScaleFactor() const noexcept override;
    KRYS_NODISCARD float ZoomedOutPageScaleFactor() const noexcept override;
    KRYS_NODISCARD FloatSize EnclosingFrameViewVisibleSize() const noexcept override;
    void DidChangePlatformLayerForLayer(const GraphicsLayer *) noexcept override
    {
    }

    KRYS_NODISCARD bool AcceleratedDrawingEnabled() const noexcept
    {
      return _acceleratedDrawingEnabled;
    }

    void DeviceOrPageScaleFactorChanged() noexcept;

    KRYS_NODISCARD GraphicsLayer *LayerForHorizontalScrollbar() const noexcept
    {
      return _layerForHorizontalScrollbar.get();
    }
    KRYS_NODISCARD GraphicsLayer *LayerForVerticalScrollbar() const noexcept
    {
      return _layerForVerticalScrollbar.get();
    }
    KRYS_NODISCARD GraphicsLayer *LayerForScrollCorner() const noexcept
    {
      return _layerForScrollCorner.get();
    }

    void UpdateRootContentsLayerBackgroundColor() noexcept;

    KRYS_NODISCARD ViewportConstrainedSublayers GetViewportConstrainedSublayers(
      const RenderLayer &layer, const RenderLayer *compositingAncestor) const noexcept;

    // FIXME: make the coordinated/async terminology consistent.
    KRYS_NODISCARD bool UseCoordinatedScrollingForLayer(const RenderLayer &layer) const noexcept;
    KRYS_NODISCARD ScrollPositioningBehavior ComputeCoordinatedPositioningForLayer(
      const RenderLayer &layer, const RenderLayer *compositingAncestor) const noexcept;

    void ResetTrackedRepaintRects() noexcept;
    void SetTracksRepaints(bool tracksRepaints) noexcept
    {
      _isTrackingRepaints = tracksRepaints;
    }

    KRYS_NODISCARD bool ViewHasTransparentBackground(Color *backgroundColor = nullptr) const noexcept;

    KRYS_NODISCARD EnumSet<CompositingReason> ReasonsForCompositing(const RenderLayer &layer) const noexcept;

    void DidPaintBacking(RenderLayerBacking *backing) noexcept;

    KRYS_NODISCARD const Color &RootExtendedBackgroundColor() const noexcept
    {
      return _rootExtendedBackgroundColor;
    }

    void UpdateRootContentLayerClipping() noexcept;

    KRYS_NODISCARD GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept;

    // For testing.
    void StartTrackingLayerFlushes() noexcept
    {
      _layerFlushCount = 0;
    }
    KRYS_NODISCARD uint32 LayerFlushCount() const noexcept
    {
      return _layerFlushCount;
    }

    void StartTrackingCompositingUpdates() noexcept
    {
      _compositingUpdateCount = 0;
    }
    KRYS_NODISCARD uint32 CompositingUpdateCount() const noexcept
    {
      return _compositingUpdateCount;
    }

  private:
    // GraphicsLayerClient implementation
    void PaintContents(const GraphicsLayer &graphicsLayer, GraphicsContext &context, const FloatRect &clip,
                       EnumSet<GraphicsLayerPaintBehavior>) noexcept override;
    KRYS_NODISCARD bool ShouldDumpPropertyForLayer(const GraphicsLayer *layer, utf8_stringview propertyName,
                                                   EnumSet<LayerTreeAsTextOptions>) const noexcept override;
    KRYS_NODISCARD bool BackdropRootIsOpaque(const GraphicsLayer *layer) const noexcept override;
    KRYS_NODISCARD bool IsFlushingLayers() const noexcept override
    {
      return _flushingLayers;
    }
    KRYS_NODISCARD bool IsTrackingRepaints() const noexcept override
    {
      return _isTrackingRepaints;
    }

    // Copy the accelerated compositing related flags from Settings
    void CacheAcceleratedCompositingFlags() noexcept;
    void CacheAcceleratedCompositingFlagsAfterLayout() noexcept;

    // Whether the given RL needs a compositing layer.
    KRYS_NODISCARD bool NeedsToBeComposited(const RenderLayer &layer,
                                            RequiresCompositingData &queryData) const noexcept;
    // Whether the layer has an intrinsic need for compositing layer.
    KRYS_NODISCARD bool RequiresCompositingLayer(const RenderLayer &layer,
                                                 RequiresCompositingData &queryData) const noexcept;
    // Whether the layer could ever be composited.
    KRYS_NODISCARD bool CanBeComposited(const RenderLayer &layer) const noexcept;

    bool UpdateBacking(RenderLayer &layer, RequiresCompositingData &queryData,
                       BackingSharingState *backingSharingState, BackingRequired backingRequired) noexcept;
    bool UpdateExplicitBacking(RenderLayer &layer, RequiresCompositingData &queryData,
                               BackingRequired backingRequired = BackingRequired::Unknown) noexcept;

    template <typename TApplyFunction>
    void ApplyToCompositedLayerIncludingDescendants(RenderLayer &layer,
                                                    const TApplyFunction &function) noexcept;

    // Repaint this and its child layers.
    void RecursiveRepaintLayer(RenderLayer &layer) noexcept;

    KRYS_NODISCARD bool
      LayerRepaintTargetsBackingSharingLayer(RenderLayer &layer,
                                             BackingSharingState &sharingState) const noexcept;

    void ComputeExtent(const LayerOverlapMap &overlapMap, const RenderLayer &layer,
                       OverlapExtent &extent) const noexcept;
    void ComputeClippingScopes(const RenderLayer &layer, OverlapExtent &extent) const noexcept;
    void AddToOverlapMap(LayerOverlapMap &overlapMap, const RenderLayer &layer,
                         OverlapExtent &extent) const noexcept;
    KRYS_NODISCARD LayoutRect ComputeClippedOverlapBounds(LayerOverlapMap &overlapMap,
                                                          const RenderLayer &layer,
                                                          OverlapExtent &extent) const noexcept;
    void AddDescendantsToOverlapMapRecursive(LayerOverlapMap &overlapMap, const RenderLayer &layer,
                                             const RenderLayer *ancestorLayer = nullptr) const noexcept;
    void UpdateOverlapMap(LayerOverlapMap &overlapMap, const RenderLayer &layer, OverlapExtent &layerExtent,
                          bool didPushContainer, bool addLayerToOverlap,
                          bool addDescendantsToOverlap = false) const noexcept;
    KRYS_NODISCARD bool LayerOverlaps(const LayerOverlapMap &overlapMap, const RenderLayer &layer,
                                      OverlapExtent &extent) const noexcept;

    Maybe<BackingSharingSnapshot> UpdateBackingSharingBeforeDescendantTraversal(
      BackingSharingState &sharingState, uint32 depth, const LayerOverlapMap &overlapMap, RenderLayer &layer,
      OverlapExtent &layerExtent, bool willBeComposited, RenderLayer *stackingContextAncestor) noexcept;
    void UpdateBackingSharingAfterDescendantTraversal(
      BackingSharingState &sharingState, uint32 depth, const LayerOverlapMap &overlapMap, RenderLayer &layer,
      OverlapExtent &layerExtent, RenderLayer *stackingContextAncestor,
      const Maybe<BackingSharingSnapshot> &backingSharingSnapshot) noexcept;

    void ClearBackingProviderSequencesInStackingContextOfLayer(RenderLayer &layer) noexcept;

    void UpdateCompositingLayersTimerFired() noexcept;

    void ComputeCompositingRequirements(RenderLayer *ancestorLayer, RenderLayer &layer,
                                        LayerOverlapMap &overlapMap, CompositingState &compositingState,
                                        BackingSharingState &backingSharingState) noexcept;
    void TraverseUnchangedSubtree(RenderLayer *ancestorLayer, RenderLayer &layer, LayerOverlapMap &overlapMap,
                                  CompositingState &compositingState,
                                  BackingSharingState &backingSharingState) noexcept;

    // Recurses down the tree, parenting descendant compositing layers and collecting an array of child layers
    // for the current compositing layer.
    void UpdateBackingAndHierarchy(RenderLayer &layer,
                                   List<Ref<GraphicsLayer>> &childGraphicsLayersOfEnclosingLayer,
                                   UpdateBackingTraversalState &traversalState,
                                   EnumSet<UpdateLevel> updateLevel = {}) noexcept;

    void AdjustOverflowScrollbarContainerLayers(RenderLayer &stackingContextLayer,
                                                const List<RenderLayer *> &overflowScrollLayers,
                                                const List<RenderLayer *> &layersClippedByScrollers,
                                                List<Ref<GraphicsLayer>> &layerChildren) noexcept;

    KRYS_NODISCARD bool LayerHas3DContent(const RenderLayer &layer) const noexcept;
    KRYS_NODISCARD bool IsRunningTransformAnimation(const RenderLayerModelObject &renderer) const noexcept;

    bool AllowBackingStoreDetachingForFixedPosition(RenderLayer &layer,
                                                    const LayoutRect &absoluteBounds) noexcept;

    KRYS_NODISCARD bool NeedsCompositingForContentOrOverlays() const noexcept;

    void ScheduleRenderingUpdate() noexcept;

    void EnsureRootLayer() noexcept;
    void DestroyRootLayer() noexcept;

    void AttachRootLayer(RootLayerAttachment attachment) noexcept;
    void DetachRootLayer() noexcept;

    void RootLayerAttachmentChanged() noexcept;

    void UpdateOverflowControlsLayers() noexcept;

    void UpdateScrollLayerPosition() noexcept;
    void UpdateScrollLayerClipping() noexcept;
    KRYS_NODISCARD FloatRect InsetClipLayerRect() const noexcept;

    KRYS_NODISCARD FloatRect VisibleRectForLayerFlushing() const noexcept;

    KRYS_NODISCARD Page *GetPage() const noexcept;

    // Non layout-dependent
    KRYS_NODISCARD bool
      RequiresCompositingForAnimation(const RenderLayerModelObject &renderer) const noexcept;
    KRYS_NODISCARD bool
      RequiresCompositingForTransform(const RenderLayerModelObject &renderer) const noexcept;
    KRYS_NODISCARD bool
      RequiresCompositingForBackfaceVisibility(const RenderLayerModelObject &renderer) const noexcept;
    KRYS_NODISCARD bool RequiresCompositingForFilters(const RenderLayerModelObject &renderer) const noexcept;
    KRYS_NODISCARD bool
      RequiresCompositingForWillChange(const RenderLayerModelObject &renderer) const noexcept;

    // Layout-dependent
    KRYS_NODISCARD bool RequiresCompositingForPosition(const RenderLayerModelObject &renderer,
                                                       const RenderLayer &layer,
                                                       RequiresCompositingData &queryData) const noexcept;
    KRYS_NODISCARD bool
      RequiresCompositingForOverflowScrolling(const RenderLayer &layer,
                                              RequiresCompositingData &queryData) const noexcept;
    KRYS_NODISCARD bool RequiresCompositingForAnchorPositioning(const RenderLayer &layer) const noexcept;

    KRYS_NODISCARD IndirectCompositingReason ComputeIndirectCompositingReason(
      const RenderLayer &layer, bool hasCompositedDescendants, bool has3DTransformedDescendants,
      bool paintsIntoProvidedBacking) const noexcept;

    KRYS_NODISCARD static ScrollPositioningBehavior
      LayerScrollBehahaviorRelativeToCompositedAncestor(const RenderLayer &layer,
                                                        const RenderLayer &compositedAncestor) noexcept;

    KRYS_NODISCARD static bool
      StyleChangeMayAffectIndirectCompositingReasons(const RenderStyle &oldStyle,
                                                     const RenderStyle &newStyle) noexcept;

    KRYS_NODISCARD LayoutRoundedRect
      ParentRelativeScrollableRect(const RenderLayer &layer, const RenderLayer *ancestorLayer) const noexcept;

    // Returns list of layers and their clip rects required to clip the given layer, which include clips in
    // the containing block chain between the given layer and its composited ancestor.
    KRYS_NODISCARD List<CompositedClipData>
      ComputeAncestorClippingStack(const RenderLayer &layer,
                                   const RenderLayer *compositingAncestor) const noexcept;

    KRYS_NODISCARD bool RequiresScrollLayer(RootLayerAttachment attachment) const noexcept;
    KRYS_NODISCARD bool RequiresHorizontalScrollbarLayer() const noexcept;
    KRYS_NODISCARD bool RequiresVerticalScrollbarLayer() const noexcept;
    KRYS_NODISCARD bool RequiresScrollCornerLayer() const noexcept;

    // True if the FrameView uses a ScrollingCoordinator.
    KRYS_NODISCARD bool HasCoordinatedScrolling() const noexcept;

    // FIXME: make the coordinated/async terminology consistent.
    KRYS_NODISCARD bool IsAsyncScrollableStickyLayer(
      const RenderLayer &layer,
      const RenderLayer **enclosingAcceleratedOverflowLayer = nullptr) const noexcept;

    KRYS_NODISCARD bool ShouldCompositeOverflowControls() const noexcept;

    KRYS_NODISCARD bool DocumentUsesTiledBacking() const noexcept;
    KRYS_NODISCARD bool IsRootFrameCompositor() const noexcept;
    KRYS_NODISCARD bool IsMainFrameCompositor() const noexcept;

    void UpdateCompositingForLayerTreeAsTextDump() noexcept;
  };

  void PaintScrollbar(Scrollbar *scrollbar, GraphicsContext &context, const IntRect &clip,
                      const Color &backgroundColor = Color()) noexcept;

  TextStream &operator<<(TextStream &ts, CompositingUpdateType updateType) noexcept;
  TextStream &operator<<(TextStream &ts, CompositingPolicy compositingPolicy) noexcept;
  TextStream &operator<<(TextStream &ts, CompositingReason compositingReason) noexcept;
}
