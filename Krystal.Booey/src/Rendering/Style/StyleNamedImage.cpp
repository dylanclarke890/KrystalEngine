#include "Krystal.Booey/Rendering/Style/StyleNamedImage.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"

namespace krys::boo
{
  StyleNamedImage::StyleNamedImage(css::CSSOMString &&name) noexcept
      : StyleGeneratedImage {Type::NamedImage, StyleNamedImage::IsFixedSize}, _name {krys::move(name)}
  {
  }

  StyleNamedImage::~StyleNamedImage() noexcept = default;

  bool StyleNamedImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherNamedImage = DynamicDowncast<StyleNamedImage>(other);
    return otherNamedImage && Equals(*otherNamedImage);
  }

  bool StyleNamedImage::Equals(const StyleNamedImage &other) const noexcept
  {
    return _name == other._name;
  }

  Ref<css::Value> StyleNamedImage::ComputedStyleValue(const RenderStyle &) const noexcept
  {
    return css::NamedImageValue::Create(_name);
  }

  bool StyleNamedImage::IsPending() const noexcept
  {
    return false;
  }
}
