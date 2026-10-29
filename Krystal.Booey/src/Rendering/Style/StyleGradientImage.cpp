#include "Krystal.Booey/Rendering/Style/StyleGradientImage.hpp"
#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/Graphics/GradientImage.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
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

  RefPtr<Image> StyleGradientImage::GetImage(const RenderElement *renderer, const FloatSize &size,
                                             const GraphicsContext &, bool isForFirstLine) const noexcept
  {
    if (!renderer)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    if (size.IsEmpty())
    {
      return nullptr;
    }

    const auto &style = isForFirstLine ? renderer->FirstLineStyle() : renderer->Style();
    auto gradient = style::CreatePlatformGradient(_gradient, size, style);
    return GradientImage::Create(*gradient, size);
  }

  bool StyleGradientImage::KnownToBeOpaque(const RenderElement &renderer) const noexcept
  {
    return style::IsOpaque(_gradient, renderer.Style());
  }

  FloatSize StyleGradientImage::FixedSize(const RenderElement &) const noexcept
  {
    return {};
  }
}
