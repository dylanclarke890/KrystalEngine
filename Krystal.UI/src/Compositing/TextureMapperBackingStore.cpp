#include "Krystal.UI/Compositing/TextureMapperBackingStore.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"

namespace krys::ui
{
  void TextureMapperPlatformLayer::DrawBorder(TextureMapper &textureMapper, const boo::Color &color,
                                              float borderWidth, const boo::FloatRect &targetRect,
                                              const boo::TransformationMatrix &transform) noexcept
  {
    textureMapper.DrawBorder(color, borderWidth, targetRect, transform);
  }

  bool TextureMapperBackingStore::AllTileEdgesExposed(const boo::FloatRect &totalRect,
                                                      const boo::FloatRect &tileRect) noexcept
  {
    return !tileRect.X() && !tileRect.Y() && tileRect.Width() + tileRect.X() >= totalRect.Width()
           && tileRect.Height() + tileRect.Y() >= totalRect.Height();
  }

  void TextureMapperSolidColorLayer::PaintToTextureMapper(TextureMapper &textureMapper,
                                                          const boo::FloatRect &targetRect,
                                                          const boo::TransformationMatrix &transform,
                                                          float opacity) noexcept
  {
    textureMapper.DrawSolidColor(targetRect, transform, _color.ColorWithAlphaMultipliedBy(opacity), true);
  }
}
