#include "Krystal.UI/Compositing/TextureMapperLayer.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"
#include "Krystal.UI/Compositing/TextureMapperBackingStore.hpp"

namespace krys::ui
{
  namespace
  {
    constexpr auto opacityVisibilityThreshold = 0.01;
  }

  class TextureMapperPaintOptions
  {
  public:
    explicit TextureMapperPaintOptions(TextureMapper &textureMapper) noexcept : TextureMapper(textureMapper)
    {
    }

    ui::TextureMapper &TextureMapper;
    boo::TransformationMatrix Transform;
    float Opacity {1};
    TextureMapperLayer *BackdropLayer {nullptr};
    TextureMapperLayer *ReplicaLayer {nullptr};
  };

  TextureMapperLayer::TextureMapperLayer() noexcept = default;

  TextureMapperLayer::~TextureMapperLayer() noexcept
  {
    for (auto *child : _children)
    {
      child->_parent = nullptr;
    }

    RemoveFromParent();
  }

  void TextureMapperLayer::ComputeTransformsRecursive() noexcept
  {
    if (_state.Size.IsEmpty() && _state.MasksToBounds)
    {
      return;
    }

    // Compute transforms recursively on the way down to leafs.
    {
      boo::TransformationMatrix parentTransform;
      if (_parent)
      {
        parentTransform = _parent->_layerTransforms.CombinedForChildren;
      }
      else if (_effectTarget)
      {
        parentTransform = _effectTarget->_layerTransforms.Combined;
      }

      const float originX = _state.AnchorPoint.X() * _state.Size.Width();
      const float originY = _state.AnchorPoint.Y() * _state.Size.Height();

      _layerTransforms.Combined = parentTransform;
      _layerTransforms.Combined
        .Translate3d(originX + (_state.Pos.X() - _state.BoundsOrigin.X()),
                     originY + (_state.Pos.Y() - _state.BoundsOrigin.Y()), _state.AnchorPoint.Z())
        .Multiply(_layerTransforms.LocalTransform);

      _layerTransforms.CombinedForChildren = _layerTransforms.Combined;
      _layerTransforms.Combined.Translate3d(-originX, -originY, -_state.AnchorPoint.Z());

      if (_isReplica)
      {
        _layerTransforms.Combined.Translate(-_state.Pos.X(), -_state.Pos.Y());
      }

      if (!_state.Preserves3D)
      {
        _layerTransforms.CombinedForChildren.Flatten();
      }
      _layerTransforms.CombinedForChildren.Multiply(_state.ChildrenTransform);
      _layerTransforms.CombinedForChildren.Translate3d(-originX, -originY, -_state.AnchorPoint.Z());
    }

    _state.Visible = _state.BackfaceVisibility || !_layerTransforms.Combined.IsBackFaceVisible();

    if (_state.BackdropLayer)
    {
      _state.BackdropLayer->ComputeTransformsRecursive();
    }

    if (_state.MaskLayer)
    {
      _state.MaskLayer->ComputeTransformsRecursive();
    }
    if (_state.ReplicaLayer)
    {
      _state.ReplicaLayer->ComputeTransformsRecursive();
    }
    for (auto *child : _children)
    {
      krys_debug_assert(child->_parent == this);
      child->ComputeTransformsRecursive();
    }
  }

  void TextureMapperLayer::PrepareForPainting(TextureMapper &) noexcept
  {
    ComputeTransformsRecursive();
  }

  void TextureMapperLayer::Paint(TextureMapper &textureMapper) noexcept
  {
    PrepareForPainting(textureMapper);
    TextureMapperPaintOptions options(textureMapper);
    PaintRecursive(options);
  }

