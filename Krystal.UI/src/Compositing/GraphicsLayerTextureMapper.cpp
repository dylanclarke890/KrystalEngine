#include "Krystal.UI/Compositing/GraphicsLayerTextureMapper.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"

namespace krys::ui
{
  Ref<boo::GraphicsLayer>
    GraphicsLayerFactoryTextureMapper::CreateGraphicsLayer(boo::GraphicsLayerType type,
                                                           boo::GraphicsLayerClient &client) noexcept
  {
    return AdoptRef(*new GraphicsLayerTextureMapper(type, client));
  }

  GraphicsLayerTextureMapper::GraphicsLayerTextureMapper(Type layerType,
                                                         boo::GraphicsLayerClient &client) noexcept
      : GraphicsLayer(layerType, client)
  {
  }

  void GraphicsLayerTextureMapper::NotifyChange(ChangeMask changeMask) noexcept
  {
    bool flushRequired = _changeMask == NoChanges;
    _changeMask |= changeMask;

    if (flushRequired)
    {
      GraphicsLayer::Client().NotifyFlushRequired(this);
    }
  }

  GraphicsLayerTextureMapper::~GraphicsLayerTextureMapper() noexcept
  {
    if (_contentsLayer)
    {
      _contentsLayer->SetClient(nullptr);
    }

    WillBeDestroyed();
  }

  void GraphicsLayerTextureMapper::SetNeedsDisplay() noexcept
  {
    if (!DrawsContent())
    {
      return;
    }

    // The current size might change, thus we need to update the whole display.
    _needsDisplay = true;
    NotifyChange(DisplayChange);
    AddRepaintRect(boo::FloatRect(boo::FloatPoint(), _size));
  }

  void GraphicsLayerTextureMapper::SetContentsNeedsDisplay() noexcept
  {
    NotifyChange(DisplayChange);
    AddRepaintRect(ContentsRect());
  }

  void GraphicsLayerTextureMapper::SetNeedsDisplayInRect(const boo::FloatRect &rect,
                                                         ShouldClipToLayer) noexcept
  {
    if (!DrawsContent())
    {
      return;
    }

    if (_needsDisplay)
    {
      return;
    }
    _needsDisplayRect.Unite(rect);
    NotifyChange(DisplayChange);
    AddRepaintRect(rect);
  }

  bool GraphicsLayerTextureMapper::SetChildren(List<Ref<GraphicsLayer>> &&children) noexcept
  {
    if (GraphicsLayer::SetChildren(krys::move(children)))
    {
      NotifyChange(ChildrenChange);
      return true;
    }
    return false;
  }

  void GraphicsLayerTextureMapper::AddChild(Ref<GraphicsLayer> &&layer) noexcept
  {
    NotifyChange(ChildrenChange);
    GraphicsLayer::AddChild(krys::move(layer));
  }

  void GraphicsLayerTextureMapper::AddChildAtIndex(Ref<GraphicsLayer> &&layer, int32 index) noexcept
  {
    GraphicsLayer::AddChildAtIndex(krys::move(layer), index);
    NotifyChange(ChildrenChange);
  }

  void GraphicsLayerTextureMapper::AddChildAbove(Ref<GraphicsLayer> &&layer, GraphicsLayer *sibling) noexcept
  {
    GraphicsLayer::AddChildAbove(krys::move(layer), sibling);
    NotifyChange(ChildrenChange);
  }

  void GraphicsLayerTextureMapper::AddChildBelow(Ref<GraphicsLayer> &&layer, GraphicsLayer *sibling) noexcept
  {
    GraphicsLayer::AddChildBelow(krys::move(layer), sibling);
    NotifyChange(ChildrenChange);
  }

  bool GraphicsLayerTextureMapper::ReplaceChild(GraphicsLayer *oldChild,
                                                Ref<GraphicsLayer> &&newChild) noexcept
  {
    if (GraphicsLayer::ReplaceChild(oldChild, krys::move(newChild)))
    {
      NotifyChange(ChildrenChange);
      return true;
    }
    return false;
  }

