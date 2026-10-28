#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Animation/BlendingKeyframes.hpp"
#include "Krystal.Booey/Graphics/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFilterAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFloatAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerKeyframeValueList.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerTransformAnimationValue.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/BackgroundPainter.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderHTMLCanvas.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Rotate.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Scale.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Translate.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  CanvasCompositingStrategy GetCanvasCompositingStrategy(const RenderObject &renderer) noexcept
  {
    krys_debug_assert(renderer.IsRenderHTMLCanvas());
    auto *context = Downcast<RenderHTMLCanvas>(renderer).CanvasElement().RenderingContext();
    if (!context)
    {
      return CanvasCompositingStrategy::CanvasPaintedToEnclosingLayer;
    }
    if (context->DelegatesDisplay())
    {
      return CanvasCompositingStrategy::CanvasAsLayerContents;
    }
    if (auto *context2D = DynamicDowncast<html::CanvasRenderingContext2DBase>(*context))
    {
      if (context2D->IsAccelerated())
      {
        return CanvasCompositingStrategy::CanvasPaintedToLayer;
      }
    }
    return CanvasCompositingStrategy::CanvasPaintedToEnclosingLayer;
  }

  // This acts as a cache of what we know about what is painting into this RenderLayerBacking.
  class PaintedContentsInfo
  {
  public:
    enum class ContentsType : uint8
    {
      Unknown,
      SimpleContainer,
      DirectlyCompositedImage,
      UnscaledBitmapOnly,
      Painted,
    };

    RenderLayerBacking &Backing;
    RequestState BoxDecorations {RequestState::Unknown};
    RequestState Content {RequestState::Unknown};

    ContentsType Contents {ContentsType::Unknown};

    explicit PaintedContentsInfo(RenderLayerBacking &inBacking) noexcept : Backing(inBacking)
    {
    }

    void DeterminePaintsBoxDecorations() noexcept;
    KRYS_NODISCARD bool PaintsBoxDecorations() noexcept
    {
      DeterminePaintsBoxDecorations();
      return BoxDecorations == RequestState::True || BoxDecorations == RequestState::Undetermined;
    }

    // FIXME: HDR content, once there's HDR.
    KRYS_NODISCARD bool IsPaintsContentSatisfied() const noexcept
    {
      return Content != RequestState::Unknown;
    }

    void DeterminePaintsContent() noexcept;
    KRYS_NODISCARD bool PaintsContent() noexcept
    {
      DeterminePaintsContent();
      return Content == RequestState::True || Content == RequestState::Undetermined;
    }

    KRYS_NODISCARD bool IsContentsTypeSatisfied() const noexcept
    {
      return Contents != ContentsType::Unknown;
    }

    void DetermineContentsType() noexcept;
    KRYS_NODISCARD bool IsSimpleContainer() noexcept
    {
      DetermineContentsType();
      return Contents == ContentsType::SimpleContainer;
    }

    KRYS_NODISCARD bool IsDirectlyCompositedImage() noexcept
    {
      DetermineContentsType();
      return Contents == ContentsType::DirectlyCompositedImage;
    }

    KRYS_NODISCARD bool IsUnscaledBitmapOnly() noexcept
    {
      DetermineContentsType();
      return Contents == ContentsType::UnscaledBitmapOnly;
    }
  };

  void PaintedContentsInfo::DeterminePaintsBoxDecorations() noexcept
  {
    if (BoxDecorations != RequestState::Unknown)
    {
      return;
    }

    BoxDecorations = Backing.PaintsBoxDecorations() ? RequestState::True : RequestState::False;
  }

  void PaintedContentsInfo::DeterminePaintsContent() noexcept
  {
    if (IsPaintsContentSatisfied())
    {
      return;
    }

    auto contentRequest = RenderLayer::PaintedContentRequest {};

    Backing.DeterminePaintsContent(contentRequest);
    Content = contentRequest.HasPaintedContent;
  }

  void PaintedContentsInfo::DetermineContentsType() noexcept
  {
    if (IsContentsTypeSatisfied())
    {
      return;
    }

    if (Backing.IsSimpleContainerCompositingLayer(*this))
    {
      Contents = ContentsType::SimpleContainer;
    }
    else if (Backing.IsDirectlyCompositedImage())
    {
      Contents = ContentsType::DirectlyCompositedImage;
    }
    else if (Backing.IsUnscaledBitmapOnly())
    {
      Contents = ContentsType::UnscaledBitmapOnly;
    }
    else
    {
      Contents = ContentsType::Painted;
    }
  }

  RenderLayerBacking::RenderLayerBacking(RenderLayer &layer) noexcept : _owningLayer(layer)
  {
    if (layer.IsRenderViewLayer())
    {
      // FIXME: Frames other than the main frame and tiled backings for the frame view, once there are any.
      _isMainFrameRenderViewLayer = true;
      _isRootFrameRenderViewLayer = true;
      _isFrameLayerWithTiledBacking = false;
    }

    CreatePrimaryGraphicsLayer();

    // FIXME: The fullscreen backdrop's background layer, once there's fullscreen.

    // FIXME: Event regions (setAncestorsHaveDescendantNeedingEventRegionUpdate), once there are event
    // regions.
  }

  RenderLayerBacking::~RenderLayerBacking() noexcept
  {
    // Note that _owningLayer->Backing() is null here.
    UpdateAncestorClipping(false, nullptr);
    UpdateDescendantClippingLayer(false);
    UpdateOverflowControlsLayers(false, false, false);
    UpdateForegroundLayer(false);
    UpdateBackgroundLayer(false);
    UpdateMaskingLayer(false, false);
    UpdateScrollingLayers(false);

    DestroyGraphicsLayers();
  }

  void RenderLayerBacking::WillBeDestroyed(EnumSet<UpdateBackingSharingFlags> flags) noexcept
  {
    krys_debug_assert(_owningLayer.Backing() == this);
    // FIXME: The scrolling coordinator (removeFromScrollCoordinatedLayers), once there's async scrolling.

    ClearBackingSharingLayers(flags);
  }

  void RenderLayerBacking::WillDestroyLayer(const GraphicsLayer *) noexcept
  {
    // FIXME: Tiled backings (layerTiledBackingUsageChanged), once there are any.
  }

  namespace
  {
    void ClearBackingSharingLayerProviders(List<WeakPtr<RenderLayer>> &sharingLayers,
                                           const RenderLayer &providerLayer,
                                           EnumSet<UpdateBackingSharingFlags> flags) noexcept
    {
      for (auto &layer : sharingLayers)
      {
        if (layer && layer->BackingProviderLayer() == &providerLayer)
        {
          layer->SetBackingProviderLayer(nullptr, flags);
        }
      }
    }
  }

  void RenderLayerBacking::SetBackingSharingLayers(ListHashSet<RenderLayer *> &&sharingLayers) noexcept
  {
    auto liveSharingLayers =
      std::ranges::count_if(_backingSharingLayers, [](const auto &layer) { return !!layer; });
    bool sharingLayersChanged = static_cast<size_t>(liveSharingLayers) != sharingLayers.size();
    ClearBackingSharingLayerProviders(_backingSharingLayers, _owningLayer,
                                      {UpdateBackingSharingFlags::DuringCompositingUpdate});

    for (auto &oldSharingLayer : _backingSharingLayers)
    {
      if (oldSharingLayer && !sharingLayers.contains(oldSharingLayer.get()))
      {
        sharingLayersChanged = true;
      }
    }

    if (sharingLayersChanged)
    {
      if (!sharingLayers.empty())
      {
        SetRequiresOwnBackingStore(true);
      }
    }

    _backingSharingLayers.clear();
    for (auto *layer : sharingLayers)
    {
      _backingSharingLayers.push_back(CreateWeakPtr(layer));
    }

    for (auto &layer : _backingSharingLayers)
    {
      layer->SetBackingProviderLayer(&_owningLayer, {UpdateBackingSharingFlags::DuringCompositingUpdate});
    }
  }

  void RenderLayerBacking::RemoveBackingSharingLayer(RenderLayer &layer,
                                                     EnumSet<UpdateBackingSharingFlags> flags) noexcept
  {
    layer.SetBackingProviderLayer(nullptr, flags);
    std::erase_if(_backingSharingLayers,
                  [&](const WeakPtr<RenderLayer> &sharingLayer) { return sharingLayer.get() == &layer; });
  }

  void RenderLayerBacking::ClearBackingSharingLayers(EnumSet<UpdateBackingSharingFlags> flags) noexcept
  {
    ClearBackingSharingLayerProviders(_backingSharingLayers, _owningLayer, flags);
    _backingSharingLayers.clear();
  }

  Ref<GraphicsLayer> RenderLayerBacking::CreateGraphicsLayer(const utf8_string &name,
                                                             GraphicsLayer::Type layerType) noexcept
  {
    auto *graphicsLayerFactory = Compositor().GetGraphicsLayerFactory();

    auto graphicsLayer = GraphicsLayer::Create(graphicsLayerFactory, *this, layerType);

    graphicsLayer->SetName(name);

    // FIXME: Layer-based SVG (setShouldUpdateRootRelativeScaleFactor), once there's SVG.

    return graphicsLayer;
  }

  void RenderLayerBacking::SetUsesDisplayListDrawing(bool usesDisplayListDrawing) noexcept
  {
    // Note that this only affects the primary layer.
    if (usesDisplayListDrawing == _graphicsLayer->UsesDisplayListDrawing())
    {
      return;
    }

    _graphicsLayer->SetUsesDisplayListDrawing(usesDisplayListDrawing);
    if (_graphicsLayer->DrawsContent())
    {
      _graphicsLayer->SetNeedsDisplay();
    }
  }

  void RenderLayerBacking::TiledBackingUsageChanged(const GraphicsLayer *, bool) noexcept
  {
    // FIXME: Tiled backings (layerTiledBackingUsageChanged), once there are any.
  }

  void RenderLayerBacking::AdjustTiledBackingCoverage() noexcept
  {
    // FIXME: Tiled backings' coverage, once there are tiled backings.
  }

  void RenderLayerBacking::UpdateDebugIndicators(bool showBorder, bool showRepaintCounter) noexcept
  {
    _graphicsLayer->SetShowDebugBorder(showBorder);
    _graphicsLayer->SetShowRepaintCounter(showRepaintCounter);

    // _viewportAnchorLayer can't show layer borders because it's a structural layer.

    if (_viewportClippingLayer)
    {
      _viewportClippingLayer->SetShowDebugBorder(showBorder);
    }

    if (_ancestorClippingStack)
    {
      for (auto &entry : _ancestorClippingStack->Stack())
      {
        entry.ClippingLayer->SetShowDebugBorder(showBorder);
      }
    }

    if (_foregroundLayer)
    {
      _foregroundLayer->SetShowDebugBorder(showBorder);
      _foregroundLayer->SetShowRepaintCounter(showRepaintCounter);
    }

    if (_contentsContainmentLayer)
    {
      _contentsContainmentLayer->SetShowDebugBorder(showBorder);
    }

    if (_childContainmentLayer)
    {
      _childContainmentLayer->SetShowDebugBorder(showBorder);
    }

    if (_backgroundLayer)
    {
      _backgroundLayer->SetShowDebugBorder(showBorder);
      _backgroundLayer->SetShowRepaintCounter(showRepaintCounter);
    }

    if (_maskLayer)
    {
      _maskLayer->SetShowDebugBorder(showBorder);
      _maskLayer->SetShowRepaintCounter(showRepaintCounter);
    }

    if (_layerForHorizontalScrollbar)
    {
      _layerForHorizontalScrollbar->SetShowDebugBorder(showBorder);
    }

    if (_layerForVerticalScrollbar)
    {
      _layerForVerticalScrollbar->SetShowDebugBorder(showBorder);
    }

    if (_layerForScrollCorner)
    {
      _layerForScrollCorner->SetShowDebugBorder(showBorder);
    }

    if (_scrollContainerLayer)
    {
      _scrollContainerLayer->SetShowDebugBorder(showBorder);
    }

    if (_scrolledContentsLayer)
    {
      _scrolledContentsLayer->SetShowDebugBorder(showBorder);
      _scrolledContentsLayer->SetShowRepaintCounter(showRepaintCounter);
    }

    if (_overflowControlsContainer)
    {
      _overflowControlsContainer->SetShowDebugBorder(showBorder);
    }
  }

  void RenderLayerBacking::CreatePrimaryGraphicsLayer() noexcept
  {
    utf8_string layerName = _owningLayer.Name();
    constexpr size_t maxLayerNameLength = 100;
    if (layerName.size() > maxLayerNameLength)
    {
      layerName = layerName.substr(0, maxLayerNameLength) + u8"...";
    }
    _graphicsLayer =
      CreateGraphicsLayer(layerName, _isFrameLayerWithTiledBacking ? GraphicsLayer::Type::PageTiledBacking
                                                                   : GraphicsLayer::Type::Normal);

    if (_isFrameLayerWithTiledBacking)
    {
      _childContainmentLayer = CreateGraphicsLayer(u8"Page TiledBacking containment");
      _graphicsLayer->AddChild(ShareRef(*_childContainmentLayer));
    }

    if (_isMainFrameRenderViewLayer)
    {
      _graphicsLayer->SetContentsOpaque(!Compositor().ViewHasTransparentBackground());
    }
    if (_isRootFrameRenderViewLayer)
    {
      _graphicsLayer->SetAppliesPageScale();
    }

    const auto &style = Renderer().Style();
    UpdateOpacity(style);
    UpdateTransform(style);
    UpdateFilters(style);
    UpdateBackdropFilters(style);
    (void)UpdateBackdropRoot();
    UpdateBlendMode(style);
    UpdateContentsScalingFilters(style);
  }

  void RenderLayerBacking::DestroyGraphicsLayers() noexcept
  {
    if (_graphicsLayer)
    {
      _graphicsLayer->SetMaskLayer(nullptr);
      _graphicsLayer->SetReplicatedByLayer(nullptr);
      WillDestroyLayer(_graphicsLayer.get());
    }

    GraphicsLayer::Clear(_maskLayer);

    if (_ancestorClippingStack)
    {
      RemoveClippingStackLayers(*_ancestorClippingStack);
    }

    if (_overflowControlsHostLayerAncestorClippingStack)
    {
      RemoveClippingStackLayers(*_overflowControlsHostLayerAncestorClippingStack);
    }

    GraphicsLayer::UnparentAndClear(_transformFlatteningLayer);
    GraphicsLayer::UnparentAndClear(_viewportAnchorLayer);
    GraphicsLayer::UnparentAndClear(_viewportClippingLayer);
    GraphicsLayer::UnparentAndClear(_contentsContainmentLayer);
    GraphicsLayer::UnparentAndClear(_foregroundLayer);
    GraphicsLayer::UnparentAndClear(_backgroundLayer);
    GraphicsLayer::UnparentAndClear(_childContainmentLayer);
    GraphicsLayer::UnparentAndClear(_scrollContainerLayer);
    GraphicsLayer::UnparentAndClear(_scrolledContentsLayer);
    GraphicsLayer::UnparentAndClear(_graphicsLayer);
  }

  namespace
  {
    KRYS_NODISCARD LayoutRect ScrollContainerLayerBox(const RenderBox &renderBox) noexcept
    {
      return renderBox.PaddingBoxRect();
    }

    KRYS_NODISCARD LayoutRect ClippingLayerBox(const RenderLayerModelObject &renderer) noexcept
    {
      LayoutRect result = LayoutRect::InfiniteRect();
      if (renderer.HasNonVisibleOverflow())
      {
        // FIXME: SVG model objects' overflow clip, once there's SVG.
        if (auto *box = DynamicDowncast<RenderBox>(renderer))
        {
          result = box->OverflowClipRect({});
        }
      }

      if (renderer.HasClip())
      {
        if (auto *box = DynamicDowncast<RenderBox>(renderer))
        {
          result.Intersect(box->ClipRect({}));
        }
      }

      return result;
    }

    KRYS_NODISCARD LayoutRect OverflowControlsHostLayerRect(const RenderBox &renderBox) noexcept
    {
      return renderBox.PaddingBoxRectIncludingScrollbar();
    }
  }

  void RenderLayerBacking::UpdateOpacity(const RenderStyle &style) noexcept
  {
    _graphicsLayer->SetOpacity(CompositingOpacity(style.GetOpacity().Value.Value));
  }

  void RenderLayerBacking::UpdateTransform(const RenderStyle &style) noexcept
  {
    TransformationMatrix t;
    // FIXME: Elements captured in a view transition, once there are view transitions.
    if (_owningLayer.IsTransformed())
    {
      _owningLayer.UpdateTransformFromStyle(t, style,
                                            style::TransformResolver::IndividualTransformOperations);
    }

    if (_contentsContainmentLayer)
    {
      _contentsContainmentLayer->SetTransform(t);
      _graphicsLayer->SetTransform({});
    }
    else
    {
      _graphicsLayer->SetTransform(t);
    }
  }

  void RenderLayerBacking::UpdateChildrenTransformAndAnchorPoint(
    const LayoutRect &primaryGraphicsLayerRect, LayoutSize offsetFromParentGraphicsLayer) noexcept
  {
    auto defaultAnchorPoint = FloatPoint3D {0.5, 0.5, 0};

    if (_owningLayer.IsRenderViewLayer())
    {
      defaultAnchorPoint = {};
    }

    if (!Renderer().HasTransformRelatedProperty())
    {
      _graphicsLayer->SetAnchorPoint(defaultAnchorPoint);
      if (_contentsContainmentLayer)
      {
        _contentsContainmentLayer->SetAnchorPoint(defaultAnchorPoint);
      }

      if (_childContainmentLayer)
      {
        _childContainmentLayer->SetAnchorPoint(defaultAnchorPoint);
      }

      if (_scrollContainerLayer)
      {
        _scrollContainerLayer->SetAnchorPoint(defaultAnchorPoint);
      }

      if (_scrolledContentsLayer)
      {
        _scrolledContentsLayer->SetPreserves3D(false);
      }
      return;
    }

    const auto deviceScaleFactor = DeviceScaleFactor();
    auto transformOrigin = _owningLayer.TransformOriginPixelSnappedIfNeeded();
    auto layerOffset =
      RoundPointToDevicePixels(ToLayoutPoint(offsetFromParentGraphicsLayer), deviceScaleFactor);
    auto anchor =
      FloatPoint3D {primaryGraphicsLayerRect.Width()
                      ? ((layerOffset.X() - primaryGraphicsLayerRect.X().ToFloat()) + transformOrigin.X())
                          / primaryGraphicsLayerRect.Width().ToFloat()
                      : 0.5f,
                    primaryGraphicsLayerRect.Height()
                      ? ((layerOffset.Y() - primaryGraphicsLayerRect.Y().ToFloat()) + transformOrigin.Y())
                          / primaryGraphicsLayerRect.Height().ToFloat()
                      : 0.5f,
                    transformOrigin.Z()};

    if (_contentsContainmentLayer)
    {
      _contentsContainmentLayer->SetAnchorPoint(anchor);
    }
    else
    {
      _graphicsLayer->SetAnchorPoint(anchor);
    }

    auto removeChildrenTransformFromLayers = [&](GraphicsLayer *layerToIgnore = nullptr)
    {
      auto *clippingLayer = ClippingLayer();
      if (clippingLayer && clippingLayer != layerToIgnore)
      {
        clippingLayer->SetChildrenTransform({});
        clippingLayer->SetAnchorPoint(defaultAnchorPoint);
      }

      if (_scrollContainerLayer && _scrollContainerLayer.get() != layerToIgnore)
      {
        _scrollContainerLayer->SetChildrenTransform({});
        _scrollContainerLayer->SetAnchorPoint(defaultAnchorPoint);
        _scrolledContentsLayer->SetPreserves3D(false);
      }

      if (_graphicsLayer.get() != layerToIgnore)
      {
        _graphicsLayer->SetChildrenTransform({});
      }
    };

    if (!Renderer().Style().HasPerspective())
    {
      removeChildrenTransformFromLayers();
      return;
    }

    auto layerForChildrenTransform = [&]() -> Pair<GraphicsLayer *, FloatRect>
    {
      if (_scrollContainerLayer)
      {
        // Scroll container layers are only created for RenderBox derived renderers.
        return {_scrollContainerLayer.get(),
                FloatRect(ScrollContainerLayerBox(Downcast<RenderBox>(Renderer())))};
      }
      if (auto *layer = ClippingLayer())
      {
        return {layer, FloatRect(ClippingLayerBox(Renderer()))};
      }

      return {_graphicsLayer.get(), Renderer().TransformReferenceBoxRect()};
    };

    auto [layerForPerspective, layerForPerspectiveRect] = layerForChildrenTransform();
    if (layerForPerspective != _graphicsLayer.get())
    {
      // If we have scrolling layers, we need the children transform on _scrollContainerLayer to affect
      // children of _scrolledContentsLayer, so set setPreserves3D(true).
      if (layerForPerspective == _scrollContainerLayer.get())
      {
        _scrolledContentsLayer->SetPreserves3D(true);
      }

      auto perspectiveAnchorPoint = FloatPoint3D {
        layerForPerspectiveRect.Width()
          ? (transformOrigin.X() - layerForPerspectiveRect.X()) / layerForPerspectiveRect.Width()
          : 0.5f,
        layerForPerspectiveRect.Height()
          ? (transformOrigin.Y() - layerForPerspectiveRect.Y()) / layerForPerspectiveRect.Height()
          : 0.5f,
        transformOrigin.Z()};

      layerForPerspective->SetAnchorPoint(perspectiveAnchorPoint);
    }

    layerForPerspective->SetChildrenTransform(_owningLayer.PerspectiveTransform());
    removeChildrenTransformFromLayers(layerForPerspective);
  }

  namespace
  {
    KRYS_NODISCARD FilterOperations ResolveFilters(const style::Filter &filter,
                                                   const RenderStyle &style) noexcept
    {
      List<Ref<FilterOperation>> operations;
      for (const auto &value : filter)
      {
        Ref<FilterOperation> operation = value.Value;
        if (auto *dropShadow =
              DynamicDowncast<style::DropShadowFilterOperationWithStyleColor>(operation.get()))
        {
          operations.push_back(dropShadow->CreateEquivalentWithResolvedColor(style));
        }
        else
        {
          operations.push_back(krys::move(operation));
        }
      }
      return FilterOperations {krys::move(operations)};
    }
  }

  void RenderLayerBacking::UpdateFilters(const RenderStyle &style) noexcept
  {
    _canCompositeFilters = _graphicsLayer->SetFilters(ResolveFilters(style.GetFilter(), style));
  }

  void RenderLayerBacking::UpdateBackdropFilters(const RenderStyle &style) noexcept
  {
    _canCompositeBackdropFilters =
      _graphicsLayer->SetBackdropFilters(ResolveFilters(style.BackdropFilter(), style));
  }

  void RenderLayerBacking::UpdateBackdropFiltersGeometry() noexcept
  {
    if (!_canCompositeBackdropFilters)
    {
      return;
    }

    auto *renderBox = DynamicDowncast<RenderBox>(Renderer());
    if (!renderBox)
    {
      return;
    }

    FloatRoundedRect backdropFiltersRect;
    if (renderBox->Style().HasBorderRadius() && !renderBox->HasClip())
    {
      auto borderShape = BorderShape::ShapeForBorderRect(renderBox->Style(), renderBox->BorderBoxRect());
      auto roundedBoxRect = borderShape.DeprecatedRoundedRect();
      roundedBoxRect.Move(ContentOffsetInCompositingLayer());
      backdropFiltersRect = roundedBoxRect.PixelSnappedRoundedRectForPainting(DeviceScaleFactor());
    }
    else
    {
      auto boxRect = renderBox->BorderBoxRect();
      if (renderBox->HasClip())
      {
        boxRect.Intersect(renderBox->ClipRect({}));
      }
      boxRect.Move(ContentOffsetInCompositingLayer());
      backdropFiltersRect = FloatRoundedRect(SnapRectToDevicePixels(boxRect, DeviceScaleFactor()));
    }

    _graphicsLayer->SetBackdropFiltersRect(backdropFiltersRect);
  }

  bool RenderLayerBacking::UpdateBackdropRoot() noexcept
  {
    // Don't try to make the RenderView's layer a backdrop root if it's going to paint into the window since
    // it won't work.
    bool willBeBackdropRoot = _owningLayer.IsBackdropRoot() && !PaintsIntoWindow();

    // If the RenderView is opaque, then that will occlude any pixels behind it and we don't need to isolate
    // it as a backdrop root.
    //
    // Always allow for the main frame so that clients can intentionally allow backdrop blending outside of
    // the view by setting it to transparent.
    if (_owningLayer.IsRenderViewLayer()
        && (_isMainFrameRenderViewLayer || !Compositor().ViewHasTransparentBackground()))
    {
      willBeBackdropRoot = false;
    }

    if (_graphicsLayer->IsBackdropRoot() == willBeBackdropRoot)
    {
      return false;
    }

    _graphicsLayer->SetIsBackdropRoot(willBeBackdropRoot);
    return true;
  }

  void RenderLayerBacking::UpdateBlendMode(const RenderStyle &style) noexcept
  {
    // FIXME: where is the blend mode updated when _ancestorClippingStacks come and go?
    if (_ancestorClippingStack)
    {
      _ancestorClippingStack->Stack().front().ClippingLayer->SetBlendMode(style.GetBlendMode());
      _graphicsLayer->SetBlendMode(BlendMode::Normal);
    }
    else
    {
      _graphicsLayer->SetBlendMode(style.GetBlendMode());
    }
  }

  bool RenderLayerBacking::ShouldSetContentsDisplayDelegate() const noexcept
  {
    if (!Renderer().IsRenderHTMLCanvas())
    {
      return false;
    }

    return GetCanvasCompositingStrategy(Renderer()) == CanvasCompositingStrategy::CanvasAsLayerContents;
  }

  void RenderLayerBacking::UpdateContentsScalingFilters(const RenderStyle &style) noexcept
  {
    if (!Renderer().IsRenderHTMLCanvas()
        || GetCanvasCompositingStrategy(Renderer()) != CanvasCompositingStrategy::CanvasAsLayerContents)
    {
      return;
    }
    auto minificationFilter = GraphicsLayer::ScalingFilter::Linear;
    auto magnificationFilter = GraphicsLayer::ScalingFilter::Linear;
    switch (style.GetImageRendering())
    {
      case ImageRendering::CrispEdges:
      case ImageRendering::Pixelated:
        // FIXME: In order to match other code-paths, we treat these the same.
        minificationFilter = GraphicsLayer::ScalingFilter::Nearest;
        magnificationFilter = GraphicsLayer::ScalingFilter::Nearest;
        break;
      default: break;
    }
    _graphicsLayer->SetContentsMinificationFilter(minificationFilter);
    _graphicsLayer->SetContentsMagnificationFilter(magnificationFilter);
  }

  namespace
  {
    KRYS_NODISCARD bool LayerOrAncestorIsTransformedOrUsingCompositedScrolling(RenderLayer &layer) noexcept
    {
      for (auto *curr = &layer; curr; curr = curr->Parent())
      {
        if (curr->IsTransformed() || curr->HasCompositedScrollableOverflow())
        {
          return true;
        }
      }

      return false;
    }
  }

  bool RenderLayerBacking::ShouldClipCompositedBounds() const noexcept
  {
    // Scrollbar layers use this layer for relative positioning, so don't clip.
    if (LayerForHorizontalScrollbar() || LayerForVerticalScrollbar())
    {
      return false;
    }

    // FIXME: Elements captured in a view transition and the view transition's new pseudo-element, once there
    // are view transitions.

    if (_isFrameLayerWithTiledBacking)
    {
      return false;
    }

    if (LayerOrAncestorIsTransformedOrUsingCompositedScrolling(_owningLayer))
    {
      return false;
    }

    return true;
  }

  namespace
  {
    KRYS_NODISCARD bool HasNonZeroTransformOrigin(const RenderLayerModelObject &renderer) noexcept
    {
      const auto &style = renderer.Style();
      auto fixedTransformOriginX = style.GetTransformOriginX().TryFixed();
      auto fixedTransformOriginY = style.GetTransformOriginY().TryFixed();
      return (fixedTransformOriginX && !fixedTransformOriginX->IsZero())
             || (fixedTransformOriginY && !fixedTransformOriginY->IsZero());
    }
  }

  bool RenderLayerBacking::UpdateCompositedBounds() noexcept
  {
    auto boundsFlags = RenderLayer::DefaultCalculateLayerBoundsFlags();
    boundsFlags.Add({RenderLayer::ExcludeHiddenDescendants, RenderLayer::DontConstrainForMask});
    LayoutRect layerBounds = _owningLayer.CalculateLayerBounds(&_owningLayer, {}, boundsFlags);
    // Clip to the size of the document or enclosing overflow-scroll layer.
    // If this or an ancestor is transformed, we can't currently compute the correct rect to intersect with.
    // We'd need RenderObject::convertContainerToLocalQuad(), which doesn't yet exist.
    if (ShouldClipCompositedBounds())
    {
      auto &view = Renderer().View();
      auto *rootLayer = view.Layer();

      LayoutRect clippingBounds;
      if (Renderer().IsFixedPositioned() && Renderer().Container() == &view)
      {
        clippingBounds = view.FrameView().RectForFixedPositionLayout();
      }
      else
      {
        clippingBounds = view.UnscaledDocumentRect();
      }

      if (&_owningLayer != rootLayer)
      {
        // FIXME: Incorrect for CSS regions.
        clippingBounds.Intersect(
          _owningLayer.BackgroundClipRect(RenderLayer::ClipRectsContext(rootLayer, AbsoluteClipRects))
            .Rect());
      }

      LayoutPoint delta =
        _owningLayer.ConvertToLayerCoords(rootLayer, LayoutPoint(), RenderLayer::AdjustForColumns);
      clippingBounds.Move(-delta.X(), -delta.Y());

      layerBounds.Intersect(clippingBounds);
    }

    // If the backing provider has overflow:clip, we know all sharing layers are affected by the clip because
    // they are containing-block descendants.
    if (!Renderer().HasNonVisibleOverflow())
    {
      for (auto &weakLayer : _backingSharingLayers)
      {
        if (!weakLayer)
        {
          continue;
        }
        auto *layer = weakLayer.get();
        auto *boundsRootLayer = &_owningLayer;
        krys_debug_assert(layer->IsDescendantOf(_owningLayer));
        auto offset = layer->OffsetFromAncestor(&_owningLayer);
        auto bounds = layer->CalculateLayerBounds(boundsRootLayer, offset, boundsFlags);
        layerBounds.Unite(bounds);
      }
    }

    // If the element has a transform-origin that has fixed lengths, and the renderer has zero size, then we
    // need to ensure that the compositing layer has non-zero size so that we can apply the transform-origin
    // via the GraphicsLayer anchorPoint (which is expressed as a fractional value).
    if (layerBounds.IsEmpty()
        && (HasNonZeroTransformOrigin(Renderer()) || Renderer().Style().HasPerspective()))
    {
      layerBounds.SetWidth(1);
      layerBounds.SetHeight(1);
      _artificiallyInflatedBounds = true;
    }
    else
    {
      _artificiallyInflatedBounds = false;
    }

    return SetCompositedBounds(layerBounds);
  }

  void RenderLayerBacking::UpdateAllowsBackingStoreDetaching(bool allowDetachingForFixed) noexcept
  {
    auto setAllowsBackingStoreDetaching = [&](bool allowDetaching)
    {
      if (_graphicsLayer->AllowsBackingStoreDetaching() == allowDetaching)
      {
        return;
      }

      // isDirectlyCompositedImage depends on this value
      _owningLayer.SetNeedsCompositingConfigurationUpdate();

      _graphicsLayer->SetAllowsBackingStoreDetaching(allowDetaching);
      if (_foregroundLayer)
      {
        _foregroundLayer->SetAllowsBackingStoreDetaching(allowDetaching);
      }
      if (_backgroundLayer)
      {
        _backgroundLayer->SetAllowsBackingStoreDetaching(allowDetaching);
      }
      if (_scrolledContentsLayer)
      {
        _scrolledContentsLayer->SetAllowsBackingStoreDetaching(allowDetaching);
      }
    };

    if (!_owningLayer.BehavesAsFixed())
    {
      setAllowsBackingStoreDetaching(true);
      return;
    }

    setAllowsBackingStoreDetaching(allowDetachingForFixed);
  }

  void RenderLayerBacking::UpdateAfterLayout(bool needsClippingUpdate, bool needsFullRepaint) noexcept
  {
    // This is the main trigger for layout changing layer geometry, but we have to do the work again in
    // updateBackingAndHierarchy() when we know the final compositing hierarchy. We can't just set dirty bits
    // from RenderLayer::setSize() because that doesn't take overflow into account.
    if (UpdateCompositedBounds())
    {
      _owningLayer.SetNeedsCompositingGeometryUpdate();
      // This layer's geometry affects those of its children.
      _owningLayer.SetChildrenNeedCompositingGeometryUpdate();
      _owningLayer.SetSubsequentLayersNeedCompositingRequirementsTraversal();
    }
    else if (needsClippingUpdate)
    {
      _owningLayer.SetNeedsCompositingConfigurationUpdate();
      _owningLayer.SetNeedsCompositingGeometryUpdate();
    }

    if (needsFullRepaint && CanIssueSetNeedsDisplay())
    {
      SetContentsNeedDisplay();
    }
  }

  // This can only update things that don't require up-to-date layout.
  void RenderLayerBacking::UpdateConfigurationAfterStyleChange() noexcept
  {
    (void)UpdateMaskingLayer(Renderer().HasMask(), Renderer().HasClipPath());

    // FIXME: Reflections (setReplicatedByLayer), once there are reflections.
    _graphicsLayer->SetReplicatedByLayer(nullptr);

    // FIXME: do we care if opacity is animating?
    const auto &style = Renderer().Style();
    UpdateOpacity(style);
    UpdateFilters(style);

    UpdateBackdropFilters(style);
    (void)UpdateBackdropRoot();
    UpdateBlendMode(style);
    UpdateContentsScalingFilters(style);
  }

  bool RenderLayerBacking::UpdateConfiguration(const RenderLayer *compositingAncestor) noexcept
  {
    krys_debug_assert(!_owningLayer.NormalFlowListDirty());
    krys_debug_assert(!_owningLayer.ZOrderListsDirty());
    krys_debug_assert(!Renderer().View().NeedsLayout());

    bool layerConfigChanged = false;
    auto &compositor = Compositor();

    if (UpdateTransformFlatteningLayer(compositingAncestor))
    {
      layerConfigChanged = true;
    }

    if (UpdateViewportConstrainedSublayers(
          compositor.GetViewportConstrainedSublayers(_owningLayer, compositingAncestor)))
    {
      layerConfigChanged = true;
    }

    SetBackgroundLayerPaintsFixedRootBackground(compositor.NeedsFixedRootBackgroundLayer(_owningLayer));

    if (UpdateBackgroundLayer(_backgroundLayerPaintsFixedRootBackground || _requiresBackgroundLayer))
    {
      layerConfigChanged = true;
    }

    if (UpdateForegroundLayer(compositor.NeedsContentsCompositingLayer(_owningLayer)))
    {
      layerConfigChanged = true;
    }

    bool needsDescendantsClippingLayer = false;
    bool usesCompositedScrolling = _owningLayer.HasCompositedScrollableOverflow();

    if (usesCompositedScrolling)
    {
      // If it's scrollable, it has to be a box.
      auto &renderBox = Downcast<RenderBox>(Renderer());
      auto borderShape = BorderShape::ShapeForBorderRect(renderBox.Style(), renderBox.BorderBoxRect());
      FloatRoundedRect contentsClippingRect =
        borderShape.DeprecatedPixelSnappedInnerRoundedRect(DeviceScaleFactor());
      needsDescendantsClippingLayer = contentsClippingRect.IsRounded();
    }
    else
    {
      needsDescendantsClippingLayer = RenderLayerCompositor::ClipsCompositingDescendants(_owningLayer);
    }

    if (UpdateScrollingLayers(usesCompositedScrolling))
    {
      layerConfigChanged = true;
    }

    if (UpdateDescendantClippingLayer(needsDescendantsClippingLayer))
    {
      layerConfigChanged = true;
    }

    krys_debug_assert(compositingAncestor == _owningLayer.AncestorCompositingLayer());
    if (UpdateAncestorClipping(compositor.ClippedByAncestor(_owningLayer, compositingAncestor),
                               compositingAncestor))
    {
      layerConfigChanged = true;
    }

    if (UpdateOverflowControlsLayers(RequiresHorizontalScrollbarLayer(), RequiresVerticalScrollbarLayer(),
                                     RequiresScrollCornerLayer()))
    {
      layerConfigChanged = true;
    }

    if (layerConfigChanged)
    {
      UpdateInternalHierarchy();
    }

    // RenderLayerCompositor::adjustOverflowScrollbarContainerLayers() may have reparented the
    // overflowControlsContainer in an earlier update, so always put it back here. We don't yet know if it
    // will get reparented again.
    if (_overflowControlsContainer && _overflowControlsContainer->Parent() != _graphicsLayer.get())
    {
      _graphicsLayer->AddChild(ShareRef(*_overflowControlsContainer));
      // Ensure that we fix up the position of _overflowControlsContainer.
      _owningLayer.SetNeedsCompositingGeometryUpdate();
    }

    // FIXME: Overlow controls need to be above the flattening layer?
    if (auto *flatteningLayer = TileCacheFlatteningLayer())
    {
      if (layerConfigChanged || flatteningLayer->Parent() != _graphicsLayer.get())
      {
        // FIXME: _graphicsLayer children are clobbered in RenderLayerCompositor::updateBackingAndHierarchy();
        // this probably doesn't work.
        _graphicsLayer->AddChild(ShareRef(*flatteningLayer));
      }
    }

    if (UpdateMaskingLayer(Renderer().HasMask(), Renderer().HasClipPath()))
    {
      layerConfigChanged = true;
    }

    // FIXME: Reflections (setReplicatedByLayer), once there are reflections.
    _graphicsLayer->SetReplicatedByLayer(nullptr);

    PaintedContentsInfo contentsInfo(*this);

    // Requires layout.
    if (!_owningLayer.IsRenderViewLayer())
    {
      bool didUpdateContentsRect = false;
      UpdateDirectlyCompositedBoxDecorations(contentsInfo, didUpdateContentsRect);
    }
    else
    {
      UpdateRootLayerConfiguration();
    }

    // Requires layout.
    if (contentsInfo.IsDirectlyCompositedImage())
    {
      UpdateImageContents(contentsInfo);
    }

    bool unscaledBitmap = contentsInfo.IsUnscaledBitmapOnly();
    if (unscaledBitmap == _graphicsLayer->AppliesDeviceScale())
    {
      _graphicsLayer->SetAppliesDeviceScale(!unscaledBitmap);
      layerConfigChanged = true;
    }

    bool shouldPaintUsingCompositeCopy =
      unscaledBitmap && Is<RenderHTMLCanvas>(Renderer()) && _owningLayer.HasVisibleContent();
    if (shouldPaintUsingCompositeCopy != _shouldPaintUsingCompositeCopy)
    {
      _shouldPaintUsingCompositeCopy = shouldPaintUsingCompositeCopy;
      _graphicsLayer->SetShouldPaintUsingCompositeCopy(shouldPaintUsingCompositeCopy);
      layerConfigChanged = true;
    }

    // FIXME: Plug-ins, video, remote frames, models and widgets' content layers, once there are any.
    if (ShouldSetContentsDisplayDelegate())
    {
      auto &canvas = Downcast<html::HTMLCanvasElement>(*Renderer().GetElement());
      if (auto *context = canvas.RenderingContext())
      {
        context->SetContentsToLayer(*_graphicsLayer);
      }

      layerConfigChanged = true;
    }

    if (UpdateBackdropRoot())
    {
      layerConfigChanged = true;
    }

    if (layerConfigChanged)
    {
      UpdatePaintingPhases();
    }

    return layerConfigChanged;
  }

  namespace
  {
    KRYS_NODISCARD bool SubpixelOffsetFromRendererChanged(const LayoutSize &oldSubpixelOffsetFromRenderer,
                                                          const LayoutSize &newSubpixelOffsetFromRenderer,
                                                          float deviceScaleFactor) noexcept
    {
      FloatSize previous =
        SnapSizeToDevicePixel(oldSubpixelOffsetFromRenderer, LayoutPoint(), deviceScaleFactor);
      FloatSize current =
        SnapSizeToDevicePixel(newSubpixelOffsetFromRenderer, LayoutPoint(), deviceScaleFactor);
      return previous != current;
    }

    KRYS_NODISCARD FloatSize SubpixelForLayerPainting(const LayoutPoint &point,
                                                      float pixelSnappingFactor) noexcept
    {
      LayoutUnit x = point.X();
      LayoutUnit y = point.Y();
      x = x >= 0 ? FloorToDevicePixel(x, pixelSnappingFactor) : CeilToDevicePixel(x, pixelSnappingFactor);
      y = y >= 0 ? FloorToDevicePixel(y, pixelSnappingFactor) : CeilToDevicePixel(y, pixelSnappingFactor);
      return FloatSize(point - LayoutPoint(x, y));
    }

    struct OffsetFromRenderer
    {
      // 1.2px - > { DevicePixelOffset = 1px SubpixelOffset = 0.2px }
      LayoutSize DevicePixelOffset;
      LayoutSize SubpixelOffset;
    };

    KRYS_NODISCARD OffsetFromRenderer ComputeOffsetFromRenderer(const LayoutSize &offset,
                                                                float deviceScaleFactor) noexcept
    {
      OffsetFromRenderer offsetFromRenderer;
      offsetFromRenderer.SubpixelOffset =
        LayoutSize(SubpixelForLayerPainting(ToLayoutPoint(offset), deviceScaleFactor));
      offsetFromRenderer.DevicePixelOffset = offset - offsetFromRenderer.SubpixelOffset;
      return offsetFromRenderer;
    }

    struct SnappedRectInfo
    {
      LayoutRect SnappedRect;
      LayoutSize SnapDelta;
    };

    KRYS_NODISCARD SnappedRectInfo SnappedGraphicsLayer(const LayoutSize &offset, const LayoutSize &size,
                                                        const RenderLayerModelObject &renderer) noexcept
    {
      SnappedRectInfo snappedGraphicsLayer;
      LayoutRect graphicsLayerRect = LayoutRect(ToLayoutPoint(offset), size);
      snappedGraphicsLayer.SnappedRect =
        LayoutRect(SnapRectToDevicePixelsIfNeeded(graphicsLayerRect, renderer));
      snappedGraphicsLayer.SnapDelta = snappedGraphicsLayer.SnappedRect.Location() - ToLayoutPoint(offset);
      return snappedGraphicsLayer;
    }

    KRYS_NODISCARD LayoutSize ComputeOffsetFromAncestorGraphicsLayer(const RenderLayer *compositedAncestor,
                                                                     const LayoutPoint &location,
                                                                     float deviceScaleFactor) noexcept
    {
      if (!compositedAncestor)
      {
        return ToLayoutSize(location);
      }

      // FIXME: This is a workaround until after webkit.org/b/162634 gets fixed.
      // ancestorSubpixelOffsetFromRenderer could be stale when a dynamic composited state change triggers a
      // pre-order updateGeometry() traversal.
      LayoutSize ancestorSubpixelOffsetFromRenderer =
        compositedAncestor->Backing()->SubpixelOffsetFromRenderer();
      LayoutRect ancestorCompositedBounds = compositedAncestor->Backing()->CompositedBounds();
      LayoutSize floored = ToLayoutSize(LayoutPoint(FloorPointToDevicePixels(
        ancestorCompositedBounds.Location() - ancestorSubpixelOffsetFromRenderer, deviceScaleFactor)));
      LayoutSize ancestorRendererOffsetFromAncestorGraphicsLayer =
        -(floored + ancestorSubpixelOffsetFromRenderer);
      return ancestorRendererOffsetFromAncestorGraphicsLayer + ToLayoutSize(location);
    }

    class ComputedOffsets
    {
    private:
      Maybe<LayoutSize> _fromAncestorGraphicsLayer;
      Maybe<LayoutSize> _fromParentGraphicsLayer;
      Maybe<LayoutSize> _fromPrimaryGraphicsLayer;

      const RenderLayer &_renderLayer;
      const RenderLayer *_compositingAncestor;
      // Location is relative to the renderer.
      const LayoutPoint _location;
      const LayoutSize _parentGraphicsLayerOffset;
      const LayoutSize _primaryGraphicsLayerOffset;
      float _deviceScaleFactor;

    public:
      ComputedOffsets(const RenderLayer &renderLayer, const RenderLayer *compositingAncestor,
                      const LayoutRect &localRect, const LayoutRect &parentGraphicsLayerRect,
                      const LayoutRect &primaryGraphicsLayerRect, float deviceScaleFactor) noexcept
          : _renderLayer(renderLayer), _compositingAncestor(compositingAncestor),
            _location(localRect.Location()),
            _parentGraphicsLayerOffset(ToLayoutSize(parentGraphicsLayerRect.Location())),
            _primaryGraphicsLayerOffset(ToLayoutSize(primaryGraphicsLayerRect.Location())),
            _deviceScaleFactor(deviceScaleFactor)
      {
      }

      KRYS_NODISCARD LayoutSize FromParentGraphicsLayer() noexcept
      {
        if (!_fromParentGraphicsLayer)
        {
          _fromParentGraphicsLayer = FromAncestorGraphicsLayer() - _parentGraphicsLayerOffset;
        }
        return *_fromParentGraphicsLayer;
      }

      KRYS_NODISCARD LayoutSize FromPrimaryGraphicsLayer() noexcept
      {
        if (!_fromPrimaryGraphicsLayer)
        {
          _fromPrimaryGraphicsLayer =
            FromAncestorGraphicsLayer() - _parentGraphicsLayerOffset - _primaryGraphicsLayerOffset;
        }
        return *_fromPrimaryGraphicsLayer;
      }

    private:
      KRYS_NODISCARD LayoutSize FromAncestorGraphicsLayer() noexcept
      {
        if (!_fromAncestorGraphicsLayer)
        {
          LayoutPoint localPointInAncestorRenderLayerCoords =
            _renderLayer.ConvertToLayerCoords(_compositingAncestor, _location, RenderLayer::AdjustForColumns);
          _fromAncestorGraphicsLayer = ComputeOffsetFromAncestorGraphicsLayer(
            _compositingAncestor, localPointInAncestorRenderLayerCoords, _deviceScaleFactor);
        }
        return *_fromAncestorGraphicsLayer;
      }
    };
  }

  LayoutRect RenderLayerBacking::ComputePrimaryGraphicsLayerRect(
    const RenderLayer *compositedAncestor, const LayoutRect &parentGraphicsLayerRect) const noexcept
  {
    ComputedOffsets compositedBoundsOffset(_owningLayer, compositedAncestor, CompositedBounds(),
                                           parentGraphicsLayerRect, {}, DeviceScaleFactor());
    return LayoutRect(EncloseRectToDevicePixels(
      LayoutRect(ToLayoutPoint(compositedBoundsOffset.FromParentGraphicsLayer()), CompositedBounds().Size()),
      DeviceScaleFactor()));
  }

  // FIXME: See if we need this now that updateGeometry() is always called in post-order traversal.
  LayoutRect
    RenderLayerBacking::ComputeParentGraphicsLayerRect(const RenderLayer *compositedAncestor) const noexcept
  {
    if (!compositedAncestor || !compositedAncestor->Backing())
    {
      return Renderer().View().DocumentRect();
    }

    auto *ancestorBacking = compositedAncestor->Backing();
    LayoutRect parentGraphicsLayerRect;
    // FIXME: Fragmented flows (isInsideFragmentedFlow), once there are any.

    auto *ancestorRenderBox = DynamicDowncast<RenderBox>(compositedAncestor->Renderer());
    if (!ancestorRenderBox)
    {
      return parentGraphicsLayerRect;
    }

    if (ancestorBacking->HasClippingLayer())
    {
      // If the compositing ancestor has a layer to clip children, we parent in that, and therefore position
      // relative to it.
      LayoutRect clippingBox = ClippingLayerBox(*ancestorRenderBox);
      LayoutSize clippingBoxOffset = ComputeOffsetFromAncestorGraphicsLayer(
        compositedAncestor, clippingBox.Location(), DeviceScaleFactor());
      parentGraphicsLayerRect =
        SnappedGraphicsLayer(clippingBoxOffset, clippingBox.Size(), Renderer()).SnappedRect;
    }

    if (compositedAncestor->HasCompositedScrollableOverflow())
    {
      auto *scrollableArea = compositedAncestor->GetScrollableArea();
      krys_debug_assert(scrollableArea);

      LayoutRect ancestorCompositedBounds = ancestorBacking->CompositedBounds();
      LayoutRect scrollContainerBox = ScrollContainerLayerBox(*ancestorRenderBox);
      ScrollOffset scrollOffset = scrollableArea->GetScrollOffset();
      parentGraphicsLayerRect =
        LayoutRect((scrollContainerBox.Location() - ToLayoutSize(ancestorCompositedBounds.Location())
                    - LayoutSize(ToIntSize(scrollOffset))),
                   scrollContainerBox.Size());
    }

    return parentGraphicsLayerRect;
  }

  void RenderLayerBacking::UpdateGeometry(const RenderLayer *compositedAncestor) noexcept
  {
    krys_debug_assert(!_owningLayer.NormalFlowListDirty());
    krys_debug_assert(!_owningLayer.ZOrderListsDirty());
    krys_debug_assert(!_owningLayer.DescendantDependentFlagsAreDirty());
    krys_debug_assert(!Renderer().View().NeedsLayout());

    const RenderStyle &style = Renderer().Style();
    const auto deviceScaleFactor = DeviceScaleFactor();

    auto styleable = style::Styleable::FromRenderer(Renderer());
    bool isRunningAcceleratedTransformAnimation =
      styleable && styleable->IsRunningAcceleratedTransformRelatedAnimation();

    UpdateTransform(style);
    UpdateOpacity(style);
    UpdateFilters(style);
    UpdateBackdropFilters(style);
    (void)UpdateBackdropRoot();
    UpdateBlendMode(style);
    UpdateContentsScalingFilters(style);

    krys_debug_assert(compositedAncestor == _owningLayer.AncestorCompositingLayer());
    LayoutRect parentGraphicsLayerRect = ComputeParentGraphicsLayerRect(compositedAncestor);

    // FIXME: Content captured in a view transition, once there are view transitions.

    if (_ancestorClippingStack)
    {
      UpdateClippingStackLayerGeometry(*_ancestorClippingStack, compositedAncestor, parentGraphicsLayerRect);
    }

    LayoutRect primaryGraphicsLayerRect =
      ComputePrimaryGraphicsLayerRect(compositedAncestor, parentGraphicsLayerRect);

    ComputedOffsets compositedBoundsOffset(_owningLayer, compositedAncestor, CompositedBounds(),
                                           parentGraphicsLayerRect, primaryGraphicsLayerRect,
                                           deviceScaleFactor);
    ComputedOffsets rendererOffset(_owningLayer, compositedAncestor, {}, parentGraphicsLayerRect,
                                   primaryGraphicsLayerRect, deviceScaleFactor);

    _compositedBoundsOffsetFromGraphicsLayer = compositedBoundsOffset.FromPrimaryGraphicsLayer();

    auto primaryLayerPosition = FloatPoint(primaryGraphicsLayerRect.Location());

    // FIXME: Reflections force transform-style to be flat, once there are reflections.
    bool preserves3D = style.Preserves3D();

    if (_viewportAnchorLayer)
    {
      if (_viewportClippingLayer)
      {
        krys_debug_assert(compositedAncestor == Renderer().View().Layer());
        auto fixedPositionRect = Renderer().View().FrameView().RectForFixedPositionLayout();
        if (_ancestorClippingStack)
        {
          for (auto &entry : _ancestorClippingStack->Stack())
          {
            if (entry.ClippingLayer)
            {
              fixedPositionRect.MoveBy(-LayoutPoint {entry.ClippingLayer->Position()});
            }
          }
        }

        // RenderView's layer has a non-zero offsetFromGraphicsLayer in RTL content; we have to account for
        // that to map rectForFixedPositionLayout from content to GraphicsLayer coordinates.
        auto *renderViewLayer = Renderer().View().Layer();
        krys_debug_assert(renderViewLayer->IsComposited());
        fixedPositionRect.Move(renderViewLayer->Backing()->ContentOffsetInCompositingLayer());

        _viewportClippingLayer->SetPosition(FloatPoint(fixedPositionRect.Location()));
        _viewportClippingLayer->SetSize(FloatSize(fixedPositionRect.Size()));
        primaryLayerPosition.MoveBy(-FloatPoint(fixedPositionRect.Location()));
      }
      _viewportAnchorLayer->SetPosition(primaryLayerPosition);
      primaryLayerPosition = {};
    }

    // FIXME: Fixed container edges (setNeedsFixedContainerEdgesUpdateIfNeeded), once hosts sample them.

    if (_contentsContainmentLayer)
    {
      _contentsContainmentLayer->SetPreserves3D(preserves3D);
      _contentsContainmentLayer->SetPosition(primaryLayerPosition);
      primaryLayerPosition = {};
      // Use the same size as _graphicsLayer so transforms behave correctly.
      _contentsContainmentLayer->SetSize(FloatSize(primaryGraphicsLayerRect.Size()));
    }

    auto computeAnimationExtent = [&]() -> Maybe<FloatRect>
    {
      LayoutRect animatedBounds;
      if (isRunningAcceleratedTransformAnimation
          && _owningLayer.GetOverlapBoundsIncludingChildrenAccountingForTransformAnimations(
            animatedBounds, {RenderLayer::IncludeCompositedDescendants}))
      {
        return FloatRect(animatedBounds);
      }
      return null;
    };
    _graphicsLayer->SetAnimationExtent(computeAnimationExtent());
    _graphicsLayer->SetPreserves3D(preserves3D);
    _graphicsLayer->SetBackfaceVisibility(style.GetBackfaceVisibility() == BackfaceVisibility::Visible);

    _graphicsLayer->SetPosition(primaryLayerPosition);
    _graphicsLayer->SetSize(FloatSize(primaryGraphicsLayerRect.Size()));
    if (HasTiledBackingFlatteningLayer())
    {
      _childContainmentLayer->SetSize(FloatSize(primaryGraphicsLayerRect.Size()));
    }

    // Compute renderer offset from primary graphics layer. Note that primaryGraphicsLayerRect is in
    // parentGraphicsLayer's coordinate system which is not necessarily the same as the ancestor graphics
    // layer.
    OffsetFromRenderer primaryGraphicsLayerOffsetFromRenderer;
    LayoutSize oldSubpixelOffsetFromRenderer = _subpixelOffsetFromRenderer;
    primaryGraphicsLayerOffsetFromRenderer =
      ComputeOffsetFromRenderer(-rendererOffset.FromPrimaryGraphicsLayer(), deviceScaleFactor);
    _subpixelOffsetFromRenderer = primaryGraphicsLayerOffsetFromRenderer.SubpixelOffset;
    _hasSubpixelRounding =
      !_subpixelOffsetFromRenderer.IsZero() || CompositedBounds().Size() != primaryGraphicsLayerRect.Size();

    if (FloatSize(primaryGraphicsLayerOffsetFromRenderer.DevicePixelOffset)
        != _graphicsLayer->OffsetFromRenderer())
    {
      _graphicsLayer->SetOffsetFromRenderer(
        FloatSize(primaryGraphicsLayerOffsetFromRenderer.DevicePixelOffset));
    }

    // If we have a layer that clips children, position it.
    LayoutRect clippingBox;
    if (auto *clipLayer = ClippingLayer())
    {
      // clipLayer is the _childContainmentLayer.
      clippingBox = ClippingLayerBox(Renderer());
      // Clipping layer is parented in the primary graphics layer.
      LayoutSize clipBoxOffsetFromGraphicsLayer =
        ToLayoutSize(clippingBox.Location()) + rendererOffset.FromPrimaryGraphicsLayer();
      SnappedRectInfo snappedClippingGraphicsLayer =
        SnappedGraphicsLayer(clipBoxOffsetFromGraphicsLayer, clippingBox.Size(), Renderer());
      clipLayer->SetPosition(FloatPoint(snappedClippingGraphicsLayer.SnappedRect.Location()));
      clipLayer->SetSize(FloatSize(snappedClippingGraphicsLayer.SnappedRect.Size()));
      clipLayer->SetOffsetFromRenderer(
        FloatSize(ToLayoutSize(clippingBox.Location() - snappedClippingGraphicsLayer.SnapDelta)));

      auto computeMasksToBoundsRect = [&]
      {
        if (Renderer().Style().HasClipPath() || Renderer().Style().HasBorderRadius())
        {
          auto borderShape =
            BorderShape::ShapeForBorderRect(Renderer().Style(), _owningLayer.RendererBorderBoxRect());
          auto contentsClippingRect = borderShape.DeprecatedPixelSnappedInnerRoundedRect(deviceScaleFactor);
          contentsClippingRect.Move(-clipLayer->OffsetFromRenderer());
          return contentsClippingRect;
        }

        return FloatRoundedRect {FloatRect {{}, FloatSize(snappedClippingGraphicsLayer.SnappedRect.Size())}};
      };

      clipLayer->SetContentsClippingRect(computeMasksToBoundsRect());
    }

    if (_maskLayer)
    {
      UpdateMaskingLayerGeometry();
    }

    UpdateChildrenTransformAndAnchorPoint(primaryGraphicsLayerRect, rendererOffset.FromParentGraphicsLayer());

    // FIXME: Reflections, once there are reflections.

    if (_scrollContainerLayer)
    {
      krys_debug_assert(_scrolledContentsLayer);
      LayoutRect scrollContainerBox = ScrollContainerLayerBox(Downcast<RenderBox>(Renderer()));
      LayoutRect parentLayerBounds = ClippingLayer() ? scrollContainerBox : CompositedBounds();

      // FIXME: need to do some pixel snapping here.
      _scrollContainerLayer->SetPosition(
        FloatPoint(scrollContainerBox.Location() - parentLayerBounds.Location()));
      _scrollContainerLayer->SetSize(
        FloatSize(RoundedIntSize(LayoutSize(scrollContainerBox.Width(), scrollContainerBox.Height()))));

      auto *scrollableArea = _owningLayer.GetScrollableArea();
      krys_debug_assert(scrollableArea);

      ScrollOffset scrollOffset = scrollableArea->GetScrollOffset();
      UpdateScrollOffset(scrollOffset);

      FloatSize oldScrollingLayerOffset = _scrollContainerLayer->OffsetFromRenderer();
      _scrollContainerLayer->SetOffsetFromRenderer(FloatSize(ToLayoutSize(scrollContainerBox.Location())));

      bool paddingBoxOffsetChanged = oldScrollingLayerOffset != _scrollContainerLayer->OffsetFromRenderer();

      IntSize scrollSize;
      if (scrollableArea)
      {
        scrollSize = IntSize(scrollableArea->ScrollWidth(), scrollableArea->ScrollHeight());
      }
      if (FloatSize(scrollSize) != _scrolledContentsLayer->Size() || paddingBoxOffsetChanged)
      {
        _scrolledContentsLayer->SetNeedsDisplay();
      }

      _scrolledContentsLayer->SetSize(FloatSize(scrollSize));
      _scrolledContentsLayer->SetScrollOffset(scrollOffset, GraphicsLayer::ShouldSetNeedsDisplay::DoNotSet);
      _scrolledContentsLayer->SetOffsetFromRenderer(FloatSize(ToLayoutSize(scrollContainerBox.Location())),
                                                    GraphicsLayer::ShouldSetNeedsDisplay::DoNotSet);

      AdjustTiledBackingCoverage();
    }

    if (_overflowControlsContainer)
    {
      LayoutRect overflowControlsBox = OverflowControlsHostLayerRect(Downcast<RenderBox>(Renderer()));
      LayoutSize boxOffsetFromGraphicsLayer =
        ToLayoutSize(overflowControlsBox.Location()) + rendererOffset.FromPrimaryGraphicsLayer();
      SnappedRectInfo snappedBoxInfo =
        SnappedGraphicsLayer(boxOffsetFromGraphicsLayer, overflowControlsBox.Size(), Renderer());

      _overflowControlsContainer->SetPosition(FloatPoint(snappedBoxInfo.SnappedRect.Location()));
      _overflowControlsContainer->SetSize(FloatSize(snappedBoxInfo.SnappedRect.Size()));
      _overflowControlsContainer->SetMasksToBounds(true);
    }

    if (_foregroundLayer)
    {
      FloatSize foregroundSize;
      FloatSize foregroundOffset;
      auto needsDisplayOnOffsetChange = GraphicsLayer::ShouldSetNeedsDisplay::Set;
      if (_scrolledContentsLayer)
      {
        foregroundSize = _scrolledContentsLayer->Size();
        foregroundOffset = _scrolledContentsLayer->OffsetFromRenderer()
                           - FloatSize(ToIntSize(_scrolledContentsLayer->GetScrollOffset()));
        needsDisplayOnOffsetChange = GraphicsLayer::ShouldSetNeedsDisplay::DoNotSet;
      }
      else if (HasClippingLayer())
      {
        // If we have a clipping layer (which clips descendants), then the foreground layer is a child of it,
        // so that it gets correctly sorted with children. In that case, position relative to the clipping
        // layer.
        foregroundSize = FloatSize(clippingBox.Size());
        foregroundOffset = FloatSize(ToLayoutSize(clippingBox.Location()));
      }
      else
      {
        foregroundSize = FloatSize(primaryGraphicsLayerRect.Size());
        foregroundOffset = _graphicsLayer->OffsetFromRenderer();
      }

      _foregroundLayer->SetPosition({});
      _foregroundLayer->SetSize(foregroundSize);
      _foregroundLayer->SetOffsetFromRenderer(foregroundOffset, needsDisplayOnOffsetChange);
    }

    if (_backgroundLayer)
    {
      FloatPoint backgroundPosition;
      FloatSize backgroundSize = FloatSize(primaryGraphicsLayerRect.Size());
      if (BackgroundLayerPaintsFixedRootBackground())
      {
        const LocalFrameView &frameView = Renderer().View().FrameView();
        backgroundPosition = FloatPoint(frameView.ScrollPositionForFixedPosition());
        backgroundSize = FloatSize(frameView.GetLayoutSize());
      }
      else
      {
        // FIXME: SVG renderers' object bounding boxes, once there's SVG: a box's is its border box.
        auto boundingBox = GetRenderBox() ? FloatRect(GetRenderBox()->BorderBoxRect()) : FloatRect();
        backgroundPosition = boundingBox.Location();
        backgroundSize = boundingBox.Size();
      }
      _backgroundLayer->SetPosition(backgroundPosition);
      _backgroundLayer->SetSize(backgroundSize);
      _backgroundLayer->SetOffsetFromRenderer(_graphicsLayer->OffsetFromRenderer());
    }

    // If this layer was created just for clipping or to apply perspective, it doesn't need its own backing
    // store.
    LayoutRect ancestorCompositedBounds =
      compositedAncestor ? compositedAncestor->Backing()->CompositedBounds() : LayoutRect();
    SetRequiresOwnBackingStore(Compositor().RequiresOwnBackingStore(
      _owningLayer, compositedAncestor,
      LayoutRect(ToLayoutPoint(compositedBoundsOffset.FromParentGraphicsLayer()), CompositedBounds().Size()),
      ancestorCompositedBounds));
    UpdateBackdropFiltersGeometry();

    // FIXME: Widgets (updateAfterWidgetResize), once there are iframes and plug-ins.

    PositionOverflowControlsLayers();

    if (SubpixelOffsetFromRendererChanged(oldSubpixelOffsetFromRenderer, _subpixelOffsetFromRenderer,
                                          deviceScaleFactor)
        && CanIssueSetNeedsDisplay())
    {
      SetContentsNeedDisplay();
    }
  }

  void RenderLayerBacking::AdjustOverflowControlsPositionRelativeToAncestor(
    const RenderLayer &ancestorLayer) noexcept
  {
    krys_debug_assert(_overflowControlsContainer);
    krys_debug_assert(ancestorLayer.IsComposited());
    auto *ancestorBacking = ancestorLayer.Backing();
    if (!ancestorBacking)
    {
      return;
    }

    LayoutRect parentGraphicsLayerRect = ComputeParentGraphicsLayerRect(&ancestorLayer);
    LayoutRect primaryGraphicsLayerRect =
      ComputePrimaryGraphicsLayerRect(&ancestorLayer, parentGraphicsLayerRect);

    auto overflowControlsRect = OverflowControlsHostLayerRect(Downcast<RenderBox>(Renderer()));

    if (OverflowControlsHostLayerAncestorClippingStack())
    {
      UpdateClippingStackLayerGeometry(*_overflowControlsHostLayerAncestorClippingStack, &ancestorLayer,
                                       parentGraphicsLayerRect);
    }

    ComputedOffsets rendererOffset(_owningLayer, &ancestorLayer, {}, parentGraphicsLayerRect,
                                   primaryGraphicsLayerRect, DeviceScaleFactor());

    LayoutSize boxOffsetFromGraphicsLayer =
      ToLayoutSize(overflowControlsRect.Location()) + rendererOffset.FromParentGraphicsLayer();
    SnappedRectInfo snappedBoxInfo =
      SnappedGraphicsLayer(boxOffsetFromGraphicsLayer, overflowControlsRect.Size(), Renderer());

    _overflowControlsContainer->SetPosition(FloatPoint(snappedBoxInfo.SnappedRect.Location()));
    _overflowControlsContainer->SetSize(FloatSize(snappedBoxInfo.SnappedRect.Size()));
  }

  void RenderLayerBacking::UpdateScrollOffset(ScrollOffset scrollOffset) noexcept
  {
    // FIXME: Scrolling outside the main thread (currentScrollType, requiresScrollPositionReconciliation),
    // once there's async scrolling. Note that we implement the contents offset via the bounds origin on this
    // layer, rather than a position on the sublayer.
    _scrollContainerLayer->SetBoundsOrigin(FloatPoint(scrollOffset));

    krys_debug_assert(_scrolledContentsLayer->Position().IsZero());
  }

  void RenderLayerBacking::UpdateAfterDescendants() noexcept
  {
    // FIXME: this potentially duplicates work we did in updateConfiguration().
    PaintedContentsInfo contentsInfo(*this);

    if (!_owningLayer.IsRenderViewLayer())
    {
      bool didUpdateContentsRect = false;
      UpdateDirectlyCompositedBoxDecorations(contentsInfo, didUpdateContentsRect);
      if (!didUpdateContentsRect && _graphicsLayer->UsesContentsLayer())
      {
        ResetContentsRect();
      }
    }

    UpdateDrawsContent(contentsInfo);

    if (!_isMainFrameRenderViewLayer && !_isFrameLayerWithTiledBacking && !_requiresBackgroundLayer)
    {
      // For non-root layers, background is always painted by the primary graphics layer.
      krys_debug_assert(!_backgroundLayer);
      _graphicsLayer->SetContentsOpaque(
        !_hasSubpixelRounding && _owningLayer.BackgroundIsKnownToBeOpaqueInRect(CompositedBounds()));
    }

    _graphicsLayer->SetContentsVisible(_owningLayer.HasVisibleContent()
                                       || HasVisibleNonCompositedDescendants());
    if (_scrollContainerLayer)
    {
      _scrollContainerLayer->SetContentsVisible(Renderer().Style().UsedVisibility() == Visibility::Visible);

      bool userInteractive = Renderer().VisibleToHitTesting();
      _scrollContainerLayer->SetUserInteractionEnabled(userInteractive);
      if (_layerForHorizontalScrollbar)
      {
        _layerForHorizontalScrollbar->SetUserInteractionEnabled(userInteractive);
      }
      if (_layerForVerticalScrollbar)
      {
        _layerForVerticalScrollbar->SetUserInteractionEnabled(userInteractive);
      }
      if (_layerForScrollCorner)
      {
        _layerForScrollCorner->SetUserInteractionEnabled(userInteractive);
      }
    }
  }

  // FIXME: Avoid repaints when clip path changes.
  void RenderLayerBacking::UpdateMaskingLayerGeometry() noexcept
  {
    _maskLayer->SetSize(_graphicsLayer->Size());
    _maskLayer->SetPosition(FloatPoint());
    _maskLayer->SetOffsetFromRenderer(_graphicsLayer->OffsetFromRenderer());

    if (!_maskLayer->DrawsContent())
    {
      if (Renderer().HasClipPath())
      {
        // FIXME: Use correct reference box for inlines: https://bugs.webkit.org/show_bug.cgi?id=129047,
        // https://github.com/w3c/csswg-drafts/issues/6383
        LayoutRect boundingBox = _owningLayer.BoundingBox(&_owningLayer);
        LayoutRect referenceBoxForClippedInline =
          LayoutRect(SnapRectToDevicePixelsIfNeeded(boundingBox, Renderer()));
        LayoutSize offset =
          LayoutSize(SnapSizeToDevicePixel(-_subpixelOffsetFromRenderer, LayoutPoint(), DeviceScaleFactor()));
        auto [clipPath, windRule] = _owningLayer.ComputeClipPath(offset, referenceBoxForClippedInline);

        FloatSize pathOffset = _maskLayer->OffsetFromRenderer();
        if (!pathOffset.IsZero())
        {
          clipPath.Translate(-pathOffset);
        }

        _maskLayer->SetShapeLayerPath(clipPath);
        _maskLayer->SetShapeLayerWindRule(windRule);
      }
    }
  }

  void RenderLayerBacking::UpdateDirectlyCompositedBoxDecorations(PaintedContentsInfo &contentsInfo,
                                                                  bool &didUpdateContentsRect) noexcept
  {
    if (!_owningLayer.HasVisibleContent())
    {
      return;
    }

    // The order of operations here matters, since the last valid type of contents needs to also update the
    // contentsRect.
    UpdateDirectlyCompositedBackgroundColor(contentsInfo, didUpdateContentsRect);
    UpdateDirectlyCompositedBackgroundImage(contentsInfo, didUpdateContentsRect);
  }

  GraphicsLayer *RenderLayerBacking::LayerForContents() const noexcept
  {
    // FIXME: Plug-ins hosted in a graphics layer, once there are plug-ins.
    return nullptr;
  }

  void RenderLayerBacking::UpdateInternalHierarchy() noexcept
  {
    // _foregroundLayer has to be inserted in the correct order with child layers, so it's not inserted here.
    GraphicsLayer *lastClippingLayer = nullptr;
    if (_ancestorClippingStack)
    {
      ConnectClippingStackLayers(*_ancestorClippingStack);
      lastClippingLayer = _ancestorClippingStack->LastLayer();
    }

    List<GraphicsLayer *> orderedLayers;

    if (lastClippingLayer)
    {
      orderedLayers.push_back(lastClippingLayer);
    }

    if (_viewportClippingLayer)
    {
      orderedLayers.push_back(_viewportClippingLayer.get());
    }

    if (_viewportAnchorLayer)
    {
      orderedLayers.push_back(_viewportAnchorLayer.get());
    }

    if (_contentsContainmentLayer)
    {
      _contentsContainmentLayer->RemoveAllChildren();

      krys_debug_assert(_backgroundLayer);
      _contentsContainmentLayer->AddChild(ShareRef(*_backgroundLayer));

      // The loop below will add a second child to the _contentsContainmentLayer.
      orderedLayers.push_back(_contentsContainmentLayer.get());
    }

    orderedLayers.push_back(_graphicsLayer.get());

    // The transform flattening layer is outside the clipping stack, so we need to make sure we add the first
    // layer in the clipping stack as its child.
    if (_transformFlatteningLayer)
    {
      if (lastClippingLayer)
      {
        _transformFlatteningLayer->AddChild(ShareRef(*_ancestorClippingStack->FirstLayer()));
      }
      else
      {
        _transformFlatteningLayer->AddChild(ShareRef(*orderedLayers[0]));
      }
    }

    if (_childContainmentLayer)
    {
      orderedLayers.push_back(_childContainmentLayer.get());
    }

    if (_scrollContainerLayer)
    {
      orderedLayers.push_back(_scrollContainerLayer.get());
    }

    GraphicsLayer *previousLayer = nullptr;
    for (auto *layer : orderedLayers)
    {
      if (previousLayer)
      {
        previousLayer->AddChild(ShareRef(*layer));
      }

      previousLayer = layer;
    }

    // The clip for child layers does not include space for overflow controls, so they exist as siblings of
    // the clipping layer if we have one. Normal children of this layer are set as children of the clipping
    // layer.
    if (_overflowControlsContainer)
    {
      if (_layerForHorizontalScrollbar)
      {
        _overflowControlsContainer->AddChild(ShareRef(*_layerForHorizontalScrollbar));
      }

      if (_layerForVerticalScrollbar)
      {
        _overflowControlsContainer->AddChild(ShareRef(*_layerForVerticalScrollbar));
      }

      if (_layerForScrollCorner)
      {
        _overflowControlsContainer->AddChild(ShareRef(*_layerForScrollCorner));
      }

      // _overflowControlsContainer may get reparented later.
      _graphicsLayer->AddChild(ShareRef(*_overflowControlsContainer));
    }
  }

  void RenderLayerBacking::UpdateContentsRects() noexcept
  {
    _graphicsLayer->SetContentsRect(SnapRectToDevicePixelsIfNeeded(ContentsBox(), Renderer()));

    if (auto *renderReplaced = DynamicDowncast<RenderReplaced>(Renderer()))
    {
      // FIXME: Support visible overflow for replaced content.
      auto borderShape = renderReplaced->BorderShapeForContentClipping(renderReplaced->BorderBoxRect());
      // Snapped in the layer's coordinates, as the contents rect is, so the clip and the contents meet the
      // same pixels.
      auto contentsClippingRect = borderShape.DeprecatedInnerRoundedRect();
      contentsClippingRect.Move(ContentOffsetInCompositingLayer());
      _graphicsLayer->SetContentsClippingRect(
        contentsClippingRect.PixelSnappedRoundedRectForPainting(DeviceScaleFactor()));
    }
  }

  void RenderLayerBacking::ResetContentsRect() noexcept
  {
    UpdateContentsRects();
    _graphicsLayer->SetContentsTileSize(FloatSize());
    _graphicsLayer->SetContentsTilePhase(FloatSize());
  }

  void RenderLayerBacking::UpdateDrawsContent() noexcept
  {
    PaintedContentsInfo contentsInfo(*this);
    UpdateDrawsContent(contentsInfo);
  }

  void RenderLayerBacking::UpdateDrawsContent(PaintedContentsInfo &contentsInfo) noexcept
  {
    if (_scrollContainerLayer)
    {
      // We don't have to consider overflow controls, because we know that the scrollbars are drawn elsewhere.
      // _graphicsLayer only needs backing store if the non-scrolling parts (background, outlines, borders,
      // shadows etc) need to paint. _scrollContainerLayer never has backing store. _scrolledContentsLayer
      // only needs backing store if the scrolled contents need to paint.
      bool hasNonScrollingPaintedContent =
        _owningLayer.HasVisibleContent() && _owningLayer.HasVisibleBoxDecorationsOrBackground();
      _graphicsLayer->SetDrawsContent(hasNonScrollingPaintedContent);

      bool hasScrollingPaintedContent =
        HasBackingSharingLayers()
        || (_owningLayer.HasVisibleContent() && (Renderer().HasBackground() || contentsInfo.PaintsContent()));
      _scrolledContentsLayer->SetDrawsContent(hasScrollingPaintedContent);
      return;
    }

    // FIXME: HDR content, once there's HDR.

    bool hasPaintedContent = ContainsPaintedContent(contentsInfo);

    // FIXME: we could refine this to only allocate backing for one of these layers if possible.
    _graphicsLayer->SetDrawsContent(hasPaintedContent);
    if (_foregroundLayer)
    {
      _foregroundLayer->SetDrawsContent(hasPaintedContent);
    }

    if (_backgroundLayer)
    {
      _backgroundLayer->SetDrawsContent(
        _backgroundLayerPaintsFixedRootBackground ? hasPaintedContent : contentsInfo.PaintsBoxDecorations());
    }
  }

  bool RenderLayerBacking::UpdateAncestorClippingStack(List<CompositedClipData> &&clippingData) noexcept
  {
    if (!_ancestorClippingStack && clippingData.empty())
    {
      return false;
    }

    if (_ancestorClippingStack && clippingData.empty())
    {
      _ancestorClippingStack->Clear();
      _ancestorClippingStack = nullptr;

      if (_overflowControlsHostLayerAncestorClippingStack)
      {
        _overflowControlsHostLayerAncestorClippingStack->Clear();
        _overflowControlsHostLayerAncestorClippingStack = nullptr;
      }
      return true;
    }

    if (!_ancestorClippingStack)
    {
      _ancestorClippingStack = CreateUnique<LayerAncestorClippingStack>(krys::move(clippingData));
      return true;
    }

    if (_ancestorClippingStack->EqualToClipData(clippingData))
    {
      return false;
    }

    if (_overflowControlsHostLayerAncestorClippingStack)
    {
      _overflowControlsHostLayerAncestorClippingStack->UpdateWithClipData(
        List<CompositedClipData>(clippingData));
    }
    (void)_ancestorClippingStack->UpdateWithClipData(krys::move(clippingData));
    return true;
  }

  void RenderLayerBacking::EnsureOverflowControlsHostLayerAncestorClippingStack(
    const RenderLayer *compositedAncestor) noexcept
  {
    auto clippingData = _ancestorClippingStack->GetCompositedClipData();

    if (_overflowControlsHostLayerAncestorClippingStack)
    {
      (void)_overflowControlsHostLayerAncestorClippingStack->UpdateWithClipData(krys::move(clippingData));
    }
    else
    {
      _overflowControlsHostLayerAncestorClippingStack =
        CreateUnique<LayerAncestorClippingStack>(krys::move(clippingData));
    }

    EnsureClippingStackLayers(*_overflowControlsHostLayerAncestorClippingStack);

    LayoutRect parentGraphicsLayerRect = ComputeParentGraphicsLayerRect(compositedAncestor);
    UpdateClippingStackLayerGeometry(*_overflowControlsHostLayerAncestorClippingStack, compositedAncestor,
                                     parentGraphicsLayerRect);

    ConnectClippingStackLayers(*_overflowControlsHostLayerAncestorClippingStack);
  }

  void RenderLayerBacking::EnsureClippingStackLayers(LayerAncestorClippingStack &clippingStack) noexcept
  {
    for (auto &entry : clippingStack.Stack())
    {
      if (!entry.ClippingLayer)
      {
        entry.ClippingLayer = CreateGraphicsLayer(entry.ClipData.IsOverflowScroll ? u8"clip for scroller"
                                                                                  : u8"ancestor clipping");
        entry.ClippingLayer->SetMasksToBounds(true);
        entry.ClippingLayer->SetPaintingPhase({});
      }

      if (entry.ClipData.IsOverflowScroll)
      {
        if (!entry.ScrollingLayer)
        {
          entry.ScrollingLayer = CreateGraphicsLayer(u8"scrolling proxy");
        }
      }
      else if (entry.ScrollingLayer)
      {
        GraphicsLayer::UnparentAndClear(entry.ScrollingLayer);
      }
    }
  }

  void RenderLayerBacking::RemoveClippingStackLayers(LayerAncestorClippingStack &clippingStack) noexcept
  {
    for (auto &entry : clippingStack.Stack())
    {
      GraphicsLayer::UnparentAndClear(entry.ClippingLayer);
      GraphicsLayer::UnparentAndClear(entry.ScrollingLayer);
    }
  }

  void RenderLayerBacking::ConnectClippingStackLayers(LayerAncestorClippingStack &clippingStack) noexcept
  {
    auto connectEntryLayers = [](LayerAncestorClippingStack::ClippingStackEntry &entry)
    {
      if (entry.ScrollingLayer)
      {
        List<Ref<GraphicsLayer>> children;
        children.push_back(ShareRef(*entry.ScrollingLayer));
        entry.ClippingLayer->SetChildren(krys::move(children));
      }
    };

    auto &clippingEntryStack = clippingStack.Stack();
    for (size_t i = 0; i + 1 < clippingEntryStack.size(); ++i)
    {
      auto &entry = clippingEntryStack[i];
      connectEntryLayers(entry);

      auto *entryParentForSublayers = entry.ParentForSublayers();
      auto *childLayer = clippingEntryStack[i + 1].ChildForSuperlayers();
      List<Ref<GraphicsLayer>> children;
      children.push_back(ShareRef(*childLayer));
      entryParentForSublayers->SetChildren(krys::move(children));
    }

    connectEntryLayers(clippingEntryStack.back());
    clippingEntryStack.back().ParentForSublayers()->RemoveAllChildren();
  }

  void RenderLayerBacking::UpdateClippingStackLayerGeometry(LayerAncestorClippingStack &clippingStack,
                                                            const RenderLayer *compositedAncestor,
                                                            LayoutRect &parentGraphicsLayerRect) noexcept
  {
    // All clipRects in the stack are computed relative to _owningLayer, so convert them back to
    // compositedAncestor.
    auto offsetFromCompositedAncestor =
      ToLayoutSize(_owningLayer.ConvertToLayerCoords(compositedAncestor, {}, RenderLayer::AdjustForColumns));
    LayoutRect lastClipLayerRect = parentGraphicsLayerRect;

    auto deviceScaleFactor = DeviceScaleFactor();
    for (auto &entry : clippingStack.Stack())
    {
      auto roundedClipRect = entry.ClipData.ClipRect;
      auto clipRect = roundedClipRect.Rect();
      LayoutSize clippingOffset = ComputeOffsetFromAncestorGraphicsLayer(
        compositedAncestor, clipRect.Location() + offsetFromCompositedAncestor, deviceScaleFactor);
      LayoutRect snappedClippingLayerRect =
        SnappedGraphicsLayer(clippingOffset, clipRect.Size(), Renderer()).SnappedRect;

      auto clippingLayerPosition =
        ToLayoutPoint(snappedClippingLayerRect.Location() - lastClipLayerRect.Location());
      entry.ClippingLayer->SetPosition(FloatPoint(clippingLayerPosition));
      entry.ClippingLayer->SetSize(FloatSize(snappedClippingLayerRect.Size()));

      clipRect.SetLocation({});
      roundedClipRect.SetRect(clipRect);
      entry.ClippingLayer->SetContentsClippingRect(FloatRoundedRect(roundedClipRect));
      entry.ClippingLayer->SetContentsRectClipsDescendants(true);

      lastClipLayerRect = snappedClippingLayerRect;

      if (entry.ClipData.IsOverflowScroll)
      {
        ScrollOffset scrollOffset;
        if (auto *scrollableArea =
              entry.ClipData.ClippingLayer ? entry.ClipData.ClippingLayer->GetScrollableArea() : nullptr)
        {
          scrollOffset = scrollableArea->GetScrollOffset();
        }

        // scrollingLayer size and position are always 0,0.
        entry.ScrollingLayer->SetBoundsOrigin(FloatPoint(scrollOffset));
        lastClipLayerRect.MoveBy(-LayoutPoint(scrollOffset));
      }
    }

    parentGraphicsLayerRect = lastClipLayerRect;
  }

  // Return true if the layer changed.
  bool RenderLayerBacking::UpdateAncestorClipping(bool needsAncestorClip,
                                                  const RenderLayer *compositingAncestor) noexcept
  {
    bool layersChanged = false;

    if (needsAncestorClip)
    {
      if (Compositor().UpdateAncestorClippingStack(_owningLayer, compositingAncestor))
      {
        if (_ancestorClippingStack)
        {
          EnsureClippingStackLayers(*_ancestorClippingStack);
        }

        layersChanged = true;
      }
    }
    else if (_ancestorClippingStack)
    {
      _ancestorClippingStack->Clear();
      _ancestorClippingStack = nullptr;

      if (_overflowControlsHostLayerAncestorClippingStack)
      {
        _overflowControlsHostLayerAncestorClippingStack->Clear();
        _overflowControlsHostLayerAncestorClippingStack = nullptr;
      }

      layersChanged = true;
    }

    return layersChanged;
  }

  // Return true if the layer changed.
  bool RenderLayerBacking::UpdateDescendantClippingLayer(bool needsDescendantClip) noexcept
  {
    bool layersChanged = false;

    if (needsDescendantClip)
    {
      if (!_childContainmentLayer && !_isFrameLayerWithTiledBacking)
      {
        _childContainmentLayer = CreateGraphicsLayer(u8"child clipping");
        _childContainmentLayer->SetMasksToBounds(true);
        _childContainmentLayer->SetContentsRectClipsDescendants(true);
        layersChanged = true;
      }
    }
    else if (HasClippingLayer())
    {
      WillDestroyLayer(_childContainmentLayer.get());
      GraphicsLayer::UnparentAndClear(_childContainmentLayer);
      layersChanged = true;
    }

    return layersChanged;
  }

  bool RenderLayerBacking::NeedsRepaintOnCompositedScroll() const noexcept
  {
    if (!HasScrollingLayer())
    {
      return false;
    }

    if (style::HasImageWithAttachment(Renderer().Style().GetBackgroundLayers(),
                                      FillAttachment::LocalBackground))
    {
      return true;
    }

    // FIXME: The scrolling coordinator's synchronous scrolling reasons, once there's async scrolling.
    return false;
  }

  void RenderLayerBacking::SetBackgroundLayerPaintsFixedRootBackground(
    bool backgroundLayerPaintsFixedRootBackground) noexcept
  {
    if (backgroundLayerPaintsFixedRootBackground == _backgroundLayerPaintsFixedRootBackground)
    {
      return;
    }

    _backgroundLayerPaintsFixedRootBackground = backgroundLayerPaintsFixedRootBackground;

    // FIXME: Fixed root backgrounds in a tiled backing (removeSlowRepaintObject), once there are tiled
    // backings.
    krys_debug_assert(!_backgroundLayerPaintsFixedRootBackground || _isFrameLayerWithTiledBacking);
  }

  void RenderLayerBacking::SetRequiresBackgroundLayer(bool requiresBackgroundLayer) noexcept
  {
    if (requiresBackgroundLayer == _requiresBackgroundLayer)
    {
      return;
    }

    _requiresBackgroundLayer = requiresBackgroundLayer;
    _owningLayer.SetNeedsCompositingConfigurationUpdate();
  }

  bool RenderLayerBacking::RequiresLayerForScrollbar(Scrollbar *scrollbar) const noexcept
  {
    // Overflow scrolls on layers whenever the page composites (asyncOverflowScrollingEnabled).
    return scrollbar && (scrollbar->IsOverlayScrollbar() || Compositor().HasAcceleratedCompositing());
  }

  bool RenderLayerBacking::RequiresHorizontalScrollbarLayer() const noexcept
  {
    if (auto *scrollableArea = _owningLayer.GetScrollableArea())
    {
      return RequiresLayerForScrollbar(scrollableArea->HorizontalScrollbar());
    }
    return false;
  }

  bool RenderLayerBacking::RequiresVerticalScrollbarLayer() const noexcept
  {
    if (auto *scrollableArea = _owningLayer.GetScrollableArea())
    {
      return RequiresLayerForScrollbar(scrollableArea->VerticalScrollbar());
    }
    return false;
  }

  bool RenderLayerBacking::RequiresScrollCornerLayer() const noexcept
  {
    if (!Is<RenderBox>(_owningLayer.Renderer()))
    {
      return false;
    }

    auto *scrollableArea = _owningLayer.GetScrollableArea();
    if (!scrollableArea)
    {
      return false;
    }

    auto cornerRect = scrollableArea->OverflowControlsRects().ScrollCornerOrResizerRect();
    if (cornerRect.IsEmpty())
    {
      return false;
    }

    auto *verticalScrollbar = scrollableArea->VerticalScrollbar();
    auto *scrollbar = verticalScrollbar ? verticalScrollbar : scrollableArea->HorizontalScrollbar();
    return RequiresLayerForScrollbar(scrollbar);
  }

  bool RenderLayerBacking::UpdateOverflowControlsLayers(bool needsHorizontalScrollbarLayer,
                                                        bool needsVerticalScrollbarLayer,
                                                        bool needsScrollCornerLayer) noexcept
  {
    auto createOrDestroyLayer =
      [&](RefPtr<GraphicsLayer> &layer, bool needLayer, bool drawsContent, utf8_stringview layerName)
    {
      if (needLayer == static_cast<bool>(layer))
      {
        return false;
      }

      if (needLayer)
      {
        layer = CreateGraphicsLayer(utf8_string(layerName));
        if (drawsContent)
        {
          layer->SetAllowsBackingStoreDetaching(false);
          layer->SetAllowsTiling(false);
        }
        else
        {
          layer->SetPaintingPhase({});
          layer->SetDrawsContent(false);
        }
      }
      else
      {
        WillDestroyLayer(layer.get());
        GraphicsLayer::UnparentAndClear(layer);
      }
      return true;
    };

    bool layersChanged = createOrDestroyLayer(_overflowControlsContainer,
                                              needsHorizontalScrollbarLayer || needsVerticalScrollbarLayer
                                                || needsScrollCornerLayer,
                                              false, u8"overflow controls container");

    bool horizontalScrollbarLayerChanged = createOrDestroyLayer(
      _layerForHorizontalScrollbar, needsHorizontalScrollbarLayer, true, u8"horizontal scrollbar");
    layersChanged |= horizontalScrollbarLayerChanged;

    bool verticalScrollbarLayerChanged = createOrDestroyLayer(
      _layerForVerticalScrollbar, needsVerticalScrollbarLayer, true, u8"vertical scrollbar");
    layersChanged |= verticalScrollbarLayerChanged;

    layersChanged |=
      createOrDestroyLayer(_layerForScrollCorner, needsScrollCornerLayer, true, u8"scroll corner");

    // FIXME: The scrolling coordinator (scrollableAreaScrollbarLayerDidChange), once there's async scrolling.

    return layersChanged;
  }

  void RenderLayerBacking::PositionOverflowControlsLayers() noexcept
  {
    auto *scrollableArea = _owningLayer.GetScrollableArea();
    if (!scrollableArea || !scrollableArea->HasScrollbars())
    {
      return;
    }
    // FIXME: Should do device-pixel snapping.
    auto *box = GetRenderBox();
    auto borderBox = SnappedIntRect(box->BorderBoxRect());

    // _overflowControlsContainer is positioned using the paddingBoxRectIncludingScrollbar.
    auto paddingBox = SnappedIntRect(box->PaddingBoxRectIncludingScrollbar());
    auto paddingBoxInset = paddingBox.Location() - borderBox.Location();

    auto positionScrollbarLayer =
      [](GraphicsLayer &layer, const IntRect &scrollbarRect, IntSize paddingBoxInset)
    {
      layer.SetPosition(FloatPoint(scrollbarRect.Location() - paddingBoxInset));
      layer.SetSize(FloatSize(scrollbarRect.Size()));
      if (layer.UsesContentsLayer())
      {
        IntRect barRect = IntRect(IntPoint(), scrollbarRect.Size());
        layer.SetContentsRect(FloatRect(barRect));
        layer.SetContentsClippingRect(FloatRoundedRect(FloatRect(barRect)));
      }
    };

    // These rects are relative to the borderBoxRect.
    auto rects = scrollableArea->OverflowControlsRects();
    if (auto *layer = LayerForHorizontalScrollbar())
    {
      positionScrollbarLayer(*layer, rects.HorizontalScrollbar, paddingBoxInset);
      layer->SetDrawsContent(scrollableArea->HorizontalScrollbar() && !layer->UsesContentsLayer());
    }

    if (auto *layer = LayerForVerticalScrollbar())
    {
      positionScrollbarLayer(*layer, rects.VerticalScrollbar, paddingBoxInset);
      layer->SetDrawsContent(scrollableArea->VerticalScrollbar() && !layer->UsesContentsLayer());
    }

    if (auto *layer = LayerForScrollCorner())
    {
      auto cornerRect = rects.ScrollCornerOrResizerRect();
      layer->SetPosition(FloatPoint(cornerRect.Location() - paddingBoxInset));
      layer->SetSize(FloatSize(cornerRect.Size()));
      layer->SetDrawsContent(!cornerRect.IsEmpty());
    }
  }

  namespace
  {
    KRYS_NODISCARD bool AncestorLayerWillCombineTransform(const RenderLayer *compositingAncestor) noexcept
    {
      if (!compositingAncestor)
      {
        return false;
      }
      return compositingAncestor->Preserves3D() || compositingAncestor->HasPerspective();
    }
  }

  bool RenderLayerBacking::UpdateTransformFlatteningLayer(const RenderLayer *compositingAncestor) noexcept
  {
    bool needsFlatteningLayer = false;
    // If our parent layer has preserve-3d or perspective, and it's not our DOM parent, then we need a
    // flattening layer to block that from being applied in 3d.
    if (AncestorLayerWillCombineTransform(compositingAncestor)
        && !_owningLayer.AncestorLayerIsDOMParent(compositingAncestor))
    {
      needsFlatteningLayer = true;
    }

    bool layerChanged = false;
    if (needsFlatteningLayer)
    {
      if (!_transformFlatteningLayer)
      {
        _transformFlatteningLayer = CreateGraphicsLayer(_owningLayer.Name() + u8" (3d flattening)");
        layerChanged = true;
      }
    }
    else if (_transformFlatteningLayer)
    {
      WillDestroyLayer(_transformFlatteningLayer.get());
      GraphicsLayer::UnparentAndClear(_transformFlatteningLayer);
      layerChanged = true;
    }

    return layerChanged;
  }

  bool RenderLayerBacking::UpdateViewportConstrainedSublayers(ViewportConstrainedSublayers sublayers) noexcept
  {
    bool layerChanged = false;

    bool needsAnchorLayer = false;
    bool needsClippingLayer = false;
    switch (sublayers)
    {
      case ViewportConstrainedSublayers::None:              break;
      case ViewportConstrainedSublayers::ClippingAndAnchor: needsClippingLayer = true; [[fallthrough]];
      case ViewportConstrainedSublayers::Anchor:            needsAnchorLayer = true; break;
    }

    if (needsAnchorLayer)
    {
      if (!_viewportAnchorLayer)
      {
        _viewportAnchorLayer =
          CreateGraphicsLayer(_owningLayer.Name() + u8" (anchor)", GraphicsLayer::Type::Structural);
        layerChanged = true;
      }
    }
    else if (_viewportAnchorLayer)
    {
      WillDestroyLayer(_viewportAnchorLayer.get());
      GraphicsLayer::UnparentAndClear(_viewportAnchorLayer);
      layerChanged = true;
    }

    if (needsClippingLayer)
    {
      if (!_viewportClippingLayer)
      {
        _viewportClippingLayer =
          CreateGraphicsLayer(_owningLayer.Name() + u8" (clipping)", GraphicsLayer::Type::Normal);
        _viewportClippingLayer->SetMasksToBounds(true);
        layerChanged = true;
      }
    }
    else if (_viewportClippingLayer)
    {
      WillDestroyLayer(_viewportClippingLayer.get());
      GraphicsLayer::UnparentAndClear(_viewportClippingLayer);
      layerChanged = true;
    }

    return layerChanged;
  }

  bool RenderLayerBacking::UpdateForegroundLayer(bool needsForegroundLayer) noexcept
  {
    bool layerChanged = false;
    if (needsForegroundLayer)
    {
      if (!_foregroundLayer)
      {
        _foregroundLayer = CreateGraphicsLayer(_owningLayer.Name() + u8" (foreground)");
        _foregroundLayer->SetDrawsContent(true);
        layerChanged = true;
      }
    }
    else if (_foregroundLayer)
    {
      WillDestroyLayer(_foregroundLayer.get());
      GraphicsLayer::UnparentAndClear(_foregroundLayer);
      layerChanged = true;
    }

    return layerChanged;
  }

  bool RenderLayerBacking::UpdateBackgroundLayer(bool needsBackgroundLayer) noexcept
  {
    bool layerChanged = false;
    if (needsBackgroundLayer)
    {
      if (!_backgroundLayer)
      {
        _backgroundLayer = CreateGraphicsLayer(_owningLayer.Name() + u8" (background)");
        _backgroundLayer->SetDrawsContent(true);
        _backgroundLayer->SetAnchorPoint(FloatPoint3D());
        layerChanged = true;
      }

      if (!_contentsContainmentLayer)
      {
        _contentsContainmentLayer = CreateGraphicsLayer(_owningLayer.Name() + u8" (contents containment)");
        _contentsContainmentLayer->SetAppliesPageScale(true);
        _graphicsLayer->SetAppliesPageScale(false);
        layerChanged = true;
      }
    }
    else
    {
      if (_backgroundLayer)
      {
        WillDestroyLayer(_backgroundLayer.get());
        GraphicsLayer::UnparentAndClear(_backgroundLayer);
        layerChanged = true;
      }
      if (_contentsContainmentLayer)
      {
        WillDestroyLayer(_contentsContainmentLayer.get());
        GraphicsLayer::UnparentAndClear(_contentsContainmentLayer);
        layerChanged = true;
        _graphicsLayer->SetAppliesPageScale(true);
      }
    }

    return layerChanged;
  }

  // Masking layer is used for masks or clip-path.
  bool RenderLayerBacking::UpdateMaskingLayer(bool hasMask, bool hasClipPath) noexcept
  {
    bool layerChanged = false;
    if (hasMask || hasClipPath)
    {
      EnumSet<GraphicsLayerPaintingPhase> maskPhases;
      if (hasMask)
      {
        maskPhases = GraphicsLayerPaintingPhase::Mask;
      }

      if (hasClipPath)
      {
        // If we have a mask, we need to paint the combined clip-path and mask into the mask layer.
        if (hasMask || Renderer().Style().GetClipPath().IsReference()
            || !GraphicsLayer::SupportsLayerType(GraphicsLayer::Type::Shape))
        {
          maskPhases.Add(GraphicsLayerPaintingPhase::ClipPath);
        }
      }

      bool paintsContent = !maskPhases.IsEmpty();
      GraphicsLayer::Type requiredLayerType =
        paintsContent ? GraphicsLayer::Type::Normal : GraphicsLayer::Type::Shape;
      if (_maskLayer && _maskLayer->GetType() != requiredLayerType)
      {
        _graphicsLayer->SetMaskLayer(nullptr);
        WillDestroyLayer(_maskLayer.get());
        GraphicsLayer::Clear(_maskLayer);
      }

      if (!_maskLayer)
      {
        _maskLayer = CreateGraphicsLayer(u8"mask", requiredLayerType);
        layerChanged = true;
        _graphicsLayer->SetMaskLayer(RefPtr<GraphicsLayer>(_maskLayer));
        // We need a geometry update to size the new mask layer.
        _owningLayer.SetNeedsCompositingGeometryUpdate();
      }
      _maskLayer->SetDrawsContent(paintsContent);
      _maskLayer->SetPaintingPhase(maskPhases);
    }
    else if (_maskLayer)
    {
      _graphicsLayer->SetMaskLayer(nullptr);
      WillDestroyLayer(_maskLayer.get());
      GraphicsLayer::Clear(_maskLayer);
      layerChanged = true;
    }

    return layerChanged;
  }

  bool RenderLayerBacking::UpdateScrollingLayers(bool needsScrollingLayers) noexcept
  {
    if (needsScrollingLayers == static_cast<bool>(_scrollContainerLayer))
    {
      return false;
    }

    if (!_scrollContainerLayer)
    {
      // Outer layer which corresponds with the scroll view. This never paints content.
      _scrollContainerLayer = CreateGraphicsLayer(u8"scroll container", GraphicsLayer::Type::ScrollContainer);
      _scrollContainerLayer->SetPaintingPhase({});
      _scrollContainerLayer->SetDrawsContent(false);
      _scrollContainerLayer->SetMasksToBounds(true);

      // Inner layer which renders the content that scrolls.
      _scrolledContentsLayer =
        CreateGraphicsLayer(u8"scrolled contents", GraphicsLayer::Type::ScrolledContents);
      _scrolledContentsLayer->SetDrawsContent(true);
      _scrolledContentsLayer->SetAnchorPoint({});
      _scrollContainerLayer->AddChild(ShareRef(*_scrolledContentsLayer));
    }
    else
    {
      // FIXME: The scrolling coordinator (willRemoveScrollingLayerWithBacking), once there's async scrolling.

      WillDestroyLayer(_scrollContainerLayer.get());
      WillDestroyLayer(_scrolledContentsLayer.get());

      GraphicsLayer::UnparentAndClear(_scrollContainerLayer);
      GraphicsLayer::UnparentAndClear(_scrolledContentsLayer);
    }

    // FIXME: The scrolling coordinator (didAddScrollingLayer), once there's async scrolling.

    return true;
  }

  float RenderLayerBacking::CompositingOpacity(float rendererOpacity) const noexcept
  {
    float finalOpacity = rendererOpacity;

    for (auto *curr = _owningLayer.StackingContext(); curr; curr = curr->StackingContext())
    {
      // If we found a compositing layer, we want to compute opacity relative to it. So we can break here.
      if (curr->IsComposited())
      {
        break;
      }

      finalOpacity *= curr->Renderer().Style().GetOpacity().Value.Value;
    }

    return finalOpacity;
  }

  namespace
  {
    // FIXME: Code is duplicated in RenderLayer. Also, we should probably not consider filters a box
    // decoration here.
    KRYS_NODISCARD bool HasVisibleBoxDecorations(const RenderStyle &style) noexcept
    {
      return style.HasVisibleBorder() || style.HasBorderRadius() || style.HasOutline()
             || style.HasUsedAppearance() || style.HasBoxShadow() || style.HasFilter();
    }

    KRYS_NODISCARD bool CanDirectlyCompositeBackgroundBackgroundImage(const RenderElement &renderer) noexcept
    {
      const RenderStyle &style = renderer.Style();

      if (!GraphicsLayer::SupportsContentsTiling())
      {
        return false;
      }

      const auto &backgroundLayers = style.GetBackgroundLayers();
      if (backgroundLayers.UsedLength() > 1)
      {
        return false;
      }

      if (!style::ImagesAreLoaded(backgroundLayers, renderer))
      {
        return false;
      }

      const auto &layer = backgroundLayers.UsedFirst();

      if (layer.GetAttachment() != FillAttachment::ScrollBackground)
      {
        return false;
      }

      // FIXME: Allow color+image compositing when it makes sense. For now bailing out.
      if (style.VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor).IsVisible())
      {
        return false;
      }

      // FIXME: support gradients with isGeneratedImage.
      RefPtr<StyleImage> styleImage = layer.GetImage().TryStyleImage();
      if (!styleImage->HasCachedImage())
      {
        return false;
      }

      RefPtr<Image> image = ShareRefPtr(styleImage->GetCachedImage()->GetImage());
      if (!image->IsBitmapImage())
      {
        return false;
      }

      return true;
    }

    KRYS_NODISCARD bool HasPaintedBoxDecorationsOrBackgroundImage(const RenderElement &renderer) noexcept
    {
      const RenderStyle &style = renderer.Style();

      if (HasVisibleBoxDecorations(style))
      {
        return true;
      }

      if (!style.HasBackgroundImage())
      {
        return false;
      }

      return !CanDirectlyCompositeBackgroundBackgroundImage(renderer);
    }

    KRYS_NODISCARD bool HasPerspectiveOrPreserves3D(const RenderStyle &style) noexcept
    {
      return style.HasPerspective() || style.Preserves3D();
    }
  }

  Color RenderLayerBacking::RendererBackgroundColor() const noexcept
  {
    const RenderElement *backgroundRenderer = nullptr;
    if (Renderer().IsDocumentElementRenderer())
    {
      backgroundRenderer = Renderer().View().RendererForRootBackground();
    }

    if (!backgroundRenderer)
    {
      backgroundRenderer = &Renderer();
    }

    return backgroundRenderer->Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
  }

  void RenderLayerBacking::UpdateDirectlyCompositedBackgroundColor(PaintedContentsInfo &contentsInfo,
                                                                   bool &didUpdateContentsRect) noexcept
  {
    if (_backgroundLayer && !_backgroundLayerPaintsFixedRootBackground
        && !contentsInfo.PaintsBoxDecorations())
    {
      _graphicsLayer->SetContentsToSolidColor(Color());
      _backgroundLayer->SetContentsToSolidColor(RendererBackgroundColor());

      FloatRect contentsRect = BackgroundBoxForSimpleContainerPainting();
      // NOTE: This is currently only used by RenderFullScreen, which we want to be big enough to hide
      // overflow areas of the root.
      contentsRect.Inflate(contentsRect.Size());
      _backgroundLayer->SetContentsRect(contentsRect);
      _backgroundLayer->SetContentsClippingRect(FloatRoundedRect(contentsRect));
      return;
    }

    if (!contentsInfo.IsSimpleContainer()
        || (Is<RenderBox>(Renderer()) && !BackgroundPainter::PaintsOwnBackground(*GetRenderBox())))
    {
      _graphicsLayer->SetContentsToSolidColor(Color());
      return;
    }

    Color backgroundColor = RendererBackgroundColor();

    // An unset (invalid) color will remove the solid color.
    _graphicsLayer->SetContentsToSolidColor(backgroundColor);
    FloatRect contentsRect = BackgroundBoxForSimpleContainerPainting();
    _graphicsLayer->SetContentsRect(contentsRect);
    _graphicsLayer->SetContentsClippingRect(FloatRoundedRect(contentsRect));
    didUpdateContentsRect = true;
  }

  void RenderLayerBacking::UpdateDirectlyCompositedBackgroundImage(PaintedContentsInfo &contentsInfo,
                                                                   bool &didUpdateContentsRect) noexcept
  {
    if (!GraphicsLayer::SupportsContentsTiling())
    {
      return;
    }

    if (contentsInfo.IsDirectlyCompositedImage())
    {
      return;
    }

    const auto &style = Renderer().Style();
    if (!contentsInfo.IsSimpleContainer() || !style.HasBackgroundImage())
    {
      _graphicsLayer->SetContentsToImage(nullptr);
      return;
    }

    const auto &backgroundLayer = style.GetBackgroundLayers().UsedFirst();
    auto backgroundBox = LayoutRect {BackgroundBoxForSimpleContainerPainting()};
    // FIXME: Absolute paint location is required here.
    auto geometry = BackgroundPainter::CalculateFillLayerImageGeometry(*GetRenderBox(), GetRenderBox(),
                                                                       backgroundLayer, {}, backgroundBox);

    _graphicsLayer->SetContentsTileSize(geometry.TileSize);
    _graphicsLayer->SetContentsTilePhase(geometry.Phase);
    _graphicsLayer->SetContentsRect(FloatRect(geometry.DestinationRect));
    _graphicsLayer->SetContentsClippingRect(FloatRoundedRect(FloatRect(geometry.DestinationRect)));
    _graphicsLayer->SetContentsToImage(
      backgroundLayer.GetImage().TryStyleImage()->GetCachedImage()->GetImage());

    didUpdateContentsRect = true;
  }

  void RenderLayerBacking::UpdateRootLayerConfiguration() noexcept
  {
    if (!_isFrameLayerWithTiledBacking)
    {
      return;
    }

    Color backgroundColor;
    bool viewIsTransparent = Compositor().ViewHasTransparentBackground(&backgroundColor);

    if (_backgroundLayerPaintsFixedRootBackground && _backgroundLayer)
    {
      if (_isMainFrameRenderViewLayer)
      {
        _backgroundLayer->SetBackgroundColor(backgroundColor);
        _backgroundLayer->SetContentsOpaque(!viewIsTransparent);
      }

      _graphicsLayer->SetBackgroundColor(Color());
      _graphicsLayer->SetContentsOpaque(false);
    }
    else if (_isMainFrameRenderViewLayer)
    {
      _graphicsLayer->SetBackgroundColor(backgroundColor);
      _graphicsLayer->SetContentsOpaque(!viewIsTransparent);
    }
  }

  void RenderLayerBacking::UpdatePaintingPhases() noexcept
  {
    // Phases for _maskLayer are set elsewhere.
    EnumSet<GraphicsLayerPaintingPhase> primaryLayerPhases = {GraphicsLayerPaintingPhase::Background,
                                                              GraphicsLayerPaintingPhase::Foreground};

    if (_foregroundLayer)
    {
      EnumSet<GraphicsLayerPaintingPhase> foregroundLayerPhases {GraphicsLayerPaintingPhase::Foreground};

      if (_scrolledContentsLayer)
      {
        foregroundLayerPhases.Add(GraphicsLayerPaintingPhase::OverflowContents);
      }

      _foregroundLayer->SetPaintingPhase(foregroundLayerPhases);
      primaryLayerPhases.Remove(GraphicsLayerPaintingPhase::Foreground);
    }

    if (_backgroundLayer)
    {
      _backgroundLayer->SetPaintingPhase(GraphicsLayerPaintingPhase::Background);
      primaryLayerPhases.Remove(GraphicsLayerPaintingPhase::Background);
    }

    if (_scrolledContentsLayer)
    {
      EnumSet<GraphicsLayerPaintingPhase> scrolledContentLayerPhases = {
        GraphicsLayerPaintingPhase::OverflowContents, GraphicsLayerPaintingPhase::CompositedScroll};
      if (!_foregroundLayer)
      {
        scrolledContentLayerPhases.Add(GraphicsLayerPaintingPhase::Foreground);
      }
      _scrolledContentsLayer->SetPaintingPhase(scrolledContentLayerPhases);

      primaryLayerPhases.Remove(GraphicsLayerPaintingPhase::Foreground);
      primaryLayerPhases.Add(GraphicsLayerPaintingPhase::CompositedScroll);
    }

    _graphicsLayer->SetPaintingPhase(primaryLayerPhases);
  }

  namespace
  {
    KRYS_NODISCARD bool
      SupportsDirectlyCompositedBoxDecorations(const RenderLayerModelObject &renderer) noexcept
    {
      const RenderStyle &style = renderer.Style();
      if (renderer.HasClip())
      {
        return false;
      }

      if (HasPaintedBoxDecorationsOrBackgroundImage(renderer))
      {
        return false;
      }

      // FIXME: We can't create a directly composited background if this layer will have children that
      // intersect with the background layer. A better solution might be to introduce a flattening layer if we
      // do direct box decoration composition. https://bugs.webkit.org/show_bug.cgi?id=119461
      if (HasPerspectiveOrPreserves3D(style))
      {
        return false;
      }

      return true;
    }
  }

  bool RenderLayerBacking::PaintsBoxDecorations() const noexcept
  {
    if (!_owningLayer.HasVisibleBoxDecorations())
    {
      return false;
    }

    return !SupportsDirectlyCompositedBoxDecorations(Renderer());
  }

  void RenderLayerBacking::DeterminePaintsContent(RenderLayer::PaintedContentRequest &request) const noexcept
  {
    _owningLayer.UpdateDescendantDependentFlags();
    bool shouldScanDescendants = _owningLayer.HasVisibleContent();

    if (shouldScanDescendants)
    {
      _owningLayer.DetermineNonLayerDescendantsPaintedContent(request);

      if (request.IsSatisfied())
      {
        return;
      }
    }

    DetermineNonCompositedLayerDescendantsPaintedContent(request);

    if (request.IsSatisfied())
    {
      return;
    }

    // FIXME: Layer-based SVG (RenderSVGModelObject), once there's SVG.

    if (request.HasPaintedContent == RequestState::Unknown)
    {
      request.HasPaintedContent = RequestState::False;
    }
  }

  // A "simple container layer" is a RenderLayer which has no visible content to render. It may have no
  // children, or all its children may be themselves composited. This is a useful optimization, because it
  // allows us to avoid allocating backing store.
  bool RenderLayerBacking::IsSimpleContainerCompositingLayer(PaintedContentsInfo &contentsInfo) const noexcept
  {
    if (_owningLayer.IsRenderViewLayer())
    {
      return false;
    }

    if (HasBackingSharingLayers())
    {
      return false;
    }

    if (auto *replaced = DynamicDowncast<RenderReplaced>(Renderer()))
    {
      if (replaced->PaintsContent())
      {
        return false;
      }
    }

    if (Renderer().IsRenderTextControl())
    {
      return false;
    }

    if (contentsInfo.PaintsBoxDecorations() || contentsInfo.PaintsContent())
    {
      return false;
    }

    if (Renderer().Style().GetBackgroundLayers().UsedFirst().GetClip() == FillBox::Text)
    {
      return false;
    }

    if (Renderer().IsDocumentElementRenderer() && _owningLayer.IsolatesCompositedBlending())
    {
      return false;
    }

    return true;
  }

  namespace
  {
    // Returning true stops the traversal.
    enum class LayerTraversal : bool
    {
      Continue,
      Stop
    };

    LayerTraversal TraverseVisibleNonCompositedDescendantLayers(
      RenderLayer &parent, const Func<LayerTraversal(const RenderLayer &)> &layerFunc) noexcept
    {
      // FIXME: We shouldn't be called with a stale z-order lists. See bug 85512.
      parent.UpdateLayerListsIfNeeded();

#if KRYS_DEBUG
      LayerListMutationDetector mutationChecker(parent);
#endif

      for (auto *childLayer : parent.NormalFlowLayers())
      {
        if (CompositedWithOwnBackingStore(*childLayer))
        {
          continue;
        }

        if (layerFunc(*childLayer) == LayerTraversal::Stop)
        {
          return LayerTraversal::Stop;
        }

        if (TraverseVisibleNonCompositedDescendantLayers(*childLayer, layerFunc) == LayerTraversal::Stop)
        {
          return LayerTraversal::Stop;
        }
      }

      if (parent.IsStackingContext() && !parent.HasVisibleDescendant())
      {
        return LayerTraversal::Continue;
      }

      // Use the _hasCompositingDescendant bit to optimize?
      for (auto *childLayer : parent.NegativeZOrderLayers())
      {
        if (CompositedWithOwnBackingStore(*childLayer))
        {
          continue;
        }

        if (layerFunc(*childLayer) == LayerTraversal::Stop)
        {
          return LayerTraversal::Stop;
        }

        if (TraverseVisibleNonCompositedDescendantLayers(*childLayer, layerFunc) == LayerTraversal::Stop)
        {
          return LayerTraversal::Stop;
        }
      }

      for (auto *childLayer : parent.PositiveZOrderLayers())
      {
        if (CompositedWithOwnBackingStore(*childLayer))
        {
          continue;
        }

        if (layerFunc(*childLayer) == LayerTraversal::Stop)
        {
          return LayerTraversal::Stop;
        }

        if (TraverseVisibleNonCompositedDescendantLayers(*childLayer, layerFunc) == LayerTraversal::Stop)
        {
          return LayerTraversal::Stop;
        }
      }

      return LayerTraversal::Continue;
    }

    KRYS_NODISCARD Maybe<bool> IntersectsWithAncestor(const RenderLayer &child, const RenderLayer &ancestor,
                                                      const LayoutRect &ancestorCompositedBounds) noexcept
    {
      // If any layers between child and ancestor are transformed, then adjusting the offset is insufficient
      // to convert coordinates into ancestor's coordinate space.
      if (!child.CanUseOffsetFromAncestor(ancestor))
      {
        return null;
      }

      auto overlap = child.BoundingBox(&ancestor, child.OffsetFromAncestor(&ancestor),
                                       {RenderLayer::UseFragmentBoxesExcludingCompositing});
      return overlap.Intersects(ancestorCompositedBounds);
    }
  }

  void RenderLayerBacking::DetermineNonCompositedLayerDescendantsPaintedContent(
    RenderLayer::PaintedContentRequest &request) const noexcept
  {
    bool hasPaintingDescendant = false;
    (void)TraverseVisibleNonCompositedDescendantLayers(
      _owningLayer,
      [&hasPaintingDescendant, &request, this](const RenderLayer &layer)
      {
        auto localRequest = RenderLayer::PaintedContentRequest {};
        if (layer.IsVisuallyNonEmpty(&localRequest))
        {
          bool mayIntersect = IntersectsWithAncestor(layer, _owningLayer, CompositedBounds()).value_or(true);
          if (mayIntersect)
          {
            hasPaintingDescendant = true;
            request.SetHasPaintedContent();
          }
        }
        return (hasPaintingDescendant && request.IsSatisfied()) ? LayerTraversal::Stop
                                                                : LayerTraversal::Continue;
      });
  }

  bool RenderLayerBacking::HasVisibleNonCompositedDescendants() const noexcept
  {
    bool hasVisibleDescendant = false;
    (void)TraverseVisibleNonCompositedDescendantLayers(_owningLayer,
                                                       [&hasVisibleDescendant](const RenderLayer &layer)
                                                       {
                                                         hasVisibleDescendant |= layer.HasVisibleContent();
                                                         return hasVisibleDescendant
                                                                  ? LayerTraversal::Stop
                                                                  : LayerTraversal::Continue;
                                                       });

    return hasVisibleDescendant;
  }

  bool RenderLayerBacking::ContainsPaintedContent(PaintedContentsInfo &contentsInfo) const noexcept
  {
    // FIXME: Reflections (isReflection), once there are reflections.
    if (contentsInfo.IsSimpleContainer() || PaintsIntoWindow() || PaintsIntoCompositedAncestor()
        || _artificiallyInflatedBounds)
    {
      return false;
    }

    if (contentsInfo.IsDirectlyCompositedImage())
    {
      return false;
    }

    if (auto styleable = style::Styleable::FromRenderer(Renderer()))
    {
      if (!styleable->MayHaveNonZeroOpacity())
      {
        return false;
      }
    }

    // FIXME: Video, once there's video.

    if (Is<RenderHTMLCanvas>(Renderer())
        && GetCanvasCompositingStrategy(Renderer()) == CanvasCompositingStrategy::CanvasAsLayerContents)
    {
      return _owningLayer.HasVisibleBoxDecorationsOrBackground();
    }

    return true;
  }

  // An image can be directly compositing if it's the sole content of the layer, and has no box decorations
  // that require painting. Direct compositing saves backing store.
  bool RenderLayerBacking::IsDirectlyCompositedImage() const noexcept
  {
    if (_owningLayer.HasVisibleBoxDecorationsOrBackground() || _owningLayer.ShouldPaintWithFilters()
        || Renderer().HasClip())
    {
      return false;
    }

    // Fixed layers that allow detaching won't have a backing store, so using a directly composited image
    // doesn't help (and has non-zero cost)
    if (_owningLayer.BehavesAsFixed() && _graphicsLayer->AllowsBackingStoreDetaching())
    {
      return false;
    }

    // FIXME: View transition captures, once there are view transitions.

    auto *imageRenderer = DynamicDowncast<RenderImage>(Renderer());
    if (!imageRenderer)
    {
      return false;
    }

    // FIXME: Media elements, once there's video.

    if (auto *cachedImage = imageRenderer->GetCachedImage())
    {
      if (!cachedImage->HasImage())
      {
        return false;
      }

      auto *image = DynamicDowncast<BitmapImage>(cachedImage->ImageForRenderer(imageRenderer));
      if (!image)
      {
        return false;
      }

      if (image->CurrentFrameOrientation() != ImageOrientation::Orientation::None)
      {
        return false;
      }

      return _graphicsLayer->ShouldDirectlyCompositeImage(image);
    }

    return false;
  }

  bool RenderLayerBacking::IsBitmapOnly() const noexcept
  {
    return _owningLayer.IsBitmapOnly();
  }

  bool RenderLayerBacking::IsUnscaledBitmapOnly() const noexcept
  {
    if (!Is<RenderImage>(Renderer()) && !Is<RenderHTMLCanvas>(Renderer()))
    {
      return false;
    }

    if (_owningLayer.HasVisibleBoxDecorationsOrBackground())
    {
      return false;
    }

    if (PageScaleFactor() < 1.0f)
    {
      return false;
    }

    auto contents = ContentsBox();
    if (contents.Location() != LayoutPoint(0, 0))
    {
      return false;
    }

    if (auto *imageRenderer = DynamicDowncast<RenderImage>(Renderer()))
    {
      if (auto *cachedImage = imageRenderer->GetCachedImage())
      {
        if (!cachedImage->HasImage())
        {
          return false;
        }

        auto *image = DynamicDowncast<BitmapImage>(cachedImage->ImageForRenderer(imageRenderer));
        if (!image)
        {
          return false;
        }

        if (image->CurrentFrameOrientation() != ImageOrientation::Orientation::None)
        {
          return false;
        }

        return contents.Size() == LayoutSize(image->Size());
      }
      return false;
    }

    if (Renderer().Style().GetImageRendering() == ImageRendering::CrispEdges
        || Renderer().Style().GetImageRendering() == ImageRendering::Pixelated)
    {
      return false;
    }

    auto &canvasRenderer = Downcast<RenderHTMLCanvas>(Renderer());
    return SnappedIntRect(contents).Size() == canvasRenderer.CanvasElement().Size();
  }

  void RenderLayerBacking::ContentChanged(ContentChangeType changeType,
                                          const Maybe<FloatRect> &dirtyRect) noexcept
  {
    PaintedContentsInfo contentsInfo(*this);
    if (changeType == ContentChangeType::Image || changeType == ContentChangeType::Canvas)
    {
      if (contentsInfo.IsDirectlyCompositedImage())
      {
        UpdateImageContents(contentsInfo);
        return;
      }
      if (contentsInfo.IsUnscaledBitmapOnly() != _graphicsLayer->AppliesDeviceScale())
      {
        Compositor().ScheduleCompositingLayerUpdate();
        return;
      }
    }

    if (changeType == ContentChangeType::Video)
    {
      Compositor().ScheduleCompositingLayerUpdate();
      return;
    }

    if (changeType == ContentChangeType::BackgroundImage
        && CanDirectlyCompositeBackgroundBackgroundImage(Renderer()))
    {
      _owningLayer.SetNeedsCompositingConfigurationUpdate();
    }

    if (changeType == ContentChangeType::MaskImage && _maskLayer)
    {
      _owningLayer.SetNeedsCompositingConfigurationUpdate();
    }

    if ((changeType == ContentChangeType::Canvas || changeType == ContentChangeType::CanvasPixels)
        && Renderer().IsRenderHTMLCanvas()
        && GetCanvasCompositingStrategy(Renderer()) == CanvasCompositingStrategy::CanvasAsLayerContents)
    {
      if (changeType == ContentChangeType::Canvas)
      {
        Compositor().ScheduleCompositingLayerUpdate();
      }

      if (dirtyRect)
      {
        _graphicsLayer->SetContentsNeedsDisplayInRect(*dirtyRect);
      }
      else
      {
        _graphicsLayer->SetContentsNeedsDisplay();
      }
      return;
    }
  }

  void RenderLayerBacking::UpdateImageContents(PaintedContentsInfo &contentsInfo) noexcept
  {
    // FIXME: View transition captures, once there are view transitions.
    auto &imageRenderer = Downcast<RenderImage>(Renderer());

    auto *cachedImage = imageRenderer.GetCachedImage();
    if (!cachedImage)
    {
      return;
    }

    auto *image = cachedImage->ImageForRenderer(&imageRenderer);
    if (!image)
    {
      return;
    }

    // We have to wait until the image is fully loaded before setting it on the layer.
    if (!cachedImage->IsLoaded())
    {
      return;
    }

    _graphicsLayer->SetContentsToImage(image);

    // Image animation is "lazy", in that it automatically stops unless someone is drawing the image. So we
    // have to kick the animation each time; this has the downside that the image will keep animating, even if
    // its layer is not visible.
    image->StartAnimation();

    UpdateContentsRects();
    UpdateDrawsContent(contentsInfo);
  }

  // Return the offset from the top-left of this compositing layer at which the renderer's contents are
  // painted.
  LayoutSize RenderLayerBacking::ContentOffsetInCompositingLayer() const noexcept
  {
    return LayoutSize(-_compositedBounds.X() + _compositedBoundsOffsetFromGraphicsLayer.Width(),
                      -_compositedBounds.Y() + _compositedBoundsOffsetFromGraphicsLayer.Height());
  }

  LayoutRect RenderLayerBacking::ContentsBox() const noexcept
  {
    auto *renderBox = DynamicDowncast<RenderBox>(Renderer());
    if (!renderBox)
    {
      return LayoutRect();
    }

    // FIXME: Video and view transition captures, once there are any.
    LayoutRect contentsRect;
    if (auto *renderReplaced = DynamicDowncast<RenderReplaced>(*renderBox))
    {
      contentsRect = renderReplaced->ReplacedContentRect();
    }
    else
    {
      contentsRect = renderBox->ContentBoxRect();
    }

    contentsRect.Move(ContentOffsetInCompositingLayer());
    return contentsRect;
  }

  namespace
  {
    KRYS_NODISCARD LayoutRect BackgroundRectForBox(const RenderBox &box) noexcept
    {
      switch (box.Style().GetBackgroundLayers().UsedFirst().GetClip())
      {
        case FillBox::BorderBox:  return box.BorderBoxRect();
        case FillBox::PaddingBox: return box.PaddingBoxRect();
        case FillBox::ContentBox: return box.ContentBoxRect();
        default:                  break;
      }

      krys_debug_assert(false);
      return {};
    }
  }

  FloatRect RenderLayerBacking::BackgroundBoxForSimpleContainerPainting() const noexcept
  {
    auto *box = DynamicDowncast<RenderBox>(Renderer());
    if (!box)
    {
      return FloatRect();
    }

    LayoutRect backgroundBox = BackgroundRectForBox(*box);
    backgroundBox.Move(ContentOffsetInCompositingLayer());
    return SnapRectToDevicePixels(backgroundBox, DeviceScaleFactor());
  }

  GraphicsLayer *RenderLayerBacking::ParentForSublayers() const noexcept
  {
    if (_scrolledContentsLayer)
    {
      return _scrolledContentsLayer.get();
    }

    return _childContainmentLayer ? _childContainmentLayer.get() : _graphicsLayer.get();
  }

  GraphicsLayer *RenderLayerBacking::ChildForSuperlayers() const noexcept
  {
    // FIXME: The document element captured in a view transition, once there are view transitions.
    return ChildForSuperlayersExcludingViewTransitions();
  }

  GraphicsLayer *RenderLayerBacking::ChildForSuperlayersExcludingViewTransitions() const noexcept
  {
    if (_transformFlatteningLayer)
    {
      return _transformFlatteningLayer.get();
    }

    if (_ancestorClippingStack)
    {
      return _ancestorClippingStack->FirstLayer();
    }

    if (auto *viewportConstrainedLayer = ViewportClippingOrAnchorLayer())
    {
      return viewportConstrainedLayer;
    }

    if (_contentsContainmentLayer)
    {
      return _contentsContainmentLayer.get();
    }

    return _graphicsLayer.get();
  }

  LayoutSize RenderLayerBacking::OffsetRelativeToRendererOriginForDescendantLayers() const noexcept
  {
    if (_scrolledContentsLayer)
    {
      return ToLayoutSize(ScrollContainerLayerBox(Downcast<RenderBox>(Renderer())).Location());
    }

    if (HasClippingLayer())
    {
      return ToLayoutSize(ClippingLayerBox(Renderer()).Location());
    }

    return {};
  }

  bool RenderLayerBacking::PaintsIntoWindow() const noexcept
  {
    // The view's layer always paints into its graphics layer: compositing is forced.
    return false;
  }

  void RenderLayerBacking::SetRequiresOwnBackingStore(bool requiresOwnBacking) noexcept
  {
    if (requiresOwnBacking == _requiresOwnBackingStore)
    {
      return;
    }

    _requiresOwnBackingStore = requiresOwnBacking;

    // This affects the answer to paintsIntoCompositedAncestor(), which in turn affects cached clip rects, so
    // we have to notify the layer.
    _owningLayer.CompositingStatusChanged(LayoutUpToDate::Yes);

    Compositor().RepaintInCompositedAncestor(_owningLayer, CompositedBounds());
  }

  void RenderLayerBacking::SetContentsNeedDisplay(GraphicsLayerShouldClipToLayer shouldClip) noexcept
  {
    krys_debug_assert(!PaintsIntoCompositedAncestor());

    // Use the repaint as a trigger to re-evaluate direct compositing (which is never used on the root layer).
    if (!_owningLayer.IsRenderViewLayer())
    {
      _owningLayer.SetNeedsCompositingConfigurationUpdate();
    }

    // FIXME: Event regions (invalidateEventRegion) and tracked repaints (addTrackedRepaintRect), once there
    // are any.

    if (_graphicsLayer && _graphicsLayer->DrawsContent())
    {
      // By default, setNeedsDisplay will clip to the size of the GraphicsLayer, which does not include margin
      // tiles.
      // FIXME: Tiled backings' margins, once there are tiled backings.
      FloatRect rectToRepaint = FloatRect(FloatPoint(0, 0), _graphicsLayer->Size());
      _graphicsLayer->SetNeedsDisplayInRect(rectToRepaint, shouldClip);
    }

    if (_foregroundLayer && _foregroundLayer->DrawsContent())
    {
      _foregroundLayer->SetNeedsDisplay();
    }

    if (_backgroundLayer && _backgroundLayer->DrawsContent())
    {
      _backgroundLayer->SetNeedsDisplay();
    }

    if (_maskLayer && _maskLayer->DrawsContent())
    {
      _maskLayer->SetNeedsDisplay();
    }

    if (_scrolledContentsLayer && _scrolledContentsLayer->DrawsContent())
    {
      _scrolledContentsLayer->SetNeedsDisplay();
    }
  }

  // r is in the coordinate space of the layer's render object
  void RenderLayerBacking::SetContentsNeedDisplayInRect(const LayoutRect &r,
                                                        GraphicsLayerShouldClipToLayer shouldClip) noexcept
  {
    krys_debug_assert(!PaintsIntoCompositedAncestor());

    // Use the repaint as a trigger to re-evaluate direct compositing (which is never used on the root layer).
    if (!_owningLayer.IsRenderViewLayer())
    {
      _owningLayer.SetNeedsCompositingConfigurationUpdate();
    }

    // FIXME: Event regions (invalidateEventRegion) and tracked repaints (addTrackedRepaintRect), once there
    // are any.

    FloatRect pixelSnappedRectForPainting = SnapRectToDevicePixelsIfNeeded(r, Renderer());

    if (_graphicsLayer && _graphicsLayer->DrawsContent())
    {
      FloatRect layerDirtyRect = pixelSnappedRectForPainting;
      layerDirtyRect.Move(-_graphicsLayer->OffsetFromRenderer() - FloatSize(_subpixelOffsetFromRenderer));
      _graphicsLayer->SetNeedsDisplayInRect(layerDirtyRect, shouldClip);
    }

    if (_foregroundLayer && _foregroundLayer->DrawsContent())
    {
      FloatRect layerDirtyRect = pixelSnappedRectForPainting;
      layerDirtyRect.Move(-_foregroundLayer->OffsetFromRenderer() - FloatSize(_subpixelOffsetFromRenderer));
      _foregroundLayer->SetNeedsDisplayInRect(layerDirtyRect, shouldClip);
    }

    // FIXME: need to split out repaints for the background.
    if (_backgroundLayer && _backgroundLayer->DrawsContent())
    {
      FloatRect layerDirtyRect = pixelSnappedRectForPainting;
      layerDirtyRect.Move(-_backgroundLayer->OffsetFromRenderer() - FloatSize(_subpixelOffsetFromRenderer));
      _backgroundLayer->SetNeedsDisplayInRect(layerDirtyRect, shouldClip);
    }

    if (_maskLayer && _maskLayer->DrawsContent())
    {
      FloatRect layerDirtyRect = pixelSnappedRectForPainting;
      layerDirtyRect.Move(-_maskLayer->OffsetFromRenderer() - FloatSize(_subpixelOffsetFromRenderer));
      _maskLayer->SetNeedsDisplayInRect(layerDirtyRect, shouldClip);
    }

    if (_scrolledContentsLayer && _scrolledContentsLayer->DrawsContent())
    {
      FloatRect layerDirtyRect = pixelSnappedRectForPainting;
      ScrollOffset scrollOffset;
      if (auto *scrollableArea = _owningLayer.GetScrollableArea())
      {
        scrollOffset = scrollableArea->GetScrollOffset();
      }
      layerDirtyRect.Move(-_scrolledContentsLayer->OffsetFromRenderer() + FloatSize(ToIntSize(scrollOffset))
                          - FloatSize(_subpixelOffsetFromRenderer));
      _scrolledContentsLayer->SetNeedsDisplayInRect(layerDirtyRect, shouldClip);
    }
  }

  void RenderLayerBacking::PaintIntoLayer(const GraphicsLayer *graphicsLayer, GraphicsContext &context,
                                          const IntRect &paintDirtyRect, // In the coords of rootLayer.
                                          EnumSet<PaintBehavior> paintBehavior) noexcept
  {
    if ((PaintsIntoWindow() || PaintsIntoCompositedAncestor())
        && graphicsLayer->PaintingPhase()
             != EnumSet<GraphicsLayerPaintingPhase>(GraphicsLayerPaintingPhase::ChildClippingMask))
    {
      krys_debug_assert(false);
      return;
    }
    auto paintFlags = PaintFlagsForLayer(*graphicsLayer);

    // FIXME: Event regions (CollectingEventRegion) and the layout-needed forbidden scope
    // (SetLayoutNeededForbiddenScope), once there are any.

    auto paintOneLayer = [&](RenderLayer &layer, EnumSet<RenderLayer::PaintLayerFlag> paintFlags)
    {
      RenderLayer::LayerPaintingInfo paintingInfo(&_owningLayer, LayoutRect(paintDirtyRect), paintBehavior,
                                                  -_subpixelOffsetFromRenderer);

      if (&layer == &_owningLayer)
      {
        layer.PaintLayerContents(context, paintingInfo, paintFlags);

        // FIXME: Dirty overlay scrollbars (containsDirtyOverlayScrollbars), once there are overlay
        // scrollbars.
      }
      else
      {
        layer.PaintLayerWithEffects(context, paintingInfo, paintFlags);
      }

      krys_debug_assert(!_owningLayer._usedTransparency);
    };

    paintOneLayer(_owningLayer, paintFlags);

    // FIXME: Need to check _foregroundLayer, masking etc. webkit.org/b/197565.
    GraphicsLayer *destinationForSharingLayers =
      _scrolledContentsLayer ? _scrolledContentsLayer.get() : _graphicsLayer.get();

    if (graphicsLayer == destinationForSharingLayers)
    {
      EnumSet<RenderLayer::PaintLayerFlag> sharingLayerPaintFlags = {
        RenderLayer::PaintLayerFlag::PaintingCompositingBackgroundPhase,
        RenderLayer::PaintLayerFlag::PaintingCompositingForegroundPhase};

      if (graphicsLayer->PaintingPhase().Contains(GraphicsLayerPaintingPhase::OverflowContents))
      {
        sharingLayerPaintFlags.Add(RenderLayer::PaintLayerFlag::PaintingOverflowContents);
      }

      for (auto &layer : _backingSharingLayers)
      {
        if (layer)
        {
          paintOneLayer(*layer, sharingLayerPaintFlags);
        }
      }
    }

    Compositor().DidPaintBacking(this);
  }

  EnumSet<RenderLayer::PaintLayerFlag>
    RenderLayerBacking::PaintFlagsForLayer(const GraphicsLayer &graphicsLayer) const noexcept
  {
    EnumSet<RenderLayer::PaintLayerFlag> paintFlags;

    auto paintingPhase = graphicsLayer.PaintingPhase();
    if (paintingPhase.Contains(GraphicsLayerPaintingPhase::Background))
    {
      paintFlags.Add(RenderLayer::PaintLayerFlag::PaintingCompositingBackgroundPhase);
    }
    if (paintingPhase.Contains(GraphicsLayerPaintingPhase::Foreground))
    {
      paintFlags.Add(RenderLayer::PaintLayerFlag::PaintingCompositingForegroundPhase);
    }
    if (paintingPhase.Contains(GraphicsLayerPaintingPhase::Mask))
    {
      paintFlags.Add(RenderLayer::PaintLayerFlag::PaintingCompositingMaskPhase);
    }
    if (paintingPhase.Contains(GraphicsLayerPaintingPhase::ClipPath))
    {
      paintFlags.Add(RenderLayer::PaintLayerFlag::PaintingCompositingClipPathPhase);
    }
    if (paintingPhase.Contains(GraphicsLayerPaintingPhase::ChildClippingMask))
    {
      paintFlags.Add(RenderLayer::PaintLayerFlag::PaintingChildClippingMaskPhase);
    }
    if (paintingPhase.Contains(GraphicsLayerPaintingPhase::OverflowContents))
    {
      paintFlags.Add({RenderLayer::PaintLayerFlag::PaintingOverflowContents,
                      RenderLayer::PaintLayerFlag::PaintingOverflowContentsRoot});
    }

    if (paintingPhase.Contains(GraphicsLayerPaintingPhase::CompositedScroll))
    {
      if (&graphicsLayer == _graphicsLayer.get())
      {
        paintFlags.Add(RenderLayer::PaintLayerFlag::PaintingOverflowContainer);
      }
    }

    if (&graphicsLayer == _backgroundLayer.get() && _backgroundLayerPaintsFixedRootBackground)
    {
      // Need PaintLayerFlag::PaintingCompositingForegroundPhase to walk child layers.
      paintFlags.Add({RenderLayer::PaintLayerFlag::PaintingRootBackgroundOnly,
                      RenderLayer::PaintLayerFlag::PaintingCompositingForegroundPhase});
    }
    else if (Compositor().FixedRootBackgroundLayer())
    {
      paintFlags.Add(RenderLayer::PaintLayerFlag::PaintingSkipRootBackground);
    }

    return paintFlags;
  }

  void RenderLayerBacking::PaintContents(const GraphicsLayer &graphicsLayer, GraphicsContext &context,
                                         const FloatRect &clip,
                                         EnumSet<GraphicsLayerPaintBehavior> layerPaintBehavior) noexcept
  {
    // The dirtyRect is in the coords of the painting root.
    FloatRect adjustedClipRect = clip;
    adjustedClipRect.Move(FloatSize(_subpixelOffsetFromRenderer));
    IntRect dirtyRect = EnclosingIntRect(adjustedClipRect);

    if (!layerPaintBehavior.Contains(GraphicsLayerPaintBehavior::ForceSynchronousImageDecode))
    {
      if (!graphicsLayer.RepaintCount())
      {
        layerPaintBehavior.Add(GraphicsLayerPaintBehavior::DefaultAsynchronousImageDecode);
      }
    }

    if (&graphicsLayer == _graphicsLayer.get() || &graphicsLayer == _foregroundLayer.get()
        || &graphicsLayer == _backgroundLayer.get() || &graphicsLayer == _maskLayer.get()
        || &graphicsLayer == _scrolledContentsLayer.get())
    {
      if (!graphicsLayer.PaintingPhase().Contains(GraphicsLayerPaintingPhase::OverflowContents))
      {
        dirtyRect.Intersect(EnclosingIntRect(FloatRect(CompositedBoundsIncludingMargin())));
      }

      // We have to use the same root as for hit testing, because both methods can compute and cache
      // clipRects.
      EnumSet<PaintBehavior> behavior;
      if (layerPaintBehavior.Contains(GraphicsLayerPaintBehavior::ForceSynchronousImageDecode))
      {
        behavior.Add(PaintBehavior::ForceSynchronousImageDecode);
      }
      else if (layerPaintBehavior.Contains(GraphicsLayerPaintBehavior::DefaultAsynchronousImageDecode))
      {
        behavior.Add(PaintBehavior::DefaultAsynchronousImageDecode);
      }

      PaintIntoLayer(&graphicsLayer, context, dirtyRect, behavior);

      // FIXME: Debug overlays for event regions (paintDebugOverlays), once there are event regions.
    }
    else if (&graphicsLayer == LayerForHorizontalScrollbar())
    {
      if (_owningLayer.HasVisibleContent())
      {
        auto *scrollableArea = _owningLayer.GetScrollableArea();
        krys_debug_assert(scrollableArea);

        PaintScrollbar(scrollableArea->HorizontalScrollbar(), context, dirtyRect);
      }
    }
    else if (&graphicsLayer == LayerForVerticalScrollbar())
    {
      if (_owningLayer.HasVisibleContent())
      {
        auto *scrollableArea = _owningLayer.GetScrollableArea();
        krys_debug_assert(scrollableArea);

        PaintScrollbar(scrollableArea->VerticalScrollbar(), context, dirtyRect);
      }
    }
    else if (&graphicsLayer == LayerForScrollCorner())
    {
      auto *scrollableArea = _owningLayer.GetScrollableArea();
      krys_debug_assert(scrollableArea);

      auto cornerRect = scrollableArea->OverflowControlsRects().ScrollCornerOrResizerRect();
      GraphicsContextStateSaver stateSaver(context);
      context.Translate(-FloatSize(static_cast<float>(cornerRect.X()), static_cast<float>(cornerRect.Y())));
      LayoutRect transformedClip = LayoutRect(clip);
      transformedClip.MoveBy(LayoutPoint(cornerRect.Location()));
      scrollableArea->PaintScrollCorner(context, IntPoint(), SnappedIntRect(transformedClip));
      scrollableArea->PaintResizer(context, LayoutPoint(), transformedClip);
    }
  }

  float RenderLayerBacking::PageScaleFactor() const noexcept
  {
    return Compositor().PageScaleFactor();
  }

  float RenderLayerBacking::ZoomedOutPageScaleFactor() const noexcept
  {
    return Compositor().ZoomedOutPageScaleFactor();
  }

  FloatSize RenderLayerBacking::EnclosingFrameViewVisibleSize() const noexcept
  {
    return Compositor().EnclosingFrameViewVisibleSize();
  }

  float RenderLayerBacking::DeviceScaleFactor() const noexcept
  {
    return Compositor().DeviceScaleFactor();
  }

  float RenderLayerBacking::ContentsScaleMultiplierForNewTiles(const GraphicsLayer *layer) const noexcept
  {
    return Compositor().ContentsScaleMultiplierForNewTiles(layer);
  }

  bool RenderLayerBacking::PaintsOpaquelyAtNonIntegralScales(const GraphicsLayer *) const noexcept
  {
    return _isMainFrameRenderViewLayer;
  }

  void RenderLayerBacking::DidChangePlatformLayerForLayer(const GraphicsLayer *layer) noexcept
  {
    Compositor().DidChangePlatformLayerForLayer(_owningLayer, layer);
  }

  bool RenderLayerBacking::GetCurrentTransform(const GraphicsLayer *graphicsLayer,
                                               TransformationMatrix &transform) const noexcept
  {
    auto *transformedLayer =
      _contentsContainmentLayer ? _contentsContainmentLayer.get() : _graphicsLayer.get();
    if (graphicsLayer != transformedLayer)
    {
      return false;
    }

    if (_owningLayer.IsTransformed())
    {
      transform = _owningLayer.CurrentTransform(style::TransformResolver::IndividualTransformOperations);
      return true;
    }
    return false;
  }

  bool RenderLayerBacking::IsFlushingLayers() const noexcept
  {
    return static_cast<GraphicsLayerClient &>(Compositor()).IsFlushingLayers();
  }

  bool RenderLayerBacking::IsTrackingRepaints() const noexcept
  {
    return static_cast<GraphicsLayerClient &>(Compositor()).IsTrackingRepaints();
  }

  bool RenderLayerBacking::ShouldSkipLayerInDump(const GraphicsLayer *layer,
                                                 EnumSet<LayerTreeAsTextOptions> options) const noexcept
  {
    if (options.Contains(LayerTreeAsTextOptions::Debug))
    {
      return false;
    }

    // Skip the root tile cache's flattening layer.
    return _isMainFrameRenderViewLayer && layer && layer == _childContainmentLayer.get();
  }

  bool RenderLayerBacking::ShouldDumpPropertyForLayer(const GraphicsLayer *layer,
                                                      utf8_stringview propertyName,
                                                      EnumSet<LayerTreeAsTextOptions> options) const noexcept
  {
    // For backwards compatibility, skip some properties on the root tile cache.
    if (_isMainFrameRenderViewLayer && layer == _graphicsLayer.get()
        && !options.Contains(LayerTreeAsTextOptions::IncludeRootLayerProperties))
    {
      if (propertyName == u8"drawsContent")
      {
        return false;
      }

      // Background color could be of interest to tests or other dumpers if it's non-white.
      if (propertyName == u8"backgroundColor" && Color::IsWhiteColor(layer->BackgroundColor()))
      {
        return false;
      }

      // The root tile cache's repaints will show up at the top with FrameView's, so don't dump them twice.
      if (propertyName == u8"repaintRects")
      {
        return false;
      }
    }

    if (_owningLayer.IsRenderViewLayer()
        && (layer == _graphicsLayer.get() || layer == _contentsContainmentLayer.get()))
    {
      if (propertyName == u8"anchorPoint")
      {
        return layer->AnchorPoint() != FloatPoint3D {};
      }

      return true;
    }

    if (propertyName == u8"anchorPoint")
    {
      return layer->AnchorPoint() != FloatPoint3D(0.5f, 0.5f, 0);
    }

    return true;
  }

  bool RenderLayerBacking::StartAnimation(double timeOffset, const GraphicsLayerAnimation &animation,
                                          const BlendingKeyframes &keyframes) noexcept
  {
    // FIXME: Renderers captured in a view transition, once there are view transitions.

    // FIXME: SVG renderers that are layer aware, once there's SVG.
    bool shouldApplyAnimationsToTargetRenderer = Renderer().IsRenderBox();

    bool hasOpacity = keyframes.ContainsProperty(css::PropertyId::Opacity);
    bool hasRotate =
      shouldApplyAnimationsToTargetRenderer && keyframes.ContainsProperty(css::PropertyId::Rotate);
    bool hasScale =
      shouldApplyAnimationsToTargetRenderer && keyframes.ContainsProperty(css::PropertyId::Scale);
    bool hasTranslate =
      shouldApplyAnimationsToTargetRenderer && keyframes.ContainsProperty(css::PropertyId::Translate);
    bool hasTransform =
      shouldApplyAnimationsToTargetRenderer && keyframes.ContainsProperty(css::PropertyId::Transform);
    bool hasFilter = keyframes.ContainsProperty(css::PropertyId::Filter);

    bool hasBackdropFilter = keyframes.ContainsProperty(css::PropertyId::BackdropFilter);

    if (!hasOpacity && !hasRotate && !hasScale && !hasTranslate && !hasTransform && !hasFilter
        && !hasBackdropFilter)
    {
      return false;
    }

    auto referenceBoxRect = Renderer().TransformReferenceBoxRect(Renderer().Style());
    referenceBoxRect = FloatRect(SnappedIntRect(LayoutRect(referenceBoxRect)));

    GraphicsLayerKeyframeValueList rotateVector(AnimatedProperty::Rotate);
    GraphicsLayerKeyframeValueList scaleVector(AnimatedProperty::Scale);
    GraphicsLayerKeyframeValueList translateVector(AnimatedProperty::Translate);
    GraphicsLayerKeyframeValueList transformVector(AnimatedProperty::Transform);
    GraphicsLayerKeyframeValueList opacityVector(AnimatedProperty::Opacity);
    GraphicsLayerKeyframeValueList filterVector(AnimatedProperty::Filter);
    GraphicsLayerKeyframeValueList backdropFilterVector(AnimatedProperty::WebkitBackdropFilter);

    for (const auto &currentKeyframe : keyframes)
    {
      const RenderStyle *keyframeStyle = currentKeyframe.Style();
      double offset = currentKeyframe.Offset();

      if (!keyframeStyle)
      {
        continue;
      }

      auto *tf = currentKeyframe.GetTimingFunction();
      auto size = referenceBoxRect.Size();

      if (currentKeyframe.AnimatesProperty(css::PropertyId::Rotate))
      {
        rotateVector.Insert(CreateUnique<GraphicsLayerTransformAnimationValue>(
          offset, style::ToPlatform<style::Rotate> {}(keyframeStyle->GetRotate(), size).get(), tf));
      }

      if (currentKeyframe.AnimatesProperty(css::PropertyId::Scale))
      {
        scaleVector.Insert(CreateUnique<GraphicsLayerTransformAnimationValue>(
          offset, style::ToPlatform<style::Scale> {}(keyframeStyle->GetScale(), size).get(), tf));
      }

      if (currentKeyframe.AnimatesProperty(css::PropertyId::Translate))
      {
        translateVector.Insert(CreateUnique<GraphicsLayerTransformAnimationValue>(
          offset, style::ToPlatform<style::Translate> {}(keyframeStyle->GetTranslate(), size).get(), tf));
      }

      if (currentKeyframe.AnimatesProperty(css::PropertyId::Transform))
      {
        transformVector.Insert(CreateUnique<GraphicsLayerTransformAnimationValue>(
          offset, style::ToPlatform<style::Transform> {}(keyframeStyle->GetTransform(), size), tf));
      }

      if (currentKeyframe.AnimatesProperty(css::PropertyId::Opacity))
      {
        opacityVector.Insert(CreateUnique<GraphicsLayerFloatAnimationValue>(
          offset, keyframeStyle->GetOpacity().Value.Value, tf));
      }

      if (currentKeyframe.AnimatesProperty(css::PropertyId::Filter))
      {
        filterVector.Insert(CreateUnique<GraphicsLayerFilterAnimationValue>(
          offset, style::ToPlatform<style::Filter> {}(keyframeStyle->GetFilter()), tf));
      }

      if (currentKeyframe.AnimatesProperty(css::PropertyId::BackdropFilter))
      {
        backdropFilterVector.Insert(CreateUnique<GraphicsLayerFilterAnimationValue>(
          offset, style::ToPlatform<style::Filter> {}(keyframeStyle->BackdropFilter()), tf));
      }
    }

    bool didAnimate = false;
    const auto &name = keyframes.AcceleratedAnimationName();

    if (hasRotate && _graphicsLayer->AddAnimation(rotateVector, &animation, name, timeOffset))
    {
      didAnimate = true;
    }

    if (hasScale && _graphicsLayer->AddAnimation(scaleVector, &animation, name, timeOffset))
    {
      didAnimate = true;
    }

    if (hasTranslate && _graphicsLayer->AddAnimation(translateVector, &animation, name, timeOffset))
    {
      didAnimate = true;
    }

    if (hasTransform && _graphicsLayer->AddAnimation(transformVector, &animation, name, timeOffset))
    {
      didAnimate = true;
    }

    if (hasOpacity && _graphicsLayer->AddAnimation(opacityVector, &animation, name, timeOffset))
    {
      didAnimate = true;
    }

    if (hasFilter && _graphicsLayer->AddAnimation(filterVector, &animation, name, timeOffset))
    {
      didAnimate = true;
    }

    if (hasBackdropFilter && _graphicsLayer->AddAnimation(backdropFilterVector, &animation, name, timeOffset))
    {
      didAnimate = true;
    }

    if (didAnimate)
    {
      _owningLayer.SetNeedsPostLayoutCompositingUpdate();
      _owningLayer.SetNeedsCompositingGeometryUpdate();
    }

    return didAnimate;
  }

  void RenderLayerBacking::AnimationPaused(double timeOffset, const utf8_string &animationName) noexcept
  {
    _graphicsLayer->PauseAnimation(animationName, timeOffset);
  }

  void RenderLayerBacking::AnimationFinished(const utf8_string &animationName) noexcept
  {
    _graphicsLayer->RemoveAnimation(animationName, null);
    _owningLayer.SetNeedsPostLayoutCompositingUpdate();
    _owningLayer.SetNeedsCompositingGeometryUpdate();
  }

  void RenderLayerBacking::TransformRelatedPropertyDidChange() noexcept
  {
    _graphicsLayer->TransformRelatedPropertyDidChange();
  }

  void RenderLayerBacking::NotifyAnimationStarted(const GraphicsLayer *, const utf8_string &,
                                                  MonotonicTime::duration) noexcept
  {
  }

  // This is used for the 'freeze' API, for testing only.
  void RenderLayerBacking::SuspendAnimations(MonotonicTime::duration time) noexcept
  {
    _graphicsLayer->SuspendAnimations(time);
  }

  void RenderLayerBacking::ResumeAnimations() noexcept
  {
    _graphicsLayer->ResumeAnimations();
  }

  void RenderLayerBacking::NotifyFlushRequired(const GraphicsLayer *layer) noexcept
  {
    if (Renderer().RenderTreeBeingDestroyed())
    {
      return;
    }

    Compositor().NotifyFlushRequired(layer);
  }

  void RenderLayerBacking::NotifySubsequentFlushRequired(const GraphicsLayer *layer) noexcept
  {
    Compositor().NotifySubsequentFlushRequired(layer);
  }

  LayoutRect RenderLayerBacking::CompositedBounds() const noexcept
  {
    return _compositedBounds;
  }

  bool RenderLayerBacking::SetCompositedBounds(const LayoutRect &bounds) noexcept
  {
    if (bounds == _compositedBounds)
    {
      return false;
    }

    _compositedBounds = bounds;
    return true;
  }

  LayoutRect RenderLayerBacking::CompositedBoundsIncludingMargin() const noexcept
  {
    // FIXME: Tiled backings' margins, once there are tiled backings.
    return CompositedBounds();
  }

  CompositingLayerType RenderLayerBacking::GetCompositingLayerType() const noexcept
  {
    if (_graphicsLayer->UsesContentsLayer())
    {
      return MediaCompositingLayer;
    }

    if (_graphicsLayer->DrawsContent())
    {
      return NormalCompositingLayer;
    }

    return ContainerCompositingLayer;
  }

  TextStream &operator<<(TextStream &ts, const RenderLayerBacking &backing) noexcept
  {
    ts << u8"RenderLayerBacking " << static_cast<const void *>(&backing) << u8" bounds "
       << backing.CompositedBounds();

    if (backing.IsFrameLayerWithTiledBacking())
    {
      ts << u8" frame layer tiled backing";
    }
    if (backing.PaintsIntoWindow())
    {
      ts << u8" paintsIntoWindow";
    }
    if (backing.PaintsIntoCompositedAncestor())
    {
      ts << u8" paintsIntoCompositedAncestor";
    }

    if (backing.AncestorClippingStack())
    {
      ts << u8" ancestor clip stack " << *backing.AncestorClippingStack();
    }

    return ts;
  }
}