  void TextureMapperLayer::PaintSelf(TextureMapperPaintOptions &options) noexcept
  {
    if (!_state.Visible || !_state.ContentsVisible)
    {
      return;
    }
    auto targetRect = LayerRect();
    if (targetRect.IsEmpty())
    {
      return;
    }

    boo::TransformationMatrix transform;
    transform.Multiply(options.Transform);
    transform.Multiply(_layerTransforms.Combined);

    TextureMapperSolidColorLayer solidColorLayer;
    TextureMapperBackingStore *backingStore = _backingStore;
    if (_state.BackgroundColor.IsValid())
    {
      solidColorLayer.SetColor(_state.BackgroundColor);
      backingStore = &solidColorLayer;
    }

    options.TextureMapper.SetWrapMode(TextureMapper::WrapMode::Stretch);
    options.TextureMapper.SetPatternTransform(boo::TransformationMatrix());

    if (backingStore)
    {
      backingStore->PaintToTextureMapper(options.TextureMapper, targetRect, transform, options.Opacity);
      if (_state.ShowDebugBorders)
      {
        backingStore->DrawBorder(options.TextureMapper, _state.DebugBorderColor, _state.DebugBorderWidth,
                                 targetRect, transform);
      }
      // Only draw repaint count for the main backing store.
      if (_state.ShowRepaintCounter)
      {
        backingStore->DrawRepaintCounter(options.TextureMapper, _state.RepaintCount, _state.DebugBorderColor,
                                         targetRect, transform);
      }
    }

    TextureMapperPlatformLayer *contentsLayer = _contentsLayer;
    if (_state.SolidColor.IsValid() && _state.SolidColor.IsVisible())
    {
      solidColorLayer.SetColor(_state.SolidColor);
      contentsLayer = &solidColorLayer;
    }

    if (!contentsLayer)
    {
      return;
    }

    if (!_state.ContentsTileSize.IsEmpty())
    {
      options.TextureMapper.SetWrapMode(TextureMapper::WrapMode::Repeat);

      auto patternTransform =
        boo::TransformationMatrix::RectToRect({{}, _state.ContentsTileSize}, {{}, _state.ContentsRect.Size()})
          .Translate(_state.ContentsTilePhase.Width() / _state.ContentsRect.Width(),
                     _state.ContentsTilePhase.Height() / _state.ContentsRect.Height());
      options.TextureMapper.SetPatternTransform(patternTransform);
    }

    bool shouldClip = _state.ContentsClippingRect.IsRounded()
                      || !_state.ContentsClippingRect.Rect().Contains(_state.ContentsRect);
    if (shouldClip)
    {
      options.TextureMapper.BeginClip(transform, _state.ContentsClippingRect);
    }

    contentsLayer->PaintToTextureMapper(options.TextureMapper, _state.ContentsRect, transform,
                                        options.Opacity);

    if (shouldClip)
    {
      options.TextureMapper.EndClip();
    }

    if (_state.ShowDebugBorders)
    {
      contentsLayer->DrawBorder(options.TextureMapper, _state.DebugBorderColor, _state.DebugBorderWidth,
                                _state.ContentsRect, transform);
    }
  }

  void TextureMapperLayer::PaintSelfAndChildren(TextureMapperPaintOptions &options) noexcept
  {
    PaintSelf(options);

    if (_children.empty())
    {
      return;
    }

    bool shouldClip = (_state.MasksToBounds || _state.ContentsRectClipsDescendants) && !_state.Preserves3D;
    if (shouldClip)
    {
      boo::TransformationMatrix clipTransform;
      clipTransform.Multiply(options.Transform);
      clipTransform.Multiply(_layerTransforms.Combined);
      if (_state.ContentsRectClipsDescendants)
      {
        options.TextureMapper.BeginClip(clipTransform, _state.ContentsClippingRect);
      }
      else
      {
        clipTransform.Translate(_state.BoundsOrigin.X(), _state.BoundsOrigin.Y());
        options.TextureMapper.BeginClip(clipTransform, boo::FloatRoundedRect(LayerRect()));
      }

      // If as a result of beginClip(), the clipping area is empty, it means that the intersection of the
      // previous clipping area and the current one don't have any pixels in common. In this case we can skip
      // painting the children as they will be clipped out.
      if (options.TextureMapper.ClipBounds().IsEmpty())
      {
        options.TextureMapper.EndClip();
        return;
      }
    }

    for (auto *child : _children)
    {
      child->PaintRecursive(options);
    }

    if (shouldClip)
    {
      options.TextureMapper.EndClip();
    }
  }

  bool TextureMapperLayer::ShouldBlend() const noexcept
  {
    if (_state.Preserves3D)
    {
      return false;
    }

    return _currentOpacity < 1;
  }

  bool TextureMapperLayer::IsVisible() const noexcept
  {
    if (_state.Size.IsEmpty() && (_state.MasksToBounds || _state.MaskLayer || _children.empty()))
    {
      return false;
    }
    if (!_state.Visible && _children.empty())
    {
      return false;
    }
    if (!_state.ContentsVisible && _children.empty())
    {
      return false;
    }
    if (_currentOpacity < opacityVisibilityThreshold)
    {
      return false;
    }
    return true;
  }

  void TextureMapperLayer::PaintSelfAndChildrenWithReplica(TextureMapperPaintOptions &options) noexcept
  {
    // FIXME: Replicas (reflections), once there are reflections.
    PaintSelfAndChildren(options);
  }