  void GraphicsLayerTextureMapper::SetMaskLayer(RefPtr<GraphicsLayer> &&value) noexcept
  {
    if (value == MaskLayer())
    {
      return;
    }

    GraphicsLayer *rawLayer = value.get();
    GraphicsLayer::SetMaskLayer(krys::move(value));
    NotifyChange(MaskLayerChange);

    if (!rawLayer)
    {
      return;
    }
    rawLayer->SetSize(Size());
    rawLayer->SetContentsVisible(ContentsAreVisible());
  }

  void GraphicsLayerTextureMapper::SetReplicatedByLayer(RefPtr<GraphicsLayer> &&value) noexcept
  {
    if (value == ReplicaLayer())
    {
      return;
    }
    GraphicsLayer::SetReplicatedByLayer(krys::move(value));
    NotifyChange(ReplicaLayerChange);
  }

  void GraphicsLayerTextureMapper::SetPosition(const boo::FloatPoint &value) noexcept
  {
    if (value == Position())
    {
      return;
    }
    GraphicsLayer::SetPosition(value);
    NotifyChange(PositionChange);
  }

  void GraphicsLayerTextureMapper::SetAnchorPoint(const boo::FloatPoint3D &value) noexcept
  {
    if (value == AnchorPoint())
    {
      return;
    }
    GraphicsLayer::SetAnchorPoint(value);
    NotifyChange(AnchorPointChange);
  }

  void GraphicsLayerTextureMapper::SetSize(const boo::FloatSize &value) noexcept
  {
    if (value == Size())
    {
      return;
    }

    GraphicsLayer::SetSize(value);
    if (MaskLayer())
    {
      MaskLayer()->SetSize(value);
    }
    NotifyChange(SizeChange);
  }

  void GraphicsLayerTextureMapper::SetTransform(const boo::TransformationMatrix &value) noexcept
  {
    if (value == Transform())
    {
      return;
    }

    GraphicsLayer::SetTransform(value);
    NotifyChange(TransformChange);
  }

  void GraphicsLayerTextureMapper::SetChildrenTransform(const boo::TransformationMatrix &value) noexcept
  {
    if (value == ChildrenTransform())
    {
      return;
    }
    GraphicsLayer::SetChildrenTransform(value);
    NotifyChange(ChildrenTransformChange);
  }

  void GraphicsLayerTextureMapper::SetPreserves3D(bool value) noexcept
  {
    if (value == Preserves3D())
    {
      return;
    }
    GraphicsLayer::SetPreserves3D(value);
    NotifyChange(Preserves3DChange);
  }

  void GraphicsLayerTextureMapper::SetMasksToBounds(bool value) noexcept
  {
    if (value == MasksToBounds())
    {
      return;
    }
    GraphicsLayer::SetMasksToBounds(value);
    NotifyChange(MasksToBoundsChange);
  }

  void GraphicsLayerTextureMapper::SetDrawsContent(bool value) noexcept
  {
    if (value == DrawsContent())
    {
      return;
    }
    GraphicsLayer::SetDrawsContent(value);
    NotifyChange(DrawsContentChange);

    if (value)
    {
      SetNeedsDisplay();
    }
  }

  void GraphicsLayerTextureMapper::SetContentsVisible(bool value) noexcept
  {
    if (value == ContentsAreVisible())
    {
      return;
    }
    NotifyChange(ContentsVisibleChange);
    GraphicsLayer::SetContentsVisible(value);
    if (MaskLayer())
    {
      MaskLayer()->SetContentsVisible(value);
    }
  }

  void GraphicsLayerTextureMapper::SetContentsOpaque(bool value) noexcept
  {
    if (value == ContentsOpaque())
    {
      return;
    }
    NotifyChange(ContentsOpaqueChange);
    GraphicsLayer::SetContentsOpaque(value);
  }

