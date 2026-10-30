#include "Krystal.UI/Compositing/BitmapTexture.hpp"

namespace krys::ui
{
  Ref<BitmapTexture> BitmapTexture::Create(gfx::Device &device, const boo::IntSize &size,
                                           EnumSet<Flags> flags) noexcept
  {
    return AdoptRef(*new BitmapTexture(device, size, flags));
  }

  BitmapTexture::BitmapTexture(gfx::Device &device, const boo::IntSize &size, EnumSet<Flags> flags) noexcept
      : _device(device), _flags(flags), _size(size)
  {
    CreateTexture();
  }

  BitmapTexture::~BitmapTexture() noexcept
  {
    DestroyTexture();
  }

  void BitmapTexture::Reset(const boo::IntSize &size, EnumSet<Flags> flags) noexcept
  {
    _flags = flags;
    if (size == _size && _texture.IsValid())
    {
      return;
    }
    DestroyTexture();
    _size = size;
    CreateTexture();
  }

  void BitmapTexture::CreateTexture() noexcept
  {
    if (_size.IsEmpty())
    {
      return;
    }
    auto texture =
      _device.CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                             .Width = static_cast<uint32>(_size.Width()),
                             .Height = static_cast<uint32>(_size.Height()),
                             .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                             .DebugName = u8"Bitmap texture"});
    if (texture)
    {
      _texture = *texture;
    }
  }

  void BitmapTexture::DestroyTexture() noexcept
  {
    if (_texture.IsValid())
    {
      _device.Destroy(_texture);
      _texture = {};
    }
  }
}
