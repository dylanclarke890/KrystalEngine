#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"

namespace krys::boo
{
  StyleGeneratedImage::StyleGeneratedImage(StyleImage::Type type, bool fixedSize) noexcept
      : StyleImage {type}, _fixedSize {fixedSize}
  {
  }

  StyleGeneratedImage::~StyleGeneratedImage() noexcept = default;
}
