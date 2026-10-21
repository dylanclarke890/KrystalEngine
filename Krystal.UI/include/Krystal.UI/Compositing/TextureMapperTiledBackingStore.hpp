#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
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
    boo::FloatSize _size;
    RefPtr<boo::Image> _image;
    float _contentsScale {1};
    bool _isScaleDirty {false};

    // A layer's tiles: their size, the rect they cover, and the rect beyond which they're dropped (in the
    // backing store's coordinates).
    boo::IntSize _tileSize;
    boo::IntRect _visibleRect;
    boo::IntRect _coverRect;
    boo::IntRect _keepRect;

  public:
    KRYS_NODISCARD static Ref<TextureMapperTiledBackingStore> Create() noexcept
    {
      return AdoptRef(*new TextureMapperTiledBackingStore);
    }
    ~TextureMapperTiledBackingStore() noexcept override = default;

    void PaintToTextureMapper(TextureMapper &textureMapper, const boo::FloatRect &targetRect,
                              const boo::TransformationMatrix &transform, float opacity) noexcept override;
    void DrawBorder(TextureMapper &textureMapper, const boo::Color &borderColor, float borderWidth,
                    const boo::FloatRect &targetRect,
                    const boo::TransformationMatrix &transform) noexcept override;
    void DrawRepaintCounter(TextureMapper &textureMapper, int32 repaintCount, const boo::Color &borderColor,
                            const boo::FloatRect &targetRect,
                            const boo::TransformationMatrix &transform) noexcept override;

    void UpdateContentsScale(float scale) noexcept;
    void UpdateContents(TextureMapper &textureMapper, boo::Image *image, const boo::FloatSize &totalSize,
                        const boo::IntRect &dirtyRect) noexcept;
    /// @brief Covers `visibleRect` (in the layer's coordinates) and a margin around it with tiles for a
    /// viewport of `viewportSize`, and paints what's dirty of them: `dirtyRect` (scaled), and new tiles.
    void UpdateContents(TextureMapper &textureMapper, const boo::GraphicsLayer &sourceLayer,
                        const boo::FloatSize &totalSize, const boo::IntRect &dirtyRect,
                        const boo::IntRect &visibleRect, const boo::IntSize &viewportSize) noexcept;
    void SetContentsToImage(boo::Image *image) noexcept;

    KRYS_NODISCARD const List<TextureMapperTile> &Tiles() const noexcept
    {
      return _tiles;
    }

  private:
    TextureMapperTiledBackingStore() noexcept = default;

    void CreateOrDestroyTilesIfNeeded(const boo::FloatSize &backingStoreSize, const boo::IntSize &tileSize,
                                      bool hasAlpha) noexcept;
    void CreateOrDestroyCoveringTiles(const boo::FloatSize &backingStoreSize, const boo::IntRect &visibleRect,
                                      const boo::IntSize &viewportSize, int32 maxTextureSize) noexcept;
    void UpdateContentsFromImageIfNeeded(TextureMapper &textureMapper) noexcept;
    KRYS_NODISCARD boo::TransformationMatrix
      AdjustedTransformForRect(const boo::FloatRect &targetRect) noexcept;

    KRYS_NODISCARD boo::IntSize ComputeTileSize(const boo::IntSize &viewportSize, int32 maxTextureSize,
                                                const boo::IntRect &contentsRect) const noexcept;
    KRYS_NODISCARD Pair<boo::IntRect, boo::IntRect>
      ComputeCoverAndKeepRect(const boo::IntRect &contentsRect) const noexcept;
    KRYS_NODISCARD boo::IntRect TileRectForPosition(const boo::IntPoint &position,
                                                    const boo::IntRect &contentsRect) const noexcept;
    KRYS_NODISCARD boo::IntPoint TilePositionForPoint(const boo::IntPoint &point) const noexcept;

    KRYS_NODISCARD boo::FloatRect Rect() const noexcept
    {
      boo::FloatRect rect(boo::FloatPoint(), _size);
      rect.Scale(_contentsScale);
      return rect;
    }
  };
}
