#include "Krystal.UI/Compositing/BitmapTexturePool.hpp"

namespace krys::ui
{
  Ref<BitmapTexture> BitmapTexturePool::AcquireTexture(const SizeI &size,
                                                       EnumSet<BitmapTexture::Flags> flags) noexcept
  {
    const uint64 frame = _device.FrameNumber();
    for (auto &entry : _textures)
    {
      if (entry.Texture->GetRefCount() == 1 && entry.Texture->Size() == size
          && entry.Texture->GetFlags().contains(BitmapTexture::Flags::DepthBuffer)
               == flags.contains(BitmapTexture::Flags::DepthBuffer))
      {
        entry.Texture->Reset(size, flags);
        entry.LastUsedFrame = frame;
        return entry.Texture;
      }
    }

    _textures.push_back({.Texture = BitmapTexture::Create(_device, size, flags), .LastUsedFrame = frame});
    return _textures.back().Texture;
  }

  void BitmapTexturePool::ReleaseUnusedTextures() noexcept
  {
    const uint64 frame = _device.FrameNumber();
    std::erase_if(
      _textures, [&](const Entry &entry)
      { return entry.Texture->GetRefCount() == 1 && entry.LastUsedFrame + ReleaseUnusedFrames < frame; });
  }
}
