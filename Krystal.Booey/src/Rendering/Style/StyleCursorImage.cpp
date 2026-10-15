#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  Ref<StyleCursorImage> StyleCursorImage::Create(const Ref<StyleImage> &image, Maybe<IntPoint> hotSpot,
                                                 const style::URL &originalURL) noexcept
  {
    return AdoptRef(*new StyleCursorImage(image, hotSpot, originalURL));
  }

  Ref<StyleCursorImage> StyleCursorImage::Create(Ref<StyleImage> &&image, Maybe<IntPoint> hotSpot,
                                                 style::URL &&originalURL) noexcept
  {
    return AdoptRef(*new StyleCursorImage(krys::move(image), hotSpot, krys::move(originalURL)));
  }

  StyleCursorImage::StyleCursorImage(const Ref<StyleImage> &image, Maybe<IntPoint> hotSpot,
                                     const style::URL &originalURL) noexcept
      : StyleMultiImage {Type::CursorImage}, _image {image}, _hotSpot {hotSpot}, _originalURL {originalURL}
  {
  }

  StyleCursorImage::StyleCursorImage(Ref<StyleImage> &&image, Maybe<IntPoint> hotSpot,
                                     style::URL &&originalURL) noexcept
      : StyleMultiImage {Type::CursorImage}, _image {krys::move(image)}, _hotSpot {hotSpot},
        _originalURL {krys::move(originalURL)}
  {
  }

  StyleCursorImage::~StyleCursorImage() noexcept = default;

  bool StyleCursorImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherCursorImage = DynamicDowncast<StyleCursorImage>(other);
    return otherCursorImage && Equals(*otherCursorImage);
  }

  bool StyleCursorImage::Equals(const StyleCursorImage &other) const noexcept
  {
    return EqualInputImages(other) && StyleMultiImage::Equals(other);
  }

  bool StyleCursorImage::EqualInputImages(const StyleCursorImage &other) const noexcept
  {
    return ArePointingToEqualData(_image, other._image);
  }

  Ref<css::Value> StyleCursorImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    RefPtr<css::Value> hotSpot;
    if (_hotSpot)
    {
      hotSpot = css::ValuePair::CreateNonCoalescing(css::PrimitiveValue::Create(_hotSpot->X()),
                                                    css::PrimitiveValue::Create(_hotSpot->Y()));
    }
    return css::CursorImageValue::Create(_image->ComputedStyleValue(style), krys::move(hotSpot),
                                         style::ConvertToCSS(_originalURL, style));
  }

  ImageWithScale StyleCursorImage::SelectBestFitImage(const dom::Document &document) noexcept
  {
    if (auto *imageSet = DynamicDowncast<StyleImageSet>(_image.get()))
    {
      return imageSet->SelectBestFitImage(document);
    }

    // FIXME: Use the `href` of the `<cursor>` SVG element the URL refers to, if any, once SVG is ported.

    return {RefPtr<StyleImage> {_image}, 1, std::nullopt};
  }

  bool StyleCursorImage::UsesDataProtocol() const noexcept
  {
    return _originalURL.Resolved.ProtocolIsData();
  }
}
