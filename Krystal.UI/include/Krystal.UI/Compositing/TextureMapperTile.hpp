#pragma once

#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"
#include "Krystal.UI/Compositing/BitmapTexture.hpp"

namespace krys::boo
{
  class GraphicsLayer;
  class Image;
}

namespace krys::ui
{
  class TextureMapper;

  /// @brief A part of a backing store, painted into a texture of its own.
  class TextureMapperTile
  {
  protected:
    RefPtr<BitmapTexture> _texture;

  private:
    RectF _rect;
    /// @brief What needs painting again, in the backing store's coordinates.
    RectI _dirtyRect;

  public:
    explicit TextureMapperTile(const RectF &rect) noexcept;

    virtual ~TextureMapperTile() noexcept;

    TextureMapperTile(TextureMapperTile &&) noexcept = default;

    TextureMapperTile &operator=(TextureMapperTile &&) noexcept = default;

    KRYS_NODISCARD RefPtr<BitmapTexture> Texture() const noexcept;

    KRYS_NODISCARD RectF Rect() const noexcept
    {
      return _rect;
    }

    void SetTexture(BitmapTexture *texture) noexcept;

    void SetRect(const RectF &rect) noexcept
    {
      _rect = rect;
    }

    KRYS_NODISCARD const RectI &DirtyRect() const noexcept
    {
      return _dirtyRect;
    }

    void AddDirtyRect(const RectI &rect) noexcept
    {
      _dirtyRect.Unite(rect);
    }

    void UpdateContents(TextureMapper &textureMapper, boo::Image &image, const RectI &dirtyRect) noexcept;

    void UpdateContents(TextureMapper &textureMapper, const boo::GraphicsLayer &sourceLayer,
                        const RectI &dirtyRect, float scale = 1) noexcept;

    virtual void Paint(TextureMapper &textureMapper, const TransformationMatrix &transform, float opacity,
                       bool allEdgesExposed) noexcept;
  };
}
