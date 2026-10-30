#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.UI/Compositing/TextureMapperPlatformLayer.hpp"

namespace krys::ui
{
  /// @brief A display delegate that shows a platform layer as a layer's contents.
  /// @note Every delegate Krystal.UI's layers are given is one of these: Krystal.UI makes them all.
  class PlatformLayerDisplayDelegate : public boo::GraphicsLayerContentsDisplayDelegate
  {
  private:
    TextureMapperPlatformLayer *_platformLayer;

  public:
    KRYS_NODISCARD static Ref<PlatformLayerDisplayDelegate>
      Create(TextureMapperPlatformLayer *platformLayer) noexcept
    {
      return AdoptRef(*new PlatformLayerDisplayDelegate(platformLayer));
    }

    KRYS_NODISCARD TextureMapperPlatformLayer *PlatformLayer() const noexcept
    {
      return _platformLayer;
    }

  protected:
    explicit PlatformLayerDisplayDelegate(TextureMapperPlatformLayer *platformLayer) noexcept
        : _platformLayer(platformLayer)
    {
    }
  };
}
