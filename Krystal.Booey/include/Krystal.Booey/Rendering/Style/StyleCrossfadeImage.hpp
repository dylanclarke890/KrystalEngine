#pragma once

#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"

namespace krys::boo
{
  struct BlendingContext;

  /// @brief A `cross-fade()` image.
  /// FIXME: Tracking the input images' cached images (`CachedImageClient`, `imageChanged`) and drawing
  /// (`CrossfadeGeneratedImage`) come with the resource loader and the graphics backend.
  class StyleCrossfadeImage final : public StyleGeneratedImage
  {
  private:
    RefPtr<StyleImage> _from;
    RefPtr<StyleImage> _to;
    double _percentage;
    bool _isPrefixed;

  public:
    static constexpr bool IsFixedSize = true;

    KRYS_NODISCARD static Ref<StyleCrossfadeImage> Create(RefPtr<StyleImage> from, RefPtr<StyleImage> to,
                                                          double percentage, bool isPrefixed) noexcept
    {
      return AdoptRef(*new StyleCrossfadeImage(krys::move(from), krys::move(to), percentage, isPrefixed));
    }
    ~StyleCrossfadeImage() noexcept override;

    KRYS_NODISCARD RefPtr<StyleCrossfadeImage> Blend(const StyleCrossfadeImage &,
                                                     const BlendingContext &) const noexcept;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleCrossfadeImage &) const noexcept;
    KRYS_NODISCARD bool EqualInputImages(const StyleCrossfadeImage &) const noexcept;

  private:
    StyleCrossfadeImage(RefPtr<StyleImage> &&, RefPtr<StyleImage> &&, double, bool) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleCrossfadeImage, IsCrossfadeImage)
