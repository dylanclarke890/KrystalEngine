#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"

namespace krys::boo
{
  Ref<FilterImage> FilterImage::Create(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                                       const IntRect &absoluteImageRect, bool isAlphaImage,
                                       bool isValidPremultiplied,
                                       const DestinationColorSpace &colorSpace) noexcept
  {
    krys_debug_assert(!ImageBuffer::SizeNeedsClamping(FloatSize(absoluteImageRect.Size())));
    return AdoptRef(*new FilterImage(primitiveSubregion, imageRect, absoluteImageRect, isAlphaImage,
                                     isValidPremultiplied, colorSpace));
  }

  Ref<FilterImage> FilterImage::Create(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                                       const IntRect &absoluteImageRect,
                                       Ref<ImageBuffer> &&imageBuffer) noexcept
  {
    return AdoptRef(
      *new FilterImage(primitiveSubregion, imageRect, absoluteImageRect, krys::move(imageBuffer)));
  }

  FilterImage::FilterImage(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                           const IntRect &absoluteImageRect, bool isAlphaImage, bool isValidPremultiplied,
                           const DestinationColorSpace &colorSpace) noexcept
      : _primitiveSubregion(primitiveSubregion), _imageRect(imageRect), _absoluteImageRect(absoluteImageRect),
        _isAlphaImage(isAlphaImage), _isValidPremultiplied(isValidPremultiplied), _colorSpace(colorSpace)
  {
  }

  FilterImage::FilterImage(const FloatRect &primitiveSubregion, const FloatRect &imageRect,
                           const IntRect &absoluteImageRect, Ref<ImageBuffer> &&imageBuffer) noexcept
      : _primitiveSubregion(primitiveSubregion), _imageRect(imageRect), _absoluteImageRect(absoluteImageRect),
        _colorSpace(DestinationColorSpace::SRGB()), _imageBuffer(krys::move(imageBuffer))
  {
  }

  FloatRect FilterImage::MaxEffectRect(const Filter &filter) const noexcept
  {
    return filter.MaxEffectRect(_primitiveSubregion);
  }

  IntRect FilterImage::AbsoluteImageRectRelativeTo(const FilterImage &origin) const noexcept
  {
    return _absoluteImageRect - origin.AbsoluteImageRect().Location();
  }

  FloatPoint FilterImage::MappedAbsolutePoint(const FloatPoint &point) const noexcept
  {
    return FloatPoint(point - _absoluteImageRect.Location());
  }

  size_t FilterImage::MemoryCost() const noexcept
  {
    // Whatever holds the pixels holds four bytes for each.
    return static_cast<size_t>(_absoluteImageRect.Width()) * static_cast<size_t>(_absoluteImageRect.Height())
           * 4u;
  }
}
