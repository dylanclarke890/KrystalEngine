#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  /// @brief Decoded pixels, one 32-bit ARGB value each (alpha in the high byte), row after row; shared by an
  /// image decoder's backing store and the images it makes from it.
  class PixelData final : public RefCounted<PixelData>
  {
    KRYS_NON_COPY_MOVABLE(PixelData);

  public:
    List<uint32> Pixels;

    explicit PixelData(size_t pixelCount) noexcept : Pixels(pixelCount, 0u)
    {
    }

    explicit PixelData(List<uint32> &&pixels) noexcept : Pixels(krys::move(pixels))
    {
    }
  };

  /// @brief The platform's image: decoded sRGB pixels in memory, which the host's renderer uploads when it
  /// draws them.
  /// @note What `NativeImage` wraps, as Skia's `SkImage` or Cairo's surface does.
  class PlatformImage final : public RefCounted<PlatformImage>
  {
    KRYS_NON_COPY_MOVABLE(PlatformImage);

  private:
    IntSize _size;
    Ref<PixelData> _pixels;
    bool _premultipliedAlpha;

  public:
    PlatformImage(const IntSize &size, Ref<PixelData> &&pixels, bool premultipliedAlpha) noexcept
        : _size(size), _pixels(krys::move(pixels)), _premultipliedAlpha(premultipliedAlpha)
    {
    }

    KRYS_NODISCARD const IntSize &Size() const noexcept
    {
      return _size;
    }

    /// @brief The pixels, `Size().Width()` a row, as ARGB (alpha in the high byte).
    KRYS_NODISCARD Span<const uint32> Pixels() const noexcept
    {
      return {_pixels->Pixels.data(), static_cast<size_t>(_size.UnclampedArea())};
    }

    KRYS_NODISCARD bool PremultipliedAlpha() const noexcept
    {
      return _premultipliedAlpha;
    }
  };

  using PlatformImagePtr = RefPtr<PlatformImage>;
}
