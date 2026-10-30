#pragma once

#include "Krystal.Booey/Graphics/PlatformImage.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.UI/Compositing/PlatformLayerDisplayDelegate.hpp"
#include "Krystal.UI/EngineTexture.hpp"
#include "Krystal.UI/TextureImage.hpp"

namespace krys::ui
{
  /// @brief Draws an engine texture as a layer's contents.
  class EngineTextureLayer final : public TextureMapperPlatformLayer
  {
  private:
    EngineTexture &_texture;

  public:
    explicit EngineTextureLayer(EngineTexture &texture) noexcept : _texture(texture)
    {
    }

    void PaintToTextureMapper(TextureMapper &textureMapper, const boo::FloatRect &targetRect,
                              const boo::TransformationMatrix &modelViewMatrix = {},
                              float opacity = 1.0) noexcept override;
  };

  /// @brief Shows an engine texture as a layer's contents, keeping the texture alive while it's shown.
  class EngineTextureDisplayDelegate final : public PlatformLayerDisplayDelegate,
                                             public CanMakeWeakPtr<EngineTextureDisplayDelegate>
  {
  private:
    Ref<EngineTexture> _texture;
    EngineTextureLayer _layer;

  public:
    KRYS_NODISCARD static Ref<EngineTextureDisplayDelegate> Create(EngineTexture &texture) noexcept
    {
      return AdoptRef(*new EngineTextureDisplayDelegate(texture));
    }

  private:
    explicit EngineTextureDisplayDelegate(EngineTexture &texture) noexcept
        : PlatformLayerDisplayDelegate(&_layer), _texture(ShareRef(texture)), _layer(texture)
    {
    }
  };

  /// @brief An engine texture as an image's platform image, drawn by the renderer without a copy.
  class EngineTextureImage final : public TextureImage
  {
  private:
    Ref<EngineTexture> _texture;

  public:
    explicit EngineTextureImage(EngineTexture &texture) noexcept : _texture(ShareRef(texture))
    {
    }

    KRYS_NODISCARD EngineTexture &Texture() const noexcept
    {
      return *_texture;
    }

    KRYS_NODISCARD gfx::TextureHandle Handle() const noexcept override
    {
      return _texture->Handle();
    }
  };
}
