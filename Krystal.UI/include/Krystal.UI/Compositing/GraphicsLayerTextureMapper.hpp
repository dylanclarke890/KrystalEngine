#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFactory.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerTransform.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.UI/Compositing/TextureMapperLayer.hpp"
#include "Krystal.UI/Compositing/TextureMapperPlatformLayer.hpp"
#include "Krystal.UI/Compositing/TextureMapperTiledBackingStore.hpp"

namespace krys::ui
{
  class TextureMapper;

  /// @brief Krystal.UI's graphics layer: it commits its changes to its `TextureMapperLayer` when it's
  /// flushed, and paints its contents into tiles that cover what's visible of it.
  // FIXME: Contents scaling filters (`image-rendering` on composited images and canvases).
  class GraphicsLayerTextureMapper final : public boo::GraphicsLayer,
                                           public TextureMapperPlatformLayer::Client
  {
  private:
    // This set of flags help us defer which properties of the layer have been modified by the compositor, so
    // we can know what to look for in the next flush.
    enum ChangeMask : uint32
    {
      NoChanges = 0,
      ChildrenChange = (1u << 1),
      MaskLayerChange = (1u << 2),
      ReplicaLayerChange = (1u << 3),
      ContentChange = (1u << 4),
      ContentsRectChange = (1u << 5),
      ContentsVisibleChange = (1u << 6),
      ContentsOpaqueChange = (1u << 7),
      PositionChange = (1u << 8),
      AnchorPointChange = (1u << 9),
      SizeChange = (1u << 10),
      TransformChange = (1u << 11),
      ChildrenTransformChange = (1u << 12),
      Preserves3DChange = (1u << 13),
      MasksToBoundsChange = (1u << 14),
      DrawsContentChange = (1u << 15),
      OpacityChange = (1u << 16),
      BackfaceVisibilityChange = (1u << 17),
      BackingStoreChange = (1u << 18),
      DisplayChange = (1u << 19),
      ContentsDisplayChange = (1u << 20),
      BackgroundColorChange = (1u << 21),
      AnimationChange = (1u << 22),
      FilterChange = (1u << 23),
      DebugVisualsChange = (1u << 24),
      RepaintCountChange = (1u << 25),
      AnimationStarted = (1u << 26),
      BackdropLayerChange = (1u << 27),
      SolidColorChange = (1u << 28),
      BoundsOriginChange = (1u << 29),
    };

    UniquePtr<TextureMapperLayer> _layer {CreateUnique<TextureMapperLayer>()};
    UniquePtr<TextureMapperLayer> _backdropLayer;
    RefPtr<TextureMapperTiledBackingStore> _compositedImage;
    RefPtr<boo::NativeImage> _compositedNativeImage;
    RefPtr<TextureMapperTiledBackingStore> _backingStore;

    uint32 _changeMask {NoChanges};
    bool _needsDisplay {false};
    boo::Color _solidColor;
    boo::Color _debugBorderColor;
    float _debugBorderWidth {0};
    TextureMapperPlatformLayer *_contentsLayer {nullptr};
    // Keeps the platform layer it shows alive.
    RefPtr<boo::GraphicsLayerContentsDisplayDelegate> _contentsDisplayDelegate;
    boo::FloatRect _needsDisplayRect;
    TextureMapperAnimations _animations;
    MonotonicTime::duration _animationStartTime {};

    // What's visible of the layer, in its coordinates, as of its last flush: its tiles cover it.
    boo::GraphicsLayerTransform _layerTransform;
    boo::IntRect _visibleRect;

  public:
    explicit GraphicsLayerTextureMapper(Type layerType, boo::GraphicsLayerClient &client) noexcept;
    ~GraphicsLayerTextureMapper() noexcept override;

    // GraphicsLayer
    bool SetChildren(List<Ref<GraphicsLayer>> &&children) noexcept override;
    void AddChild(Ref<GraphicsLayer> &&layer) noexcept override;
    void AddChildAtIndex(Ref<GraphicsLayer> &&layer, int32 index) noexcept override;
    void AddChildAbove(Ref<GraphicsLayer> &&layer, GraphicsLayer *sibling) noexcept override;
    void AddChildBelow(Ref<GraphicsLayer> &&layer, GraphicsLayer *sibling) noexcept override;
    bool ReplaceChild(GraphicsLayer *oldChild, Ref<GraphicsLayer> &&newChild) noexcept override;

    void SetMaskLayer(RefPtr<GraphicsLayer> &&value) noexcept override;
    void SetReplicatedByLayer(RefPtr<GraphicsLayer> &&value) noexcept override;
    void SetPosition(const boo::FloatPoint &value) noexcept override;
    void SetBoundsOrigin(const boo::FloatPoint &value) noexcept override;
    void SetAnchorPoint(const boo::FloatPoint3D &value) noexcept override;
    void SetSize(const boo::FloatSize &value) noexcept override;
    void SetTransform(const boo::TransformationMatrix &value) noexcept override;
    void SetChildrenTransform(const boo::TransformationMatrix &value) noexcept override;
    void SetPreserves3D(bool value) noexcept override;
    void SetMasksToBounds(bool value) noexcept override;
    void SetDrawsContent(bool value) noexcept override;
    void SetContentsVisible(bool value) noexcept override;
    void SetContentsOpaque(bool value) noexcept override;
    void SetBackfaceVisibility(bool value) noexcept override;
    void SetBackgroundColor(const boo::Color &value) noexcept override;
    void SetOpacity(float value) noexcept override;
    bool SetFilters(const boo::FilterOperations &filters) noexcept override;
    bool SetBackdropFilters(const boo::FilterOperations &filters) noexcept override;
    void SetBackdropFiltersRect(const boo::FloatRoundedRect &backdropFiltersRect) noexcept override;

    void SetNeedsDisplay() noexcept override;
    void SetNeedsDisplayInRect(const boo::FloatRect &rect,
                               ShouldClipToLayer = ShouldClipToLayer::Clip) noexcept override;
    void SetContentsNeedsDisplay() noexcept override;
    void SetContentsRect(const boo::FloatRect &value) noexcept override;
    void SetContentsClippingRect(const boo::FloatRoundedRect &rect) noexcept override;
    void SetContentsRectClipsDescendants(bool contentsRectClipsDescendants) noexcept override;

    void SetContentsToImage(boo::Image *image) noexcept override;
    void SetContentsToSolidColor(const boo::Color &color) noexcept override;
    void SetContentsToPlatformLayer(TextureMapperPlatformLayer *platformLayer,
                                    ContentsLayerPurpose purpose) noexcept;
    void SetContentsDisplayDelegate(RefPtr<boo::GraphicsLayerContentsDisplayDelegate> &&delegate,
                                    ContentsLayerPurpose purpose) noexcept override;
    KRYS_NODISCARD bool UsesContentsLayer() const noexcept override
    {
      return _contentsLayer != nullptr;
    }

    bool AddAnimation(const boo::GraphicsLayerKeyframeValueList &valueList,
                      const boo::GraphicsLayerAnimation *animation, const utf8_string &keyframesName,
                      double timeOffset) noexcept override;
    void PauseAnimation(const utf8_string &animationName, double timeOffset) noexcept override;
    void RemoveAnimation(const utf8_string &animationName, Maybe<boo::AnimatedProperty>) noexcept override;

    void SetShowDebugBorder(bool show) noexcept override;
    void SetDebugBorder(const boo::Color &color, float width) noexcept override;
    void SetShowRepaintCounter(bool show) noexcept override;

    void FlushCompositingState(const boo::FloatRect &visibleRect) noexcept override;
    void FlushCompositingStateForThisLayerOnly() noexcept override;

    void UpdateBackingStoreIncludingSubLayers(TextureMapper &textureMapper) noexcept;

    KRYS_NODISCARD TextureMapperLayer &Layer() noexcept
    {
      return *_layer;
    }
    KRYS_NODISCARD boo::Color DebugBorderColor() const noexcept
    {
      return _debugBorderColor;
    }
    KRYS_NODISCARD float DebugBorderWidth() const noexcept
    {
      return _debugBorderWidth;
    }

    /// @brief What's visible of the layer, in its coordinates, as of its last flush.
    KRYS_NODISCARD const boo::IntRect &VisibleRect() const noexcept
    {
      return _visibleRect;
    }
    KRYS_NODISCARD TextureMapperTiledBackingStore *BackingStore() const noexcept
    {
      return _backingStore.get();
    }

    KRYS_NODISCARD bool IsGraphicsLayerTextureMapper() const noexcept override
    {
      return true;
    }

  private:
    // TextureMapperPlatformLayer::Client
    void PlatformLayerWillBeDestroyed() noexcept override
    {
      SetContentsToPlatformLayer(nullptr, ContentsLayerPurpose::None);
    }
    void SetPlatformLayerNeedsDisplay() noexcept override
    {
      SetContentsNeedsDisplay();
    }

    void CommitLayerChanges() noexcept;
    void UpdateDebugBorderAndRepaintCount() noexcept;
    void UpdateBackingStoreIfNeeded(TextureMapper &textureMapper) noexcept;
    void PrepareBackingStoreIfNeeded() noexcept;
    KRYS_NODISCARD bool ShouldHaveBackingStore() const noexcept;
    KRYS_NODISCARD bool FiltersCanBeComposited(const boo::FilterOperations &filters) const noexcept;
    void UpdateVisibleRects(const boo::FloatRect &visibleRect) noexcept;

    void NotifyChange(ChangeMask changeMask) noexcept;
  };

  /// @brief Creates Krystal.UI's graphics layers.
  class GraphicsLayerFactoryTextureMapper final : public boo::GraphicsLayerFactory
  {
  public:
    KRYS_NODISCARD Ref<boo::GraphicsLayer>
      CreateGraphicsLayer(boo::GraphicsLayerType type, boo::GraphicsLayerClient &client) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_GRAPHICSLAYER(krys::ui::GraphicsLayerTextureMapper,
                                          IsGraphicsLayerTextureMapper())
