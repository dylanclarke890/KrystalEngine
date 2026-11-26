#include "Krystal.Booey/EngineTexture.hpp"
#include "Krystal.Booey/Compositing/EngineTextureLayer.hpp"
#include "Krystal.Booey/Compositing/TextureMapper.hpp"
#include "Krystal.Web/Graphics/Images/NativeImage.hpp"

namespace krys::boo
{
  void EngineTextureLayer::PaintToTextureMapper(TextureMapper &textureMapper, const RectF &targetRect,
                                                const TransformationMatrix &modelViewMatrix,
                                                float opacity) noexcept
  {
    textureMapper.DrawTexture(_texture.Handle(), _texture.Size(), targetRect, modelViewMatrix, opacity,
                              _texture.Alpha());
  }

  EngineTexture::EngineTexture(gfx::TextureHandle texture, const SizeI &size,
                               AlphaPremultiplication alpha) noexcept
      : _texture(texture), _size(size), _alpha(alpha)
  {
  }

  Ref<EngineTexture> EngineTexture::Create(gfx::TextureHandle texture, const SizeI &size,
                                           AlphaPremultiplication alpha) noexcept
  {
    return AdoptRef(*new EngineTexture(texture, size, alpha));
  }

  RefPtr<web::GraphicsLayerContentsDisplayDelegate> EngineTexture::LayerContentsDisplayDelegate() noexcept
  {
    // One delegate while any layer shows it, so the layer keeps the same contents.
    if (auto *delegate = _delegate.get())
    {
      return ShareRefPtr<web::GraphicsLayerContentsDisplayDelegate>(delegate);
    }
    auto delegate = EngineTextureDisplayDelegate::Create(*this);
    _delegate = CreateWeakPtr(delegate.get());
    return delegate;
  }

  RefPtr<web::NativeImage> EngineTexture::GetNativeImage() noexcept
  {
    if (auto *image = _image.get())
    {
      return ShareRefPtr(image);
    }
    Ref<web::ExternalImage> external = CreateRef<EngineTextureImage>(*this);
    auto platformImage = CreateRefPtr<web::PlatformImage>(_size, krys::move(external),
                                                          _alpha == AlphaPremultiplication::Premultiplied);
    auto image = web::NativeImage::Create(krys::move(platformImage));
    _image = CreateWeakPtr(image.get());
    return image;
  }
}
