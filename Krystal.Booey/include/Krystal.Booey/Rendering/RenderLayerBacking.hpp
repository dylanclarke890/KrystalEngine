#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerClient.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerEnums.hpp"
#include "Krystal.Booey/Rendering/LayerAncestorClippingStack.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsLayer;
  class PaintedContentsInfo;
  class RenderLayerCompositor;
  class Scrollbar;
  class TransformationMatrix;

  enum CompositingLayerType : uint8
  {
    NormalCompositingLayer,    // non-tiled layer with backing store
    TiledCompositingLayer,     // tiled layer (always has backing store)
    MediaCompositingLayer,     // layer that contains an image, video, WebGL or plugin
    ContainerCompositingLayer, // layer with no backing store
  };

  /// @brief Controls the compositing behavior for a single RenderLayer. It holds the various GraphicsLayers,
  /// and makes decisions about intra-layer rendering optimizations.
  ///
  /// There is one RenderLayerBacking for each RenderLayer that is composited.
  // FIXME: The scrolling coordinator's nodes, event regions, accelerated animations, tiled backings,
  // reflections, video, canvases, plug-ins, models, iframes, view transitions, HDR and display list drawing
  // for tests, once there are any.
  class RenderLayerBacking final : public GraphicsLayerClient
  {
    friend class PaintedContentsInfo;
    KRYS_NON_COPY_MOVABLE(RenderLayerBacking);

  private:
    RenderLayer &_owningLayer;

    // A list other layers that paint into this backing store, later than _owningLayer in paint order.
    ListHashSet<RenderLayer *> _backingSharingLayers;

    // Only used if we are clipped by an ancestor which is not a stacking context.
    UniquePtr<LayerAncestorClippingStack> _ancestorClippingStack;
    // Used when we have an overflow controls host layer which was reparented, and needs clipping by
    // ancestors.
    UniquePtr<LayerAncestorClippingStack> _overflowControlsHostLayerAncestorClippingStack;

    RefPtr<GraphicsLayer>
      _contentsContainmentLayer; // Only used if we have a background layer; takes the transform.
    RefPtr<GraphicsLayer> _graphicsLayer;
    RefPtr<GraphicsLayer>
      _foregroundLayer; // Only used in cases where we need to draw the foreground separately.
    RefPtr<GraphicsLayer>
      _backgroundLayer; // Only used in cases where we need to draw the background separately.
    // Only used if we have clipping on a stacking context with compositing children, or if the layer has a
    // tile cache.
    RefPtr<GraphicsLayer> _childContainmentLayer;
    // Only used on fixed/sticky elements. Contains the viewport anchor layer.
    RefPtr<GraphicsLayer> _viewportClippingLayer;
    RefPtr<GraphicsLayer> _viewportAnchorLayer; // Only used on fixed/sticky elements.
    RefPtr<GraphicsLayer> _maskLayer;           // Only used if we have a mask and/or clip-path.
    RefPtr<GraphicsLayer> _transformFlatteningLayer;

    RefPtr<GraphicsLayer> _layerForHorizontalScrollbar;
    RefPtr<GraphicsLayer> _layerForVerticalScrollbar;
    RefPtr<GraphicsLayer> _layerForScrollCorner;
    RefPtr<GraphicsLayer> _overflowControlsContainer;

    RefPtr<GraphicsLayer> _scrollContainerLayer;  // Only used if the layer is using composited scrolling.
    RefPtr<GraphicsLayer> _scrolledContentsLayer; // Only used if the layer is using composited scrolling.

    LayoutRect _compositedBounds;
    // This is the subpixel distance between the primary graphics layer and the associated renderer's bounds.
    LayoutSize _subpixelOffsetFromRenderer;
    // This is the subpixel distance between the primary graphics layer and the render layer bounds.
    LayoutSize _compositedBoundsOffsetFromGraphicsLayer;

    bool _artificiallyInflatedBounds {false}; // bounds had to be made non-zero to make transform-origin work
    bool _isMainFrameRenderViewLayer {false};
    bool _isRootFrameRenderViewLayer {false};
    bool _isFrameLayerWithTiledBacking {false};
    bool _requiresOwnBackingStore {true};
    bool _canCompositeFilters {false};
    bool _canCompositeBackdropFilters {false};
    bool _backgroundLayerPaintsFixedRootBackground {false};
    bool _requiresBackgroundLayer {false};
    bool _hasSubpixelRounding {false};

  public:
    explicit RenderLayerBacking(RenderLayer &layer) noexcept;
    ~RenderLayerBacking() noexcept override;

    // Do cleanup while layer->backing() is still valid.
    void WillBeDestroyed(EnumSet<UpdateBackingSharingFlags> flags) noexcept;

    KRYS_NODISCARD RenderLayer &OwningLayer() const noexcept
    {
      return _owningLayer;
    }

    // Included layers are non-z-order descendant layers that are painted into this backing.
    KRYS_NODISCARD const ListHashSet<RenderLayer *> &BackingSharingLayers() const noexcept
    {
      return _backingSharingLayers;
    }
    void SetBackingSharingLayers(ListHashSet<RenderLayer *> &&sharingLayers) noexcept;

    KRYS_NODISCARD bool HasBackingSharingLayers() const noexcept
    {
      return !_backingSharingLayers.empty();
    }

    void RemoveBackingSharingLayer(RenderLayer &layer, EnumSet<UpdateBackingSharingFlags> flags) noexcept;
    void ClearBackingSharingLayers(EnumSet<UpdateBackingSharingFlags> flags) noexcept;

    void UpdateConfigurationAfterStyleChange() noexcept;

    // Returns true if layer configuration changed.
    bool UpdateConfiguration(const RenderLayer *compositingAncestor) noexcept;

    // Update graphics layer position and bounds.
    void UpdateGeometry(const RenderLayer *compositingAncestor) noexcept;

    // Update state the requires that descendant layers have been updated.
    void UpdateAfterDescendants() noexcept;

    // Update contents and clipping structure.
    void UpdateDrawsContent() noexcept;

    void UpdateAfterLayout(bool needsClippingUpdate, bool needsFullRepaint) noexcept;

    KRYS_NODISCARD GraphicsLayer *GetGraphicsLayer() const noexcept
    {
      return _graphicsLayer.get();
    }

    // Layer to clip children
    KRYS_NODISCARD bool HasClippingLayer() const noexcept
    {
      return _childContainmentLayer && !_isFrameLayerWithTiledBacking;
    }
    KRYS_NODISCARD GraphicsLayer *ClippingLayer() const noexcept
    {
      return !_isFrameLayerWithTiledBacking ? _childContainmentLayer.get() : nullptr;
    }

    KRYS_NODISCARD bool HasAncestorClippingLayers() const noexcept
    {
      return static_cast<bool>(_ancestorClippingStack);
    }
    KRYS_NODISCARD LayerAncestorClippingStack *AncestorClippingStack() const noexcept
    {
      return _ancestorClippingStack.get();
    }
    bool UpdateAncestorClippingStack(List<CompositedClipData> &&clippingData) noexcept;

    void EnsureOverflowControlsHostLayerAncestorClippingStack(const RenderLayer *compositedAncestor) noexcept;
    KRYS_NODISCARD LayerAncestorClippingStack *OverflowControlsHostLayerAncestorClippingStack() const noexcept
    {
      return _overflowControlsHostLayerAncestorClippingStack.get();
    }

    KRYS_NODISCARD GraphicsLayer *ContentsContainmentLayer() const noexcept
    {
      return _contentsContainmentLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *ViewportAnchorLayer() const noexcept
    {
      return _viewportAnchorLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *ViewportClippingOrAnchorLayer() const noexcept
    {
      return _viewportClippingLayer ? _viewportClippingLayer.get() : ViewportAnchorLayer();
    }

    KRYS_NODISCARD GraphicsLayer *ForegroundLayer() const noexcept
    {
      return _foregroundLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *BackgroundLayer() const noexcept
    {
      return _backgroundLayer.get();
    }
    KRYS_NODISCARD bool BackgroundLayerPaintsFixedRootBackground() const noexcept
    {
      return _backgroundLayerPaintsFixedRootBackground;
    }

    KRYS_NODISCARD bool NeedsRepaintOnCompositedScroll() const noexcept;

    KRYS_NODISCARD bool RequiresBackgroundLayer() const noexcept
    {
      return _requiresBackgroundLayer;
    }
    void SetRequiresBackgroundLayer(bool requiresBackgroundLayer) noexcept;

    KRYS_NODISCARD bool HasScrollingLayer() const noexcept
    {
      return _scrollContainerLayer != nullptr;
    }
    KRYS_NODISCARD GraphicsLayer *ScrollContainerLayer() const noexcept
    {
      return _scrollContainerLayer.get();
    }
    KRYS_NODISCARD GraphicsLayer *ScrolledContentsLayer() const noexcept
    {
      return _scrolledContentsLayer.get();
    }

    KRYS_NODISCARD bool HasMaskLayer() const noexcept
    {
      return static_cast<bool>(_maskLayer);
    }

    KRYS_NODISCARD GraphicsLayer *ParentForSublayers() const noexcept;
    KRYS_NODISCARD GraphicsLayer *ChildForSuperlayers() const noexcept;
    KRYS_NODISCARD GraphicsLayer *ChildForSuperlayersExcludingViewTransitions() const noexcept;

    // RenderLayers with backing normally short-circuit paintLayer() because their content is rendered via
    // callbacks from GraphicsLayer. However, the document layer is special, because it has a GraphicsLayer to
    // act as a container for the GraphicsLayers for descendants, but its contents usually render into the
    // window (in which case this returns true). This returns false for other layers, and when the document
    // layer actually needs to paint into its backing store for some reason.
    KRYS_NODISCARD bool PaintsIntoWindow() const noexcept;

    // Returns true for a composited layer that has no backing store of its own, so paints into some ancestor
    // layer.
    KRYS_NODISCARD bool PaintsIntoCompositedAncestor() const noexcept
    {
      return !_requiresOwnBackingStore;
    }

    void SetRequiresOwnBackingStore(bool requiresOwnBacking) noexcept;

    void SetContentsNeedDisplay(
      GraphicsLayerShouldClipToLayer shouldClip = GraphicsLayerShouldClipToLayer::Clip) noexcept;
    // r is in the coordinate space of the layer's render object
    void SetContentsNeedDisplayInRect(const LayoutRect &r, GraphicsLayerShouldClipToLayer shouldClip =
                                                             GraphicsLayerShouldClipToLayer::Clip) noexcept;

    // Notification from the renderer that its content changed.
    void ContentChanged(ContentChangeType changeType, const Maybe<FloatRect> &dirtyRect) noexcept;

    KRYS_NODISCARD LayoutRect CompositedBounds() const noexcept;
    // Returns true if changed.
    bool SetCompositedBounds(const LayoutRect &bounds) noexcept;
    // Returns true if changed.
    bool UpdateCompositedBounds() noexcept;

    void UpdateAllowsBackingStoreDetaching(bool allowDetachingForFixed) noexcept;

    void PositionOverflowControlsLayers() noexcept;

    KRYS_NODISCARD bool IsFrameLayerWithTiledBacking() const noexcept
    {
      return _isFrameLayerWithTiledBacking;
    }

    void AdjustTiledBackingCoverage() noexcept;

    void UpdateDebugIndicators(bool showBorder, bool showRepaintCounter) noexcept;

    bool StartAnimation(double timeOffset, const GraphicsLayerAnimation &animation,
                        const BlendingKeyframes &keyframes) noexcept;
    void AnimationPaused(double timeOffset, const utf8_string &animationName) noexcept;
    void AnimationFinished(const utf8_string &animationName) noexcept;
    void TransformRelatedPropertyDidChange() noexcept;
    void SuspendAnimations(MonotonicTime::duration time = {}) noexcept;
    void ResumeAnimations() noexcept;

    // GraphicsLayerClient interface
    void TiledBackingUsageChanged(const GraphicsLayer *, bool usingTiledBacking) noexcept override;
    void NotifyAnimationStarted(const GraphicsLayer *, const utf8_string &,
                                MonotonicTime::duration) noexcept override;
    void NotifyFlushRequired(const GraphicsLayer *layer) noexcept override;
    void NotifySubsequentFlushRequired(const GraphicsLayer *layer) noexcept override;

    void PaintContents(const GraphicsLayer &graphicsLayer, GraphicsContext &context, const FloatRect &clip,
                       EnumSet<GraphicsLayerPaintBehavior> layerPaintBehavior) noexcept override;

    KRYS_NODISCARD float DeviceScaleFactor() const noexcept override;
    KRYS_NODISCARD float
      ContentsScaleMultiplierForNewTiles(const GraphicsLayer *layer) const noexcept override;

    KRYS_NODISCARD bool PaintsOpaquelyAtNonIntegralScales(const GraphicsLayer *) const noexcept override;

    KRYS_NODISCARD float PageScaleFactor() const noexcept override;
    KRYS_NODISCARD float ZoomedOutPageScaleFactor() const noexcept override;

    KRYS_NODISCARD FloatSize EnclosingFrameViewVisibleSize() const noexcept override;

    void DidChangePlatformLayerForLayer(const GraphicsLayer *layer) noexcept override;
    bool GetCurrentTransform(const GraphicsLayer *graphicsLayer,
                             TransformationMatrix &transform) const noexcept override;

    KRYS_NODISCARD bool IsFlushingLayers() const noexcept override;
    KRYS_NODISCARD bool IsTrackingRepaints() const noexcept override;
    KRYS_NODISCARD bool
      ShouldSkipLayerInDump(const GraphicsLayer *layer,
                            EnumSet<LayerTreeAsTextOptions> options) const noexcept override;
    KRYS_NODISCARD bool
      ShouldDumpPropertyForLayer(const GraphicsLayer *layer, utf8_stringview propertyName,
                                 EnumSet<LayerTreeAsTextOptions> options) const noexcept override;

    KRYS_NODISCARD bool NeedsPixelAligment() const noexcept override
    {
      return !_isMainFrameRenderViewLayer;
    }

    KRYS_NODISCARD LayoutSize SubpixelOffsetFromRenderer() const noexcept
    {
      return _subpixelOffsetFromRenderer;
    }

    KRYS_NODISCARD LayoutRect ContentsBox() const noexcept;

    // For informative purposes only.
    KRYS_NODISCARD CompositingLayerType GetCompositingLayerType() const noexcept;

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
    KRYS_NODISCARD GraphicsLayer *OverflowControlsContainer() const noexcept
    {
      return _overflowControlsContainer.get();
    }

    KRYS_NODISCARD GraphicsLayer *LayerForContents() const noexcept;

    void AdjustOverflowControlsPositionRelativeToAncestor(const RenderLayer &ancestorLayer) noexcept;

    KRYS_NODISCARD bool CanCompositeFilters() const noexcept
    {
      return _canCompositeFilters;
    }
    KRYS_NODISCARD bool CanCompositeBackdropFilters() const noexcept
    {
      return _canCompositeBackdropFilters;
    }

    // For testing only.
    void SetUsesDisplayListDrawing(bool usesDisplayListDrawing) noexcept;

  private:
    KRYS_NODISCARD FloatRect BackgroundBoxForSimpleContainerPainting() const noexcept;

    void CreatePrimaryGraphicsLayer() noexcept;
    void DestroyGraphicsLayers() noexcept;

    void WillDestroyLayer(const GraphicsLayer *layer) noexcept;

    KRYS_NODISCARD LayoutRect CompositedBoundsIncludingMargin() const noexcept;

    KRYS_NODISCARD Ref<GraphicsLayer>
      CreateGraphicsLayer(const utf8_string &name,
                          GraphicsLayer::Type layerType = GraphicsLayer::Type::Normal) noexcept;

    KRYS_NODISCARD RenderLayerModelObject &Renderer() const noexcept
    {
      return _owningLayer.Renderer();
    }
    KRYS_NODISCARD RenderBox *GetRenderBox() const noexcept
    {
      return _owningLayer.GetRenderBox();
    }
    KRYS_NODISCARD RenderLayerCompositor &Compositor() const noexcept
    {
      return _owningLayer.Compositor();
    }

    void UpdateInternalHierarchy() noexcept;
    bool UpdateViewportConstrainedSublayers(ViewportConstrainedSublayers sublayers) noexcept;
    bool UpdateAncestorClipping(bool needsAncestorClip, const RenderLayer *compositingAncestor) noexcept;
    bool UpdateDescendantClippingLayer(bool needsDescendantClip) noexcept;
    bool UpdateOverflowControlsLayers(bool needsHorizontalScrollbarLayer, bool needsVerticalScrollbarLayer,
                                      bool needsScrollCornerLayer) noexcept;
    bool UpdateForegroundLayer(bool needsForegroundLayer) noexcept;
    bool UpdateBackgroundLayer(bool needsBackgroundLayer) noexcept;
    bool UpdateMaskingLayer(bool hasMask, bool hasClipPath) noexcept;
    bool UpdateTransformFlatteningLayer(const RenderLayer *compositingAncestor) noexcept;

    KRYS_NODISCARD bool RequiresLayerForScrollbar(Scrollbar *scrollbar) const noexcept;
    KRYS_NODISCARD bool RequiresHorizontalScrollbarLayer() const noexcept;
    KRYS_NODISCARD bool RequiresVerticalScrollbarLayer() const noexcept;
    KRYS_NODISCARD bool RequiresScrollCornerLayer() const noexcept;
    bool UpdateScrollingLayers(bool needsScrollingLayers) noexcept;

    void UpdateScrollOffset(ScrollOffset scrollOffset) noexcept;

    void UpdateMaskingLayerGeometry() noexcept;
    void UpdateRootLayerConfiguration() noexcept;
    void UpdatePaintingPhases() noexcept;

    void SetBackgroundLayerPaintsFixedRootBackground(bool backgroundLayerPaintsFixedRootBackground) noexcept;

    KRYS_NODISCARD LayoutSize ContentOffsetInCompositingLayer() const noexcept;
    KRYS_NODISCARD LayoutSize OffsetRelativeToRendererOriginForDescendantLayers() const noexcept;

    void EnsureClippingStackLayers(LayerAncestorClippingStack &clippingStack) noexcept;
    void RemoveClippingStackLayers(LayerAncestorClippingStack &clippingStack) noexcept;

    void UpdateClippingStackLayerGeometry(LayerAncestorClippingStack &clippingStack,
                                          const RenderLayer *compositedAncestor,
                                          LayoutRect &parentGraphicsLayerRect) noexcept;

    void ConnectClippingStackLayers(LayerAncestorClippingStack &clippingStack) noexcept;

    void UpdateOpacity(const RenderStyle &style) noexcept;
    void UpdateTransform(const RenderStyle &style) noexcept;
    void UpdateChildrenTransformAndAnchorPoint(const LayoutRect &primaryGraphicsLayerRect,
                                               LayoutSize offsetFromParentGraphicsLayer) noexcept;
    void UpdateFilters(const RenderStyle &style) noexcept;
    void UpdateBackdropFilters(const RenderStyle &style) noexcept;
    void UpdateBackdropFiltersGeometry() noexcept;
    bool UpdateBackdropRoot() noexcept;
    void UpdateBlendMode(const RenderStyle &style) noexcept;
    void UpdateContentsScalingFilters(const RenderStyle &style) noexcept;

    // Return the opacity value that this layer should use for compositing.
    KRYS_NODISCARD float CompositingOpacity(float rendererOpacity) const noexcept;
    KRYS_NODISCARD Color RendererBackgroundColor() const noexcept;

    KRYS_NODISCARD bool PaintsBoxDecorations() const noexcept;
    void DeterminePaintsContent(RenderLayer::PaintedContentRequest &request) const noexcept;

    void UpdateDrawsContent(PaintedContentsInfo &contentsInfo) noexcept;

    // Returns true if this compositing layer has no visible content.
    KRYS_NODISCARD bool IsSimpleContainerCompositingLayer(PaintedContentsInfo &contentsInfo) const noexcept;
    // Returns true if this layer has content that needs to be rendered by painting into the backing store.
    KRYS_NODISCARD bool ContainsPaintedContent(PaintedContentsInfo &contentsInfo) const noexcept;
    // Returns true if the RenderLayer just contains an image that we can composite directly.
    KRYS_NODISCARD bool IsDirectlyCompositedImage() const noexcept;
    void UpdateImageContents(PaintedContentsInfo &contentsInfo) noexcept;
    KRYS_NODISCARD bool IsUnscaledBitmapOnly() const noexcept;
    KRYS_NODISCARD bool IsBitmapOnly() const noexcept;

    void UpdateDirectlyCompositedBoxDecorations(PaintedContentsInfo &contentsInfo,
                                                bool &didUpdateContentsRect) noexcept;
    void UpdateDirectlyCompositedBackgroundColor(PaintedContentsInfo &contentsInfo,
                                                 bool &didUpdateContentsRect) noexcept;
    void UpdateDirectlyCompositedBackgroundImage(PaintedContentsInfo &contentsInfo,
                                                 bool &didUpdateContentsRect) noexcept;

    void ResetContentsRect() noexcept;
    void UpdateContentsRects() noexcept;

    void DetermineNonCompositedLayerDescendantsPaintedContent(
      RenderLayer::PaintedContentRequest &request) const noexcept;
    KRYS_NODISCARD bool HasVisibleNonCompositedDescendants() const noexcept;

    KRYS_NODISCARD bool ShouldClipCompositedBounds() const noexcept;

    KRYS_NODISCARD bool HasTiledBackingFlatteningLayer() const noexcept
    {
      return _childContainmentLayer && _isFrameLayerWithTiledBacking;
    }
    KRYS_NODISCARD GraphicsLayer *TileCacheFlatteningLayer() const noexcept
    {
      return _isFrameLayerWithTiledBacking ? _childContainmentLayer.get() : nullptr;
    }

    void PaintIntoLayer(const GraphicsLayer *graphicsLayer, GraphicsContext &context,
                        const IntRect &paintDirtyRect, EnumSet<PaintBehavior> paintBehavior) noexcept;
    KRYS_NODISCARD EnumSet<RenderLayer::PaintLayerFlag>
      PaintFlagsForLayer(const GraphicsLayer &graphicsLayer) const noexcept;

    KRYS_NODISCARD bool CanIssueSetNeedsDisplay() const noexcept
    {
      return !PaintsIntoWindow() && !PaintsIntoCompositedAncestor();
    }
    KRYS_NODISCARD LayoutRect
      ComputeParentGraphicsLayerRect(const RenderLayer *compositedAncestor) const noexcept;
    KRYS_NODISCARD LayoutRect ComputePrimaryGraphicsLayerRect(
      const RenderLayer *compositedAncestor, const LayoutRect &parentGraphicsLayerRect) const noexcept;
  };

  TextStream &operator<<(TextStream &ts, const RenderLayerBacking &backing) noexcept;
}