  void TextureMapperLayer::PaintWithGroup(TextureMapperPaintOptions &options) noexcept
  {
    // What the layer and its children draw is composited as one at the layer's opacity, through its filters.
    options.TextureMapper.BeginGroup(options.Opacity, BlendMode::Normal, _currentFilters);
    {
      SetForScope scopedOpacity(options.Opacity, 1.f);
      // FIXME: Masks, once masks are composited.
      PaintSelfAndChildrenWithReplica(options);
    }
    options.TextureMapper.EndGroup();
  }

  void TextureMapperLayer::PaintSelfChildrenReplicaFilterAndMask(TextureMapperPaintOptions &options) noexcept
  {
    bool hasFilterOrMask = HasFilters() || HasMask();
    if (hasFilterOrMask)
    {
      PaintWithGroup(options);
    }
    else
    {
      PaintSelfAndChildrenWithReplica(options);
    }
  }

  void TextureMapperLayer::PaintRecursive(TextureMapperPaintOptions &options) noexcept
  {
    if (!IsVisible())
    {
      return;
    }

    SetForScope scopedOpacity(options.Opacity, options.Opacity * _currentOpacity);

    // FIXME: 3D rendering contexts and flattening, once Render2D draws in 3D: such layers draw flattened.
    if (ShouldBlend())
    {
      PaintWithGroup(options);
    }
    else
    {
      PaintSelfChildrenReplicaFilterAndMask(options);
    }
  }

  void TextureMapperLayer::SetChildren(const List<TextureMapperLayer *> &newChildren) noexcept
  {
    RemoveAllChildren();
    for (auto *child : newChildren)
    {
      AddChild(child);
    }
  }

  void TextureMapperLayer::AddChild(TextureMapperLayer *childLayer) noexcept
  {
    krys_debug_assert(childLayer != this);

    if (childLayer->_parent)
    {
      childLayer->RemoveFromParent();
    }

    childLayer->_parent = this;
    _children.push_back(childLayer);
  }

  void TextureMapperLayer::RemoveFromParent() noexcept
  {
    if (_parent)
    {
      auto &siblings = _parent->_children;
      auto it = std::find(siblings.begin(), siblings.end(), this);
      krys_debug_assert(it != siblings.end());
      if (it != siblings.end())
      {
        siblings.erase(it);
      }
    }

    _parent = nullptr;
  }

  void TextureMapperLayer::RemoveAllChildren() noexcept
  {
    auto oldChildren = krys::move(_children);
    _children.clear();
    for (auto *child : oldChildren)
    {
      child->_parent = nullptr;
    }
  }

  void TextureMapperLayer::SetMaskLayer(TextureMapperLayer *maskLayer) noexcept
  {
    if (maskLayer)
    {
      maskLayer->_effectTarget = _isReplica ? _effectTarget : CreateWeakPtr(this);
      _state.MaskLayer = CreateWeakPtr(maskLayer);
    }
    else
    {
      _state.MaskLayer = nullptr;
    }
  }

  void TextureMapperLayer::SetReplicaLayer(TextureMapperLayer *replicaLayer) noexcept
  {
    if (replicaLayer)
    {
      replicaLayer->_isReplica = true;
      replicaLayer->_effectTarget = CreateWeakPtr(this);
      _state.ReplicaLayer = CreateWeakPtr(replicaLayer);
    }
    else
    {
      _state.ReplicaLayer = nullptr;
    }
  }

  void TextureMapperLayer::SetBackdropLayer(TextureMapperLayer *backdropLayer) noexcept
  {
    if (backdropLayer)
    {
      backdropLayer->_isBackdrop = true;
      backdropLayer->_effectTarget = CreateWeakPtr(this);
      _state.BackdropLayer = CreateWeakPtr(backdropLayer);
    }
    else
    {
      _state.BackdropLayer = nullptr;
    }
  }

  void TextureMapperLayer::SetBackdropFiltersRect(const boo::FloatRoundedRect &backdropFiltersRect) noexcept
  {
    _state.BackdropFiltersRect = backdropFiltersRect;
  }

  void TextureMapperLayer::SetPosition(const boo::FloatPoint &position) noexcept
  {
    _state.Pos = position;
  }

  void TextureMapperLayer::SetBoundsOrigin(const boo::FloatPoint &boundsOrigin) noexcept
  {
    _state.BoundsOrigin = boundsOrigin;
  }

  void TextureMapperLayer::SetSize(const boo::FloatSize &size) noexcept
  {
    _state.Size = size;
  }

  void TextureMapperLayer::SetAnchorPoint(const boo::FloatPoint3D &anchorPoint) noexcept
  {
    _state.AnchorPoint = anchorPoint;
  }

  void TextureMapperLayer::SetPreserves3D(bool preserves3D) noexcept
  {
    _state.Preserves3D = preserves3D;
  }

