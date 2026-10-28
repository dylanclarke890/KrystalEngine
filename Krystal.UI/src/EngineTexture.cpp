#include "Krystal.UI/EngineTexture.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.UI/Compositing/EngineTextureLayer.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"

namespace krys::ui
{
  void EngineTextureLayer::PaintToTextureMapper(TextureMapper &textureMapper,
                                                const boo::FloatRect &targetRect,
                                                const boo::TransformationMatrix &modelViewMatrix,
                                                float opacity) noexcept
  {
    textureMapper.DrawTexture(_texture.Handle(), targetRect, modelViewMatrix, opacity, _texture.Alpha());
  }

  EngineTexture::EngineTexture(gfx::TextureHandle texture, const boo::IntSize &size,
                               AlphaPremultiplication alpha) noexcept
      : _texture(texture), _size(size), _alpha(alpha)
  {
  }

  Ref<EngineTexture> EngineTexture::Create(gfx::TextureHandle texture, const boo::IntSize &size,
                                           AlphaPremultiplication alpha) noexcept
  {
    return AdoptRef(*new EngineTexture(texture, size, alpha));
  }

  RefPtr<boo::GraphicsLayerContentsDisplayDelegate> EngineTexture::LayerContentsDisplayDelegate() noexcept
  {
    // One delegate while any layer shows it, so the layer keeps the same contents.
    if (auto *delegate = _delegate.get())
    {
      return ShareRefPtr<boo::GraphicsLayerContentsDisplayDelegate>(delegate);
    }
    auto delegate = EngineTextureDisplayDelegate::Create(*this);
    _delegate = CreateWeakPtr(delegate.get());
    return delegate;
  }

  RefPtr<boo::NativeImage> EngineTexture::GetNativeImage() noexcept
  {
    if (auto *image = _image.get())
    {
      return ShareRefPtr(image);
    }
    Ref<boo::ExternalImage> external = CreateRef<EngineTextureImage>(*this);
    auto platformImage = CreateRefPtr<boo::PlatformImage>(_size, krys::move(external),
                                                          _alpha == AlphaPremultiplication::Premultiplied);
    auto image = boo::NativeImage::Create(krys::move(platformImage));
    _image = WeakPtr<boo::NativeImage>::WithRef(*image);
    return image;
  }
}
