#pragma once

#include "Krystal.Booey/Graphics/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/HTML/Canvas/EngineTexture.hpp"

namespace krys::boo::tests
{
  /// @brief A delegate that shows nothing: compositing tests check which layer has it.
  class FakeDisplayDelegate final : public GraphicsLayerContentsDisplayDelegate
  {
  };

  /// @brief An engine texture in memory, of one colour, with a delegate of its own.
  class FakeEngineTexture final : public html::EngineTexture
  {
  public:
    IntSize TextureSize;
    Ref<FakeDisplayDelegate> Delegate {AdoptRef(*new FakeDisplayDelegate())};
    RefPtr<NativeImage> Image;

    explicit FakeEngineTexture(IntSize size, uint32 argb = 0xFF'33'66'99u) noexcept : TextureSize(size)
    {
      auto pixels = CreateRef<PixelData>(static_cast<size_t>(size.UnclampedArea()));
      std::fill(pixels->Pixels.begin(), pixels->Pixels.end(), argb);
      Image = NativeImage::Create(CreateRefPtr<PlatformImage>(size, krys::move(pixels), true));
    }

    KRYS_NODISCARD IntSize Size() const noexcept override
    {
      return TextureSize;
    }

    KRYS_NODISCARD RefPtr<GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() noexcept override
    {
      return Delegate;
    }

    KRYS_NODISCARD RefPtr<NativeImage> GetNativeImage() noexcept override
    {
      return Image;
    }
  };
}
