#include "Krystal.Booey/Rendering/Style/StyleInvalidImage.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"

namespace krys::boo
{
  Ref<StyleInvalidImage> StyleInvalidImage::Create() noexcept
  {
    return AdoptRef(*new StyleInvalidImage);
  }

  StyleInvalidImage::~StyleInvalidImage() noexcept = default;

  StyleInvalidImage::StyleInvalidImage() noexcept
      : StyleGeneratedImage(StyleImage::Type::InvalidImage, IsFixedSize)
  {
  }

  Ref<css::Value> StyleInvalidImage::ComputedStyleValue(const RenderStyle &) const noexcept
  {
    return css::ImageValue::Create();
  }
}