  void GraphicsLayerTextureMapper::SetBackfaceVisibility(bool value) noexcept
  {
    if (value == BackfaceVisibility())
    {
      return;
    }
    GraphicsLayer::SetBackfaceVisibility(value);
    NotifyChange(BackfaceVisibilityChange);
  }

  void GraphicsLayerTextureMapper::SetBackgroundColor(const boo::Color &value) noexcept
  {
    if (value == BackgroundColor())
    {
      return;
    }
    GraphicsLayer::SetBackgroundColor(value);
    NotifyChange(BackgroundColorChange);
  }

  void GraphicsLayerTextureMapper::SetOpacity(float value) noexcept
  {
    if (value == Opacity())
    {
      return;
    }
    GraphicsLayer::SetOpacity(value);
    NotifyChange(OpacityChange);
  }

  void GraphicsLayerTextureMapper::SetContentsRect(const boo::FloatRect &value) noexcept
  {
    if (value == ContentsRect())
    {
      return;
    }
    GraphicsLayer::SetContentsRect(value);
    NotifyChange(ContentsRectChange);
  }

  void GraphicsLayerTextureMapper::SetContentsClippingRect(const boo::FloatRoundedRect &rect) noexcept
  {
    if (rect == _contentsClippingRect)
    {
      return;
    }

    GraphicsLayer::SetContentsClippingRect(rect);
    NotifyChange(ContentsRectChange);
  }

  void GraphicsLayerTextureMapper::SetContentsRectClipsDescendants(bool contentsRectClipsDescendants) noexcept
  {
    if (contentsRectClipsDescendants == _contentsRectClipsDescendants)
    {
      return;
    }

    GraphicsLayer::SetContentsRectClipsDescendants(contentsRectClipsDescendants);
    NotifyChange(ContentsRectChange);
  }

  void GraphicsLayerTextureMapper::SetContentsToSolidColor(const boo::Color &color) noexcept
  {
    if (color == _solidColor)
    {
      return;
    }

    _solidColor = color;
    NotifyChange(SolidColorChange);
  }

  void GraphicsLayerTextureMapper::SetContentsToImage(boo::Image *image) noexcept
  {
    if (image)
    {
      // Make the decision about whether the image has changed. This code makes the assumption that pointer
      // equality on an image is a valid way to tell if the image is changed.
      // FIXME: The image's current frame (currentNativeImage), once animated images are composited.
      if (_compositedNativeImage.get() == image)
      {
        return;
      }

      _compositedNativeImage = ShareRefPtr(image);
      if (!_compositedImage)
      {
        _compositedImage = TextureMapperTiledBackingStore::Create();
      }
      _compositedImage->SetContentsToImage(image);
      _compositedImage->UpdateContentsScale(PageScaleFactor() * DeviceScaleFactor());
    }
    else
    {
      _compositedNativeImage = nullptr;
      _compositedImage = nullptr;
    }

    SetContentsToPlatformLayer(_compositedImage.get(), ContentsLayerPurpose::Image);
    NotifyChange(ContentChange);
    GraphicsLayer::SetContentsToImage(image);
  }

  void GraphicsLayerTextureMapper::SetContentsToPlatformLayer(TextureMapperPlatformLayer *platformLayer,
                                                              ContentsLayerPurpose) noexcept
  {
    if (platformLayer == _contentsLayer)
    {
      return;
    }

    NotifyChange(ContentChange);

    if (_contentsLayer)
    {
      _contentsLayer->SetClient(nullptr);
    }

    _contentsLayer = platformLayer;

    if (_contentsLayer)
    {
      _contentsLayer->SetClient(this);
    }
  }

  void GraphicsLayerTextureMapper::SetShowDebugBorder(bool show) noexcept
  {
    if (IsShowingDebugBorder() == show)
    {
      return;
    }

    GraphicsLayer::SetShowDebugBorder(show);
    NotifyChange(DebugVisualsChange);
  }

  void GraphicsLayerTextureMapper::SetShowRepaintCounter(bool show) noexcept
  {
    if (IsShowingRepaintCounter() == show)
    {
      return;
    }

    GraphicsLayer::SetShowRepaintCounter(show);
    NotifyChange(RepaintCountChange);
  }

