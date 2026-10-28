#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Colour/ColourUtilities.hpp"

namespace krys::boo
{
  RefPtr<NativeImage> NativeImage::Create(PlatformImagePtr &&platformImage) noexcept
  {
    if (!platformImage)
    {
      return nullptr;
    }

    return AdoptRef(*new NativeImage(krys::move(platformImage)));
  }

  RefPtr<NativeImage> NativeImage::CreateTransient(PlatformImagePtr &&image) noexcept
  {
    return Create(krys::move(image));
  }

  NativeImage::NativeImage(PlatformImagePtr &&platformImage) noexcept
      : _platformImage(krys::move(platformImage))
  {
  }

  NativeImage::~NativeImage() noexcept = default;

  const PlatformImagePtr &NativeImage::GetPlatformImage() const noexcept
  {
    return _platformImage;
  }

  IntSize NativeImage::Size() const noexcept
  {
    return _platformImage ? _platformImage->Size() : IntSize();
  }

  bool NativeImage::HasAlpha() const noexcept
  {
    // Decoded images keep an alpha channel.
    return true;
  }

  Maybe<Color> NativeImage::SinglePixelSolidColor() const noexcept
  {
    if (Size() != IntSize(1, 1) || _platformImage->External())
    {
      return null;
    }

    auto pixel = AsSRGBA(PackedColour::ARGB {_platformImage->Pixels()[0]});
    if (_platformImage->PremultipliedAlpha())
    {
      pixel = Unpremultiplied(pixel);
    }
    return Color(pixel);
  }

  void NativeImage::ReplacePlatformImage(PlatformImagePtr &&platformImage) noexcept
  {
    krys_debug_assert(platformImage);
    _platformImage = krys::move(platformImage);
  }
}
