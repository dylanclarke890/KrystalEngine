#pragma once

#include "Krystal.UI/Compositing/TextureMapperPlatformLayer.hpp"

namespace krys::ui
{
  /// @brief A layer's painted contents, drawn by the compositor.
  class TextureMapperBackingStore : public TextureMapperPlatformLayer
  {
  public:
    ~TextureMapperBackingStore() noexcept override = default;

    void PaintToTextureMapper(TextureMapper &textureMapper, const RectF &targetRect,
                              const TransformationMatrix &modelViewMatrix,
                              float opacity) noexcept override = 0;

    virtual void DrawRepaintCounter(TextureMapper &, int32, const boo::Color &, const RectF &,
                                    const TransformationMatrix &) noexcept
    {
    }

  protected:
    KRYS_NODISCARD static bool AllTileEdgesExposed(const RectF &totalRect, const RectF &tileRect) noexcept;
  };

  /// @brief A backing store of one colour.
  class TextureMapperSolidColorLayer : public TextureMapperBackingStore
  {
  private:
    boo::Color _color;

  public:
    void SetColor(boo::Color color) noexcept
    {
      _color = krys::move(color);
    }

    void PaintToTextureMapper(TextureMapper &textureMapper, const RectF &targetRect,
                              const TransformationMatrix &transform, float opacity) noexcept override;
  };
}