  void GraphicsLayerTextureMapper::FlushCompositingStateForThisLayerOnly() noexcept
  {
    PrepareBackingStoreIfNeeded();
    CommitLayerChanges();
    _layer->SyncAnimations(MonotonicTime::Now());
  }

  void GraphicsLayerTextureMapper::PrepareBackingStoreIfNeeded() noexcept
  {
    if (ShouldHaveBackingStore())
    {
      if (!_backingStore)
      {
        _backingStore = TextureMapperTiledBackingStore::Create();
        _changeMask |= BackingStoreChange;
      }
    }
    else
    {
      if (_backingStore)
      {
        _backingStore = nullptr;
        _changeMask |= BackingStoreChange;
      }
    }

    UpdateDebugBorderAndRepaintCount();
  }

  void GraphicsLayerTextureMapper::UpdateDebugBorderAndRepaintCount() noexcept
  {
    if (IsShowingDebugBorder())
    {
      UpdateDebugIndicators();
    }

    // When this has its own backing store, update the repaint count before calling
    // TextureMapperLayer::flushCompositingStateForThisLayerOnly().
    bool needsToRepaint = ShouldHaveBackingStore() && (_needsDisplay || !_needsDisplayRect.IsEmpty());
    if (IsShowingRepaintCounter() && needsToRepaint)
    {
      IncrementRepaintCount();
      _changeMask |= RepaintCountChange;
    }
  }

  void GraphicsLayerTextureMapper::SetDebugBorder(const boo::Color &color, float width) noexcept
  {
    _debugBorderColor = color;
    _debugBorderWidth = width;
    _changeMask |= DebugVisualsChange;
  }

  void GraphicsLayerTextureMapper::CommitLayerChanges() noexcept
  {
    if (_changeMask == NoChanges)
    {
      return;
    }

    if (_changeMask & ChildrenChange)
    {
      List<TextureMapperLayer *> rawChildren;
      rawChildren.reserve(Children().size());
      for (auto &child : Children())
      {
        rawChildren.push_back(&Downcast<GraphicsLayerTextureMapper>(*child).Layer());
      }
      _layer->SetChildren(rawChildren);
    }

    if (_changeMask & MaskLayerChange)
    {
      auto *layer = DynamicDowncast<GraphicsLayerTextureMapper>(MaskLayer());
      _layer->SetMaskLayer(layer ? &layer->Layer() : nullptr);
    }

    if (_changeMask & ReplicaLayerChange)
    {
      auto *layer = DynamicDowncast<GraphicsLayerTextureMapper>(ReplicaLayer());
      _layer->SetReplicaLayer(layer ? &layer->Layer() : nullptr);
    }

    // FIXME: Backdrop layers (BackdropLayerChange), with backdrop-filter.

    if (_changeMask & PositionChange)
    {
      _layer->SetPosition(Position());
    }

    if (_changeMask & AnchorPointChange)
    {
      _layer->SetAnchorPoint(AnchorPoint());
    }

    if (_changeMask & SizeChange)
    {
      _layer->SetSize(Size());
    }

    if (_changeMask & TransformChange)
    {
      _layer->SetTransform(Transform());
    }

    if (_changeMask & ChildrenTransformChange)
    {
      _layer->SetChildrenTransform(ChildrenTransform());
    }

    if (_changeMask & Preserves3DChange)
    {
      _layer->SetPreserves3D(Preserves3D());
    }

    if (_changeMask & ContentsRectChange)
    {
      _layer->SetContentsRect(ContentsRect());
      _layer->SetContentsClippingRect(ContentsClippingRect());
      _layer->SetContentsRectClipsDescendants(ContentsRectClipsDescendants());
    }

    if (_changeMask & MasksToBoundsChange)
    {
      _layer->SetMasksToBounds(MasksToBounds());
    }

    if (_changeMask & DrawsContentChange)
    {
      _layer->SetDrawsContent(DrawsContent());
    }

    if (_changeMask & ContentsVisibleChange)
    {
      _layer->SetContentsVisible(ContentsAreVisible());
    }

    if (_changeMask & ContentsOpaqueChange)
    {
      _layer->SetContentsOpaque(ContentsOpaque());
    }

    if (_changeMask & BackfaceVisibilityChange)
    {
      _layer->SetBackfaceVisibility(BackfaceVisibility());
    }

    if (_changeMask & BackgroundColorChange)
    {
      _layer->SetBackgroundColor(BackgroundColor());
    }

    if (_changeMask & OpacityChange)
    {
      _layer->SetOpacity(Opacity());
    }

    if (_changeMask & SolidColorChange)
    {
      _layer->SetSolidColor(_solidColor);
    }

    if (_changeMask & FilterChange)
    {
      _layer->SetFilters(Filters());
    }

    if (_changeMask & BackingStoreChange)
    {
      _layer->SetBackingStore(_backingStore.get());
    }

    if (_changeMask & DebugVisualsChange)
    {
      _layer->SetShowDebugBorder(IsShowingDebugBorder());
      _layer->SetDebugBorderColor(DebugBorderColor());
      _layer->SetDebugBorderWidth(DebugBorderWidth());
    }

    if (_changeMask & RepaintCountChange)
    {
      _layer->SetShowRepaintCounter(IsShowingRepaintCounter());
      _layer->SetRepaintCount(RepaintCount());
    }

    if (_changeMask & ContentChange)
    {
      _layer->SetContentsLayer(_contentsLayer);
    }

    _changeMask = NoChanges;
  }

