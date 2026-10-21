#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"

namespace krys::ui
{
  class TextureMapper;

  /// @brief What a layer draws besides its contents: a backing store, a solid colour, an image.
  class TextureMapperPlatformLayer
  {
  public:
    class Client
    {
    public:
      virtual ~Client() noexcept = default;

      virtual void PlatformLayerWillBeDestroyed() noexcept = 0;
      virtual void SetPlatformLayerNeedsDisplay() noexcept = 0;
    };

  private:
    Client *_client {nullptr};

  public:
    TextureMapperPlatformLayer() noexcept = default;
    virtual ~TextureMapperPlatformLayer() noexcept = default;

    virtual void PaintToTextureMapper(TextureMapper &textureMapper, const boo::FloatRect &targetRect,
                                      const boo::TransformationMatrix &modelViewMatrix = {},
                                      float opacity = 1.0) noexcept = 0;
    virtual void DrawBorder(TextureMapper &textureMapper, const boo::Color &color, float borderWidth,
                            const boo::FloatRect &targetRect,
                            const boo::TransformationMatrix &transform) noexcept;

    void SetClient(Client *client) noexcept
    {
      _client = client;
    }

  protected:
    KRYS_NODISCARD Client *GetClient() noexcept
    {
      return _client;
    }
  };
}
