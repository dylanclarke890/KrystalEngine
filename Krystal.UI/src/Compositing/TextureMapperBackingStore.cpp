#include "Krystal.UI/Compositing/TextureMapperBackingStore.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"

namespace krys::ui
{
  void TextureMapperPlatformLayer::DrawBorder(TextureMapper &textureMapper, const boo::Color &color,
                                              float borderWidth, const RectF &targetRect,
                                              const TransformationMatrix &transform) noexcept
  {
    textureMapper.DrawBorder(color, borderWidth, targetRect, transform);
  }

  bool TextureMapperBackingStore::AllTileEdgesExposed(const RectF &totalRect, const RectF &tileRect) noexcept
  {
    return !tileRect.X() && !tileRect.Y() && tileRect.Width() + tileRect.X() >= totalRect.Width()
           && tileRect.Height() + tileRect.Y() >= totalRect.Height();
  }

  void TextureMapperSolidColorLayer::PaintToTextureMapper(TextureMapper &textureMapper,
                                                          const RectF &targetRect,
                                                          const TransformationMatrix &transform,
                                                          float opacity) noexcept
  {
    textureMapper.DrawSolidColor(targetRect, transform, _color.ColorWithAlphaMultipliedBy(opacity), true);
  }
}
