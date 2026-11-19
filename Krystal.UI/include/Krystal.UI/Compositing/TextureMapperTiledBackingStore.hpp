#pragma once

#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.UI/Compositing/TextureMapperBackingStore.hpp"
#include "Krystal.UI/Compositing/TextureMapperTile.hpp"

namespace krys::boo
{
  class GraphicsLayer;
  class Image;
}

namespace krys::ui
{
  /// @brief A layer's contents in tiles. An image's tiles cover all of it; a layer's cover what's visible of
  /// it and a margin around that, and are painted where they're dirty.
  class TextureMapperTiledBackingStore final : public RefCounted<TextureMapperTiledBackingStore>,
                                               public TextureMapperBackingStore
  {
  private:
    List<TextureMapperTile> _tiles;
    SizeF _size;
    RefPtr<boo::Image> _image;
    float _contentsScale {1};
    bool _isScaleDirty {false};

    // A layer's tiles: their size, the rect they cover, and the rect beyond which they're dropped (in the
    // backing store's coordinates).
    SizeI _tileSize;
    RectI _visibleRect;
    RectI _coverRect;
    RectI _keepRect;

  public:
    KRYS_NODISCARD static Ref<TextureMapperTiledBackingStore> Create() noexcept
    {
      return AdoptRef(*new TextureMapperTiledBackingStore);
    }

    ~TextureMapperTiledBackingStore() noexcept override = default;

    void PaintToTextureMapper(TextureMapper &textureMapper, const RectF &targetRect,
                              const TransformationMatrix &transform, float opacity) noexcept override;

    void DrawBorder(TextureMapper &textureMapper, const boo::Color &borderColor, float borderWidth,
                    const RectF &targetRect, const TransformationMatrix &transform) noexcept override;

    void DrawRepaintCounter(TextureMapper &textureMapper, int32 repaintCount, const boo::Color &borderColor,
                            const RectF &targetRect, const TransformationMatrix &transform) noexcept override;

    void UpdateContentsScale(float scale) noexcept;

    void UpdateContents(TextureMapper &textureMapper, boo::Image *image, const SizeF &totalSize,
                        const RectI &dirtyRect) noexcept;

    /// @brief Covers `visibleRect` (in the layer's coordinates) and a margin around it with tiles for a
    /// viewport of `viewportSize`, and paints what's dirty of them: `dirtyRect` (scaled), and new tiles.
    void UpdateContents(TextureMapper &textureMapper, const boo::GraphicsLayer &sourceLayer,
                        const SizeF &totalSize, const RectI &dirtyRect, const RectI &visibleRect,
                        const SizeI &viewportSize) noexcept;

    void SetContentsToImage(boo::Image *image) noexcept;

    KRYS_NODISCARD const List<TextureMapperTile> &Tiles() const noexcept
    {
      return _tiles;
    }

  private:
    TextureMapperTiledBackingStore() noexcept = default;

    void CreateOrDestroyTilesIfNeeded(const SizeF &backingStoreSize, const SizeI &tileSize,
                                      bool hasAlpha) noexcept;

    void CreateOrDestroyCoveringTiles(const SizeF &backingStoreSize, const RectI &visibleRect,
                                      const SizeI &viewportSize, int32 maxTextureSize) noexcept;

    void UpdateContentsFromImageIfNeeded(TextureMapper &textureMapper) noexcept;

    KRYS_NODISCARD TransformationMatrix AdjustedTransformForRect(const RectF &targetRect) noexcept;

    KRYS_NODISCARD static SizeI ComputeTileSize(const SizeI &viewportSize, int32 maxTextureSize,
                                                const RectI &contentsRect) noexcept;

    KRYS_NODISCARD Pair<RectI, RectI> ComputeCoverAndKeepRect(const RectI &contentsRect) const noexcept;

    KRYS_NODISCARD RectI TileRectForPosition(const PointI &position,
                                             const RectI &contentsRect) const noexcept;

    KRYS_NODISCARD PointI TilePositionForPoint(const PointI &point) const noexcept;

    KRYS_NODISCARD RectF Rect() const noexcept
    {
      RectF rect(PointF(), _size);
      rect.Scale(_contentsScale);
      return rect;
    }
  };
}