  void GraphicsLayerTextureMapper::FlushCompositingState(const boo::FloatRect &rect) noexcept
  {
    FlushCompositingStateForThisLayerOnly();
    UpdateVisibleRects(rect);

    if (MaskLayer())
    {
      MaskLayer()->FlushCompositingState(rect);
    }
    if (ReplicaLayer())
    {
      ReplicaLayer()->FlushCompositingState(rect);
    }
    for (auto &child : Children())
    {
      child->FlushCompositingState(rect);
    }
  }

  void GraphicsLayerTextureMapper::UpdateVisibleRects(const boo::FloatRect &rect) noexcept
  {
    _layerTransform.SetLocalTransform(Transform());
    _layerTransform.SetAnchorPoint(AnchorPoint());
    _layerTransform.SetPosition(boo::FloatPoint(Position() - BoundsOrigin()));
    _layerTransform.SetSize(Size());
    _layerTransform.SetFlattening(!Preserves3D());
    _layerTransform.SetChildrenTransform(ChildrenTransform());
    auto *parent = DynamicDowncast<GraphicsLayerTextureMapper>(Parent());
    _layerTransform.CombineTransforms(parent ? parent->_layerTransform.CombinedForChildren()
                                             : boo::TransformationMatrix());

    // Non-invertible layers are not visible.
    auto inverse = _layerTransform.Combined().Inverse();
    if (!inverse)
    {
      _visibleRect = {};
      return;
    }

    // A projection of the rect (surface coordinates) onto the layer's plane (layer coordinates). The
    // resulting quad might be skewed and the result is the bounding box of this quad, so it might spread
    // further than the real visible area.
    boo::FloatRect visibleRect(inverse->ClampedBoundsOfProjectedQuad(boo::FloatQuad(rect)));
    if (visibleRect.Width() >= boo::LayoutUnit::NearlyMax().ToFloat()
        || visibleRect.Width() <= boo::LayoutUnit::NearlyMin().ToFloat())
    {
      visibleRect.SetX(0);
      visibleRect.SetWidth(_size.Width());
    }
    if (visibleRect.Height() >= boo::LayoutUnit::NearlyMax().ToFloat()
        || visibleRect.Height() <= boo::LayoutUnit::NearlyMin().ToFloat())
    {
      visibleRect.SetY(0);
      visibleRect.SetHeight(_size.Height());
    }
    _visibleRect = boo::EnclosingIntRect(visibleRect);
  }

