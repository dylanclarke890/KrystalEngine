#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleFilterImage::StyleFilterImage(RefPtr<StyleImage> &&image, style::Filter &&filter) noexcept
      : StyleGeneratedImage {Type::FilterImage, StyleFilterImage::IsFixedSize}, _image {krys::move(image)},
        _filter {krys::move(filter)}
  {
  }

  StyleFilterImage::~StyleFilterImage() noexcept = default;

  bool StyleFilterImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherFilterImage = DynamicDowncast<StyleFilterImage>(other);
    return otherFilterImage && Equals(*otherFilterImage);
  }

  bool StyleFilterImage::Equals(const StyleFilterImage &other) const noexcept
  {
    return EqualInputImages(other) && _filter == other._filter;
  }

  bool StyleFilterImage::EqualInputImages(const StyleFilterImage &other) const noexcept
  {
    return ArePointingToEqualData(_image, other._image);
  }

  Ref<css::Value> StyleFilterImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    return css::FilterImageValue::Create(
      _image ? _image->ComputedStyleValue(style)
             : Ref<css::Value> {css::PrimitiveValue::Create(css::ValueId::None)},
      style::ConvertToCSS(_filter, style));
  }

  bool StyleFilterImage::IsPending() const noexcept
  {
    return _image && _image->IsPending();
  }
}
