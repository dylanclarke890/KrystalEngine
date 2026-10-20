#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  namespace
  {
    // The types the image decoders support (PNG, JPEG, GIF, WebP, BMP and ICO).
    // FIXME: A MIME type registry (MIMETypeRegistry::isSupportedImageMIMEType), and SVG and AVIF images once
    // they're supported.
    KRYS_NODISCARD bool IsSupportedImageMIMEType(const css::CSSOMString &mimeType) noexcept
    {
      constexpr utf8_stringview supportedTypes[] = {
        u8"image/png",
        u8"image/apng",
        u8"image/jpeg",
        u8"image/jpg",
        u8"image/pjpeg",
        u8"image/gif",
        u8"image/bmp",
        u8"image/x-bmp",
        u8"image/webp",
        u8"image/x-icon",
        u8"image/vnd.microsoft.icon",
      };
      return std::ranges::any_of(supportedTypes,
                                 [&](auto type) { return text::IsASCIICaselessEqual(mimeType, type); });
    }
  }

  Ref<StyleImageSet> StyleImageSet::Create(List<ImageWithScale> &&images,
                                           List<size_t> &&sortedIndices) noexcept
  {
    krys_debug_assert(images.size() == sortedIndices.size());
    return AdoptRef(*new StyleImageSet(krys::move(images), krys::move(sortedIndices)));
  }

  StyleImageSet::StyleImageSet(List<ImageWithScale> &&images, List<size_t> &&sortedIndices) noexcept
      : StyleMultiImage {Type::ImageSet}, _images {krys::move(images)},
        _sortedIndices {krys::move(sortedIndices)}
  {
  }

  StyleImageSet::~StyleImageSet() noexcept = default;

  bool StyleImageSet::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherImageSet = DynamicDowncast<StyleImageSet>(other);
    return otherImageSet && Equals(*otherImageSet);
  }

  bool StyleImageSet::Equals(const StyleImageSet &other) const noexcept
  {
    return _images == other._images && StyleMultiImage::Equals(other);
  }

  Ref<css::Value> StyleImageSet::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    css::ValueListBuilder builder;
    for (const auto &image : _images)
    {
      auto resolution = css::PrimitiveValue::Create(image.ScaleFactor, css::UnitType::dppx);
      if (image.MimeType)
      {
        builder.push_back(css::ImageSetOptionValue::Create(image.Image->ComputedStyleValue(style),
                                                           krys::move(resolution), *image.MimeType));
      }
      else
      {
        builder.push_back(
          css::ImageSetOptionValue::Create(image.Image->ComputedStyleValue(style), krys::move(resolution)));
      }
    }
    return css::ImageSetValue::Create(krys::move(builder));
  }

  ImageWithScale StyleImageSet::SelectBestFitImage(const dom::Document &document) noexcept
  {
    UpdateDeviceScaleFactor(document);

    if (!_accessedBestFitImage)
    {
      _accessedBestFitImage = true;
      _bestFitImage = BestImageForScaleFactor();
    }

    return _bestFitImage;
  }

  ImageWithScale StyleImageSet::BestImageForScaleFactor() noexcept
  {
    ImageWithScale result;
    for (auto index : _sortedIndices)
    {
      const auto &image = _images[index];
      if (image.MimeType && !IsSupportedImageMIMEType(*image.MimeType))
      {
        continue;
      }
      if (!result.Image->IsInvalidImage() && result.ScaleFactor == image.ScaleFactor)
      {
        continue;
      }
      if (image.ScaleFactor >= _deviceScaleFactor)
      {
        return image;
      }

      result = image;
    }

    krys_debug_assert(result.ScaleFactor >= 0);
    if (result.Image->IsInvalidImage() || !result.ScaleFactor)
    {
      result = ImageWithScale {StyleInvalidImage::Create(), 1, std::nullopt};
    }

    return result;
  }

  void StyleImageSet::UpdateDeviceScaleFactor(const dom::Document &) noexcept
  {
    // FIXME: In the future, we want to take much more than deviceScaleFactor into acount here.
    // All forms of scale should be included: Page::pageScaleFactor(), Frame::pageZoomFactor(),
    // and any CSS transforms. https://bugs.webkit.org/show_bug.cgi?id=81698
    // FIXME: Use the page's device scale factor, once documents have a page.
    float deviceScaleFactor = 1;
    if (deviceScaleFactor == _deviceScaleFactor)
    {
      return;
    }

    _deviceScaleFactor = deviceScaleFactor;
    _accessedBestFitImage = false;
  }
}
