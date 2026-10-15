#include "Krystal.Booey/Rendering/Style/StyleGradientImage.hpp"
#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo
{
  StyleGradientImage::StyleGradientImage(style::Gradient &&gradient) noexcept
      : StyleGeneratedImage {Type::GradientImage, StyleGradientImage::IsFixedSize},
        _gradient {krys::move(gradient)}, _knownCacheableBarringFilter {style::StopsAreCacheable(_gradient)}
  {
  }

  StyleGradientImage::~StyleGradientImage() noexcept = default;

  bool StyleGradientImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherGradientImage = DynamicDowncast<StyleGradientImage>(other);
    return otherGradientImage && Equals(*otherGradientImage);
  }

  bool StyleGradientImage::Equals(const StyleGradientImage &other) const noexcept
  {
    return _gradient == other._gradient;
  }

  Ref<css::Value> StyleGradientImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    return css::GradientValue::Create(style::ConvertToCSS(_gradient, style));
  }

  bool StyleGradientImage::IsPending() const noexcept
  {
    return false;
  }
}
