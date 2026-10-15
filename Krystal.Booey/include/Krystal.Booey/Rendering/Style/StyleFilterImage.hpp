#pragma once

#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"

namespace krys::boo
{
  /// @brief A `filter()` image: an image with filters applied.
  /// FIXME: Tracking the input image's cached image (`CachedImageClient`, `imageChanged`), loading reference
  /// filters' documents and drawing come with the resource loader and the graphics backend.
  class StyleFilterImage final : public StyleGeneratedImage
  {
  private:
    RefPtr<StyleImage> _image;
    style::Filter _filter;

  public:
    static constexpr bool IsFixedSize = true;

    KRYS_NODISCARD static Ref<StyleFilterImage> Create(RefPtr<StyleImage> image,
                                                       style::Filter filter) noexcept
    {
      return AdoptRef(*new StyleFilterImage(krys::move(image), krys::move(filter)));
    }
    ~StyleFilterImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &other) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleFilterImage &) const noexcept;
    KRYS_NODISCARD bool EqualInputImages(const StyleFilterImage &) const noexcept;

    KRYS_NODISCARD RefPtr<StyleImage> InputImage() const noexcept
    {
      return _image;
    }
    KRYS_NODISCARD const style::Filter &GetFilter() const noexcept
    {
      return _filter;
    }

  private:
    StyleFilterImage(RefPtr<StyleImage> &&, style::Filter &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleFilterImage, IsFilterImage)
