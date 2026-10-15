#pragma once

#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"
#include "Krystal.Booey/Style/Values/Images/Gradient.hpp"

namespace krys::boo
{
  /// @brief A gradient image.
  /// FIXME: Drawing (`GradientImage` and the per-size cache) comes with the graphics backend.
  class StyleGradientImage final : public StyleGeneratedImage
  {
  private:
    style::Gradient _gradient;
    // FIXME: Used by drawing, to decide whether the generated image may be cached.
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
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleGradientImage, IsGradientImage)
