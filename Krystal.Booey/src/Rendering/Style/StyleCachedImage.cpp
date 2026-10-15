#include "Krystal.Booey/Rendering/Style/StyleCachedImage.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo
{
  Ref<StyleCachedImage> StyleCachedImage::Create(style::URL &&url, Ref<css::ImageValue> &&cssValue,
                                                 float scaleFactor) noexcept
  {
    return AdoptRef(*new StyleCachedImage(krys::move(url), krys::move(cssValue), scaleFactor));
  }

  Ref<StyleCachedImage> StyleCachedImage::Create(const style::URL &url, const Ref<css::ImageValue> &cssValue,
                                                 float scaleFactor) noexcept
  {
    return AdoptRef(*new StyleCachedImage(url, cssValue, scaleFactor));
  }

  Ref<StyleCachedImage> StyleCachedImage::CopyOverridingScaleFactor(StyleCachedImage &other,
                                                                    float scaleFactor) noexcept
  {
    if (other._scaleFactor == scaleFactor)
    {
      return ShareRef(other);
    }
    return StyleCachedImage::Create(other._url, other._cssValue, scaleFactor);
  }

  StyleCachedImage::StyleCachedImage(style::URL &&url, Ref<css::ImageValue> &&cssValue,
                                     float scaleFactor) noexcept
      : StyleImage {Type::CachedImage}, _url {krys::move(url)}, _cssValue {krys::move(cssValue)},
        _scaleFactor {scaleFactor}
  {
    // FIXME: Take the CSS value's cached image (and so stop being pending) once the resource loader is
    // ported.
  }

  StyleCachedImage::StyleCachedImage(const style::URL &url, const Ref<css::ImageValue> &cssValue,
                                     float scaleFactor) noexcept
      : StyleImage {Type::CachedImage}, _url {url}, _cssValue {cssValue}, _scaleFactor {scaleFactor}
  {
    // FIXME: Take the CSS value's cached image (and so stop being pending) once the resource loader is
    // ported.
  }

  StyleCachedImage::~StyleCachedImage() noexcept = default;

  bool StyleCachedImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherCachedImage = DynamicDowncast<StyleCachedImage>(other);
    return otherCachedImage && Equals(*otherCachedImage);
  }

  bool StyleCachedImage::Equals(const StyleCachedImage &other) const noexcept
  {
    if (&other == this)
    {
      return true;
    }
    if (_scaleFactor != other._scaleFactor)
    {
      return false;
    }
    if (_cssValue.get() == other._cssValue.get() || _cssValue->Equals(*other._cssValue))
    {
      return true;
    }
    // FIXME: Also equal when both share a cached image, once the resource loader is ported.
    return false;
  }

  style::URL StyleCachedImage::Url() const noexcept
  {
    return _url;
  }

  Ref<css::Value> StyleCachedImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    return _cssValue->CopyForComputedStyle(style::ConvertToCSS(_url, style));
  }

  bool StyleCachedImage::IsPending() const noexcept
  {
    return _isPending;
  }

  bool StyleCachedImage::ImageHasRelativeWidth() const noexcept
  {
    // FIXME: Ask the cached image, once there is one.
    return false;
  }

  bool StyleCachedImage::ImageHasRelativeHeight() const noexcept
  {
    // FIXME: Ask the cached image, once there is one.
    return false;
  }

  bool StyleCachedImage::UsesImageContainerSize() const noexcept
  {
    // FIXME: Ask the cached image, once there is one.
    return false;
  }

  float StyleCachedImage::ImageScaleFactor() const noexcept
  {
    return _scaleFactor;
  }

  bool StyleCachedImage::UsesDataProtocol() const noexcept
  {
    return _url.Resolved.ProtocolIsData();
  }
}
