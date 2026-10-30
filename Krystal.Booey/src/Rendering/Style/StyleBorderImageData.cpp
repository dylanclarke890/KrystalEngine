#include "Krystal.Booey/Rendering/Style/StyleBorderImageData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  Ref<StyleBorderImageData> StyleBorderImageData::Copy() const noexcept
  {
    return AdoptRef(*new StyleBorderImageData(*this));
  }

  StyleBorderImageData::StyleBorderImageData() noexcept
      : BorderImage {
          style::ComputedStyle::InitialBorderImageSource(), style::ComputedStyle::InitialBorderImageSlice(),
          style::ComputedStyle::InitialBorderImageWidth(), style::ComputedStyle::InitialBorderImageOutset(),
          style::ComputedStyle::InitialBorderImageRepeat()}
  {
  }

  inline StyleBorderImageData::StyleBorderImageData(const StyleBorderImageData &other) noexcept
      : RefCounted<StyleBorderImageData>(), BorderImage {other.BorderImage}
  {
  }

  bool StyleBorderImageData::operator==(const StyleBorderImageData &other) const noexcept
  {
    return BorderImage == other.BorderImage;
  }
}
