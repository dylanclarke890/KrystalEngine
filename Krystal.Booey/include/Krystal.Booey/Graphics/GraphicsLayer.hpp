#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerClient.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerEnums.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/PlatformLayerIdentifier.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsContext;
  class GraphicsLayerAnimation;
  class GraphicsLayerFactory;
  class GraphicsLayerKeyframeValueList;
  class Image;
  class GraphicsLayerContentsDisplayDelegate;
  class ImageBuffer;

  using TileCoverage = uint32;

  KRYS_NODISCARD utf8_stringview AnimatedPropertyIDAsString(AnimatedProperty property) noexcept;

  /// @brief An abstraction for a rendering surface with backing store, which may have associated
  /// transformation and animations.
  // FIXME: Event regions (EventRegion), once there's touch and interaction region tracking; asynchronous
  // contents display delegates and video gravity, once there are videos; tiled
  // backings (TiledBacking), which only Core Animation's layers have; threaded animations
  // (AcceleratedEffectStack), with threaded compositing; scrolling node IDs, with async scrolling.
  class GraphicsLayer : public RefCounted<GraphicsLayer>, public CanMakeWeakPtr<GraphicsLayer>
  {
  public:
    // Enums from GraphicsLayerEnums.hpp:
    using Type = GraphicsLayerType;
    using LayerMode = GraphicsLayerMode;
    using ShouldSetNeedsDisplay = GraphicsLayerShouldSetNeedsDisplay;
    using ShouldClipToLayer = GraphicsLayerShouldClipToLayer;
    using CompositingCoordinatesOrientation = GraphicsLayerCompositingCoordinatesOrientation;
    using ScalingFilter = GraphicsLayerScalingFilter;
    using CustomAppearance = GraphicsLayerCustomAppearance;
    using ContentsLayerPurpose = GraphicsLayerContentsLayerPurpose;

    struct AcceleratedAnimationForTesting
    {
      utf8_string Property;
      double Speed;
      bool IsThreaded;
    };

  protected:
    GraphicsLayerClient *_client; // Always non-null.
    utf8_string _name;

    // Offset from the owning renderer
    FloatSize _offsetFromRenderer;

    // Scroll offset of the content layer inside its scrolling parent layer.
    ScrollOffset _scrollOffset;

    // Position is relative to the parent GraphicsLayer
    FloatPoint _position;

    // If set, overrides _position. Only used for coverage computation.
    Maybe<FloatPoint> _approximatePosition;

    FloatPoint3D _anchorPoint {0.5f, 0.5f, 0};
    FloatSize _size;
    FloatPoint _boundsOrigin;

    UniquePtr<TransformationMatrix> _transform;
    UniquePtr<TransformationMatrix> _childrenTransform;

    Color _backgroundColor;
    float _opacity {1};
    float _zPosition {0};

    FilterOperations _filters;
    FilterOperations _backdropFilters;

    BlendMode _blendMode {BlendMode::Normal};

    const Type _type;
    CustomAppearance _customAppearance {CustomAppearance::None};

    EnumSet<GraphicsLayerPaintingPhase> _paintingPhase {GraphicsLayerPaintingPhase::Foreground,
                                                        GraphicsLayerPaintingPhase::Background};
    // affects orientation of layer contents
    CompositingCoordinatesOrientation _contentsOrientation {CompositingCoordinatesOrientation::TopDown};

    bool _beingDestroyed : 1;
    bool _contentsOpaque : 1;
    bool _preserves3D : 1;
    bool _backfaceVisibility : 1;
    bool _masksToBounds : 1;
    bool _drawsContent : 1;
    bool _contentsVisible : 1;
    bool _contentsRectClipsDescendants : 1;
    bool _acceleratesDrawing : 1;
    bool _usesDisplayListDrawing : 1;
    bool _allowsTiling : 1;
    bool _appliesPageScale : 1; // Set for the layer which has the page scale applied to it.
    bool _appliesDeviceScale : 1;
    bool _showDebugBorder : 1;
    bool _showRepaintCounter : 1;
    bool _showFrameProcessBorders : 1;
    bool _isMaskLayer : 1;
    bool _isBackdropRoot : 1;
    bool _isTrackingDisplayListReplay : 1;
    bool _userInteractionEnabled : 1;
    bool _canDetachBackingStore : 1;
    bool _shouldPaintUsingCompositeCopy : 1;
    bool _renderingIsSuppressedIncludingDescendants : 1 {false};

    int32 _repaintCount {0};

    List<Ref<GraphicsLayer>> _children;
    GraphicsLayer *_parent {nullptr};

    RefPtr<GraphicsLayer> _maskLayer; // Reference to mask layer.

    // A layer that replicates this layer. We only allow one, for now. The replica is not parented; this is
    // the primary reference to it.
    RefPtr<GraphicsLayer> _replicaLayer;
    GraphicsLayer *_replicatedLayer {nullptr}; // For a replica layer, a reference to the original layer.
    FloatPoint _replicatedLayerPosition;       // For a replica layer, the position of the replica.

    FloatRect _contentsRect;
    FloatRoundedRect _contentsClippingRect;
    FloatSize _contentsTilePhase;
    FloatSize _contentsTileSize;
    ScalingFilter _contentsMinificationFilter {ScalingFilter::Linear};
    ScalingFilter _contentsMagnificationFilter {ScalingFilter::Linear};
    FloatRoundedRect _backdropFiltersRect;
    Maybe<FloatRect> _animationExtent;

  public:
    KRYS_NODISCARD virtual LayerMode GetLayerMode() const noexcept
    {
      return LayerMode::PlatformLayer;
    }

    /// @brief A layer from @p factory, which must be given: graphics layers are the host's.
    KRYS_NODISCARD static Ref<GraphicsLayer>
      Create(GraphicsLayerFactory *factory, GraphicsLayerClient &client, Type type = Type::Normal) noexcept;

    virtual ~GraphicsLayer() noexcept;

    // Unparent, clear the client, and clear the RefPtr.
    static void UnparentAndClear(RefPtr<GraphicsLayer> &layer) noexcept;
    // Clear the client, and clear the RefPtr, but leave parented.
    static void Clear(RefPtr<GraphicsLayer> &layer) noexcept;

    void ClearClient() noexcept;
    void SetClient(GraphicsLayerClient &client) noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    virtual void Initialize(Type) noexcept
    {
    }

    KRYS_NODISCARD virtual Maybe<PlatformLayerIdentifier> PrimaryLayerID() const noexcept
    {
      return null;
    }
    KRYS_NODISCARD virtual Maybe<PlatformLayerIdentifier> LayerIDIgnoringStructuralLayer() const noexcept
    {
      return PrimaryLayerID();
    }

    KRYS_NODISCARD GraphicsLayerClient &Client() const noexcept
    {
      return *_client;
    }

    // Layer name. Only used to identify layers in debug output
    KRYS_NODISCARD const utf8_string &Name() const noexcept
    {
      return _name;
    }
    virtual void SetName(const utf8_string &name) noexcept
    {
      _name = name;
    }
    KRYS_NODISCARD virtual utf8_string DebugName() const noexcept;

    KRYS_NODISCARD GraphicsLayer *Parent() const noexcept
    {
      return _parent;
    }
    void SetParent(GraphicsLayer *layer) noexcept; // Internal use only.

    // Returns true if the layer has the given layer as an ancestor (excluding self).
    KRYS_NODISCARD bool HasAncestor(GraphicsLayer *ancestor) const noexcept;

    KRYS_NODISCARD const List<Ref<GraphicsLayer>> &Children() const noexcept
    {
      return _children;
    }
    KRYS_NODISCARD List<Ref<GraphicsLayer>> &Children() noexcept
    {
      return _children;
    }

    // Returns true if the child list changed.
    virtual bool SetChildren(List<Ref<GraphicsLayer>> &&newChildren) noexcept;

    // Add child layers. If the child is already parented, it will be removed from its old parent.
    virtual void AddChild(Ref<GraphicsLayer> &&childLayer) noexcept;
    virtual void AddChildAtIndex(Ref<GraphicsLayer> &&childLayer, int32 index) noexcept;
    virtual void AddChildAbove(Ref<GraphicsLayer> &&childLayer, GraphicsLayer *sibling) noexcept;
    virtual void AddChildBelow(Ref<GraphicsLayer> &&childLayer, GraphicsLayer *sibling) noexcept;
    virtual bool ReplaceChild(GraphicsLayer *oldChild, Ref<GraphicsLayer> &&newChild) noexcept;

    void RemoveAllChildren() noexcept;
    void RemoveFromParent() noexcept;

    // The parent() of a maskLayer is set to the layer being masked.
    KRYS_NODISCARD GraphicsLayer *MaskLayer() const noexcept
    {
      return _maskLayer.get();
    }
    virtual void SetMaskLayer(RefPtr<GraphicsLayer> &&layer) noexcept;

    void SetIsMaskLayer(bool isMask) noexcept
    {
      _isMaskLayer = isMask;
    }
    KRYS_NODISCARD bool IsMaskLayer() const noexcept
    {
      return _isMaskLayer;
    }

    virtual void SetIsBackdropRoot(bool isBackdropRoot) noexcept
    {
      _isBackdropRoot = isBackdropRoot;
    }
    KRYS_NODISCARD bool IsBackdropRoot() const noexcept
    {
      return _isBackdropRoot;
    }

    // The given layer will replicate this layer and its children; the replica renders behind this layer.
    virtual void SetReplicatedByLayer(RefPtr<GraphicsLayer> &&layer) noexcept;
    // Whether this layer is being replicated by another layer.
    KRYS_NODISCARD bool IsReplicated() const noexcept
    {
      return _replicaLayer != nullptr;
    }
    // The layer that replicates this layer (if any).
    KRYS_NODISCARD GraphicsLayer *ReplicaLayer() const noexcept
    {
      return _replicaLayer.get();
    }

    KRYS_NODISCARD const FloatPoint &ReplicatedLayerPosition() const noexcept
    {
      return _replicatedLayerPosition;
    }
    void SetReplicatedLayerPosition(const FloatPoint &p) noexcept
    {
      _replicatedLayerPosition = p;
    }

    // Offset is origin of the renderer minus origin of the graphics layer.
    KRYS_NODISCARD FloatSize OffsetFromRenderer() const noexcept
    {
      return _offsetFromRenderer;
    }
    void SetOffsetFromRenderer(const FloatSize &offset, ShouldSetNeedsDisplay shouldSetNeedsDisplay =
                                                          ShouldSetNeedsDisplay::Set) noexcept;

    // Scroll offset of the content layer inside its scrolling parent layer.
    KRYS_NODISCARD ScrollOffset GetScrollOffset() const noexcept
    {
      return _scrollOffset;
    }
    void SetScrollOffset(const ScrollOffset &offset,
                         ShouldSetNeedsDisplay shouldSetNeedsDisplay = ShouldSetNeedsDisplay::Set) noexcept;

    // The position of the layer (the location of its top-left corner in its parent)
    KRYS_NODISCARD const FloatPoint &Position() const noexcept
    {
      return _position;
    }
    virtual void SetPosition(const FloatPoint &p) noexcept
    {
      _approximatePosition = null;
      _position = p;
    }

    // approximatePosition, if set, overrides position() and is used during coverage rect computation.
    KRYS_NODISCARD FloatPoint ApproximatePosition() const noexcept
    {
      return _approximatePosition ? _approximatePosition.value() : _position;
    }
    virtual void SetApproximatePosition(const FloatPoint &p) noexcept
    {
      _approximatePosition = p;
    }

    // For platforms that move underlying platform layers on a different thread for scrolling; just update the
    // GraphicsLayer state.
    virtual void SyncPosition(const FloatPoint &p) noexcept
    {
      _approximatePosition = null;
      _position = p;
    }

    // Anchor point: (0, 0) is top left, (1, 1) is bottom right. The anchor point affects the origin of the
    // transforms.
    KRYS_NODISCARD const FloatPoint3D &AnchorPoint() const noexcept
    {
      return _anchorPoint;
    }
    virtual void SetAnchorPoint(const FloatPoint3D &p) noexcept
    {
      _anchorPoint = p;
    }

    // The size of the layer.
    KRYS_NODISCARD const FloatSize &Size() const noexcept
    {
      return _size;
    }
    virtual void SetSize(const FloatSize &size) noexcept;

    // The boundOrigin affects the offset at which content is rendered, and sublayers are positioned.
    KRYS_NODISCARD const FloatPoint &BoundsOrigin() const noexcept
    {
      return _boundsOrigin;
    }
    virtual void SetBoundsOrigin(const FloatPoint &origin) noexcept
    {
      _boundsOrigin = origin;
    }

    // For platforms that move underlying platform layers on a different thread for scrolling; just update the
    // GraphicsLayer state.
    virtual void SyncBoundsOrigin(const FloatPoint &origin) noexcept
    {
      _boundsOrigin = origin;
    }

    KRYS_NODISCARD const TransformationMatrix &Transform() const noexcept;
    virtual void SetTransform(const TransformationMatrix &matrix) noexcept;
    KRYS_NODISCARD bool HasNonIdentityTransform() const noexcept
    {
      return _transform && !_transform->IsIdentity();
    }

    KRYS_NODISCARD const TransformationMatrix &ChildrenTransform() const noexcept;
    virtual void SetChildrenTransform(const TransformationMatrix &matrix) noexcept;
    KRYS_NODISCARD bool HasNonIdentityChildrenTransform() const noexcept
    {
      return _childrenTransform && !_childrenTransform->IsIdentity();
    }

    KRYS_NODISCARD bool Preserves3D() const noexcept
    {
      return _preserves3D;
    }
    virtual void SetPreserves3D(bool b) noexcept;

    KRYS_NODISCARD bool MasksToBounds() const noexcept
    {
      return _masksToBounds;
    }
    virtual void SetMasksToBounds(bool b) noexcept;

    KRYS_NODISCARD bool DrawsContent() const noexcept
    {
      return _drawsContent;
    }
    virtual void SetDrawsContent(bool b) noexcept;

    KRYS_NODISCARD bool ContentsAreVisible() const noexcept
    {
      return _contentsVisible;
    }
    virtual void SetContentsVisible(bool b) noexcept
    {
      _contentsVisible = b;
    }

    KRYS_NODISCARD bool UserInteractionEnabled() const noexcept
    {
      return _userInteractionEnabled;
    }
    virtual void SetUserInteractionEnabled(bool b) noexcept
    {
      _userInteractionEnabled = b;
    }

    KRYS_NODISCARD bool AcceleratesDrawing() const noexcept
    {
      return _acceleratesDrawing;
    }
    virtual void SetAcceleratesDrawing(bool b) noexcept
    {
      _acceleratesDrawing = b;
    }

    KRYS_NODISCARD bool UsesDisplayListDrawing() const noexcept
    {
      return _usesDisplayListDrawing;
    }
    virtual void SetUsesDisplayListDrawing(bool b) noexcept
    {
      _usesDisplayListDrawing = b;
    }

    KRYS_NODISCARD bool NeedsBackdrop() const noexcept;

    // The color used to paint the layer background. Pass an invalid color to remove it. Note that this covers
    // the entire layer. Use setContentsToSolidColor() if the color should only cover the contentsRect.
    KRYS_NODISCARD const Color &BackgroundColor() const noexcept
    {
      return _backgroundColor;
    }
    virtual void SetBackgroundColor(const Color &color) noexcept;

    // opaque means that we know the layer contents have no alpha
    KRYS_NODISCARD bool ContentsOpaque() const noexcept
    {
      return _contentsOpaque;
    }
    virtual void SetContentsOpaque(bool b) noexcept
    {
      _contentsOpaque = b;
    }

    KRYS_NODISCARD bool BackfaceVisibility() const noexcept
    {
      return _backfaceVisibility;
    }
    virtual void SetBackfaceVisibility(bool b) noexcept
    {
      _backfaceVisibility = b;
    }

    KRYS_NODISCARD float Opacity() const noexcept
    {
      return _opacity;
    }
    virtual void SetOpacity(float opacity) noexcept;

    KRYS_NODISCARD const FilterOperations &Filters() const noexcept
    {
      return _filters;
    }
    // Returns true if filter can be rendered by the compositor.
    virtual bool SetFilters(const FilterOperations &filters) noexcept;

    KRYS_NODISCARD const FilterOperations &BackdropFilters() const noexcept
    {
      return _backdropFilters;
    }
    virtual bool SetBackdropFilters(const FilterOperations &filters) noexcept
    {
      _backdropFilters = filters;
      return true;
    }

    virtual void SetBackdropFiltersRect(const FloatRoundedRect &backdropFiltersRect) noexcept
    {
      _backdropFiltersRect = backdropFiltersRect;
    }
    KRYS_NODISCARD const FloatRoundedRect &BackdropFiltersRect() const noexcept
    {
      return _backdropFiltersRect;
    }

    KRYS_NODISCARD BlendMode GetBlendMode() const noexcept
    {
      return _blendMode;
    }
    virtual void SetBlendMode(BlendMode blendMode) noexcept
    {
      _blendMode = blendMode;
    }

    // Some GraphicsLayers paint only the foreground or the background content
    KRYS_NODISCARD EnumSet<GraphicsLayerPaintingPhase> PaintingPhase() const noexcept
    {
      return _paintingPhase;
    }
    void SetPaintingPhase(EnumSet<GraphicsLayerPaintingPhase> phase) noexcept;

    virtual void SetNeedsDisplay() noexcept = 0;
    // mark the given rect (in layer coords) as needing dispay. Never goes deep.
    virtual void SetNeedsDisplayInRect(const FloatRect &rect,
                                       ShouldClipToLayer shouldClip = ShouldClipToLayer::Clip) noexcept = 0;

    virtual void SetContentsNeedsDisplay() noexcept
    {
    }
    virtual void SetContentsNeedsDisplayInRect(const FloatRect &) noexcept
    {
      SetContentsNeedsDisplay();
    }

    // The tile phase is relative to the GraphicsLayer bounds.
    virtual void SetContentsTilePhase(const FloatSize &p) noexcept
    {
      _contentsTilePhase = p;
    }
    KRYS_NODISCARD FloatSize ContentsTilePhase() const noexcept
    {
      return _contentsTilePhase;
    }

    virtual void SetContentsTileSize(const FloatSize &s) noexcept
    {
      _contentsTileSize = s;
    }
    KRYS_NODISCARD FloatSize ContentsTileSize() const noexcept
    {
      return _contentsTileSize;
    }
    KRYS_NODISCARD bool HasContentsTiling() const noexcept
    {
      return !_contentsTileSize.IsEmpty();
    }

    // Set that the position/size of the contents (image or video).
    KRYS_NODISCARD FloatRect ContentsRect() const noexcept
    {
      return _contentsRect;
    }
    virtual void SetContentsRect(const FloatRect &r) noexcept
    {
      _contentsRect = r;
    }

    // Set a rounded rect that will be used to clip the layer contents.
    KRYS_NODISCARD FloatRoundedRect ContentsClippingRect() const noexcept
    {
      return _contentsClippingRect;
    }
    virtual void SetContentsClippingRect(const FloatRoundedRect &roundedRect) noexcept
    {
      _contentsClippingRect = roundedRect;
    }

    // If true, contentsClippingRect is used to clip child GraphicsLayers.
    KRYS_NODISCARD bool ContentsRectClipsDescendants() const noexcept
    {
      return _contentsRectClipsDescendants;
    }
    virtual void SetContentsRectClipsDescendants(bool b) noexcept
    {
      _contentsRectClipsDescendants = b;
    }

    KRYS_NODISCARD Path ShapeLayerPath() const noexcept;
    virtual void SetShapeLayerPath(const Path &path) noexcept;

    KRYS_NODISCARD WindRule ShapeLayerWindRule() const noexcept;
    virtual void SetShapeLayerWindRule(WindRule windRule) noexcept;

    // Transitions are identified by a special animation name that cannot clash with a keyframe identifier.
    KRYS_NODISCARD static utf8_string AnimationNameForTransition(AnimatedProperty property) noexcept;

    // Return true if the animation is handled by the compositing system.
    virtual bool AddAnimation(const GraphicsLayerKeyframeValueList &, const GraphicsLayerAnimation *,
                              const utf8_string & /*animationName*/, double /*timeOffset*/) noexcept
    {
      return false;
    }
    virtual void PauseAnimation(const utf8_string & /*animationName*/, double /*timeOffset*/) noexcept
    {
    }
    virtual void RemoveAnimation(const utf8_string & /*animationName*/, Maybe<AnimatedProperty>) noexcept
    {
    }
    virtual void TransformRelatedPropertyDidChange() noexcept
    {
    }
    virtual void SuspendAnimations(MonotonicTime::duration) noexcept;
    virtual void ResumeAnimations() noexcept;

    KRYS_NODISCARD virtual List<AcceleratedAnimationForTesting>
      AcceleratedAnimationsForTesting() const noexcept
    {
      return {};
    }

    // Layer contents
    virtual void SetContentsToImage(Image *) noexcept
    {
    }
    KRYS_NODISCARD virtual bool ShouldDirectlyCompositeImage(Image *) const noexcept
    {
      return true;
    }

    // FIXME: Merge this with setContentsToImage once we can efficiently convert an ImageBuffer to NativeImage
    // without GPUP readback.
    virtual void SetContentsToImageBuffer(ImageBuffer *) noexcept
    {
    }
    KRYS_NODISCARD virtual bool ShouldDirectlyCompositeImageBuffer(ImageBuffer *) const noexcept
    {
      return false;
    }

    // Pass an invalid color to remove the contents layer.
    virtual void SetContentsToSolidColor(const Color &) noexcept
    {
    }

    /// @brief Shows what @p delegate displays as the layer's contents, or removes them if it's null.
    virtual void SetContentsDisplayDelegate(RefPtr<GraphicsLayerContentsDisplayDelegate> &&,
                                            ContentsLayerPurpose) noexcept
    {
    }
    KRYS_NODISCARD virtual bool UsesContentsLayer() const noexcept
    {
      return false;
    }

    // Callback from the underlying graphics system to draw layer contents.
    void
      PaintGraphicsLayerContents(GraphicsContext &context, const FloatRect &clip,
                                 EnumSet<GraphicsLayerPaintBehavior> layerPaintBehavior = {}) const noexcept;

    // Flippedness of the contents of this layer. Does not affect sublayer geometry.
    virtual void SetContentsOrientation(CompositingCoordinatesOrientation orientation) noexcept
    {
      _contentsOrientation = orientation;
    }
    KRYS_NODISCARD CompositingCoordinatesOrientation ContentsOrientation() const noexcept
    {
      return _contentsOrientation;
    }

    virtual void SetContentsMinificationFilter(ScalingFilter filter) noexcept
    {
      _contentsMinificationFilter = filter;
    }
    KRYS_NODISCARD ScalingFilter ContentsMinificationFilter() const noexcept
    {
      return _contentsMinificationFilter;
    }
    virtual void SetContentsMagnificationFilter(ScalingFilter filter) noexcept
    {
      _contentsMagnificationFilter = filter;
    }
    KRYS_NODISCARD ScalingFilter ContentsMagnificationFilter() const noexcept
    {
      return _contentsMagnificationFilter;
    }

    void DumpLayer(TextStream &ts, EnumSet<LayerTreeAsTextOptions> options = {}) const noexcept;

    virtual void SetShowDebugBorder(bool show) noexcept
    {
      _showDebugBorder = show;
    }
    KRYS_NODISCARD bool IsShowingDebugBorder() const noexcept
    {
      return _showDebugBorder;
    }

    virtual void SetShowRepaintCounter(bool show) noexcept
    {
      _showRepaintCounter = show;
    }
    KRYS_NODISCARD bool IsShowingRepaintCounter() const noexcept
    {
      return _showRepaintCounter;
    }

    virtual void SetShowFrameProcessBorders(bool show) noexcept
    {
      _showFrameProcessBorders = show;
    }
    KRYS_NODISCARD bool IsShowingFrameProcessBorders() const noexcept
    {
      return _showFrameProcessBorders;
    }

    // FIXME: this is really a paint count.
    KRYS_NODISCARD int32 RepaintCount() const noexcept
    {
      return _repaintCount;
    }
    int32 IncrementRepaintCount() noexcept
    {
      return ++_repaintCount;
    }

    virtual void SetDebugBackgroundColor(const Color &) noexcept
    {
    }
    virtual void SetDebugBorder(const Color &, float /*borderWidth*/) noexcept
    {
    }

    virtual void SetCustomAppearance(CustomAppearance customAppearance) noexcept
    {
      _customAppearance = customAppearance;
    }
    KRYS_NODISCARD CustomAppearance GetCustomAppearance() const noexcept
    {
      return _customAppearance;
    }

    // z-position is the z-equivalent of position(). It's only used for debugging purposes.
    KRYS_NODISCARD virtual float ZPosition() const noexcept
    {
      return _zPosition;
    }
    virtual void SetZPosition(float position) noexcept;

    KRYS_NODISCARD virtual FloatSize PixelAlignmentOffset() const noexcept
    {
      return {};
    }

    virtual void SetAppliesPageScale(bool appliesScale = true) noexcept
    {
      _appliesPageScale = appliesScale;
    }
    KRYS_NODISCARD virtual bool AppliesPageScale() const noexcept
    {
      return _appliesPageScale;
    }

    void SetAppliesDeviceScale(bool appliesScale = true) noexcept
    {
      _appliesDeviceScale = appliesScale;
    }
    KRYS_NODISCARD bool AppliesDeviceScale() const noexcept
    {
      return _appliesDeviceScale;
    }

    KRYS_NODISCARD float PageScaleFactor() const noexcept
    {
      return Client().PageScaleFactor();
    }
    KRYS_NODISCARD float DeviceScaleFactor() const noexcept
    {
      return AppliesDeviceScale() ? Client().DeviceScaleFactor() : 1.f;
    }

    // Whether this layer can throw away backing store to save memory. False for layers that can be revealed
    // by async scrolling.
    virtual void SetAllowsBackingStoreDetaching(bool) noexcept
    {
    }
    KRYS_NODISCARD virtual bool AllowsBackingStoreDetaching() const noexcept
    {
      return true;
    }

    virtual void SetAllowsTiling(bool allowsTiling) noexcept
    {
      _allowsTiling = allowsTiling;
    }
    KRYS_NODISCARD virtual bool AllowsTiling() const noexcept
    {
      return _allowsTiling;
    }

    virtual void DeviceOrPageScaleFactorChanged() noexcept
    {
    }
    virtual void SetShouldUpdateRootRelativeScaleFactor(bool) noexcept
    {
    }

    void NoteDeviceOrPageScaleFactorChangedIncludingDescendants() noexcept;

    void SetIsInWindow(bool inWindow) noexcept;

    // Some compositing systems may do internal batching to synchronize compositing updates with updates drawn
    // into the window. These methods flush internal batched state on this layer and descendant layers, and
    // this layer only.
    virtual void FlushCompositingState(const FloatRect & /* clipRect */) noexcept
    {
    }
    virtual void FlushCompositingStateForThisLayerOnly() noexcept
    {
    }

    // If the exposed rect of this layer changes, returns true if this or descendant layers need a flush, for
    // example to allocate new tiles.
    KRYS_NODISCARD virtual bool
      VisibleRectChangeRequiresFlush(const FloatRect & /* clipRect */) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD static FloatRect
      AdjustCoverageRectForMovement(const FloatRect &coverageRect, const FloatRect &previousVisibleRect,
                                    const FloatRect &currentVisibleRect) noexcept;

    // Return a string with a human readable form of the layer tree, If debug is true pointers for the layers
    // and timing data will be included in the returned string.
    KRYS_NODISCARD utf8_string LayerTreeAsText(EnumSet<LayerTreeAsTextOptions> options = {},
                                               uint32 baseIndent = 0) const noexcept;

    KRYS_NODISCARD virtual utf8_string
      PlatformLayerTreeAsText(EnumSet<PlatformLayerTreeAsTextFlags>) const noexcept
    {
      return {};
    }

    virtual void SetIsTrackingDisplayListReplay(bool isTracking) noexcept
    {
      _isTrackingDisplayListReplay = isTracking;
    }
    KRYS_NODISCARD virtual bool IsTrackingDisplayListReplay() const noexcept
    {
      return _isTrackingDisplayListReplay;
    }

    // Return an estimate of the backing store memory cost (in bytes). May be incorrect for tiled layers.
    KRYS_NODISCARD virtual double BackingStoreMemoryEstimate() const noexcept;

    KRYS_NODISCARD virtual bool BackingStoreAttached() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual bool BackingStoreAttachedForTesting() const noexcept
    {
      return BackingStoreAttached();
    }

    virtual void SetTileCoverage(TileCoverage) noexcept;

    void ResetTrackedRepaints() noexcept;
    void AddRepaintRect(const FloatRect &repaintRect) noexcept;

    KRYS_NODISCARD static bool SupportsLayerType(Type type) noexcept;
    KRYS_NODISCARD static bool SupportsContentsTiling() noexcept;

    void UpdateDebugIndicators() noexcept;

    KRYS_NODISCARD virtual bool IsGraphicsLayerTextureMapper() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool ShouldPaintUsingCompositeCopy() const noexcept
    {
      return _shouldPaintUsingCompositeCopy;
    }
    void SetShouldPaintUsingCompositeCopy(bool copy) noexcept
    {
      _shouldPaintUsingCompositeCopy = copy;
    }

    KRYS_NODISCARD bool RenderingIsSuppressedIncludingDescendants() const noexcept
    {
      return _renderingIsSuppressedIncludingDescendants;
    }
    void SetRenderingIsSuppressedIncludingDescendants(bool suppressed) noexcept
    {
      _renderingIsSuppressedIncludingDescendants = suppressed;
    }

    KRYS_NODISCARD const Maybe<FloatRect> &AnimationExtent() const noexcept
    {
      return _animationExtent;
    }
    void SetAnimationExtent(Maybe<FloatRect> animationExtent) noexcept
    {
      _animationExtent = animationExtent;
    }

    static void Traverse(GraphicsLayer &layer, const Func<void(GraphicsLayer &)> &traversalFunc) noexcept;

  protected:
    explicit GraphicsLayer(Type type, GraphicsLayerClient &layerClient) noexcept;

    // Should be called from derived class destructors. Should call willBeDestroyed() on super.
    void WillBeDestroyed() noexcept;
    KRYS_NODISCARD bool BeingDestroyed() const noexcept
    {
      return _beingDestroyed;
    }

    virtual void WillModifyChildren() noexcept
    {
    }

    // This method is used by platform GraphicsLayer classes to clear the filters when compositing is not done
    // in hardware. It is not virtual, so the caller needs to notifiy the change to the platform layer as
    // needed.
    void ClearFilters() noexcept
    {
      _filters = {};
    }
    void ClearBackdropFilters() noexcept
    {
      _backdropFilters = {};
    }

    // Given a GraphicsLayerKeyframeValueList containing filterOperations, return true if the operations are
    // valid.
    KRYS_NODISCARD static int32
      ValidateFilterOperations(const GraphicsLayerKeyframeValueList &valueList) noexcept;

    KRYS_NODISCARD virtual bool ShouldRepaintOnSizeChange() const noexcept
    {
      return DrawsContent();
    }

    void RemoveFromParentInternal() noexcept;

    // The layer being replicated.
    KRYS_NODISCARD GraphicsLayer *ReplicatedLayer() const noexcept
    {
      return _replicatedLayer;
    }
    virtual void SetReplicatedLayer(GraphicsLayer *layer) noexcept
    {
      _replicatedLayer = layer;
    }

    void DumpProperties(TextStream &ts, EnumSet<LayerTreeAsTextOptions> options) const noexcept;
    virtual void DumpAdditionalProperties(TextStream &, EnumSet<LayerTreeAsTextOptions>) const noexcept
    {
    }

    virtual void GetDebugBorderInfo(Color &color, float &width) const noexcept;
  };

  TextStream &operator<<(TextStream &ts, const List<PlatformLayerIdentifier> &layers) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsLayer::CustomAppearance &customAppearance) noexcept;
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_GRAPHICSLAYER(ToValueTypeName, predicate)                                \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::GraphicsLayer &layer) noexcept                        \
    {                                                                                                        \
      return layer.predicate;                                                                                \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
