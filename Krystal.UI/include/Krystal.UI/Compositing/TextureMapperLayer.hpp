#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.UI/Compositing/TextureMapperAnimation.hpp"

namespace krys::ui
{
  class TextureMapper;
  class TextureMapperBackingStore;
  class TextureMapperPaintOptions;
  class TextureMapperPlatformLayer;

  /// @brief A layer of the composited tree as the compositor draws it: its state, committed from its graphics
  /// layer, its transforms, and how it paints its backing store, its contents and its children.
  // FIXME: Masks, once masks are composited; backdrops, with backdrop-filter; replicas (reflections); 3D
  // rendering contexts and flattening (their layers draw flattened); damage tracking (the whole tree is
  // composited each frame).
  class TextureMapperLayer : public CanMakeWeakPtr<TextureMapperLayer>,
                             public CanMakeCheckedPtr<TextureMapperLayer>
  {
    KRYS_NON_COPY_MOVABLE(TextureMapperLayer);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TextureMapperLayer);

  private:
    struct State
    {
      boo::FloatPoint Pos;
      boo::FloatPoint3D AnchorPoint {0.5, 0.5, 0};
      boo::FloatPoint BoundsOrigin;
      boo::FloatSize Size;
      boo::TransformationMatrix Transform;
      boo::TransformationMatrix ChildrenTransform;
      float Opacity {1};
      boo::FloatRect ContentsRect;
      boo::FloatSize ContentsTileSize;
      boo::FloatSize ContentsTilePhase;
      boo::FloatRoundedRect ContentsClippingRect;
      WeakPtr<TextureMapperLayer> MaskLayer;
      WeakPtr<TextureMapperLayer> ReplicaLayer;
      WeakPtr<TextureMapperLayer> BackdropLayer;
      boo::FloatRoundedRect BackdropFiltersRect;
      boo::Color SolidColor;
      boo::Color BackgroundColor;
      boo::FilterOperations Filters;
      boo::Color DebugBorderColor;
      float DebugBorderWidth {0};
      int32 RepaintCount {0};
      bool Preserves3D : 1 {false};
      bool MasksToBounds : 1 {false};
      bool DrawsContent : 1 {false};
      bool ContentsVisible : 1 {true};
      bool ContentsOpaque : 1 {false};
      bool ContentsRectClipsDescendants : 1 {false};
      bool BackfaceVisibility : 1 {true};
      bool Visible : 1 {true};
      bool ShowDebugBorders : 1 {false};
      bool ShowRepaintCounter : 1 {false};
    };

    struct LayerTransforms
    {
      boo::TransformationMatrix LocalTransform;
      boo::TransformationMatrix Combined;
      boo::TransformationMatrix CombinedForChildren;
    };

    List<TextureMapperLayer *> _children;
    TextureMapperLayer *_parent {nullptr};
    WeakPtr<TextureMapperLayer> _effectTarget;
    TextureMapperBackingStore *_backingStore {nullptr};
    TextureMapperPlatformLayer *_contentsLayer {nullptr};
    float _currentOpacity {1.0};
    boo::FilterOperations _currentFilters;
    State _state;
    bool _isBackdrop {false};
    bool _isReplica {false};
    LayerTransforms _layerTransforms;
    TextureMapperAnimations _animations;

  public:
    TextureMapperLayer() noexcept;
    virtual ~TextureMapperLayer() noexcept;

    KRYS_NODISCARD const List<TextureMapperLayer *> &Children() const noexcept
    {
      return _children;
    }
    void SetChildren(const List<TextureMapperLayer *> &newChildren) noexcept;
    void SetMaskLayer(TextureMapperLayer *maskLayer) noexcept;
    void SetReplicaLayer(TextureMapperLayer *replicaLayer) noexcept;
    void SetBackdropLayer(TextureMapperLayer *backdropLayer) noexcept;
    void SetBackdropFiltersRect(const boo::FloatRoundedRect &backdropFiltersRect) noexcept;
    void SetPosition(const boo::FloatPoint &position) noexcept;
    void SetBoundsOrigin(const boo::FloatPoint &boundsOrigin) noexcept;
    void SetSize(const boo::FloatSize &size) noexcept;
    void SetAnchorPoint(const boo::FloatPoint3D &anchorPoint) noexcept;
    void SetPreserves3D(bool preserves3D) noexcept;
    void SetTransform(const boo::TransformationMatrix &transform) noexcept;
    void SetChildrenTransform(const boo::TransformationMatrix &childrenTransform) noexcept;
    void SetContentsRect(const boo::FloatRect &contentsRect) noexcept;
    void SetMasksToBounds(bool masksToBounds) noexcept;
    void SetDrawsContent(bool drawsContent) noexcept;

