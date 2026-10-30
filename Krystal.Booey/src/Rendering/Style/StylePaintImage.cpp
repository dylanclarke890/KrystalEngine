#include "Krystal.Booey/Rendering/Style/StylePaintImage.hpp"
#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"

namespace krys::boo
{
  StylePaintImage::StylePaintImage(css::CSSOMString &&name, Ref<css::VariableData> &&arguments) noexcept
      : StyleGeneratedImage {Type::PaintImage, StylePaintImage::IsFixedSize}, _name {krys::move(name)},
        _arguments {krys::move(arguments)}
  {
  }

  StylePaintImage::~StylePaintImage() noexcept = default;

  bool StylePaintImage::Equals(const StyleImage &other) const noexcept
  {
    // FIXME: Should probably also compare arguments?
    const auto *otherPaintImage = DynamicDowncast<StylePaintImage>(other);
    return otherPaintImage && otherPaintImage->_name == _name;
  }

  Ref<css::Value> StylePaintImage::ComputedStyleValue(const RenderStyle &) const noexcept
  {
    return css::PaintImageValue::Create(_name, _arguments);
  }

  bool StylePaintImage::IsPending() const noexcept
  {
    return false;
  }
}
