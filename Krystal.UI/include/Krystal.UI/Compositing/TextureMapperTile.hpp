#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
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
    boo::FloatRect _rect;
    /// @brief What needs painting again, in the backing store's coordinates.
    boo::IntRect _dirtyRect;

  public:
    explicit TextureMapperTile(const boo::FloatRect &rect) noexcept;
    virtual ~TextureMapperTile() noexcept;

    TextureMapperTile(TextureMapperTile &&) noexcept = default;
    TextureMapperTile &operator=(TextureMapperTile &&) noexcept = default;

    KRYS_NODISCARD RefPtr<BitmapTexture> Texture() const noexcept;
    KRYS_NODISCARD boo::FloatRect Rect() const noexcept
    {
      return _rect;
    }
    void SetTexture(BitmapTexture *texture) noexcept;
    void SetRect(const boo::FloatRect &rect) noexcept
    {
      _rect = rect;
    }

    KRYS_NODISCARD const boo::IntRect &DirtyRect() const noexcept
    {
      return _dirtyRect;
    }
    void AddDirtyRect(const boo::IntRect &rect) noexcept
    {
      _dirtyRect.Unite(rect);
    }

    void UpdateContents(TextureMapper &textureMapper, boo::Image &image,
                        const boo::IntRect &dirtyRect) noexcept;
    void UpdateContents(TextureMapper &textureMapper, const boo::GraphicsLayer &sourceLayer,
                        const boo::IntRect &dirtyRect, float scale = 1) noexcept;
    virtual void Paint(TextureMapper &textureMapper, const boo::TransformationMatrix &transform,
                       float opacity, bool allEdgesExposed) noexcept;
  };
}
