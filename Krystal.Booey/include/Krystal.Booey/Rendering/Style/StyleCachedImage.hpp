#pragma once

#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo
{
  /// @brief A `url()` image.
  /// FIXME: The cached image (`m_cachedImage`) and the SVG resource lookups come with the resource loader.
  class StyleCachedImage final : public StyleImage
  {
  private:
    style::URL _url;
    const Ref<css::ImageValue> _cssValue;
    bool _isPending {true};
    mutable float _scaleFactor {1};

  public:
    KRYS_NODISCARD static Ref<StyleCachedImage> Create(style::URL &&, Ref<css::ImageValue> &&,
                                                       float scaleFactor = 1) noexcept;
    KRYS_NODISCARD static Ref<StyleCachedImage> Create(const style::URL &, const Ref<css::ImageValue> &,
                                                       float scaleFactor = 1) noexcept;
    KRYS_NODISCARD static Ref<StyleCachedImage> CopyOverridingScaleFactor(StyleCachedImage &,
                                                                          float scaleFactor) noexcept;
    ~StyleCachedImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleCachedImage &) const noexcept;

    KRYS_NODISCARD WrappedImagePtr Data() const noexcept final
    {
      // FIXME: The cached image, once there is one.
      return nullptr;
    }

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;

    KRYS_NODISCARD bool IsPending() const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept final;
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept final;
    KRYS_NODISCARD bool UsesImageContainerSize() const noexcept final;
    KRYS_NODISCARD float ImageScaleFactor() const noexcept final;
    KRYS_NODISCARD bool UsesDataProtocol() const noexcept final;
    KRYS_NODISCARD style::URL Url() const noexcept final;

  private:
    StyleCachedImage(style::URL &&, Ref<css::ImageValue> &&, float) noexcept;
    StyleCachedImage(const style::URL &, const Ref<css::ImageValue> &, float) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleCachedImage, IsCachedImage)
