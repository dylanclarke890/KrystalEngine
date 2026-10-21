#pragma once

#include "Krystal.UI/Compositing/TextureMapperPlatformLayer.hpp"

namespace krys::ui
{
  /// @brief A layer's painted contents, drawn by the compositor.
  class TextureMapperBackingStore : public TextureMapperPlatformLayer
  {
  public:
    ~TextureMapperBackingStore() noexcept override = default;

    void PaintToTextureMapper(TextureMapper &textureMapper, const boo::FloatRect &targetRect,
                              const boo::TransformationMatrix &modelViewMatrix,
                              float opacity) noexcept override = 0;
    virtual void DrawRepaintCounter(TextureMapper &, int32, const boo::Color &, const boo::FloatRect &,
                                    const boo::TransformationMatrix &) noexcept
    {
    }

  protected:
    KRYS_NODISCARD static bool AllTileEdgesExposed(const boo::FloatRect &totalRect,
                                                   const boo::FloatRect &tileRect) noexcept;
  };

  /// @brief A backing store of one colour.
  class TextureMapperSolidColorLayer : public TextureMapperBackingStore
  {
  private:
    boo::Color _color;

  public:
    void SetColor(boo::Color color) noexcept
    {
      _color = color;
    }

    void PaintToTextureMapper(TextureMapper &textureMapper, const boo::FloatRect &targetRect,
                              const boo::TransformationMatrix &transform, float opacity) noexcept override;
  };
}
