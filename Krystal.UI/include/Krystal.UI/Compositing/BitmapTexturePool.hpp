#pragma once

#include "Krystal.Core/Types/List.hpp"
#include "Krystal.UI/Compositing/BitmapTexture.hpp"

namespace krys::ui
{
  /// @brief Textures for surfaces, reused while nothing else holds them, and released after a while unused.
  class BitmapTexturePool
  {
    KRYS_NON_COPY_MOVABLE(BitmapTexturePool);

  private:
    struct Entry
    {
      Ref<BitmapTexture> Texture;
      uint64 LastUsedFrame {0};
    };

    /// @brief How many frames a texture stays unused before it's released.
    static constexpr uint64 ReleaseUnusedFrames = 180;

    gfx::Device &_device;
    List<Entry> _textures;

  public:
    explicit BitmapTexturePool(gfx::Device &device) noexcept : _device(device)
    {
    }

    KRYS_NODISCARD Ref<BitmapTexture> AcquireTexture(const boo::IntSize &size,
                                                     EnumSet<BitmapTexture::Flags> flags) noexcept;

    /// @brief Releases the textures unused for a while.
    void ReleaseUnusedTextures() noexcept;
  };
}