  void TextureMapperLayer::SetTransform(const boo::TransformationMatrix &transform) noexcept
  {
    _state.Transform = transform;
  }

  void TextureMapperLayer::SetChildrenTransform(const boo::TransformationMatrix &childrenTransform) noexcept
  {
    _state.ChildrenTransform = childrenTransform;
  }

  void TextureMapperLayer::SetContentsRect(const boo::FloatRect &contentsRect) noexcept
  {
    _state.ContentsRect = contentsRect;
  }

  void TextureMapperLayer::SetContentsTileSize(const boo::FloatSize &size) noexcept
  {
    _state.ContentsTileSize = size;
  }

  void TextureMapperLayer::SetContentsTilePhase(const boo::FloatSize &phase) noexcept
  {
    _state.ContentsTilePhase = phase;
  }

  void TextureMapperLayer::SetContentsClippingRect(const boo::FloatRoundedRect &contentsClippingRect) noexcept
  {
    _state.ContentsClippingRect = contentsClippingRect;
  }

  void TextureMapperLayer::SetContentsRectClipsDescendants(bool clips) noexcept
  {
    _state.ContentsRectClipsDescendants = clips;
  }

  void TextureMapperLayer::SetMasksToBounds(bool masksToBounds) noexcept
  {
    _state.MasksToBounds = masksToBounds;
  }

  void TextureMapperLayer::SetDrawsContent(bool drawsContent) noexcept
  {
    _state.DrawsContent = drawsContent;
  }

  void TextureMapperLayer::SetContentsVisible(bool contentsVisible) noexcept
  {
    _state.ContentsVisible = contentsVisible;
  }

  void TextureMapperLayer::SetContentsOpaque(bool contentsOpaque) noexcept
  {
    _state.ContentsOpaque = contentsOpaque;
  }

  void TextureMapperLayer::SetBackfaceVisibility(bool backfaceVisibility) noexcept
  {
    _state.BackfaceVisibility = backfaceVisibility;
  }

  void TextureMapperLayer::SetOpacity(float opacity) noexcept
  {
    _state.Opacity = opacity;
  }

  void TextureMapperLayer::SetSolidColor(const boo::Color &color) noexcept
  {
    _state.SolidColor = color;
  }

  void TextureMapperLayer::SetBackgroundColor(const boo::Color &color) noexcept
  {
    _state.BackgroundColor = color;
  }

  void TextureMapperLayer::SetFilters(const boo::FilterOperations &filters) noexcept
  {
    _state.Filters = filters;
  }

  void TextureMapperLayer::SetContentsLayer(TextureMapperPlatformLayer *platformLayer) noexcept
  {
    _contentsLayer = platformLayer;
  }

  void TextureMapperLayer::SetBackingStore(TextureMapperBackingStore *backingStore) noexcept
  {
    _backingStore = backingStore;
  }

  void TextureMapperLayer::SetAnimations(const TextureMapperAnimations &animations) noexcept
  {
    _animations = animations;
  }

  bool TextureMapperLayer::DescendantsOrSelfHaveRunningAnimations() const noexcept
  {
    if (_animations.HasRunningAnimations())
    {
      return true;
    }

    return std::ranges::any_of(_children, [](TextureMapperLayer *child)
                               { return child->DescendantsOrSelfHaveRunningAnimations(); });
  }

  bool TextureMapperLayer::ApplyAnimationsRecursively(MonotonicTime::duration time) noexcept
  {
    bool hasRunningAnimations = SyncAnimations(time);
    if (_state.ReplicaLayer)
    {
      hasRunningAnimations |= _state.ReplicaLayer->ApplyAnimationsRecursively(time);
    }
    if (_state.BackdropLayer)
    {
      hasRunningAnimations |= _state.BackdropLayer->SyncAnimations(time);
    }
    for (auto *child : _children)
    {
      hasRunningAnimations |= child->ApplyAnimationsRecursively(time);
    }
    return hasRunningAnimations;
  }

  bool TextureMapperLayer::SyncAnimations(MonotonicTime::duration time) noexcept
  {
    TextureMapperAnimation::ApplicationResult applicationResults;
    _animations.Apply(applicationResults, time);

    _layerTransforms.LocalTransform = applicationResults.Transform.value_or(_state.Transform);
    _currentOpacity = static_cast<float>(applicationResults.Opacity.value_or(_state.Opacity));
    _currentFilters = applicationResults.Filters.value_or(_state.Filters);

    return applicationResults.HasRunningAnimations;
  }

  boo::FloatRect TextureMapperLayer::EffectiveLayerRect() const noexcept
  {
    return LayerRect();
  }
}
