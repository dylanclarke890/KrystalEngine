#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/PlatformImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Colour/ColourUtilities.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <algorithm>

namespace krys::boo
{
  /// @brief The pixels an image decoder decodes a frame into: ARGB, premultiplied if asked, with the frame's
  /// rect (a GIF or APNG frame may cover part of the image).
  class ImageBackingStore
  {
  private:
    RefPtr<PixelData> _pixels;
    IntSize _size;
    IntRect _frameRect; // This will always just be the entire buffer except for GIF and PNG frames
    bool _premultiplyAlpha {true};

  public:
    KRYS_NODISCARD static UniquePtr<ImageBackingStore> Create(const IntSize &size,
                                                              bool premultiplyAlpha = true) noexcept
    {
      return UniquePtr<ImageBackingStore>(new ImageBackingStore(size, premultiplyAlpha));
    }

    KRYS_NODISCARD static UniquePtr<ImageBackingStore> Create(const ImageBackingStore &other) noexcept
    {
      return UniquePtr<ImageBackingStore>(new ImageBackingStore(other));
    }

    /// @brief An image of the pixels, sharing them.
    KRYS_NODISCARD PlatformImagePtr Image() const noexcept
    {
      return AdoptRef(*new PlatformImage(_size, ShareRef(*_pixels), _premultiplyAlpha));
    }

    bool SetSize(const IntSize &size) noexcept
    {
      if (size.IsEmpty())
      {
        return false;
      }

      _pixels = AdoptRef(*new PixelData(static_cast<size_t>(size.UnclampedArea())));
      _size = size;
      _frameRect = IntRect(IntPoint(), _size);
      Clear();
      return true;
    }

    void SetFrameRect(const IntRect &frameRect) noexcept
    {
      krys_debug_assert(!_size.IsEmpty());
      krys_debug_assert(InBounds(frameRect));
      _frameRect = frameRect;
    }

    KRYS_NODISCARD const IntSize &Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD const IntRect &FrameRect() const noexcept
    {
      return _frameRect;
    }

    void Clear() noexcept
    {
      std::ranges::fill(_pixels->Pixels, 0u);
    }

    void ClearRect(const IntRect &rect) noexcept
    {
      if (rect.IsEmpty() || !InBounds(rect))
      {
        return;
      }

      for (int32 i = 0; i < rect.Height(); ++i)
      {
        auto *row = PixelsStartingAt(rect.X(), rect.Y() + i);
        std::fill(row, row + rect.Width(), 0u);
      }
    }

    void FillRect(const IntRect &rect, uint8 r, uint8 g, uint8 b, uint8 a) noexcept
    {
      if (rect.IsEmpty() || !InBounds(rect))
      {
        return;
      }

      uint32 value = PixelValue(r, g, b, a);
      for (int32 i = 0; i < rect.Height(); ++i)
      {
        auto *row = PixelsStartingAt(rect.X(), rect.Y() + i);
        std::fill(row, row + rect.Width(), value);
      }
    }

    void RepeatFirstRow(const IntRect &rect) noexcept
    {
      if (rect.IsEmpty() || !InBounds(rect))
      {
        return;
      }

      auto *sourceRow = PixelsStartingAt(rect.X(), rect.Y());
      for (int32 i = 1; i < rect.Height(); ++i)
      {
        std::copy(sourceRow, sourceRow + rect.Width(), PixelsStartingAt(rect.X(), rect.Y() + i));
      }
    }

    KRYS_NODISCARD uint32 *PixelsStartingAt(int32 x, int32 y) noexcept
    {
      krys_debug_assert(InBounds(IntPoint(x, y)));
      return _pixels->Pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(_size.Width()) + x;
    }

    KRYS_NODISCARD uint32 &PixelAt(int32 x, int32 y) noexcept
    {
      return *PixelsStartingAt(x, y);
    }

    void SetPixel(uint32 &destination, uint8 r, uint8 g, uint8 b, uint8 a) noexcept
    {
      destination = PixelValue(r, g, b, a);
    }

    void SetPixel(int32 x, int32 y, uint8 r, uint8 g, uint8 b, uint8 a) noexcept
    {
      SetPixel(PixelAt(x, y), r, g, b, a);
    }

    void BlendPixel(uint32 &destination, uint8 r, uint8 g, uint8 b, uint8 a) noexcept
    {
      if (!a)
      {
        return;
      }

      auto pixel = AsSRGBA(PackedColour::ARGB {destination});
      auto [pixelRed, pixelGreen, pixelBlue, pixelAlpha] = pixel.Resolved();
      if (a >= 255 || !pixelAlpha)
      {
        SetPixel(destination, r, g, b, a);
        return;
      }

      if (!_premultiplyAlpha)
      {
        auto [premultipliedRed, premultipliedGreen, premultipliedBlue, premultipliedAlpha] =
          PremultipliedFlooring(pixel).Resolved();
        pixelRed = premultipliedRed;
        pixelGreen = premultipliedGreen;
        pixelBlue = premultipliedBlue;
        pixelAlpha = premultipliedAlpha;
      }

      uint32 d = 255u - a;

      r = static_cast<uint8>(FastDivideBy255(static_cast<uint16>(r * a + pixelRed * d)));
      g = static_cast<uint8>(FastDivideBy255(static_cast<uint16>(g * a + pixelGreen * d)));
      b = static_cast<uint8>(FastDivideBy255(static_cast<uint16>(b * a + pixelBlue * d)));
      a = static_cast<uint8>(a + FastDivideBy255(static_cast<uint16>(d * pixelAlpha)));

      auto result = SRGBA<uint8> {r, g, b, a};

      if (!_premultiplyAlpha)
      {
        result = Unpremultiplied(result);
      }

      destination = PackedColour::ARGB {result}.Value;
    }

    KRYS_NODISCARD static bool IsOverSize(const IntSize &size) noexcept
    {
      constexpr uint64 maxPixels = (1u << 29) - 1;
      uint64 pixels = static_cast<uint64>(size.Width()) * static_cast<uint64>(size.Height());
      return pixels > maxPixels;
    }

  private:
    ImageBackingStore(const IntSize &size, bool premultiplyAlpha = true) noexcept
        : _premultiplyAlpha(premultiplyAlpha)
    {
      krys_debug_assert(!size.IsEmpty() && !IsOverSize(size));
      SetSize(size);
    }

    ImageBackingStore(const ImageBackingStore &other) noexcept
        : _pixels(AdoptRef(*new PixelData(List<uint32>(other._pixels->Pixels)))), _size(other._size),
          _frameRect(other._frameRect), _premultiplyAlpha(other._premultiplyAlpha)
    {
      krys_debug_assert(!_size.IsEmpty() && !IsOverSize(_size));
    }

    KRYS_NODISCARD bool InBounds(const IntPoint &point) const noexcept
    {
      return IntRect(IntPoint(), _size).Contains(point);
    }

    KRYS_NODISCARD bool InBounds(const IntRect &rect) const noexcept
    {
      return IntRect(IntPoint(), _size).Contains(rect);
    }

    KRYS_NODISCARD uint32 PixelValue(uint8 r, uint8 g, uint8 b, uint8 a) const noexcept
    {
      if (_premultiplyAlpha && !a)
      {
        return 0;
      }

      auto result = SRGBA<uint8> {r, g, b, a};

      if (_premultiplyAlpha && a < 255)
      {
        result = PremultipliedFlooring(result);
      }

      return PackedColour::ARGB {result}.Value;
    }
  };
}
