#include "Krystal.Booey/Rendering/Style/StyleMultiImage.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleMultiImage::StyleMultiImage(Type type) noexcept : StyleImage {type}
  {
  }

  StyleMultiImage::~StyleMultiImage() noexcept = default;

  bool StyleMultiImage::Equals(const StyleMultiImage &other) const noexcept
  {
    return !_isPending && !other._isPending && ArePointingToEqualData(_selectedImage, other._selectedImage);
  }

  CachedImage *StyleMultiImage::GetCachedImage() const noexcept
  {
    if (!_selectedImage)
    {
      return nullptr;
    }
    return _selectedImage->GetCachedImage();
  }

  WrappedImagePtr StyleMultiImage::Data() const noexcept
  {
    if (!_selectedImage)
    {
      return nullptr;
    }
    return _selectedImage->Data();
  }

  bool StyleMultiImage::IsPending() const noexcept
  {
    return _isPending;
  }

  bool StyleMultiImage::ErrorOccurred() const noexcept
  {
    return _selectedImage && _selectedImage->ErrorOccurred();
  }

  bool StyleMultiImage::ImageHasRelativeWidth() const noexcept
  {
    return _selectedImage && _selectedImage->ImageHasRelativeWidth();
  }

  bool StyleMultiImage::ImageHasRelativeHeight() const noexcept
  {
    return _selectedImage && _selectedImage->ImageHasRelativeHeight();
  }

  bool StyleMultiImage::UsesImageContainerSize() const noexcept
  {
    return _selectedImage && _selectedImage->UsesImageContainerSize();
  }

  float StyleMultiImage::ImageScaleFactor() const noexcept
  {
    if (!_selectedImage)
    {
      return 1;
    }
    return _selectedImage->ImageScaleFactor();
  }
}
