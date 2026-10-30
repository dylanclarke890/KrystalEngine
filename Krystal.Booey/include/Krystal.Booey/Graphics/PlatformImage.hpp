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

  /// @brief An image the host holds outside Booey, such as a texture on its GPU: only the host's renderer
  /// can draw it, and Booey can't read its pixels.
  class ExternalImage : public RefCounted<ExternalImage>
  {
    KRYS_NON_COPY_MOVABLE(ExternalImage);

  public:
    virtual ~ExternalImage() noexcept = default;

  protected:
    ExternalImage() noexcept = default;
  };

  /// @brief The platform's image: decoded sRGB pixels in memory, which the host's renderer uploads when it
  /// draws them, or an image the host holds itself.
  /// @note What `NativeImage` wraps, as Skia's `SkImage` or Cairo's surface does.
  class PlatformImage final : public RefCounted<PlatformImage>
  {
    KRYS_NON_COPY_MOVABLE(PlatformImage);

  private:
    IntSize _size;
    RefPtr<PixelData> _pixels;
    RefPtr<ExternalImage> _external;
    bool _premultipliedAlpha;

  public:
    PlatformImage(const IntSize &size, Ref<PixelData> &&pixels, bool premultipliedAlpha) noexcept
        : _size(size), _pixels(krys::move(pixels)), _premultipliedAlpha(premultipliedAlpha)
    {
    }

    PlatformImage(const IntSize &size, Ref<ExternalImage> &&external, bool premultipliedAlpha) noexcept
        : _size(size), _external(krys::move(external)), _premultipliedAlpha(premultipliedAlpha)
    {
    }

    KRYS_NODISCARD const IntSize &Size() const noexcept
    {
      return _size;
    }

    /// @brief The pixels, `Size().Width()` a row, as ARGB (alpha in the high byte); empty for an external
    /// image.
    KRYS_NODISCARD Span<const uint32> Pixels() const noexcept
    {
      if (!_pixels)
      {
        return {};
      }
      return {_pixels->Pixels.data(), static_cast<size_t>(_size.UnclampedArea())};
    }

    /// @brief The host's image, if this is one.
    KRYS_NODISCARD ExternalImage *External() const noexcept
    {
      return _external.get();
    }

    KRYS_NODISCARD bool PremultipliedAlpha() const noexcept
    {
      return _premultipliedAlpha;
    }
  };

  using PlatformImagePtr = RefPtr<PlatformImage>;
}
