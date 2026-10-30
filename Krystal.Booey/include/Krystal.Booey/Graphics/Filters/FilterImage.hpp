#pragma once

#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Filter;

  /// @brief An input or result of a filter effect: where it is, and what holds its pixels.
  /// @note A filter's source is an image buffer. Effects' results are held by the accelerated backend that
  /// applied them, as a handle of its own; the backend converts an image between colour spaces as it reads
  /// it.
  // FIXME: Pixel buffers (pixelBuffer, getPixelBuffer, copyPixelBuffer, correctPremultipliedPixelBuffer) and
  // results in image buffers, once filters can be applied in software.
  class FilterImage : public RefCounted<FilterImage>
  {
  private:
    FloatRect _primitiveSubregion;
    FloatRect _imageRect;
    IntRect _absoluteImageRect;
    bool _isAlphaImage {false};
    bool _isValidPremultiplied {true};
    DestinationColorSpace _colorSpace;
    RefPtr<ImageBuffer> _imageBuffer;
    Maybe<uint32> _acceleratedImage;

  public:
    KRYS_NODISCARD static Ref<FilterImage> Create(const FloatRect &primitiveSubregion,
                                                  const FloatRect &imageRect,
                                                  const IntRect &absoluteImageRect, bool isAlphaImage,
                                                  bool isValidPremultiplied,
                                                  const DestinationColorSpace &colorSpace) noexcept;
    KRYS_NODISCARD static Ref<FilterImage> Create(const FloatRect &primitiveSubregion,
                                                  const FloatRect &imageRect,
                                                  const IntRect &absoluteImageRect,
                                                  Ref<ImageBuffer> &&imageBuffer) noexcept;

    // The return values are in filter coordinates.
    KRYS_NODISCARD FloatRect PrimitiveSubregion() const noexcept
    {
      return _primitiveSubregion;
    }
    KRYS_NODISCARD FloatRect MaxEffectRect(const Filter &filter) const noexcept;
    KRYS_NODISCARD FloatRect ImageRect() const noexcept
    {
      return _imageRect;
    }

    // The return values are in user-space coordinates.
    KRYS_NODISCARD IntRect AbsoluteImageRect() const noexcept
    {
      return _absoluteImageRect;
    }
    KRYS_NODISCARD IntRect AbsoluteImageRectRelativeTo(const FilterImage &origin) const noexcept;
    KRYS_NODISCARD FloatPoint MappedAbsolutePoint(const FloatPoint &point) const noexcept;

    KRYS_NODISCARD bool IsAlphaImage() const noexcept
    {
      return _isAlphaImage;
    }
    KRYS_NODISCARD bool IsValidPremultiplied() const noexcept
    {
      return _isValidPremultiplied;
    }
    KRYS_NODISCARD const DestinationColorSpace &ColorSpace() const noexcept
    {
      return _colorSpace;
    }

    KRYS_NODISCARD size_t MemoryCost() const noexcept;

    /// @brief The image buffer holding the image's pixels: a filter's source.
    KRYS_NODISCARD ImageBuffer *GetImageBuffer() const noexcept
    {
      return _imageBuffer.get();
    }

    /// @brief The accelerated backend's handle to the image's pixels, once it has applied the effect whose
    /// result this is (or read the source).
    KRYS_NODISCARD Maybe<uint32> AcceleratedImage() const noexcept
    {
      return _acceleratedImage;
    }
    void SetAcceleratedImage(uint32 image) noexcept
    {
      _acceleratedImage = image;
    }

    /// @brief Nothing: the accelerated backend converts the image to the colour space each effect operates
    /// in as it reads it.
    void TransformToColorSpace(const DestinationColorSpace &) noexcept
    {
    }

  private:
    FilterImage(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                const IntRect &absoluteImageRect, bool isAlphaImage, bool isValidPremultiplied,
                const DestinationColorSpace &colorSpace) noexcept;
    FilterImage(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                const IntRect &absoluteImageRect, Ref<ImageBuffer> &&imageBuffer) noexcept;
  };

  using FilterImageVector = List<Ref<FilterImage>>;
}
