/*
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include "Krystal.Booey/Compositing/TextureMapperLayer.hpp"
#include "Krystal.Booey/Compositing/TextureMapperPlatformLayer.hpp"
#include "Krystal.Booey/Compositing/TextureMapperTiledBackingStore.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Web/Graphics/Images/NativeImage.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayer.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerFactory.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerTransform.hpp"

namespace krys::boo
{
  class TextureMapper;

  /// @brief Krystal.Booey's graphics layer: it commits its changes to its `TextureMapperLayer` when it's
  /// flushed, and paints its contents into tiles that cover what's visible of it.
  // FIXME: Contents scaling filters (`image-rendering` on composited images and canvases).
  class GraphicsLayerTextureMapper final : public web::GraphicsLayer,
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
      ContentsScalingFiltersChange = (1u << 30),
    };

    UniquePtr<TextureMapperLayer> _layer {CreateUnique<TextureMapperLayer>()};
    UniquePtr<TextureMapperLayer> _backdropLayer;
    RefPtr<TextureMapperTiledBackingStore> _compositedImage;
    RefPtr<web::NativeImage> _compositedNativeImage;
    RefPtr<TextureMapperTiledBackingStore> _backingStore;

    uint32 _changeMask {NoChanges};
    bool _needsDisplay {false};
    web::Color _solidColor;
    web::Color _debugBorderColor;
    float _debugBorderWidth {0};
    TextureMapperPlatformLayer *_contentsLayer {nullptr};
    // Keeps the platform layer it shows alive.
    RefPtr<web::GraphicsLayerContentsDisplayDelegate> _contentsDisplayDelegate;
    RectF _needsDisplayRect;
    TextureMapperAnimations _animations;
    MonotonicTime::duration _animationStartTime {};

    // What's visible of the layer, in its coordinates, as of its last flush: its tiles cover it.
    web::GraphicsLayerTransform _layerTransform;
    RectI _visibleRect;

  public:
    explicit GraphicsLayerTextureMapper(Type layerType, web::GraphicsLayerClient &client) noexcept;

    ~GraphicsLayerTextureMapper() noexcept override;

    // GraphicsLayer
    bool SetChildren(List<Ref<GraphicsLayer>> children) noexcept override;

    void AddChild(Ref<GraphicsLayer> &&layer) noexcept override;

    void AddChildAtIndex(Ref<GraphicsLayer> &&layer, int32 index) noexcept override;

    void AddChildAbove(Ref<GraphicsLayer> &&layer, GraphicsLayer *sibling) noexcept override;

    void AddChildBelow(Ref<GraphicsLayer> &&layer, GraphicsLayer *sibling) noexcept override;

    bool ReplaceChild(GraphicsLayer *oldChild, Ref<GraphicsLayer> &&newChild) noexcept override;

    void SetMaskLayer(RefPtr<GraphicsLayer> &&value) noexcept override;

    void SetReplicatedByLayer(RefPtr<GraphicsLayer> &&value) noexcept override;

    void SetPosition(const PointF &value) noexcept override;

    void SetBoundsOrigin(const PointF &value) noexcept override;

    void SetAnchorPoint(const Point3F &value) noexcept override;

    void SetSize(const SizeF &value) noexcept override;

    void SetTransform(const TransformationMatrix &value) noexcept override;

    void SetChildrenTransform(const TransformationMatrix &value) noexcept override;

    void SetPreserves3D(bool value) noexcept override;

    void SetMasksToBounds(bool value) noexcept override;

    void SetDrawsContent(bool value) noexcept override;

    void SetContentsVisible(bool value) noexcept override;

    void SetContentsMinificationFilter(ScalingFilter filter) noexcept override;

    void SetContentsMagnificationFilter(ScalingFilter filter) noexcept override;

    void SetContentsOpaque(bool value) noexcept override;

    void SetBackfaceVisibility(bool value) noexcept override;

    void SetBackgroundColor(const web::Color &value) noexcept override;

    void SetOpacity(float value) noexcept override;

    bool SetFilters(const web::FilterOperations &filters) noexcept override;

    bool SetBackdropFilters(const web::FilterOperations &filters) noexcept override;

    void SetBackdropFiltersRect(const web::FloatRoundedRect &backdropFiltersRect) noexcept override;

    void SetNeedsDisplay() noexcept override;

    void SetNeedsDisplayInRect(const RectF &rect,
                               ShouldClipToLayer = ShouldClipToLayer::Clip) noexcept override;

    void SetContentsNeedsDisplay() noexcept override;

    void SetContentsRect(const RectF &value) noexcept override;

    void SetContentsClippingRect(const web::FloatRoundedRect &rect) noexcept override;

    void SetContentsRectClipsDescendants(bool contentsRectClipsDescendants) noexcept override;

    void SetContentsToImage(web::Image *image) noexcept override;

    void SetContentsToSolidColor(const web::Color &color) noexcept override;

    void SetContentsToPlatformLayer(TextureMapperPlatformLayer *platformLayer,
                                    ContentsLayerPurpose purpose) noexcept;

    void SetContentsDisplayDelegate(RefPtr<web::GraphicsLayerContentsDisplayDelegate> &&delegate,
                                    ContentsLayerPurpose purpose) noexcept override;

    KRYS_NODISCARD bool UsesContentsLayer() const noexcept override
    {
      return _contentsLayer != nullptr;
    }

    bool AddAnimation(const web::GraphicsLayerKeyframeValueList &valueList,
                      const web::GraphicsLayerAnimation *animation, const utf8_string &keyframesName,
                      double timeOffset) noexcept override;

    void PauseAnimation(const utf8_string &animationName, double timeOffset) noexcept override;

    void SuspendAnimations(MonotonicTime::duration time) noexcept override;

    void ResumeAnimations() noexcept override;

    void RemoveAnimation(const utf8_string &animationName, Maybe<web::AnimatedProperty>) noexcept override;

    void SetShowDebugBorder(bool show) noexcept override;

    void SetDebugBorder(const web::Color &color, float width) noexcept override;

    void SetShowRepaintCounter(bool show) noexcept override;

    void FlushCompositingState(const RectF &visibleRect) noexcept override;

    void FlushCompositingStateForThisLayerOnly() noexcept override;

    void UpdateBackingStoreIncludingSubLayers(TextureMapper &textureMapper) noexcept;

    KRYS_NODISCARD TextureMapperLayer &Layer() noexcept
    {
      return *_layer;
    }

    KRYS_NODISCARD web::Color DebugBorderColor() const noexcept
    {
      return _debugBorderColor;
    }

    KRYS_NODISCARD float DebugBorderWidth() const noexcept
    {
      return _debugBorderWidth;
    }

    /// @brief What's visible of the layer, in its coordinates, as of its last flush.
    KRYS_NODISCARD const RectI &VisibleRect() const noexcept
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

    KRYS_NODISCARD static bool FiltersCanBeComposited(const web::FilterOperations &filters) noexcept;

    void UpdateVisibleRects(const RectF &visibleRect) noexcept;

    void NotifyChange(ChangeMask changeMask) noexcept;
  };

  /// @brief Creates Krystal.Booey's graphics layers.
  class GraphicsLayerFactoryTextureMapper final : public web::GraphicsLayerFactory
  {
  public:
    KRYS_NODISCARD Ref<web::GraphicsLayer>
      CreateGraphicsLayer(web::GraphicsLayerType type, web::GraphicsLayerClient &client) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_GRAPHICSLAYER(krys::boo::GraphicsLayerTextureMapper,
                                          IsGraphicsLayerTextureMapper())