    KRYS_NODISCARD bool DrawsContent() const noexcept
    {
      return _state.DrawsContent;
    }
    KRYS_NODISCARD bool ContentsAreVisible() const noexcept
    {
      return _state.ContentsVisible;
    }
    KRYS_NODISCARD boo::FloatSize Size() const noexcept
    {
      return _state.Size;
    }
    KRYS_NODISCARD float Opacity() const noexcept
    {
      return _state.Opacity;
    }
    KRYS_NODISCARD boo::TransformationMatrix Transform() const noexcept
    {
      return _state.Transform;
    }
    KRYS_NODISCARD const boo::TransformationMatrix &ToSurfaceTransform() const noexcept
    {
      return _layerTransforms.Combined;
    }

    void SetContentsVisible(bool contentsVisible) noexcept;
    void SetContentsOpaque(bool contentsOpaque) noexcept;
    void SetBackfaceVisibility(bool backfaceVisibility) noexcept;
    void SetOpacity(float opacity) noexcept;
    void SetSolidColor(const boo::Color &color) noexcept;
    void SetBackgroundColor(const boo::Color &color) noexcept;
    void SetContentsTileSize(const boo::FloatSize &size) noexcept;
    void SetContentsTilePhase(const boo::FloatSize &phase) noexcept;
    void SetContentsClippingRect(const boo::FloatRoundedRect &contentsClippingRect) noexcept;
    void SetContentsRectClipsDescendants(bool clips) noexcept;
    void SetFilters(const boo::FilterOperations &filters) noexcept;

    KRYS_NODISCARD bool HasFilters() const noexcept
    {
      return !_currentFilters.IsEmpty();
    }

    void SetShowDebugBorder(bool showDebugBorder) noexcept
    {
      _state.ShowDebugBorders = showDebugBorder;
    }
    void SetDebugBorderColor(boo::Color debugBorderColor) noexcept
    {
      _state.DebugBorderColor = debugBorderColor;
    }
    void SetDebugBorderWidth(float debugBorderWidth) noexcept
    {
      _state.DebugBorderWidth = debugBorderWidth;
    }
    void SetShowRepaintCounter(bool showRepaintCounter) noexcept
    {
      _state.ShowRepaintCounter = showRepaintCounter;
    }
    void SetRepaintCount(int32 repaintCount) noexcept
    {
      _state.RepaintCount = repaintCount;
    }

    void SetContentsLayer(TextureMapperPlatformLayer *platformLayer) noexcept;
    void SetAnimations(const TextureMapperAnimations &animations) noexcept;
    void SetBackingStore(TextureMapperBackingStore *backingStore) noexcept;

    KRYS_NODISCARD bool DescendantsOrSelfHaveRunningAnimations() const noexcept;
    bool ApplyAnimationsRecursively(MonotonicTime::duration time) noexcept;
    bool SyncAnimations(MonotonicTime::duration time) noexcept;

    void PrepareForPainting(TextureMapper &textureMapper) noexcept;
    void Paint(TextureMapper &textureMapper) noexcept;

    void AddChild(TextureMapperLayer *childLayer) noexcept;

    KRYS_NODISCARD boo::FloatRect EffectiveLayerRect() const noexcept;

  private:
    void ComputeTransformsRecursive() noexcept;

    void RemoveFromParent() noexcept;
    void RemoveAllChildren() noexcept;

    void PaintRecursive(TextureMapperPaintOptions &options) noexcept;
    void PaintSelfChildrenReplicaFilterAndMask(TextureMapperPaintOptions &options) noexcept;
    void PaintWithGroup(TextureMapperPaintOptions &options) noexcept;
    void PaintSelf(TextureMapperPaintOptions &options) noexcept;
    void PaintSelfAndChildren(TextureMapperPaintOptions &options) noexcept;
    void PaintSelfAndChildrenWithReplica(TextureMapperPaintOptions &options) noexcept;

    KRYS_NODISCARD bool IsVisible() const noexcept;
    KRYS_NODISCARD bool ShouldBlend() const noexcept;
    KRYS_NODISCARD bool Preserves3D() const noexcept
    {
      return _state.Preserves3D;
    }
    KRYS_NODISCARD bool HasMask() const noexcept
    {
      return _state.MaskLayer != nullptr;
    }
    KRYS_NODISCARD bool HasBackdrop() const noexcept
    {
      return _state.BackdropLayer != nullptr;
    }

    KRYS_NODISCARD boo::FloatRect LayerRect() const noexcept
    {
      return {boo::FloatPoint(), _state.Size};
    }
  };
}
