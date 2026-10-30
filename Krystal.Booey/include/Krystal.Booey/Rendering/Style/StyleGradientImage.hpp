#pragma once

#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"
#include "Krystal.Booey/Style/Values/Images/Gradient.hpp"

namespace krys::boo
{
  /// @brief A gradient image.
  class StyleGradientImage final : public StyleGeneratedImage
  {
  private:
    style::Gradient _gradient;
    // FIXME: Decides whether the generated image may be cached per size, once it is.
    bool _knownCacheableBarringFilter {false};

  public:
    static constexpr bool IsFixedSize = false;

    KRYS_NODISCARD static Ref<StyleGradientImage> Create(style::Gradient gradient) noexcept
    {
      return AdoptRef(*new StyleGradientImage(krys::move(gradient)));
    }
    ~StyleGradientImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleGradientImage &) const noexcept;

  private:
    explicit StyleGradientImage(style::Gradient &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final
    {
    }

    KRYS_NODISCARD RefPtr<Image> GetImage(const RenderElement *, const FloatSize &, const GraphicsContext &,
                                          bool isForFirstLine) const noexcept final;
    KRYS_NODISCARD bool KnownToBeOpaque(const RenderElement &) const noexcept final;
    KRYS_NODISCARD FloatSize FixedSize(const RenderElement &) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleGradientImage, IsGradientImage)
