#include "Krystal.Booey/Rendering/Style/StyleCanvasImage.hpp"
#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"

namespace krys::boo
{
  StyleCanvasImage::StyleCanvasImage(css::CSSOMString &&name) noexcept
      : StyleGeneratedImage {Type::CanvasImage, StyleCanvasImage::IsFixedSize}, _name {krys::move(name)}
  {
  }

  StyleCanvasImage::~StyleCanvasImage() noexcept = default;

  bool StyleCanvasImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherCanvasImage = DynamicDowncast<StyleCanvasImage>(other);
    return otherCanvasImage && Equals(*otherCanvasImage);
  }

  bool StyleCanvasImage::Equals(const StyleCanvasImage &other) const noexcept
  {
    return _name == other._name;
  }

  Ref<css::Value> StyleCanvasImage::ComputedStyleValue(const RenderStyle &) const noexcept
  {
    return css::CanvasValue::Create(_name);
  }

  bool StyleCanvasImage::IsPending() const noexcept
  {
    return false;
  }
}