  void GraphicsLayerTextureMapper::UpdateBackingStoreIncludingSubLayers(TextureMapper &textureMapper) noexcept
  {
    UpdateBackingStoreIfNeeded(textureMapper);

    if (MaskLayer())
    {
      Downcast<GraphicsLayerTextureMapper>(*MaskLayer()).UpdateBackingStoreIfNeeded(textureMapper);
    }
    if (ReplicaLayer())
    {
      Downcast<GraphicsLayerTextureMapper>(*ReplicaLayer())
        .UpdateBackingStoreIncludingSubLayers(textureMapper);
    }
    for (auto &child : Children())
    {
      Downcast<GraphicsLayerTextureMapper>(*child).UpdateBackingStoreIncludingSubLayers(textureMapper);
    }
  }

  void GraphicsLayerTextureMapper::UpdateBackingStoreIfNeeded(TextureMapper &textureMapper) noexcept
  {
    if (!ShouldHaveBackingStore())
    {
      krys_debug_assert(!_backingStore);
      return;
    }
    krys_debug_assert(_backingStore);

    boo::IntRect dirtyRect = boo::EnclosingIntRect(boo::FloatRect(boo::FloatPoint(), _size));
    if (!_needsDisplay)
    {
      dirtyRect.Intersect(boo::EnclosingIntRect(_needsDisplayRect));
    }

    // The tiles follow what's visible of the layer even when nothing in it changed.
    float scale = PageScaleFactor() * DeviceScaleFactor();
    _backingStore->UpdateContentsScale(scale);

    dirtyRect.Scale(scale);
    _backingStore->UpdateContents(
      textureMapper, *this, _size, dirtyRect, _visibleRect,
      boo::ExpandedIntSize(GraphicsLayer::Client().EnclosingFrameViewVisibleSize()));

    _needsDisplay = false;
    _needsDisplayRect = {};
  }

  bool GraphicsLayerTextureMapper::ShouldHaveBackingStore() const noexcept
  {
    return DrawsContent() && ContentsAreVisible() && !_size.IsEmpty();
  }

  bool GraphicsLayerTextureMapper::FiltersCanBeComposited(const boo::FilterOperations &filters) const noexcept
  {
    if (!filters.size())
    {
      return false;
    }

    return !filters.HasReferenceFilter();
  }

  bool GraphicsLayerTextureMapper::SetFilters(const boo::FilterOperations &filters) noexcept
  {
    bool canCompositeFilters = FiltersCanBeComposited(filters);
    if (GraphicsLayer::Filters() == filters)
    {
      return canCompositeFilters;
    }

    if (canCompositeFilters)
    {
      if (!GraphicsLayer::SetFilters(filters))
      {
        return false;
      }
      NotifyChange(FilterChange);
    }
    else if (GraphicsLayer::Filters().size())
    {
      ClearFilters();
      NotifyChange(FilterChange);
    }

    return canCompositeFilters;
  }

  bool GraphicsLayerTextureMapper::SetBackdropFilters(const boo::FilterOperations &filters) noexcept
  {
    bool canCompositeFilters = FiltersCanBeComposited(filters);
    if (_backdropFilters == filters)
    {
      return canCompositeFilters;
    }

    if (canCompositeFilters)
    {
      GraphicsLayer::SetBackdropFilters(filters);
    }
    else
    {
      ClearBackdropFilters();
    }

    NotifyChange(BackdropLayerChange);

    return canCompositeFilters;
  }

  void GraphicsLayerTextureMapper::SetBackdropFiltersRect(
    const boo::FloatRoundedRect &backdropFiltersRect) noexcept
  {
    if (_backdropFiltersRect == backdropFiltersRect)
    {
      return;
    }

    GraphicsLayer::SetBackdropFiltersRect(backdropFiltersRect);
    NotifyChange(BackdropLayerChange);
  }
}
