#include "Krystal.UI/Compositing/TextureMapperTile.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"

namespace krys::ui
{
  TextureMapperTile::TextureMapperTile(const boo::FloatRect &rect) noexcept : _rect(rect)
  {
  }

  TextureMapperTile::~TextureMapperTile() noexcept = default;

  RefPtr<BitmapTexture> TextureMapperTile::Texture() const noexcept
  {
    return _texture;
  }

  void TextureMapperTile::SetTexture(BitmapTexture *texture) noexcept
  {
    _texture = ShareRefPtr(texture);
  }

  void TextureMapperTile::UpdateContents(TextureMapper &textureMapper, boo::Image &image,
                                         const boo::IntRect &dirtyRect) noexcept
  {
    boo::IntRect targetRect = boo::EnclosingIntRect(_rect);
    targetRect.Intersect(dirtyRect);
    if (targetRect.IsEmpty())
    {
      return;
    }
    boo::IntPoint sourceOffset = targetRect.Location();

    // Normalize sourceRect to the buffer's coordinates.
    sourceOffset.Move(-dirtyRect.X(), -dirtyRect.Y());

    // Normalize targetRect to the texture's coordinates.
    targetRect.Move(-static_cast<int32>(_rect.X()), -static_cast<int32>(_rect.Y()));
    if (!_texture)
    {
      EnumSet<BitmapTexture::Flags> flags;
      if (!image.CurrentFrameKnownToBeOpaque())
      {
        flags.Add(BitmapTexture::Flags::SupportsAlpha);
      }
      _texture = BitmapTexture::Create(textureMapper.Device(), boo::EnclosingIntRect(_rect).Size(), flags);
    }

    textureMapper.UpdateContents(*_texture, image, targetRect, sourceOffset);
  }

  void TextureMapperTile::UpdateContents(TextureMapper &textureMapper, const boo::GraphicsLayer &sourceLayer,
                                         const boo::IntRect &dirtyRect, float scale) noexcept
  {
    boo::IntRect targetRect = boo::EnclosingIntRect(_rect);
    targetRect.Intersect(dirtyRect);
    _dirtyRect = {};
    if (targetRect.IsEmpty())
    {
      return;
    }
    boo::IntPoint sourceOffset = targetRect.Location();

    // Normalize targetRect to the texture's coordinates.
    targetRect.Move(-static_cast<int32>(_rect.X()), -static_cast<int32>(_rect.Y()));
    if (!_texture)
    {
      _texture = BitmapTexture::Create(textureMapper.Device(), boo::EnclosingIntRect(_rect).Size(),
                                       {BitmapTexture::Flags::SupportsAlpha});
    }

    textureMapper.UpdateContents(*_texture, sourceLayer, targetRect, sourceOffset, scale);
  }

  void TextureMapperTile::Paint(TextureMapper &textureMapper, const boo::TransformationMatrix &transform,
                                float opacity, bool allEdgesExposed) noexcept
  {
    if (auto texture = Texture())
    {
      textureMapper.DrawTexture(*texture, Rect(), transform, opacity,
                                allEdgesExposed ? TextureMapper::AllEdgesExposed::Yes
                                                : TextureMapper::AllEdgesExposed::No);
    }
  }
}
